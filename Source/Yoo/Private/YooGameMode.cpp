// Copyright Epic Games, Inc. All Rights Reserved.

#include "YooGameMode.h"
#include "LevelSaveData.h"
#include "MainEditController.h"
#include "Yoo.h"
#include "YooCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/RuntimeErrors.h"
#include "UObject/ConstructorHelpers.h"
#include "UObject/SavePackage.h"

static FName Name_DefaultLevel(TEXT("Default"));

void AYooGameMode::OpenLevel(AMainEditController* Controller, FName LevelName, FOnLevelOpenComplete CompleteDelegate)
{
	if (ensureAsRuntimeWarning(Controller))
	{
		if (!CurrentLevel || LevelName != CurrentLevel->LevelName)
		{
			UWorld* World = GetWorld();
			
			if (CurrentLevel)
			{
				Controller->SaveToLevelData(CurrentLevel);
				if (auto* Collection = CurrentLevel->Level->GetCachedLevelCollection())
				{
					Collection->RemoveLevel(CurrentLevel->Level);
				}
				World->RemoveFromWorld(CurrentLevel->Level);
				ULevelStreaming* LocalLevel = FStreamLevelAction::FindAndCacheLevelStreamingObject(CurrentLevel->LevelName, World);
				LocalLevel->SetShouldBeLoaded(false);
				LocalLevel->bShouldBlockOnUnload = false;
				SaveLevelPackage(CurrentLevel);
			}

			ULevelSaveData* LevelData = nullptr;
			UPackage* LoadedPackage = LoadPackage(nullptr, *GetSaveFileName(LevelName), LOAD_MemoryReader | LOAD_UncookedGamePackage);
			if (LoadedPackage)
			{
				LoadedPackage->FullyLoad();

				LevelData = FindObject<ULevelSaveData>(LoadedPackage, TEXT("LevelData"));
			}
			else
			{
				LoadedPackage = CreatePackage(*GetPackageLongName(LevelName));
			}
				
			ULevelStreaming* LocalLevel = FStreamLevelAction::FindAndCacheLevelStreamingObject(LevelName, World);
			LocalLevel->SetShouldBeLoaded(true);
			LocalLevel->SetShouldBeVisible(true);
			LocalLevel->bShouldBlockOnLoad = false;
			World->BlockTillLevelStreamingCompleted();
			
			if (!LevelData)
			{
				LevelData = CreateNewLevelData(LoadedPackage, LevelName);
				if (const AActor* PlayerStart = ChoosePlayerStart(Controller))
				{
					LevelData->ViewTransform = PlayerStart->GetActorTransform();
				}
			}
			
			CurrentLevel = LevelData;
			GetWorld()->AddToWorld(CurrentLevel->Level, FTransform::Identity, false);
			if (auto* Collection = CurrentLevel->Level->GetCachedLevelCollection())
			{
				Collection->RemoveLevel(CurrentLevel->Level);
			}
			FLevelCollection& LC = World->FindOrAddCollectionByType(ELevelCollectionType::DynamicSourceLevels);
			LC.AddLevel(CurrentLevel->Level);
			
			Controller->LoadFromLevelData(CurrentLevel);
			CompleteDelegate.ExecuteIfBound();
			
			LastOpenedLevel = LevelName;
			SaveConfig();

			for (AActor* Actor : CurrentLevel->Level->Actors)
			{
				if (!Actor->HasActorBegunPlay())
				{
					Actor->DispatchBeginPlay();
				}
			}
		}
	}
}

void AYooGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (LastOpenedLevel == NAME_None)
	{
		OpenLevel(GetWorld()->GetFirstPlayerController<AMainEditController>(), Name_DefaultLevel, FOnLevelOpenComplete());
	}
	else
	{
		OpenLevel(GetWorld()->GetFirstPlayerController<AMainEditController>(), LastOpenedLevel, FOnLevelOpenComplete());
	}
}

void AYooGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (ensure(CurrentLevel))
	{
		SaveLevelPackage(CurrentLevel);
	}
	
	Super::EndPlay(EndPlayReason);
}

bool AYooGameMode::SaveLevelPackage(ULevelSaveData* LevelToSave)
{
	SaveConfig(CPF_Config, *FPaths::Combine(FPaths::ProjectConfigDir(), TEXT("YooEditor.ini")));
	UPackage* Package = LevelToSave->GetOutermost();
	if (ensure(Package))
	{
		FSavePackageArgs SaveArgs;
		SaveArgs.TopLevelFlags = RF_Standalone;
		SaveArgs.SaveFlags = SAVE_Async | SAVE_UncookedGamePackage;
		SaveArgs.Error = GWarn;

		FString FileName = GetSaveFileName(LevelToSave->LevelName);
		UE_LOG(LogYoo, Log, TEXT("Save level data %s to %s"), *LevelToSave->LevelName.ToString(), *FileName);

		FSavePackageResultStruct Result = UPackage::Save(Package, LevelToSave, *FileName, SaveArgs);
		if (!ensure(Result.Result == ESavePackageResult::Success))
		{
			TArray<FString> OutErrors;
			GWarn->GetErrorsAndWarningsAndEmpty(OutErrors);
			FString LogString;
			for (auto& Err : OutErrors)
			{
				LogString.Appendf(TEXT("%s\n"), *Err);
			}
			UE_LOG(LogYoo, Error, TEXT("SavePackage Error:\n %s"), *LogString);
			return false;
		}
		return true;
	}
	return false;
}

FString AYooGameMode::GetSaveFileName(FName InName)
{
	return FPackageName::LongPackageNameToFilename(GetPackageLongName(InName), TEXT(".sav"));
}

FString AYooGameMode::GetPackageLongName(FName InName)
{
	return FString::Printf(TEXT("/Save/Levels/%s"), *InName.ToString());
}

ULevelSaveData* AYooGameMode::CreateNewLevelData(UPackage* LoadedPackage, FName InLevelName) const
{
	UWorld* NewWorld = UWorld::CreateWorld(EWorldType::None, false, InLevelName, LoadedPackage, false, ERHIFeatureLevel::Num, nullptr, true);
	ULevelSaveData* NewLevelData = NewObject<ULevelSaveData>(LoadedPackage, ULevelSaveData::StaticClass(), TEXT("LevelData"), RF_Public | RF_Standalone);

	NewLevelData->DummyWorld = NewWorld;
	NewLevelData->ViewTransform = FTransform::Identity;
	NewLevelData->Level = NewWorld->PersistentLevel;
	NewLevelData->LevelName = InLevelName;

	if (auto* Collection = NewWorld->PersistentLevel->GetCachedLevelCollection())
	{
		Collection->RemoveLevel(NewWorld->PersistentLevel);
	}
	
	LoadedPackage->MarkPackageDirty();
	
	return NewLevelData;
}

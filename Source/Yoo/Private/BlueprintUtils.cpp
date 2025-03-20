// Fill out your copyright notice in the Description page of Project Settings.


#include "BlueprintUtils.h"
#include "EngineUtils.h"
#include "Yoo.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Blueprint/UserWidget.h"
#include "Engine/AssetManager.h"
#include "Widgets/Colors/SColorPicker.h"

void UBlueprintUtils::OpenColorPickerDialog(FLinearColor OldColor, UUserWidget* ParentWidget, FOnColorChanged OnColorChangedEvent)
{
	FColorPickerArgs PickerArgs(OldColor, FOnLinearColorValueChanged::CreateStatic(&OnColorChanged, OnColorChangedEvent));
	PickerArgs.bUseAlpha = false;
	PickerArgs.ParentWidget = ParentWidget->TakeWidget();
	PickerArgs.bIsModal = true;
	PickerArgs.bOnlyRefreshOnOk = true;
	OpenColorPicker(PickerArgs);
}

void UBlueprintUtils::OnColorChanged(FLinearColor NewColor, FOnColorChanged OnColorChangedEvent)
{
	OnColorChangedEvent.Execute(NewColor);
}

TArray<UClass*> UBlueprintUtils::FindBlueprintClass(UClass* BaseClass, const FString& Path, bool bSortAlphabetically)
{
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();
	
	TArray<FAssetData> AssetData;
	FARFilter Filter;
	Filter.PackagePaths.Emplace(Path);
	Filter.ClassPaths.Add(UObject::StaticClass()->GetClassPathName());
	Filter.bRecursivePaths = true;
	Filter.bRecursiveClasses = true;
	Filter.bIncludeOnlyOnDiskAssets = true;
	AssetRegistry.GetAssets(Filter, AssetData);
	
	UE_LOG(LogYoo, Verbose, TEXT("[FindBlueprintClass] Found %d Assets"), AssetData.Num());
	TArray<UClass*> Subclasses;
	for (const auto& Asset : AssetData)
	{
		if (!Asset.IsTopLevelAsset())
			continue;

		if (Asset.AssetName.ToString().StartsWith(TEXT("SKEL_")))
			continue;

		const UAssetManager& AssetManager = UAssetManager::Get();
		FSoftObjectPath AssetPath = AssetManager.GetAssetPathForData(Asset);
		UE_LOG(LogYoo, Verbose, TEXT("[FindBlueprintClass] Found Asset %s"), *AssetPath.ToString());
		
		UClass* BPClass = Cast<UClass>(AssetPath.ResolveObject());
		if (!BPClass)
		{
			BPClass = Cast<UClass>(AssetPath.TryLoad());
		}
		if (BPClass)
		{
			if (BPClass->ClassFlags & CLASS_Abstract)
				continue;

			// Only find blueprint class
			if (BPClass->ClassFlags & CLASS_Native)
				continue;
			
			// Exclude the base class
			if (BPClass == BaseClass)
				continue;
			
			if (!BPClass->IsChildOf(BaseClass))
				continue;

			Subclasses.AddUnique(BPClass);
		}
	}

	if (bSortAlphabetically)
	{
		Algo::Sort(Subclasses, [](const UClass* A, const UClass* B)
		{
			return A->GetName() < B->GetName();
		});
	}

	return Subclasses;
}

bool UBlueprintUtils::GetAllSubjectsOfType(UObject* WorldContextObject, EDataSourceType Type, TArray<FDataSourceSubject>& Subjects)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);

	for (TActorIterator<AMainEditDataSource> It(World); It; ++It)
	{
		if (DataSourceType::HasFlags(It->Type, Type))
		{
			for (FName SubjectName : It->GetSubjectNames())
			{
				Subjects.Emplace(It->TypeName, SubjectName, It->GetClass());
			}
		}
	}

	return !Subjects.IsEmpty();
}

bool UBlueprintUtils::FindSubjectByName(UObject* WorldContextObject, FName Name, FDataSourceSubject& Subject)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	Subject.SubjectName = Name;

	for (TActorIterator<AMainEditDataSource> It(World); It; ++It)
	{
		for (FName SubjectName : It->GetSubjectNames())
		{
			if (SubjectName == Name)
			{
				Subject.TypeName = It->TypeName;
				Subject.TypeClass = It->GetClass();
				return true;
			}
		}
	}
	
	return false;
}

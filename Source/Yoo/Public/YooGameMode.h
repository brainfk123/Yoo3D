// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "YooGameMode.generated.h"

class ULevelSaveData;
class AMainEditController;

DECLARE_DYNAMIC_DELEGATE(FOnLevelOpenComplete);

UCLASS(Config=YooEditor)
class AYooGameMode : public AGameModeBase
{
	GENERATED_BODY()
public:
	
	UFUNCTION(BlueprintCallable)
	void OpenLevel(AMainEditController* Controller, FName LevelName, FOnLevelOpenComplete CompleteDelegate);
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	ULevelSaveData* GetCurrentLevel() const { return CurrentLevel; }

	UPROPERTY(Config)
	FName LastOpenedLevel;

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
protected:
	bool SaveLevelPackage(ULevelSaveData* LevelToSave);
	static FString GetSaveFileName(FName InName);
	static FString GetPackageLongName(FName InName);
	ULevelSaveData* CreateNewLevelData(UPackage* LoadedPackage, FName InLevelName) const;

	UPROPERTY(Transient)
	TObjectPtr<ULevelSaveData> CurrentLevel;
};




// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MainEditDataSource.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "BlueprintUtils.generated.h"

DECLARE_DYNAMIC_DELEGATE_OneParam(FOnColorChanged, FLinearColor, NewColor);

UCLASS()
class YOO_API UBlueprintUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable)
	static void OpenColorPickerDialog(FLinearColor OldColor, UUserWidget* ParentWidget, FOnColorChanged OnColorChangedEvent);

	static void OnColorChanged(FLinearColor NewColor, FOnColorChanged OnColorChangedEvent);
	
	UFUNCTION(BlueprintPure, BlueprintInternalUseOnly)
	static TArray<UClass*> FindBlueprintClass(UClass* BaseClass, const FString& Path, bool bSortAlphabetically);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Data Source", meta = (WorldContext="WorldContextObject"))
	static bool GetAllSubjectsOfType(UObject* WorldContextObject, EDataSourceType Type, TArray<FDataSourceSubject>& Subjects);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Data Source", meta = (WorldContext="WorldContextObject"))
	static bool FindSubjectByName(UObject* WorldContextObject, FName Name, FDataSourceSubject& Subject);
};

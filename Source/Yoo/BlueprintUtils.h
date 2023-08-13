// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "BlueprintUtils.generated.h"

DECLARE_DYNAMIC_DELEGATE_OneParam(FOnColorChanged, FLinearColor, NewColor);

UCLASS()
class UBlueprintUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable)
	static void OpenColorPickerDialog(FLinearColor OldColor, UUserWidget* ParentWidget, FOnColorChanged OnColorChangedEvent);

	static void OnColorChanged(FLinearColor NewColor, FOnColorChanged OnColorChangedEvent);
};

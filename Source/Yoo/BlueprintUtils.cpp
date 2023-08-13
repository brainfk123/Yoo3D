// Fill out your copyright notice in the Description page of Project Settings.


#include "BlueprintUtils.h"
#include "Blueprint/UserWidget.h"
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

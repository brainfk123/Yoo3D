// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MainEditWidget.h"
#include "EditableObjectDetailWidget.generated.h"

class AEditableObject;

UCLASS(Abstract)
class YOO_API UEditableObjectDetailWidget : public UMainEditWidget
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, BlueprintSetter=SetEditableObject, Category = "EditableObject")
	TObjectPtr<AEditableObject> EditableObject;

	UFUNCTION(BlueprintSetter, Category = "EditableObject")
	void SetEditableObject(AEditableObject* InEditableObject);

	UFUNCTION(BlueprintImplementableEvent, Category = "EditableObject")
	void OnSetEditableObject();
};

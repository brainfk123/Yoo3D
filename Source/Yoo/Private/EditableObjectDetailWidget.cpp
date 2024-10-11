// Fill out your copyright notice in the Description page of Project Settings.


#include "EditableObjectDetailWidget.h"

void UEditableObjectDetailWidget::SetEditableObject(AEditableObject* InEditableObject)
{
	EditableObject = InEditableObject;
	OnSetEditableObject();
}

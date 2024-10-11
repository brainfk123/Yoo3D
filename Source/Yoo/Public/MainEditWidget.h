// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Border.h"
#include "MainEditWidget.generated.h"

UCLASS()
class YOO_API UMainEditWidget : public UUserWidget
{
	GENERATED_BODY()
};

UCLASS()
class UStackWidgetBox : public UPanelWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="Layout|Stack Widget Box")
	UPanelSlot* PushChild(UWidget* Content);
	
	UFUNCTION(BlueprintCallable, Category="Layout|Stack Widget Box")
	bool PopChild();
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Layout|Stack Widget Box")
	UMainEditWidget* TopChild();

	virtual UClass* GetSlotClass() const override;
	virtual void OnSlotAdded(UPanelSlot* InSlot) override;
	virtual void OnSlotRemoved(UPanelSlot* InSlot) override;
	
#if WITH_EDITOR
	virtual const FText GetPaletteCategory() override;
#endif

protected:
	// UWidget interface
	virtual TSharedRef<SWidget> RebuildWidget() override;
	// End of UWidget interface

	virtual void ReleaseSlateResources(bool bReleaseChildren) override;

	TSharedPtr<SBox> MyTopBox;
};
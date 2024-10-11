// Fill out your copyright notice in the Description page of Project Settings.
#include "MainEditWidget.h"
#include "InstalledPlatformInfo.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Components/SizeBoxSlot.h"

#define LOCTEXT_NAMESPACE "UMG"

UPanelSlot* UStackWidgetBox::PushChild(UWidget* Content)
{
	return AddChild(Content);
}

bool UStackWidgetBox::PopChild()
{
	return RemoveChildAt(Slots.Num() - 1);
}

UMainEditWidget* UStackWidgetBox::TopChild()
{
	if (Slots.IsEmpty())
		return nullptr;
	return Cast<UMainEditWidget>(Slots.Top()->Content);
}

UClass* UStackWidgetBox::GetSlotClass() const
{
	return USizeBoxSlot::StaticClass();
}

void UStackWidgetBox::OnSlotAdded(UPanelSlot* InSlot)
{
	// Add the child to the live slot if it already exists
	if (MyTopBox.IsValid())
	{
		CastChecked<USizeBoxSlot>(InSlot)->BuildSlot(MyTopBox.ToSharedRef());
	}
}

void UStackWidgetBox::OnSlotRemoved(UPanelSlot* InSlot)
{
	if (MyTopBox.IsValid())
	{
		CastChecked<USizeBoxSlot>(Slots.Top())->BuildSlot(MyTopBox.ToSharedRef());
	}
}

#if WITH_EDITOR
const FText UStackWidgetBox::GetPaletteCategory()
{
	return LOCTEXT("Panel", "Panel");
}
#endif

TSharedRef<SWidget> UStackWidgetBox::RebuildWidget()
{
	MyTopBox = SNew(SBox);

	if (GetChildrenCount() > 0)
	{
		Cast<USizeBoxSlot>(Slots.Top())->BuildSlot(MyTopBox.ToSharedRef());
	}

	return MyTopBox.ToSharedRef();
}

void UStackWidgetBox::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);

	MyTopBox.Reset();
}

#undef LOCTEXT_NAMESPACE

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "K2Node.h"
#include "KismetCompiler.h"
#include "K2Node_FindBlueprintClass.generated.h"

UCLASS()
class YOOGRAPHNODE_API UK2Node_FindBlueprintClass : public UK2Node
{
	GENERATED_BODY()
public:
	UK2Node_FindBlueprintClass();

	UPROPERTY()
	UClass* ClassToFind;

	UPROPERTY()
	FString FindPath;

	UPROPERTY()
	bool bSortResult;
	
	//~ Begin UEdGraphNode Interface.
	virtual void AllocateDefaultPins() override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual void PinDefaultValueChanged(UEdGraphPin* Pin) override;
	virtual FText GetTooltipText() const override;
	virtual FText GetKeywords() const override;
	virtual bool HasExternalDependencies(TArray<class UStruct*>* OptionalOutput) const override;
	virtual bool IsCompatibleWithGraph(const UEdGraph* TargetGraph) const override;
	virtual void PinConnectionListChanged(UEdGraphPin* Pin) override;
	virtual void GetPinHoverText(const UEdGraphPin& Pin, FString& HoverTextOut) const override;
	virtual void AddSearchMetaDataInfo(TArray<struct FSearchTagDataPair>& OutTaggedMetaData) const override;
	virtual FString GetPinMetaData(FName InPinName, FName InKey) override;
	virtual FSlateIcon GetIconAndTint(FLinearColor& OutColor) const override;
	virtual FLinearColor GetNodeTitleColor() const override;
	//~ End UEdGraphNode Interface.

	//~ Begin UK2Node Interface
	virtual bool IsNodePure() const override { return true; }
	virtual bool IsNodeSafeToIgnore() const override { return true; }
	virtual void GetNodeAttributes( TArray<TKeyValuePair<FString, FString>>& OutNodeAttributes ) const override;
	virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
	virtual FText GetMenuCategory() const override;
	virtual void ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph) override;
	//~ End UK2Node Interface

	/** Get the blueprint input pin */
	UEdGraphPin* GetClassPin(const TArray<UEdGraphPin*>* InPinsToSearch = nullptr) const;
	UEdGraphPin* GetPathPin(const TArray<UEdGraphPin*>* InPinsToSearch = nullptr) const;
	UEdGraphPin* GetSortPin(const TArray<UEdGraphPin*>* InPinsToSearch = nullptr) const;
	/** Get the result output pin */
	UEdGraphPin* GetResultPin() const;

	/** Get the class that we are going to find, if it's defined as default value */
	UClass* GetClassToFind(const TArray<UEdGraphPin*>* InPinsToSearch = nullptr) const;

protected:
	/** Gets the node for use in lists and menus */
	virtual FText GetBaseNodeTitle() const;
	/** Gets the default node title when no class is selected */
	virtual FText GetDefaultNodeTitle() const;
	/** Gets the node title when a class has been selected. */
	virtual FText GetNodeTitleFormat() const;

	/**
	 * Takes the specified "MutatablePin" and sets its 'PinToolTip' field (according
	 * to the specified description)
	 * 
	 * @param   MutatablePin	The pin you want to set tool-tip text on
	 * @param   PinDescription	A string describing the pin's purpose
	 */
	void SetPinToolTip(UEdGraphPin& MutatablePin, const FText& PinDescription) const;

	/** Refresh pins when class was changed */
	void OnClassPinChanged() const;

	/** Tooltip text for this node. */
	FText NodeTooltip;

	/** Constructing FText strings can be costly, so we cache the node's title */
	FNodeTextCache CachedNodeTitle;
};

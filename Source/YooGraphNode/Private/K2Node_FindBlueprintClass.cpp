#include "K2Node_FindBlueprintClass.h"
#include "BlueprintActionDatabaseRegistrar.h"
#include "BlueprintNodeSpawner.h"
#include "EdGraphUtilities.h"
#include "EditorCategoryUtils.h"
#include "FindInBlueprintManager.h"
#include "GraphEditorSettings.h"
#include "K2Node_CallFunction.h"
#include "K2Node_Tunnel.h"
#include "KismetCompiler.h"
#include "KismetCompilerMisc.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "BlueprintUtils.h"

struct FKismetFunctionContext;

struct FK2Node_FindBlueprintClassHelper
{
	static FName GetClassPinName()
	{
		return TEXT("Base Class");
	}

	static FName GetPathPinName()
	{
		return TEXT("Path");
	}

	static FString GetDefaultPath()
	{
		return TEXT("/Game");
	}

	static FName GetSortPinName()
	{
		return TEXT("Sort Alphabetically");
	}

	static FString GetDefaultSortValue()
	{
		return TEXT("true");
	}

	static FName GetOuterPinFriendlyName()
	{
		return TEXT("Outer");
	}
};

#define LOCTEXT_NAMESPACE "K2Node_FindBlueprintClass"

UK2Node_FindBlueprintClass::UK2Node_FindBlueprintClass()
{
	NodeTooltip = LOCTEXT("NodeTooltip", "Find all inherited blueprint class in a given path");
	ClassToFind = nullptr;
	FindPath = TEXT("/Game");
	bSortResult = true;
}

void UK2Node_FindBlueprintClass::AllocateDefaultPins()
{
	// Add blueprint pin
	UEdGraphPin* ClassPin = CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Class, UObject::StaticClass(), FK2Node_FindBlueprintClassHelper::GetClassPinName());
	UEdGraphPin* PathPin = CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_String, FK2Node_FindBlueprintClassHelper::GetPathPinName());
	UEdGraphPin* SortPin = CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Boolean, FK2Node_FindBlueprintClassHelper::GetSortPinName());

	ClassPin->DefaultObject = ClassToFind;
	PathPin->DefaultValue = FindPath;
	SortPin->DefaultValue = bSortResult ? TEXT("true") : TEXT("false");
	
	// Result pin
	UEdGraphPin* ResultPin = CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Class, ClassToFind ? ClassToFind : UObject::StaticClass(), UEdGraphSchema_K2::PN_ReturnValue);
	ResultPin->PinType.ContainerType = EPinContainerType::Array;

	Super::AllocateDefaultPins();
}

void UK2Node_FindBlueprintClass::SetPinToolTip(UEdGraphPin& MutatablePin, const FText& PinDescription) const
{
	MutatablePin.PinToolTip = UEdGraphSchema_K2::TypeToText(MutatablePin.PinType).ToString();

	UEdGraphSchema_K2 const* const K2Schema = Cast<const UEdGraphSchema_K2>(GetSchema());
	if (K2Schema != nullptr)
	{
		MutatablePin.PinToolTip += TEXT(" ");
		MutatablePin.PinToolTip += K2Schema->GetPinDisplayName(&MutatablePin).ToString();
	}

	MutatablePin.PinToolTip += FString(TEXT("\n")) + PinDescription.ToString();
}

UClass* UK2Node_FindBlueprintClass::GetClassToFind(const TArray<UEdGraphPin*>* InPinsToSearch /*=NULL*/) const
{
	UClass* LocalClassToFind = nullptr;
	const TArray<UEdGraphPin*>* PinsToSearch = InPinsToSearch ? InPinsToSearch : &Pins;

	UEdGraphPin* ClassPin = GetClassPin(PinsToSearch);
	if (ClassPin && ClassPin->DefaultObject && ClassPin->LinkedTo.Num() == 0)
	{
		LocalClassToFind = CastChecked<UClass>(ClassPin->DefaultObject);
	}
	else if (ClassPin && ClassPin->LinkedTo.Num())
	{
		UEdGraphPin* ClassSource = ClassPin->LinkedTo[0];
		LocalClassToFind = ClassSource ? Cast<UClass>(ClassSource->PinType.PinSubCategoryObject.Get()) : nullptr;
	}

	return LocalClassToFind;
}

FString UK2Node_FindBlueprintClass::GetPinMetaData(FName InPinName, FName InKey)
{
	FString MetaData = Super::GetPinMetaData(InPinName, InKey);

	if (MetaData.IsEmpty())
	{
		if (InPinName == FK2Node_FindBlueprintClassHelper::GetClassPinName() && InKey == FBlueprintMetadata::MD_AllowAbstractClasses)
		{
			MetaData = TEXT("true");
		}
	}

	return MetaData;
}

FSlateIcon UK2Node_FindBlueprintClass::GetIconAndTint(FLinearColor& OutColor) const
{
	static FSlateIcon Icon("YooGraphNodeStyle", "GraphEditor.FindBlueprint_16x");
	return Icon;
}

FLinearColor UK2Node_FindBlueprintClass::GetNodeTitleColor() const
{
	return GetDefault<UGraphEditorSettings>()->ClassPinTypeColor;
}

void UK2Node_FindBlueprintClass::AddSearchMetaDataInfo(TArray<struct FSearchTagDataPair>& OutTaggedMetaData) const
{
	Super::AddSearchMetaDataInfo(OutTaggedMetaData);
	OutTaggedMetaData.Add(FSearchTagDataPair(FFindInBlueprintSearchTags::FiB_NativeName, CachedNodeTitle.GetCachedText()));
}

void UK2Node_FindBlueprintClass::OnClassPinChanged() const
{
	CachedNodeTitle.MarkDirty();

	GetResultPin()->PinType.PinSubCategoryObject = ClassToFind;

	// Refresh the UI for the graph so the pin changes show up
	GetGraph()->NotifyGraphChanged();

	// Mark dirty
	FBlueprintEditorUtils::MarkBlueprintAsModified(GetBlueprint());
}

void UK2Node_FindBlueprintClass::PinConnectionListChanged(UEdGraphPin* Pin)
{
	Super::PinConnectionListChanged(Pin);

	if (Pin && (Pin->PinName == FK2Node_FindBlueprintClassHelper::GetClassPinName()))
	{
		OnClassPinChanged();
	}
}

void UK2Node_FindBlueprintClass::GetPinHoverText(const UEdGraphPin& Pin, FString& HoverTextOut) const
{
	if (UEdGraphPin* ClassPin = GetClassPin())
	{
		SetPinToolTip(*ClassPin, LOCTEXT("ClassPinDescription", "The blueprint class you want to find"));
	}
	if (UEdGraphPin* ResultPin = GetResultPin())
	{
		SetPinToolTip(*ResultPin, LOCTEXT("ResultPinDescription", "The found blueprints array"));
	}

	return Super::GetPinHoverText(Pin, HoverTextOut);
}

void UK2Node_FindBlueprintClass::PinDefaultValueChanged(UEdGraphPin* ChangedPin) 
{
	if (ChangedPin)
	{
		if (ChangedPin->PinName == FK2Node_FindBlueprintClassHelper::GetClassPinName())
		{
			ClassToFind = GetClassToFind();
			OnClassPinChanged();
		}
		else if (ChangedPin->PinName == FK2Node_FindBlueprintClassHelper::GetPathPinName())
		{
			FindPath = ChangedPin->DefaultValue;
		}
		else if (ChangedPin->PinName == FK2Node_FindBlueprintClassHelper::GetSortPinName())
		{
			bSortResult = FCString::ToBool(*ChangedPin->DefaultValue);
		}
	}
}

FText UK2Node_FindBlueprintClass::GetTooltipText() const
{
	return NodeTooltip;
}

UEdGraphPin* UK2Node_FindBlueprintClass::GetClassPin(const TArray<UEdGraphPin*>* InPinsToSearch /*= NULL*/) const
{
	const TArray<UEdGraphPin*>* PinsToSearch = InPinsToSearch ? InPinsToSearch : &Pins;

	UEdGraphPin* Pin = nullptr;
	for (UEdGraphPin* TestPin : *PinsToSearch)
	{
		if (TestPin && TestPin->PinName == FK2Node_FindBlueprintClassHelper::GetClassPinName())
		{
			Pin = TestPin;
			break;
		}
	}
	check(Pin == nullptr || Pin->Direction == EGPD_Input);
	return Pin;
}

UEdGraphPin* UK2Node_FindBlueprintClass::GetPathPin(const TArray<UEdGraphPin*>* InPinsToSearch) const
{
	const TArray<UEdGraphPin*>* PinsToSearch = InPinsToSearch ? InPinsToSearch : &Pins;

	UEdGraphPin* Pin = nullptr;
	for (UEdGraphPin* TestPin : *PinsToSearch)
	{
		if (TestPin && TestPin->PinName == FK2Node_FindBlueprintClassHelper::GetPathPinName())
		{
			Pin = TestPin;
			break;
		}
	}
	check(Pin == nullptr || Pin->Direction == EGPD_Input);
	return Pin;
}

UEdGraphPin* UK2Node_FindBlueprintClass::GetSortPin(const TArray<UEdGraphPin*>* InPinsToSearch) const
{
	const TArray<UEdGraphPin*>* PinsToSearch = InPinsToSearch ? InPinsToSearch : &Pins;

	UEdGraphPin* Pin = nullptr;
	for (UEdGraphPin* TestPin : *PinsToSearch)
	{
		if (TestPin && TestPin->PinName == FK2Node_FindBlueprintClassHelper::GetSortPinName())
		{
			Pin = TestPin;
			break;
		}
	}
	check(Pin == nullptr || Pin->Direction == EGPD_Input);
	return Pin;
}

UEdGraphPin* UK2Node_FindBlueprintClass::GetResultPin() const
{
	UEdGraphPin* Pin = FindPinChecked(UEdGraphSchema_K2::PN_ReturnValue);
	check(Pin->Direction == EGPD_Output);
	return Pin;
}

FText UK2Node_FindBlueprintClass::GetBaseNodeTitle() const
{
	return NSLOCTEXT("K2Node", "FindBlueprintClass_BaseTitle", "Find Blueprint Classes");
}

FText UK2Node_FindBlueprintClass::GetDefaultNodeTitle() const
{
	return NSLOCTEXT("K2Node", "FindBlueprintClass_Title_NONE", "Find Blueprint Classes");
}

FText UK2Node_FindBlueprintClass::GetNodeTitleFormat() const
{
	return NSLOCTEXT("K2Node", "FindBlueprintClass_Title", "Find Blueprints of {ClassName}");
}

FText UK2Node_FindBlueprintClass::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	if (TitleType == ENodeTitleType::ListView || TitleType == ENodeTitleType::MenuTitle)
	{
		return GetBaseNodeTitle();
	}
	else if (ClassToFind)
	{
		if (CachedNodeTitle.IsOutOfDate(this))
		{
			FFormatNamedArguments Args;
			Args.Add(TEXT("ClassName"), ClassToFind->GetDisplayNameText());
			// FText::Format() is slow, so we cache this to save on performance
			CachedNodeTitle.SetCachedText(FText::Format(GetNodeTitleFormat(), Args), this);
		}
		return CachedNodeTitle;
	}
	return GetDefaultNodeTitle();
}

bool UK2Node_FindBlueprintClass::IsCompatibleWithGraph(const UEdGraph* TargetGraph) const 
{
	UBlueprint* Blueprint = FBlueprintEditorUtils::FindBlueprintForGraph(TargetGraph);
	return Super::IsCompatibleWithGraph(TargetGraph) && (!Blueprint || FBlueprintEditorUtils::FindUserConstructionScript(Blueprint) != TargetGraph);
}

void UK2Node_FindBlueprintClass::GetNodeAttributes(TArray<TKeyValuePair<FString, FString>>& OutNodeAttributes) const
{
	const FString ClassToFindStr = ClassToFind ? ClassToFind->GetName() : TEXT("InvalidClass");
	OutNodeAttributes.Add(TKeyValuePair<FString, FString>(TEXT("Type"), TEXT("FindBlueprintClass")));
	OutNodeAttributes.Add(TKeyValuePair<FString, FString>(TEXT("Class"), GetClass()->GetName()));
	OutNodeAttributes.Add(TKeyValuePair<FString, FString>(TEXT("Name"), GetName()));
	OutNodeAttributes.Add(TKeyValuePair<FString, FString>(TEXT("ObjectClass"), ClassToFindStr));
}

void UK2Node_FindBlueprintClass::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
	UClass* ActionKey = GetClass();
	if (ActionRegistrar.IsOpenForRegistration(ActionKey))
	{
		UBlueprintNodeSpawner* NodeSpawner = UBlueprintNodeSpawner::Create(GetClass());
		ActionRegistrar.AddBlueprintAction(ActionKey, NodeSpawner);
	}
}

FText UK2Node_FindBlueprintClass::GetMenuCategory() const
{
	return FEditorCategoryUtils::GetCommonCategory(FCommonEditorCategory::Utilities);
}

bool UK2Node_FindBlueprintClass::HasExternalDependencies(TArray<class UStruct*>* OptionalOutput) const
{
	UClass* SourceClass = GetClassToFind();
	const UBlueprint* SourceBlueprint = GetBlueprint();
	const bool bResult = (SourceClass && (SourceClass->ClassGeneratedBy.Get() != SourceBlueprint));
	if (bResult && OptionalOutput)
	{
		OptionalOutput->AddUnique(SourceClass);
	}
	const bool bSuperResult = Super::HasExternalDependencies(OptionalOutput);
	return bSuperResult || bResult;
}

FText UK2Node_FindBlueprintClass::GetKeywords() const
{
	return LOCTEXT("FindBlueprintClassKeywords", "Find Blueprint");
}

void UK2Node_FindBlueprintClass::ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph)
{
	Super::ExpandNode(CompilerContext, SourceGraph);
	
	UK2Node_CallFunction* const CallImplementNode = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this, SourceGraph);
	CallImplementNode->FunctionReference.SetExternalMember(GET_FUNCTION_NAME_CHECKED(UBlueprintUtils, FindBlueprintClass), UBlueprintUtils::StaticClass());
	CallImplementNode->AllocateDefaultPins();
	
	UEdGraphPin* BaseClassPin = CallImplementNode->FindPinChecked(TEXT("BaseClass"));
	UEdGraphPin* PathPin = CallImplementNode->FindPinChecked(TEXT("Path"));
	UEdGraphPin* SortPin = CallImplementNode->FindPinChecked(TEXT("bSortAlphabetically"));
	UEdGraphPin* ResultPin = CallImplementNode->GetReturnValuePin();
	ResultPin->PinType.PinSubCategoryObject = ClassToFind;
	
	CompilerContext.MovePinLinksToIntermediate(*GetClassPin(), *BaseClassPin);
	CompilerContext.MovePinLinksToIntermediate(*GetPathPin(), *PathPin);
	CompilerContext.MovePinLinksToIntermediate(*GetSortPin(), *SortPin);
	CompilerContext.MovePinLinksToIntermediate(*GetResultPin(), *ResultPin);

	BreakAllNodeLinks();
}

#undef LOCTEXT_NAMESPACE

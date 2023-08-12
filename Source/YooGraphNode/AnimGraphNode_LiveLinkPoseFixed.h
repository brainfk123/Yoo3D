// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AnimNode_LiveLinkPose.h"
#include "AnimGraph/Public/AnimGraphNode_Base.h"
#include "AnimGraphNode_LiveLinkPoseFixed.generated.h"

UCLASS()
class YOOGRAPHNODE_API UAnimGraphNode_LiveLinkPoseFixed : public UAnimGraphNode_Base
{
	GENERATED_UCLASS_BODY()

	UPROPERTY(EditAnywhere, Category=Settings)
	FAnimNode_LiveLinkPose Node;

	// UEdGraphNode interface
	virtual FText GetTooltipText() const override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FLinearColor GetNodeTitleColor() const override;
	virtual FText GetMenuCategory() const override;
	// End of UEdGraphNode interface
};

// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimGraphNode_LiveLinkPoseFixed.h"
#include "Internationalization/Internationalization.h"

#define LOCTEXT_NAMESPACE "LiveLinkAnimNode"

UAnimGraphNode_LiveLinkPoseFixed::UAnimGraphNode_LiveLinkPoseFixed(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
}

FText UAnimGraphNode_LiveLinkPoseFixed::GetTooltipText() const
{
	return LOCTEXT("AnimGraphNode_LiveLinkPose_Tooltip", "Retrieves the current pose associated with the supplied subject");
}

FText UAnimGraphNode_LiveLinkPoseFixed::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return LOCTEXT("AnimGraphNode_LiveLinkPose_Title", "Live Link Pose Fixed");
}

FText UAnimGraphNode_LiveLinkPoseFixed::GetMenuCategory() const
{
	return LOCTEXT("AnimGraphNode_LiveLinkPose_Category", "Live Link");
}

FLinearColor UAnimGraphNode_LiveLinkPoseFixed::GetNodeTitleColor() const
{
	return FLinearColor(0.8f, 0.2f, 0.8f);
}
#undef LOCTEXT_NAMESPACE
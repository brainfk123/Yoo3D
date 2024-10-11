// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "LiveLinkTypes.h"
#include "Modules/ModuleManager.h"

DECLARE_DELEGATE_RetVal_OneParam(float, FGetLiveLinkSkeletonHeight, FName)

class FNeuronLiveLinkModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	FGetLiveLinkSkeletonHeight GetLiveLinkSkeletonHeight;

	NEURONLIVELINK_API static FNeuronLiveLinkModule& Get();
private:
	void* MocapApiLibraryHandle = nullptr;
};

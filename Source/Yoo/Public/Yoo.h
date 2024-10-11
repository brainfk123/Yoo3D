// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FSlateStyleSet;
class UMainEditWidget;
DECLARE_LOG_CATEGORY_EXTERN(LogYoo, Log, All);

class FYooModule : public FDefaultGameModuleImpl
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
private:
	static TSharedPtr<FSlateStyleSet> Style;
};
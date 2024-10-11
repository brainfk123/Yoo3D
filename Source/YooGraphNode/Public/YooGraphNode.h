#pragma once

#include "CoreMinimal.h"

class FYooGraphNodeModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
private:
	static TSharedPtr<FSlateStyleSet> Style;
};
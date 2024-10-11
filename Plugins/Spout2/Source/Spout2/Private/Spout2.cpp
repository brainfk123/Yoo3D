// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "Spout2.h"

#include "ShaderCore.h"
#include "Interfaces/IPluginManager.h"

#define LOCTEXT_NAMESPACE "FSpout2Module"

void FSpout2Module::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	FString PluginShaderDir = FPaths::Combine(IPluginManager::Get().FindPlugin(TEXT("Spout2"))->GetBaseDir(), TEXT("Shaders"));
	AddShaderSourceDirectoryMapping(TEXT("/Plugin/Spout2"), PluginShaderDir);
	
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	FString ThisPlugin = TEXT("Spout2");
	FString BaseDir = IPluginManager::Get().FindPlugin(ThisPlugin)->GetBaseDir();
	FString LibraryPath;
#if PLATFORM_WINDOWS
	LibraryPath = FPaths::Combine(*BaseDir, TEXT("ThirdParty/Spout/lib/amd64/Spout.dll"));
#endif // PLATFORM_WINDOWS

	Spout2LibraryHandle = !LibraryPath.IsEmpty() ? FPlatformProcess::GetDllHandle(*LibraryPath) : nullptr;

	if (Spout2LibraryHandle)
	{
		UE_LOG(LogSpout2, Log, TEXT("Spout2 dll loaded success."));
		FMD5Hash hash = FMD5Hash::HashFile(*LibraryPath);
		UE_LOG(LogSpout2, Log, TEXT("Spout2.dll Hash: %s"), *LexToString(hash));
	}
	else
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("ThirdPartyLibraryError", "Failed to load Spout2.dll."));
	}
}

void FSpout2Module::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	
	if (Spout2LibraryHandle)
	{
		FPlatformProcess::FreeDllHandle(Spout2LibraryHandle);
		Spout2LibraryHandle = nullptr;
	}
}

DEFINE_LOG_CATEGORY(LogSpout2);

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FSpout2Module, Spout2)
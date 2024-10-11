// Copyright Epic Games, Inc. All Rights Reserved.

#include "Yoo.h"
#include "MainEditWidget.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Engine/UserInterfaceSettings.h"
#include "Misc/PackageName.h"
#include "Modules/ModuleManager.h"
#include "Styling/SlateStyle.h"
#include "Styling/SlateStyleRegistry.h"
#include "Windows/WindowsPlatformApplicationMisc.h"

IMPLEMENT_PRIMARY_GAME_MODULE(FYooModule, Yoo, "Yoo3D");
DEFINE_LOG_CATEGORY(LogYoo);

#define IMAGE_BRUSH_SVG(RelativePath, ...) FSlateVectorImageBrush(Style->RootToContentDir(RelativePath, TEXT(".svg")), __VA_ARGS__)

TSharedPtr<FSlateStyleSet> FYooModule::Style;

void FYooModule::StartupModule()
{
#if !WITH_EDITOR
	FDisplayMetrics DisplayMetrics;
	FSlateApplication::Get().GetDisplayMetrics(DisplayMetrics);
	const float DPIScaleFactor = FPlatformApplicationMisc::GetDPIScaleFactorAtPoint(DisplayMetrics.PrimaryDisplayWorkAreaRect.Left, DisplayMetrics.PrimaryDisplayWorkAreaRect.Top);

	UUserInterfaceSettings* UISettings = GetMutableDefault<UUserInterfaceSettings>(UUserInterfaceSettings::StaticClass());
	UISettings->ApplicationScale = DPIScaleFactor;
#endif
	if (!Style.IsValid())
	{
		Style = MakeShareable(new FSlateStyleSet("CustomUMGStyle"));
		Style->SetContentRoot(FPaths::EngineContentDir() / TEXT("Editor/Slate/UMG"));
	}
	const FVector2D Icon16x16(16.0f, 16.0f);
	Style->Set("ClassIcon.StackWidgetBox", new IMAGE_BRUSH_SVG(TEXT("Overlay"), Icon16x16));
	FSlateStyleRegistry::RegisterSlateStyle(*Style);
	
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();
	TArray<FString> ContentPaths;
	ContentPaths.Emplace(TEXT("/Game/NewEditor"));
	AssetRegistry.ScanPathsSynchronous(ContentPaths);

	FPackageName::RegisterMountPoint(TEXT("/Save/"), FPaths::ProjectSavedDir());
}

void FYooModule::ShutdownModule()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*Style);
	ensure(Style.IsUnique());
	Style.Reset();
}

#undef IMAGE_BRUSH_SVG

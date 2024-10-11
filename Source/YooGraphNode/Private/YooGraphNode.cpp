#include "YooGraphNode.h"
#include "Styling/SlateStyleRegistry.h"

IMPLEMENT_MODULE(FYooGraphNodeModule, YooEditorNode);

TSharedPtr<FSlateStyleSet> FYooGraphNodeModule::Style;

#define IMAGE_BRUSH(RelativePath, ...) FSlateImageBrush(Style->RootToContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__)
void FYooGraphNodeModule::StartupModule()
{
	if (!Style.IsValid())
	{
		Style = MakeShareable(new FSlateStyleSet("YooGraphNodeStyle"));
		Style->SetContentRoot(FPaths::EngineContentDir() / TEXT("Editor/Slate"));
	}
	const FVector2D Icon16x16(16.0f, 16.0f);
	Style->Set("GraphEditor.FindBlueprint_16x", new FSlateImageBrush(Style->RootToContentDir("Icons/icon_Blueprint_Find_40px", TEXT(".png")), Icon16x16));
	FSlateStyleRegistry::RegisterSlateStyle(*Style);
}
#undef IMAGE_BRUSH

void FYooGraphNodeModule::ShutdownModule()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*Style);
	ensure(Style.IsUnique());
	Style.Reset();
}

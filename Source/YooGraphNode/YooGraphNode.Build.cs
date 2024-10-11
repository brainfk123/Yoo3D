using UnrealBuildTool;

public class YooGraphNode : ModuleRules
{
    public YooGraphNode(ReadOnlyTargetRules Target) : base(Target)
    {
        OptimizeCode = CodeOptimization.InShippingBuildsOnly;
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		OverridePackageType = PackageOverrideType.GameUncookedOnly;

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
                "LiveLinkAnimationCore",
                "Yoo",
                "AnimGraphRuntime",
                "AnimGraph",
                "EditorFramework",
                "UnrealEd",
                "GraphEditor",
                "Kismet",
                "KismetCompiler",
                "BlueprintGraph",
            }
        );
    }
}
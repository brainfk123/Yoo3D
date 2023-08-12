using UnrealBuildTool;

public class YooGraphNode : ModuleRules
{
    public YooGraphNode(ReadOnlyTargetRules Target) : base(Target)
    {
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
            }
        );
        
        if (Target.bBuildEditor)
        {
            PrivateDependencyModuleNames.AddRange(
                new string[]
                {
                    "AnimGraph",
                    "EditorFramework",
                    "UnrealEd",
                    "Kismet",
                    "BlueprintGraph",
                }
            );
        }
    }
}
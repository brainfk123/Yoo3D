// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Yoo : ModuleRules
{
	public Yoo(ReadOnlyTargetRules Target) : base(Target)
	{
		OptimizeCode = CodeOptimization.InShippingBuildsOnly;
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PrivateDependencyModuleNames.AddRange(new string[] 
		{ 
			"Spout2", 
			"ApplicationCore", 
			"AppleARKitFaceSupport", 
			"AugmentedReality" 
		});
		
		PublicDependencyModuleNames.AddRange(new string[]
			{ 
				"Core",
				"CoreUObject", 
				"Engine", 
				"Networking", 
				"InputCore", 
				"HeadMountedDisplay",
				"EnhancedInput", 
				"AnimGraphRuntime",
				"LiveLinkInterface",
				"LiveLinkAnimationCore",
				"NeuronLiveLink",
				"SlateCore",
				"UMG",
				"Slate",
				"LiveLink",
				"AppFramework"
			});
	}
}

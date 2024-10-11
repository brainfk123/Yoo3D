// Some copyright should be here...

using UnrealBuildTool;
using System.IO;

public class Spout2 : ModuleRules
{
	private string ThirdPartyPath
	{
		get { return Path.GetFullPath(Path.Combine(PluginDirectory, "ThirdParty/")); }
	}

	public Spout2(ReadOnlyTargetRules Target) : base(Target)
	{
		OptimizeCode = CodeOptimization.InShippingBuildsOnly;
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				Path.Combine(ThirdPartyPath, "Spout/include")
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core", 
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"RenderCore",
				"RHI",
				"Projects",
				"D3D11RHI",
				"Media",
				// ... add private dependencies that you statically link with here ...	
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);

		if ((Target.Platform == UnrealTargetPlatform.Win64))
		{
			string PlatformString = (Target.Platform == UnrealTargetPlatform.Win64) ? "amd64" : "x86";
			string LibPath = Path.Combine(ThirdPartyPath, "Spout/lib", PlatformString);
            
			PublicAdditionalLibraries.Add(Path.Combine(LibPath, "Spout.lib"));
            
			RuntimeDependencies.Add(Path.Combine(LibPath, "Spout.dll"));

			// Delay-load the DLL, so we can load it from the right place first
			PublicDelayLoadDLLs.Add("Spout.dll");
		}
	}
}

// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class CiF : ModuleRules
{
	public CiF(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				// this dependency isn't the best but if i intend to use this UI in all my projects,
				// then it is ok, as long as i don't upload this to market
				"GLS",
				"ModelViewViewModel",
				"MK_UI",
				"UMG",
				"CommonUI"
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"InputCore",
				"Slate",
				"SlateCore",
				"FileSystemUtilities", // for loading and writing files from file system like json
				"Json",
				"JsonUtilities"
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
	}
}

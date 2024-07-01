using UnrealBuildTool;

public class FileSystemUtilities : ModuleRules
{
	public FileSystemUtilities(ReadOnlyTargetRules Target) : base(Target)
	{
		PrivateDependencyModuleNames.AddRange(new string[] {"Core", "CoreUObject", "Engine", "Json"}); 
	}
}
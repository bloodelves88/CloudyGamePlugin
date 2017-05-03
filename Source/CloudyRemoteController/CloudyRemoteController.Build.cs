using UnrealBuildTool;
using System.IO;

public class CloudyRemoteController : ModuleRules
{
	public CloudyRemoteController(TargetInfo Target)
	{
		PrivateIncludePaths.AddRange(
            new string[] {
			    "CloudyRemoteController/Private",
		    }
        );


		PublicIncludePaths.AddRange(
            new string[] {
                "CloudyRemoteController/Public"
            }
        );


		PublicDependencyModuleNames.AddRange(
            new string[] {
                "Engine", "Core", "CoreUObject", "Sockets", "Networking", "InputCore"
            }
        );

	}
}
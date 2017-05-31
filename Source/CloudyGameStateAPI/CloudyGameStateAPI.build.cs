using UnrealBuildTool;
using System.IO;
 
public class CloudyGameStateAPI : ModuleRules
{
    public CloudyGameStateAPI(TargetInfo Target)
    {
        PrivateIncludePaths.AddRange(
            new string[] {
                "CloudyGameStateAPI/Private",
            }
        );
        PublicIncludePaths.AddRange(
            new string[] { 
                "CloudyGameStateAPI/Public",
            }
        );

        PublicDependencyModuleNames.AddRange(
            new string[] {
                "Core",
                "CoreUObject",
                "Engine",
                "Http",
                "Sockets",
                "Networking"
            }
        );
        PrivateDependencyModuleNames.AddRange(
            new string[] { 
                "Sockets",
                "Networking"
            }
        );
    }
}
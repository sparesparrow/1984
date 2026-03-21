using UnrealBuildTool;
using System.Collections.Generic;

public class Project1984Target : TargetRules
{
    public Project1984Target(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Game;
        DefaultBuildSettings = BuildSettingsVersion.V4;
        IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_3;

        ExtraModuleNames.AddRange(new string[] { "Project1984" });

        // Android / Meta Quest optimizations
        if (Platform == UnrealTargetPlatform.Android)
        {
            bUseUnityBuild = true;
        }
    }
}

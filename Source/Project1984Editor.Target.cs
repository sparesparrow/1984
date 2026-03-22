using UnrealBuildTool;
using System.Collections.Generic;

public class Project1984EditorTarget : TargetRules
{
    public Project1984EditorTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Editor;
        DefaultBuildSettings = BuildSettingsVersion.V4;
        IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_3;

        ExtraModuleNames.AddRange(new string[] { "Project1984" });
    }
}

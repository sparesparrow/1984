using UnrealBuildTool;

public class Project1984 : ModuleRules
{
    public Project1984(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "InputCore",
            "EnhancedInput",
            "HeadMountedDisplay",
            "UMG",
            "Slate",
            "SlateCore",
            "AIModule",
            "NavigationSystem"
        });

        PrivateDependencyModuleNames.AddRange(new string[]
        {
            "OpenXRHMD",
            "OpenXRHandTracking"
        });

        // Meta Quest / Android specific
        if (Target.Platform == UnrealTargetPlatform.Android)
        {
            PrivateDependencyModuleNames.Add("MetaXR");
        }
    }
}

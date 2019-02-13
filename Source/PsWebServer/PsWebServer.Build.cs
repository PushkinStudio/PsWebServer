// Copyright 2015-2019 Mail.Ru Group. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class PsWebServer : ModuleRules
{
    public PsWebServer(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PrivateIncludePaths.AddRange(
            new string[] {
                "PsWebServer/Private",
                "PsWebServer/Private/civetweb",
                "PsWebServer/Private/civetweb/include",
            }
        );

        PublicIncludePaths.AddRange(
            new string[] {
                Path.Combine(ModuleDirectory, "Private"),
                Path.Combine(ModuleDirectory, "Private/civetweb"),
                Path.Combine(ModuleDirectory, "Private/civetweb/include"),
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
				// ... add private dependencies that you statically link with here ...	
			}
            );


        DynamicallyLoadedModuleNames.AddRange(
            new string[]
            {
				// ... add any modules that your module loads dynamically here ...
			}
            );

        if ((Target.Platform == UnrealTargetPlatform.Win64) ||
            (Target.Platform == UnrealTargetPlatform.Win32) ||
            (Target.Platform == UnrealTargetPlatform.Mac) ||
            (Target.Platform == UnrealTargetPlatform.Linux))
        {
            PublicDefinitions.Add("WITH_CIVET=1");
        }
        else
        {
            PublicDefinitions.Add("WITH_CIVET=0");
        }

        // Enable exceptions to allow error handling
        bEnableExceptions = true;
    }
}

// Copyright 2015-2021 Mail.Ru Group. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class PsWebServer : ModuleRules
{
    public PsWebServer(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

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

        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            PublicDefinitions.Add("WITH_CIVET=1");
            PublicDefinitions.Add("OPENSSL_API_1_0");
            bEnableExceptions = true;

            PrivateIncludePaths.AddRange(
                new string[] {
                    "PsWebServer/Private",
                    "PsWebServer/Private/Win64",
                    "PsWebServer/Private/Win64/include",
                    }
            );

            PublicIncludePaths.AddRange(
                new string[] {
                    Path.Combine(ModuleDirectory, "Private"),
                    Path.Combine(ModuleDirectory, "Private/Win64"),
                    Path.Combine(ModuleDirectory, "Private/Win64/include"),
                }
            );
        }
        else if (Target.Platform == UnrealTargetPlatform.Mac)
        {
            PublicDefinitions.Add("WITH_CIVET=1");
            PublicDefinitions.Add("OPENSSL_API_1_0");
            bEnableExceptions = true;

            PrivateIncludePaths.AddRange(
                new string[] {
                    "PsWebServer/Private",
                    "PsWebServer/Private/Mac",
                    "PsWebServer/Private/Mac/include",
                    }
            );

            PublicIncludePaths.AddRange(
                new string[] {
                    Path.Combine(ModuleDirectory, "Private"),
                    Path.Combine(ModuleDirectory, "Private/Mac"),
                    Path.Combine(ModuleDirectory, "Private/Mac/include"),
                }
            );
        }
        else if (Target.Platform == UnrealTargetPlatform.Linux)
        {
            PublicDefinitions.Add("WITH_CIVET=1");
            PublicDefinitions.Add("OPENSSL_API_1_0");
            //PublicDefinitions.Add("USE_ZLIB");
            bEnableExceptions = true;

            PrivateIncludePaths.AddRange(
                new string[] {
                    "PsWebServer/Private",
                    "PsWebServer/Private/Linux",
                    "PsWebServer/Private/Linux/include",
                    }
            );

            PublicIncludePaths.AddRange(
                new string[] {
                    Path.Combine(ModuleDirectory, "Private"),
                    Path.Combine(ModuleDirectory, "Private/Linux"),
                    Path.Combine(ModuleDirectory, "Private/Linux/include"),
                }
            );
        }
        else
        {
            PublicDefinitions.Add("WITH_CIVET=0");
        }
    }
}

// Copyright 2015-2019 Mail.Ru Group. All Rights Reserved.

#include "PsWebServer.h"

#include "PsWebServerSettings.h"

#include "Developer/Settings/Public/ISettingsModule.h"
#include "UObject/Package.h"

#define LOCTEXT_NAMESPACE "FPsWebServerModule"

void FPsWebServerModule::StartupModule()
{
	WebServerSettings = NewObject<UPsWebServerSettings>(GetTransientPackage(), "PsWebServerSettings", RF_Standalone);
	WebServerSettings->AddToRoot();
	WebServerSettings->Load();

	// Register settings
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->RegisterSettings("Project", "Plugins", "PsWebServer",
			LOCTEXT("RuntimeSettingsName", "PsWebServer"),
			LOCTEXT("RuntimeSettingsDescription", "Configure web server"),
			WebServerSettings);
	}
}

void FPsWebServerModule::ShutdownModule()
{
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->UnregisterSettings("Project", "Plugins", "PsWebServer");
	}

	if (!GExitPurge)
	{
		// If we're in exit purge, this object has already been destroyed
		WebServerSettings->RemoveFromRoot();
	}
	else
	{
		WebServerSettings = nullptr;
	}
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FPsWebServerModule, PsWebServer)
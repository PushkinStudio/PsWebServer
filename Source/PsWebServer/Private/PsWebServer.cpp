// Copyright 2015-2019 Mail.Ru Group. All Rights Reserved.

#include "PsWebServer.h"

#include "PsCivetWebServer.h"

#define LOCTEXT_NAMESPACE "FPsWebServerModule"

void FPsWebServerModule::StartupModule()
{
	WebServer = NewObject<UPsCivetWebServer>(GetTransientPackage());
	WebServer->SetFlags(RF_Standalone);
	WebServer->AddToRoot();
}

void FPsWebServerModule::ShutdownModule()
{
	if (!GExitPurge)
	{
		// If we're in exit purge, this object has already been destroyed
		WebServer->RemoveFromRoot();
	}
	else
	{
		WebServer = nullptr;
	}
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FPsWebServerModule, PsWebServer)
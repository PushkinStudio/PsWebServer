// Copyright 2015-2019 Mail.Ru Group. All Rights Reserved.

#include "PsWebServer.h"

#include "PsWebServerWrapper.h"

#define LOCTEXT_NAMESPACE "FPsWebServerModule"

void FPsWebServerModule::StartupModule()
{
	WebServer = NewObject<UPsWebServerWrapper>(GetTransientPackage());
	WebServer->SetFlags(RF_Standalone);
	WebServer->AddToRoot();
}

void FPsWebServerModule::ShutdownModule()
{
	// Stop server first
	WebServer->StopServer();

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
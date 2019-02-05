// Copyright 2015-2019 Mail.Ru Group. All Rights Reserved.

#include "PsWebServer.h"

#define LOCTEXT_NAMESPACE "FPsWebServerModule"

void FPsWebServerModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
}

void FPsWebServerModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FPsWebServerModule, PsWebServer)
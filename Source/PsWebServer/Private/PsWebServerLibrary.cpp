// Copyright 2015-2019 Mail.Ru Group. All Rights Reserved.

#include "PsWebServerLibrary.h"

#include "PsWebServer.h"

UPsWebServerLibrary::UPsWebServerLibrary(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UPsCivetWebServer* UPsWebServerLibrary::GetWebServer()
{
	return FPsWebServerModule::Get().WebServer;
}

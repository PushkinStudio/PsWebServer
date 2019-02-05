// Copyright 2015-2019 Mail.Ru Group. All Rights Reserved.

#include "PsWebServerLibrary.h"

#include "PsWebServerWrapper.h"

UPsWebServerLibrary::UPsWebServerLibrary(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UPsWebServerWrapper* UPsWebServerLibrary::GetWebServer()
{
	return FPsWebServerModule::Get().WebServer;
}

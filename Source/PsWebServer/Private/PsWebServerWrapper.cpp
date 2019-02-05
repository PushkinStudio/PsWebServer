// Copyright 2015-2019 Mail.Ru Group. All Rights Reserved.

#include "PsWebServerWrapper.h"

UPsWebServerWrapper::UPsWebServerWrapper(const class FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	Server = nullptr;
}

void UPsWebServerWrapper::StopServer()
{
	if (Server)
	{
		Server->close();

		delete Server;
		Server = nullptr;
	}
}

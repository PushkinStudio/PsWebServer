// Copyright 2015-2019 Mail.Ru Group. All Rights Reserved.

#pragma once

#include "civetweb/include/CivetServer.h"

#include "CoreMinimal.h"

class PsWebServerHandler : public CivetHandler
{
public:
	PsWebServerHandler();

	bool handlePost(CivetServer* server, struct mg_connection* conn);
	bool handleGet(CivetServer* server, struct mg_connection* conn);

public:
	/** Request URI */
	FString URI;

	/** Timeout in msec for the entire http request to complete (see UPsWebServerSettings::RequestTimeout)*/
	int32 RequestTimeout;
};

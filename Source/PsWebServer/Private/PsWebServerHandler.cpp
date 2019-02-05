// Copyright 2015-2019 Mail.Ru Group. All Rights Reserved.

#include "PsWebServerHandler.h"

PsWebServerHandler::PsWebServerHandler()
{
	RequestTimeout = 0;
}

bool PsWebServerHandler::handlePost(CivetServer* server, struct mg_connection* conn)
{
	return true;
}

bool PsWebServerHandler::handleGet(CivetServer* server, struct mg_connection* conn)
{
	return true;
}

// Copyright 2015-2019 Mail.Ru Group. All Rights Reserved.

#include "PsWebServerHandler.h"

#include "PsWebServerDefines.h"
#include "PsWebServerSettings.h"
#include "PsWebServerWrapper.h"

WebServerHandler::WebServerHandler()
{
	RequestTimeout = 0;
}

bool WebServerHandler::handlePost(CivetServer* server, struct mg_connection* conn)
{
	// Create async event waiter
	FEvent* RequestReadyEvent = FGenericPlatformProcess::GetSynchEventFromPool();

	// Prepare vars for lambda
	TWeakObjectPtr<UPsWebServerHandler> RequestHandler = OwnerHandler;

	AsyncTask(ENamedThreads::GameThread, [RequestHandler, RequestReadyEvent]() {
		if (RequestHandler.IsValid())
		{
			RequestHandler.Get()->ProcessPost();
		}

		// @TODO Test long event processing here

		RequestReadyEvent->Trigger();
	});

	// Wait for event or timeout
	bool EventTriggered = RequestReadyEvent->Wait(RequestTimeout > 0 ? RequestTimeout : 2000);
	FGenericPlatformProcess::ReturnSynchEventToPool(RequestReadyEvent);

	// Temporary example for now
	const char* msg = "Hello world";
	unsigned long len = (unsigned long)strlen(msg);

	mg_printf(conn,
		"HTTP/1.1 200 OK\r\n"
		"Content-Length: %lu\r\n"
		"Content-Type: text/plain\r\n"
		"Connection: close\r\n\r\n",
		len);

	mg_write(conn, msg, len);

	return true;
}

bool WebServerHandler::handleGet(CivetServer* server, struct mg_connection* conn)
{
	// Temporary solution for dev purposes
	return handlePost(server, conn);
}

UPsWebServerHandler::UPsWebServerHandler(const class FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UPsWebServerHandler::BeginDestroy()
{
	if (Wrapper.IsValid())
	{
		Wrapper.Get()->GetServer()->removeHandler(TCHAR_TO_ANSI(*HandlerURI));
	}
}

bool UPsWebServerHandler::BindHandler(UPsWebServerWrapper* ServerWrapper, const FString& URI)
{
	if (ServerWrapper == nullptr)
	{
		UE_LOG(LogPwsAll, Error, TEXT("%s: Can't bind hadler: invalid ServerWrapper"), *PS_FUNC_LINE);
		return false;
	}

	if (URI.IsEmpty())
	{
		UE_LOG(LogPwsAll, Error, TEXT("%s: Can't bind hadler: URI is empty"), *PS_FUNC_LINE);
		return false;
	}

	// Cache ServerWrapper pointer and its URI (it's reqired by UPsWebServerHandler::BeginDestroy())
	HandlerURI = URI;
	Wrapper = ServerWrapper;

	// Cache request timeout from config
	const UPsWebServerSettings* ServerSettings = GetDefault<UPsWebServerSettings>();
	check(ServerSettings);
	Handler.RequestTimeout = ServerSettings->RequestTimeout;

	// Bind handler to civet server internal instance
	Handler.OwnerHandler = (const_cast<UPsWebServerHandler*>(this));
	ServerWrapper->GetServer()->addHandler(TCHAR_TO_ANSI(*HandlerURI), Handler);

	return true;
}

void UPsWebServerHandler::ProcessPost()
{
	UE_LOG(LogPwsAll, Warning, TEXT("%s: Override me"), *PS_FUNC_LINE);
}

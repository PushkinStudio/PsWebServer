// Copyright 2015-2019 Mail.Ru Group. All Rights Reserved.

#include "PsWebServerHandler.h"

#include "PsWebServerDefines.h"
#include "PsWebServerSettings.h"
#include "PsWebServerWrapper.h"

#include "HAL/CriticalSection.h"
#include "Misc/ScopeLock.h"

#if WITH_CIVET
WebServerHandler::WebServerHandler()
{
	RequestTimeout = 0;
}

bool WebServerHandler::handlePost(CivetServer* server, struct mg_connection* conn)
{
	// Create async event waiter
	FEvent* RequestReadyEvent = FGenericPlatformProcess::GetSynchEventFromPool();

	// Generate unique for request processor
	const FGuid RequestUniqueId = FGuid::NewGuid();
	{
		FScopeLock Lock(&CriticalSection);
		ResponseDatas.Add(RequestUniqueId);
	}

	// Prepare vars for lambda
	TWeakObjectPtr<UPsWebServerHandler> RequestHandler = OwnerHandler;
	FString PostData = CivetServer::getPostData(conn).c_str();

	AsyncTask(ENamedThreads::GameThread, [RequestHandler, RequestUniqueId, PostData = std::move(PostData), RequestReadyEvent]() {
		if (RequestHandler.IsValid())
		{
			RequestHandler.Get()->ProcessRequest(RequestUniqueId, PostData);
		}

		// @TODO Test long event processing here (more than RequestTimeout)
		RequestReadyEvent->Trigger();
	});

	// Wait for event or timeout
	int32 WaitTime = RequestTimeout;
	bool EventTriggered = RequestReadyEvent->Wait(WaitTime > 0 ? WaitTime : 2000);
	FGenericPlatformProcess::ReturnSynchEventToPool(RequestReadyEvent);

	// Fetch response data
	std::string OutputDataBody("{}");
	{
		FScopeLock Lock(&CriticalSection);
		OutputDataBody = std::string(TCHAR_TO_UTF8(*ResponseDatas.FindAndRemoveChecked(RequestUniqueId)));
	}

	// @TODO Should be more elegant and externally controlled
	FString ReplyCode = TEXT("200 OK");
	FString ContentType = TEXT("application/json");

	FString ResponseHeader = FString::Printf(TEXT("HTTP/1.1 %s\r\n"
												  "Server: Pushkin Web Server\r\n"
												  "Content-Type: %s\r\n"
												  "Content-Length: %d\r\n"
												  "Connection: keep-alive\r\n"
												  "\r\n"),
		*ReplyCode, *ContentType, (int32)OutputDataBody.size());

	std::string OutputDataHeader = std::string(TCHAR_TO_UTF8(*ResponseHeader));

	mg_printf(conn, "%s", OutputDataHeader.c_str());
	int32 NumWritten = mg_write(conn, OutputDataBody.c_str(), OutputDataBody.size());

	return true;
}

bool WebServerHandler::handleGet(CivetServer* server, struct mg_connection* conn)
{
	// We support only POST for now
	return false;
}

bool WebServerHandler::SetResponseData(const FGuid& RequestUniqueId, const FString& ResponseData)
{
	FScopeLock Lock(&CriticalSection);

	if (ResponseDatas.Contains(RequestUniqueId))
	{
		ResponseDatas.Emplace(RequestUniqueId, ResponseData);
		return true;
	}
	else
	{
		UE_LOG(LogPwsAll, Error, TEXT("%s: No GUID %s is found for data: %s"), *PS_FUNC_LINE, *RequestUniqueId.ToString(), *ResponseData);
	}

	return false;
}
#endif // WITH_CIVET

void UPsWebServerHandler::BeginDestroy()
{
#if WITH_CIVET
	if (Wrapper.IsValid())
	{
		Wrapper.Get()->RemoveHandler(HandlerURI);
	}
#endif

	Super::BeginDestroy();
}

void UPsWebServerHandler::ProcessRequest_Implementation(const FGuid& RequestUniqueId, const FString& RequestData)
{
	UE_LOG(LogPwsAll, Warning, TEXT("%s: Override me"), *PS_FUNC_LINE);
}

bool UPsWebServerHandler::SetResponseData_Implementation(const FGuid& RequestUniqueId, const FString& ResponseData)
{
#if WITH_CIVET
	return Handler.SetResponseData(RequestUniqueId, ResponseData);
#else
	return false;
#endif
}

FString UPsWebServerHandler::GetURI() const
{
	return HandlerURI;
}

bool UPsWebServerHandler::BindHandler(UPsWebServerWrapper* ServerWrapper, const FString& URI)
{
#if WITH_CIVET
	if (ServerWrapper == nullptr)
	{
		UE_LOG(LogPwsAll, Error, TEXT("%s: Can't bind hadler: invalid ServerWrapper"), *PS_FUNC_LINE);
		return false;
	}

	if (ServerWrapper->Server == nullptr)
	{
		UE_LOG(LogPwsAll, Error, TEXT("%s: Can't bind hadler: you should run server first"), *PS_FUNC_LINE);
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
	ServerWrapper->Server->addHandler(TCHAR_TO_ANSI(*HandlerURI), Handler);

	return true;
#else
	UE_LOG(LogPwsAll, Error, TEXT("%s: Civet is not supported"), *PS_FUNC_LINE);

	return false;
#endif // WITH_CIVET
}

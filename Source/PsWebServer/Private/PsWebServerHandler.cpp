// Copyright 2015-2019 Mail.Ru Group. All Rights Reserved.

#include "PsWebServerHandler.h"

#include "PsWebServer.h"
#include "PsWebServerDefines.h"
#include "PsWebServerSettings.h"
#include "PsWebServerWrapper.h"

#include "HAL/CriticalSection.h"
#include "Misc/ScopeLock.h"

#if WITH_CIVET
WebServerHandler::WebServerHandler()
{
	RequestTimeout = 0;

	const UPsWebServerSettings* ServerSettings = FPsWebServerModule::Get().GetSettings();
	ResponseHeaders.Emplace(TEXT("Server"), TEXT("Pushkin Web Server"));
	ResponseHeaders.Emplace(TEXT("Content-Type"), TEXT("application/json"));
	ResponseHeaders.Emplace(TEXT("Connection"), ServerSettings->bEnableKeepAlive ? TEXT("keep-alive") : TEXT("close"));
	CachedResponseHeaders = PrintHeadersToString(ResponseHeaders);
}

bool WebServerHandler::handlePost(CivetServer* server, struct mg_connection* conn)
{
	// Create async event waiter and generate unique for request processor
	FEvent* RequestReadyEvent = FGenericPlatformProcess::GetSynchEventFromPool();
	const FGuid RequestUniqueId = FGuid::NewGuid();
	{
		FScopeLock Lock(&CriticalSection);
		ResponseDatas.Add(RequestUniqueId);
		RequestReadyEvents.Emplace(RequestUniqueId, RequestReadyEvent);
	}

	// Prepare vars for lambda
	TWeakObjectPtr<UPsWebServerHandler> RequestHandler = OwnerHandler;
	FString PostData = CivetServer::getPostData(conn).c_str();

	AsyncTask(ENamedThreads::GameThread, [RequestHandler, RequestUniqueId, PostData = std::move(PostData)]() {
		if (RequestHandler.IsValid())
		{
			RequestHandler.Get()->ProcessRequest(RequestUniqueId, PostData);
		}
	});

	// Wait for event or timeout
	int32 WaitTime = RequestTimeout;
	bool bEventTriggered = RequestReadyEvent->Wait(WaitTime > 0 ? WaitTime : 2000);
	{
		FScopeLock Lock(&CriticalSection);
		RequestReadyEvents.Remove(RequestUniqueId);
	}
	FGenericPlatformProcess::ReturnSynchEventToPool(RequestReadyEvent);

	// Fetch response data
	std::string OutputDataBody("{}");
	if (bEventTriggered)
	{
		FScopeLock Lock(&CriticalSection);
		OutputDataBody = std::string(TCHAR_TO_UTF8(*ResponseDatas.FindAndRemoveChecked(RequestUniqueId)));
	}

	// Update unique per-request params
	TMap<FString, FString> AdditionalHeaders;
	AdditionalHeaders.Emplace(TEXT("X-Request-ID"), RequestUniqueId.ToString());
	AdditionalHeaders.Emplace(TEXT("Content-Length"), FString::FromInt((int32)OutputDataBody.size()));

	// @TODO Reply code should be more elegant and externally controlled
	FString ReplyCode = bEventTriggered ? TEXT("200 OK") : TEXT("503 Service Unavailable");
	FString ResponseHeader = FString::Printf(TEXT("HTTP/1.1 %s\r\n"), *ReplyCode);
	{
		// Lock is not used because header set is allowed only before handler bind
		// FScopeLock Lock(&CriticalSection);
		ResponseHeader.Append(CachedResponseHeaders);
	}
	ResponseHeader.Append(PrintHeadersToString(AdditionalHeaders));
	ResponseHeader.Append("\r\n");

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

void WebServerHandler::ProcessRequest(const FGuid& RequestUniqueId, const FString& RequestData)
{
	FScopeLock Lock(&CriticalSection);

	if (auto SynchEvent = RequestReadyEvents.Find(RequestUniqueId))
	{
		(*SynchEvent)->Trigger();
	}
	else
	{
		UE_LOG(LogPwsAll, Error, TEXT("%s: No GUID %s is found for processing request (maybe RequestTimeout): %s"), *PS_FUNC_LINE, *RequestUniqueId.ToString(), *RequestData);
	}
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
void WebServerHandler::SetHeader(const FString& HeaderName, const FString& HeaderValue)
{
	FScopeLock Lock(&CriticalSection);

	ResponseHeaders.Emplace(HeaderName, HeaderValue);
	CachedResponseHeaders = PrintHeadersToString(ResponseHeaders);
}

FString WebServerHandler::PrintHeadersToString(const TMap<FString, FString>& Headers)
{
	FString OutputStr;
	for (auto& Header : Headers)
	{
		OutputStr.Append(Header.Key + ": " + Header.Value + "\r\n");
	}

	return OutputStr;
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
#if WITH_CIVET
	Handler.ProcessRequest(RequestUniqueId, RequestData);
#endif
}

bool UPsWebServerHandler::SetResponseData_Implementation(const FGuid& RequestUniqueId, const FString& ResponseData)
{
#if WITH_CIVET
	return Handler.SetResponseData(RequestUniqueId, ResponseData);
#else
	return false;
#endif
}

void UPsWebServerHandler::SetHeader(const FString& HeaderName, const FString& HeaderValue)
{
#if WITH_CIVET
	if (bHandlerBinned)
	{
		UE_LOG(LogPwsAll, Error, TEXT("%s: Can't set handler header: it's already binned"), *PS_FUNC_LINE);
		return;
	}

	Handler.SetHeader(HeaderName, HeaderValue);
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
	const UPsWebServerSettings* ServerSettings = FPsWebServerModule::Get().GetSettings();
	Handler.RequestTimeout = ServerSettings->RequestTimeout;

	// Bind handler to civet server internal instance
	Handler.OwnerHandler = (const_cast<UPsWebServerHandler*>(this));
	ServerWrapper->Server->addHandler(TCHAR_TO_ANSI(*HandlerURI), Handler);

	// Set as binned
	bHandlerBinned = true;

	return true;
#else
	UE_LOG(LogPwsAll, Error, TEXT("%s: Civet is not supported"), *PS_FUNC_LINE);

	return false;
#endif // WITH_CIVET
}

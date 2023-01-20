// Copyright 2015-2023 MY.GAMES. All Rights Reserved.

#if WITH_CIVET

#include "PsWebServerHandlerImpl.h"

#include "PsWebServerDefines.h"
#include "PsWebServerHandler.h"
#include "PsWebServerPlugin.h"
#include "PsWebServerSettings.h"
#include "PsWebServerUtils.h"

#include "Async/Async.h"
#include "Containers/StringConv.h"
#include "Misc/ScopeLock.h"
#include "Templates/UnrealTemplate.h"

FPsWebServerHandlerImpl::FPsWebServerHandlerImpl()
{
	RequestTimeout = 0;

	OwnerHandler = nullptr;

	const UPsWebServerSettings* ServerSettings = FPsWebServerModule::Get().GetSettings();
	ResponseHeaders.Emplace(TEXT("Server"), TEXT("Pushkin Web Server"));
	ResponseHeaders.Emplace(TEXT("Content-Type"), TEXT("application/json"));
	ResponseHeaders.Emplace(TEXT("Connection"), ServerSettings->bEnableKeepAlive ? TEXT("keep-alive") : TEXT("close"));
	ResponseHeaders.Emplace(TEXT("Transfer-Encoding"), TEXT("chunked"));
	CachedResponseHeaders = PrintHeadersToString(ResponseHeaders);
}

bool FPsWebServerHandlerImpl::handlePost(CivetServer* Server, mg_connection* RequestConnection)
{
	// Unique request id
	const FGuid RequestId = FGuid::NewGuid();

	const auto Handler = OwnerHandler.Load();
	if (!Handler || Handler->IsPendingKill())
	{
		const FString StatusCode = TEXT("503 Service Unavailable");
		const FString ResponseData = TEXT("Request handler is not valid");
		return SendResponse(RequestConnection, RequestId, StatusCode, ResponseData);
	}

	// Create cancellation source
	auto CancellationSource = FPsWebCancellationSource{};
	const auto CancellationToken = CancellationSource.GetToken();

	FEvent* const RequestReadyEvent = CreateContext(RequestConnection, RequestId, MoveTemp(CancellationSource));

	FString PostData = PsWebServerUtils::GetPostData(RequestConnection);

	// Set request processing on the game thread
	DECLARE_DELEGATE(FTaskDelegate);
	auto TaskDelegate = FTaskDelegate::CreateWeakLambda(Handler, [Handler, RequestId, PostData = MoveTemp(PostData), CancellationToken]() mutable {
		Handler->SetRequestOnNextTick(RequestId, MoveTemp(PostData), CancellationToken);
	});
	AsyncTask(ENamedThreads::GameThread, [TaskDelegate = MoveTemp(TaskDelegate)] { TaskDelegate.Execute(); });

	return WaitForResponse(RequestConnection, RequestReadyEvent, RequestId);
}

void FPsWebServerHandlerImpl::ProcessRequestFinish(const FGuid& RequestId, const FString& ResponseData)
{
	UE_LOG(LogPwsAll, Verbose, TEXT("%s: request id %s"), *PS_FUNC_LINE, *RequestId.ToString());

	FScopeLock Lock(&CriticalSection);

	const auto ContextPtr = Contexts.Find(RequestId);
	if (!ContextPtr)
	{
		UE_LOG(LogPwsAll, Error, TEXT("%s: No GUID %s is found for processing request (RequestTimeout)"), *PS_FUNC_LINE, *RequestId.ToString());
		return;
	}

	auto& Context = *ContextPtr;
	if (Context.bFinished)
	{
		UE_LOG(LogPwsAll, Error, TEXT("%s: request processing is already finished"), *PS_FUNC_LINE, *RequestId.ToString());
		return;
	}

	Context.bFinished = true;
	Context.Response = ResponseData;

	const auto ReadyEvent = Context.ReadyEvent;
	ReadyEvent->Trigger();
}

void FPsWebServerHandlerImpl::SetHeader(const FString& HeaderName, const FString& HeaderValue)
{
	FScopeLock Lock(&CriticalSection);

	ResponseHeaders.Emplace(HeaderName, HeaderValue);
	CachedResponseHeaders = PrintHeadersToString(ResponseHeaders);
}

FString FPsWebServerHandlerImpl::GetHeader(const FGuid& RequestId, const FString& HeaderName) const
{
	FScopeLock Lock(&CriticalSection);

	const auto ContextPtr = Contexts.Find(RequestId);
	if (!ContextPtr)
	{
		UE_LOG(LogPwsAll, Error, TEXT("%s: No GUID %s is found for processing request (RequestTimeout)"), *PS_FUNC_LINE, *RequestId.ToString());
		return FString{};
	}

	const auto& Context = *ContextPtr;

	const auto RequestConnection = Context.Connection;
	const std::string HeaderNameStd = std::string(TCHAR_TO_UTF8(*HeaderName));

	const auto ValueCStr = CivetServer::getHeader(RequestConnection, HeaderNameStd);
	return FString{ValueCStr};
}

FEvent* FPsWebServerHandlerImpl::CreateContext(mg_connection* RequestConnection, const FGuid& RequestId, FPsWebCancellationSource CancellationSource)
{
	UE_LOG(LogPwsAll, Verbose, TEXT("%s: request id %s"), *PS_FUNC_LINE, *RequestId.ToString());

	// Create async event waiter
	FEvent* const RequestReadyEvent = FGenericPlatformProcess::GetSynchEventFromPool();

	// Add a new request context
	auto Context = FContext{RequestConnection, RequestReadyEvent};
	Context.CancellationSource = MoveTemp(CancellationSource);

	FScopeLock Lock(&CriticalSection);
	Contexts.Add(RequestId, MoveTemp(Context));

	return RequestReadyEvent;
}

bool FPsWebServerHandlerImpl::WaitForResponse(mg_connection* RequestConnection, FEvent* RequestReadyEvent, const FGuid& RequestId)
{
	// Hold shared pointer to the instance
	const auto This = AsShared();

	// Wait for event or timeout
	const int32 WaitTime = RequestTimeout > 0 ? RequestTimeout : 5000;
	const bool bIgnoreThreadIdleStats = true;
	const bool bEventTriggered = RequestReadyEvent->Wait(WaitTime, bIgnoreThreadIdleStats);
	FGenericPlatformProcess::ReturnSynchEventToPool(RequestReadyEvent);

	const bool bTimeout = !bEventTriggered;
	UE_LOG(LogPwsAll, Verbose, TEXT("%s: request id %s: wait is over: timeout %d"), *PS_FUNC_LINE, *RequestId.ToString(), static_cast<int32>(bTimeout));

	const FString ResponseData = GetResponseData(RequestId, bTimeout);

	const FString StatusCode = !bTimeout ? TEXT("200 OK") : TEXT("503 Service Unavailable");

	return SendResponse(RequestConnection, RequestId, StatusCode, ResponseData);
}

FString FPsWebServerHandlerImpl::GetResponseData(const FGuid& RequestId, bool bTimeout)
{
	UE_LOG(LogPwsAll, Verbose, TEXT("%s: request id %s"), *PS_FUNC_LINE, *RequestId.ToString());

	FScopeLock Lock(&CriticalSection);
	auto Context = Contexts.FindAndRemoveChecked(RequestId);

	if (bTimeout)
	{
		UE_LOG(LogPwsAll, Error, TEXT("%s: request id %s: timeout reached"), *PS_FUNC_LINE, *RequestId.ToString());
		Context.CancellationSource.Cancel();

		const auto Handler = OwnerHandler.Load();
		if (Handler && !Handler->IsPendingKill())
		{
			return Handler->GetTimeoutResponse();
		}

		return TEXT("Timeout");
	}

	return Context.Response;
}

bool FPsWebServerHandlerImpl::SendResponse(mg_connection* RequestConnection, const FGuid& RequestId, const FString& StatusCode, const FString& ResponseData)
{
	UE_LOG(LogPwsAll, Verbose, TEXT("%s: request id %s"), *PS_FUNC_LINE, *RequestId.ToString());

	// Update unique per-request params
	TMap<FString, FString> AdditionalHeaders;
	AdditionalHeaders.Emplace(TEXT("X-Request-ID"), RequestId.ToString());

	FString ResponseHeader = FString::Printf(TEXT("HTTP/1.1 %s\r\n"), *StatusCode);
	ResponseHeader.Append(CachedResponseHeaders);
	ResponseHeader.Append(PrintHeadersToString(AdditionalHeaders));
	ResponseHeader.Append("\r\n");

	if (!Write(RequestConnection, ResponseHeader))
	{
		return false;
	}

	if (!WriteChunked(RequestConnection, ResponseData))
	{
		return false;
	}

	return true;
}

bool FPsWebServerHandlerImpl::Write(mg_connection* RequestConnection, const FString& Data)
{
	// Convert to UTF-8
	const auto Converter = FTCHARToUTF8(*Data);
	const char* const Ptr = Converter.Get();
	const size_t Len = Converter.Length();

	return mg_write(RequestConnection, Ptr, Len) > 0;
}

bool FPsWebServerHandlerImpl::WriteChunked(mg_connection* RequestConnection, const FString& Data)
{
	const int32 ChunkSize = 10240;

	for (int32 i = 0; i < Data.Len(); i += ChunkSize)
	{
		const int32 Size = FMath::Min(Data.Len(), i + ChunkSize) - i;
		const auto& Array = Data.GetCharArray();

		const auto* const DataPtr = Array.GetData() + i;

		// Convert to UTF-8
		const auto Converter = FTCHARToUTF8(DataPtr, Size);
		const char* const Ptr = Converter.Get();
		const size_t Len = Converter.Length();

		// Send a chunk of data
		if (mg_send_chunk(RequestConnection, Ptr, Len) <= 0)
		{
			UE_LOG(LogPwsAll, Error, TEXT("%s: cannot write data in the connection"), *PS_FUNC_LINE);
			return false;
		}
	}

	// Send a trailer
	const char Trailer[] = "0\r\n\r\n";
	const size_t TrailerSize = sizeof(Trailer) - 1;
	return mg_write(RequestConnection, Trailer, TrailerSize) > 0;
}

FString FPsWebServerHandlerImpl::PrintHeadersToString(const TMap<FString, FString>& Headers)
{
	FString OutputStr;
	for (auto& Header : Headers)
	{
		OutputStr.Append(Header.Key + ": " + Header.Value + "\r\n");
	}

	return OutputStr;
}

#endif // WITH_CIVET

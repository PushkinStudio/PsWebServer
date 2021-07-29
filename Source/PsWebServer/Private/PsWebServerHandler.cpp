// Copyright 2015-2021 Mail.Ru Group. All Rights Reserved.

#include "PsWebServerHandler.h"

#include "PsWebServer.h"
#include "PsWebServerDefines.h"
#include "PsWebServerPlugin.h"
#include "PsWebServerSettings.h"

#include "Engine/World.h"
#include "TimerManager.h"

#if WITH_CIVET
#include "PsWebServerHandlerImpl.h"
#endif

void UPsWebServerHandler::BeginDestroy()
{
	if (Server.IsValid())
	{
		Server->RemoveHandler(HandlerURI);
	}

	// Processing for scheduled requests will never be started
	for (const auto& RequestId : ScheduledRequests)
	{
		UE_LOG(LogPwsAll, Warning, TEXT("%s: request id '%s': reset with empty response"), *PS_FUNC_LINE, *RequestId.ToString());
#if WITH_CIVET
		Impl->ProcessRequestFinish(RequestId, FString{});
#endif
	}
	ScheduledRequests.Empty();

#if WITH_CIVET
	Impl->OwnerHandler = nullptr;
	Impl.Reset();
#endif

	Super::BeginDestroy();
}

UPsWebServerHandler::UPsWebServerHandler()
	: bProcessing(false)
	, bAborted(false)
#if WITH_CIVET
	, Impl(FPimpl{new FPsWebServerHandlerImpl{}})
#endif
{
}

void UPsWebServerHandler::ProcessRequest_Implementation(const FGuid& RequestId, const FString& RequestData)
{
	ProcessRequestFinish(RequestId, FString{});
}

void UPsWebServerHandler::ProcessRequestFinish(const FGuid& RequestId, const FString& ResponseData)
{
	if (bAborted)
	{
		UE_LOG(LogPwsAll, Error, TEXT("%s: request was aborted"), *PS_FUNC_LINE);
		return;
	}

	check(bProcessing);
	bProcessing = false;

	CancellationTokenPtr.Reset();

	UE_LOG(LogPwsAll, Verbose, TEXT("%s: request id '%s'"), *PS_FUNC_LINE, *RequestId.ToString());

#if WITH_CIVET
	Impl->ProcessRequestFinish(RequestId, ResponseData);
#endif
}

bool UPsWebServerHandler::IsCancelled() const
{
	check(bProcessing);
	return CancellationTokenPtr->IsCanceled();
}

bool UPsWebServerHandler::IsProcessing() const
{
	return bProcessing;
}

bool UPsWebServerHandler::IsAborted() const
{
	return bAborted;
}

void UPsWebServerHandler::SetHeader(const FString& HeaderName, const FString& HeaderValue)
{
#if WITH_CIVET
	if (bHandlerBinned)
	{
		UE_LOG(LogPwsAll, Error, TEXT("%s: Can't set handler header: it's already binned"), *PS_FUNC_LINE);
		return;
	}

	Impl->SetHeader(HeaderName, HeaderValue);
#endif
}

FString UPsWebServerHandler::GetHeader(const FGuid& RequestId, const FString& HeaderName) const
{
	check(bProcessing);

#if WITH_CIVET
	return Impl->GetHeader(RequestId, HeaderName);
#else
	return FString{};
#endif
}

FString UPsWebServerHandler::GetURI() const
{
	return HandlerURI;
}

FPsWebCancellationTokenRef UPsWebServerHandler::GetCancellationToken() const
{
	check(bProcessing);
	return CancellationTokenPtr.ToSharedRef();
}

FString UPsWebServerHandler::GetAbortAsyncResponse() const
{
	return FString{};
}

FString UPsWebServerHandler::GetTimeoutResponse() const
{
	return FString{};
}

bool UPsWebServerHandler::BindHandler(UPsWebServer* InServer, const FString& URI, CivetServer* ServerImpl)
{
#if WITH_CIVET
	check(InServer);
	check(ServerImpl);
	check(!URI.IsEmpty());

	// Cache ServerWrapper pointer and its URI (it's reqired by UPsWebServerHandler::BeginDestroy())
	HandlerURI = URI;
	Server = InServer;

	// Cache request timeout from config
	const UPsWebServerSettings* ServerSettings = FPsWebServerModule::Get().GetSettings();
	Impl->RequestTimeout = ServerSettings->RequestTimeout;

	// Bind handler to civet server internal instance
	Impl->OwnerHandler = this;
	ServerImpl->addHandler(TCHAR_TO_ANSI(*HandlerURI), Impl.Get());

	// Set as binned
	bHandlerBinned = true;

	return true;
#else
	UE_LOG(LogPwsAll, Error, TEXT("%s: Civet is not supported"), *PS_FUNC_LINE);

	return false;
#endif // WITH_CIVET
}

void UPsWebServerHandler::SetRequestOnNextTick(const FGuid& RequestId, FString RequestData, const FPsWebCancellationTokenRef& InCancellationToken)
{
	UE_LOG(LogPwsAll, Verbose, TEXT("%s: request id %s"), *PS_FUNC_LINE, *RequestId.ToString());

	const auto World = GetWorld();
	check(World);

	ScheduledRequests.Add(RequestId);

	const auto OnNextTick = FTimerDelegate::CreateWeakLambda(this, [this, RequestId, RequestData = MoveTemp(RequestData), InCancellationToken] {
		ScheduledRequests.Remove(RequestId);
		OnRequest(RequestId, RequestData, InCancellationToken);
	});

	auto& TimerManager = World->GetTimerManager();
	TimerManager.SetTimerForNextTick(OnNextTick);
}

void UPsWebServerHandler::OnRequest(const FGuid& RequestId, const FString& RequestData, const FPsWebCancellationTokenRef& InCancellationToken)
{
	UE_LOG(LogPwsAll, Verbose, TEXT("%s: request id '%s'"), *PS_FUNC_LINE, *RequestId.ToString());

	check(!bProcessing);
	bProcessing = true;
	bAborted = false;
	CancellationTokenPtr = InCancellationToken;

	if (CancellationTokenPtr->IsCanceled())
	{
		UE_LOG(LogPwsAll, Error, TEXT("%s: request was already canceled"), *PS_FUNC_LINE);
		ProcessRequestFinish(RequestId, FString{});
		return;
	}

	ProcessRequest(RequestId, RequestData);

	if (IsProcessing())
	{
		UE_LOG(LogPwsAll, Error, TEXT("%s: async handlers is not allowed"), *PS_FUNC_LINE);
		AbortAsyncRequest(RequestId);
	}
}

void UPsWebServerHandler::AbortAsyncRequest(const FGuid& RequestId)
{
	UE_LOG(LogPwsAll, Warning, TEXT("%s: request id %s"), *PS_FUNC_LINE, *RequestId.ToString());

	check(!bAborted);
	check(bProcessing);

	const auto Response = GetAbortAsyncResponse();
	ProcessRequestFinish(RequestId, Response);

	bAborted = true;
}

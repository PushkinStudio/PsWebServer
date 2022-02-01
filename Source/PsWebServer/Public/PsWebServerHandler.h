// Copyright 2015-2022 MY.GAMES. All Rights Reserved.

#pragma once

#include "PsWebServerCancellationToken.h"

#include "CoreMinimal.h"

#include "PsWebServerHandler.generated.h"

class UPsWebServer;
class CivetServer;
class FPsWebServerHandlerImpl;

/** Handler of POST requests on the specific URI */
UCLASS(Blueprintable, BlueprintType)
class PSWEBSERVER_API UPsWebServerHandler : public UObject
{
	GENERATED_BODY()

public:
	UPsWebServerHandler();

	//~ Begin UObject Interface
	virtual void BeginDestroy() override;
	//~ End UObject Interface

	/** Override it with custom processing logic */
	UFUNCTION(BlueprintNativeEvent, Category = "PsWebServer|Handler")
	void ProcessRequest(const FGuid& RequestId, const FString& RequestData);

	/** Finish request processing with given response data */
	UFUNCTION(BlueprintCallable, Category = "PsWebServer|Handler")
	void ProcessRequestFinish(const FGuid& RequestId, const FString& ResponseData);

	/** Whether the current request is cancelled */
	UFUNCTION(BlueprintCallable, Category = "PsWebServer|Handler")
	bool IsCancelled() const;

	/** Whether the request is in processing */
	UFUNCTION(BlueprintPure, Category = "PsWebServer|Handler")
	bool IsProcessing() const;

	/** Whether the current request is aborted */
	UFUNCTION(BlueprintPure, Category = "PsWebServer|Handler")
	bool IsAborted() const;

	/** Sets optional header info (should be global for handler) */
	UFUNCTION(BlueprintCallable, Category = "PsWebServer|Handler")
	void SetHeader(const FString& HeaderName, const FString& HeaderValue);

	/** Get header value for given request id */
	UFUNCTION(BlueprintCallable, Category = "PsWebServer|Handler")
	FString GetHeader(const FGuid& RequestId, const FString& HeaderName) const;

	/** Get handler URI value */
	UFUNCTION(BlueprintPure, Category = "PsWebServer")
	FString GetURI() const;

protected:
	/** Get cancellation token */
	FPsWebCancellationTokenRef GetCancellationToken() const;

	/** Get error response in case of async handler */
	virtual FString GetAbortAsyncResponse() const;

	/** (Called not from the game thread!) Get error response in case of timeout */
	virtual FString GetTimeoutResponse() const;

private:
	friend UPsWebServer;

	/** Called from wrapped to bind handler to server */
	bool BindHandler(UPsWebServer* Server, const FString& URI, CivetServer* ServerImpl);

	/** Cached handler URI */
	FString HandlerURI;

	/** Weak pointer to server used for handler binding */
	TWeakObjectPtr<UPsWebServer> Server;

	/** Internal binned status */
	bool bHandlerBinned;

	/** Processing in progress flag */
	bool bProcessing;

	/** Aborted flag */
	bool bAborted;

	/** Cancellation token */
	FPsWebCancellationTokenPtr CancellationTokenPtr;

#if WITH_CIVET
	/** Pointer to the internal civet-based handler type */
	using FPimpl = TSharedPtr<FPsWebServerHandlerImpl>;

	/** Pointer to the internal civet-based handler instance for uri */
	FPimpl Impl;

	friend FPsWebServerHandlerImpl;
#endif // WITH_CIVET

	/** Set request processing on the next tick */
	void SetRequestOnNextTick(const FGuid& RequestId, FString RequestData, const FPsWebCancellationTokenRef& CancellationToken);

	/** Start request processing */
	void OnRequest(const FGuid& RequestId, const FString& RequestData, const FPsWebCancellationTokenRef& CancellationToken);

	/** Abort async request processing */
	void AbortAsyncRequest(const FGuid& RequestId);

	/** Scheduled requests */
	TSet<FGuid> ScheduledRequests;
};

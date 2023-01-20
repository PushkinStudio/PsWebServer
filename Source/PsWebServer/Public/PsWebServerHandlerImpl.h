// Copyright 2015-2023 MY.GAMES. All Rights Reserved.

#pragma once

#if !WITH_CIVET
#error "Civet is not supported"
#endif

#include "PsWebServerCancellationToken.h"

#include "CivetServer.h"

#include "CoreMinimal.h"
#include "HAL/CriticalSection.h"
#include "HAL/Event.h"
#include "Misc/Guid.h"
#include "Templates/Atomic.h"
#include "Templates/SharedPointer.h"

class UPsWebServerHandler;

/**
 * Native C++ wrapper to connect civet and ue4
 */
class FPsWebServerHandlerImpl : public CivetHandler, public TSharedFromThis<FPsWebServerHandlerImpl>
{
public:
	FPsWebServerHandlerImpl();

	/** Handle POST requests */
	virtual bool handlePost(CivetServer* Server, mg_connection* RequestConnection) override;

	/** Processing finish callback */
	void ProcessRequestFinish(const FGuid& RequestUniqueId, const FString& ResponseData);

	/** Set common header for all responses */
	void SetHeader(const FString& HeaderName, const FString& HeaderValue);

	/** Get header for given request id */
	FString GetHeader(const FGuid& RequestId, const FString& HeaderName) const;

	/** Timeout in msec for the entire http request to complete (see UPsWebServerSettings::RequestTimeout) */
	int32 RequestTimeout;

	/** Parent processing object that lives in GameThread */
	TAtomic<UPsWebServerHandler*> OwnerHandler;

private:
	/** Create a new request context */
	FEvent* CreateContext(mg_connection* RequestConnection, const FGuid& RequestId, FPsWebCancellationSource CancellationSource);

	/** Wait the response from the request processing or timeout */
	bool WaitForResponse(mg_connection* RequestConnection, FEvent* RequestReadyEvent, const FGuid& RequestId);

	/** Get response data for the given request id */
	FString GetResponseData(const FGuid& RequestId, bool bTimeout);

	/** Send given response with status code to the request connection */
	bool SendResponse(mg_connection* RequestConnection, const FGuid& RequestId, const FString& StatusCode, const FString& ResponseData);

	/** Write given data string into the request connection */
	bool Write(mg_connection* RequestConnection, const FString& Data);

	/** Write given data string into the request connection using chunks */
	bool WriteChunked(mg_connection* RequestConnection, const FString& Data);

	/** Critical section used to lock the request data for access */
	mutable FCriticalSection CriticalSection;

	/** Request context */
	struct FContext
	{
		mg_connection* Connection;
		bool bFinished = false;
		FString Response;
		FEvent* ReadyEvent;
		FPsWebCancellationSource CancellationSource;

		FContext(mg_connection* InConnection, FEvent* InReadyEvent)
			: Connection(InConnection)
			, ReadyEvent(InReadyEvent)
		{
			check(InConnection);
			check(InReadyEvent);
		}
	};

	/** Processed request contexts map */
	TMap<FGuid, FContext> Contexts;

	/** Request headers that will be added to the response */
	TMap<FString, FString> ResponseHeaders;

	/** Request headers cached as a string */
	FString CachedResponseHeaders;

	/** Print given headers into the single string */
	FString PrintHeadersToString(const TMap<FString, FString>& Headers);
};

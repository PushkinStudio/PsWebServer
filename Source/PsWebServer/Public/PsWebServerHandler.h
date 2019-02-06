// Copyright 2015-2019 Mail.Ru Group. All Rights Reserved.

#pragma once

#include "civetweb/include/CivetServer.h"

#include "CoreMinimal.h"

#include "PsWebServerHandler.generated.h"

class UPsWebServerHandler;
class UPsWebServerWrapper;

/**
 * Simple struct to share request result between threads 
 */
struct FRequestResult
{
public:
	FString ReplyCode;
	FString ContentType;
	FString Data;

	FRequestResult()
	{
		ReplyCode = FString("200 OK");
		ContentType = "application/json";
	}
};

/**
 * Native C++ wrapper to connect civet and ue4 
 */
class WebServerHandler : public CivetHandler
{
public:
	WebServerHandler();

	bool handlePost(CivetServer* server, struct mg_connection* conn);
	bool handleGet(CivetServer* server, struct mg_connection* conn);

public:
	/** Timeout in msec for the entire http request to complete (see UPsWebServerSettings::RequestTimeout)*/
	int32 RequestTimeout;

	/** Parent processing object that lives in GameThread */
	TWeakObjectPtr<UPsWebServerHandler> OwnerHandler;
};

UCLASS(Blueprintable, BlueprintType)
class PSWEBSERVER_API UPsWebServerHandler : public UObject
{
	GENERATED_UCLASS_BODY()

public:
	//~ Begin UObject Interface
	virtual void BeginDestroy() override;
	//~ End UObject Interface

	UFUNCTION(BlueprintCallable, Category = "PsWebServer|Handler")
	bool BindHandler(UPsWebServerWrapper* ServerWrapper, const FString& URI);

	UFUNCTION(BlueprintNativeEvent, Category = "PsWebServer|Handler")
	void ProcessRequest(const FString& RequestData);

public:
	/** Stores result after request processing */
	TSharedPtr<FRequestResult, ESPMode::ThreadSafe> RequestResult;

private:
	/** Interlal civet-based handler for uri */
	WebServerHandler Handler;

	/** Cached handler URI */
	FString HandlerURI;

	/** Weak pointer to server used for handler binding */
	TWeakObjectPtr<UPsWebServerWrapper> Wrapper;
};

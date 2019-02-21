// Copyright 2015-2019 Mail.Ru Group. All Rights Reserved.

#pragma once

#if WITH_CIVET
#include "CivetServer.h"
#endif

#include "CoreMinimal.h"

#include "PsWebServerHandler.generated.h"

class UPsWebServerHandler;
class UPsWebServerWrapper;

#if WITH_CIVET

/**
 * Native C++ wrapper to connect civet and ue4 
 */
class PSWEBSERVER_API WebServerHandler : public CivetHandler
{
public:
	WebServerHandler();

	bool handlePost(CivetServer* server, struct mg_connection* conn);
	bool handleGet(CivetServer* server, struct mg_connection* conn);

	void ProcessRequest(const FGuid& RequestUniqueId, const FString& RequestData);
	bool SetResponseData(const FGuid& RequestUniqueId, const FString& ResponseData);

public:
	/** Timeout in msec for the entire http request to complete (see UPsWebServerSettings::RequestTimeout)*/
	TAtomic<int32> RequestTimeout;

	/** Parent processing object that lives in GameThread */
	TWeakObjectPtr<UPsWebServerHandler> OwnerHandler;

private:
	/** Critical section used to lock the request data for access */
	FCriticalSection CriticalSection;

	/** Internal container for cached response data from game thread */
	TMap<FGuid, FString> ResponseDatas;

	/** Internal container for cached event triggers */
	TMap<FGuid, FEvent*> RequestReadyEvents;
};

#endif // WITH_CIVET

UCLASS(Blueprintable, BlueprintType)
class PSWEBSERVER_API UPsWebServerHandler : public UObject
{
	GENERATED_BODY()

public:
	//~ Begin UObject Interface
	virtual void BeginDestroy() override;
	//~ End UObject Interface

	/** Override it with custom processing logic */
	UFUNCTION(BlueprintNativeEvent, Category = "PsWebServer|Handler")
	void ProcessRequest(const FGuid& RequestUniqueId, const FString& RequestData);

	/** Override it if you want to have any data validation level */
	UFUNCTION(BlueprintNativeEvent, Category = "PsWebServer|Handler")
	bool SetResponseData(const FGuid& RequestUniqueId, const FString& ResponseData);

	/** Get HandlerURI value */
	UFUNCTION(BlueprintCallable, Category = "PsWebServer")
	FString GetURI() const;

	/** Called from wrapped to bind handler to server */
	bool BindHandler(UPsWebServerWrapper* ServerWrapper, const FString& URI);

private:
	/** Cached handler URI */
	FString HandlerURI;

	/** Weak pointer to server used for handler binding */
	TWeakObjectPtr<UPsWebServerWrapper> Wrapper;

#if WITH_CIVET
private:
	/** Interlal civet-based handler for uri */
	WebServerHandler Handler;
#endif // WITH_CIVET
};

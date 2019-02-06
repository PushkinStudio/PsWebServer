// Copyright 2015-2019 Mail.Ru Group. All Rights Reserved.

#pragma once

#include "civetweb/include/CivetServer.h"

#include "CoreMinimal.h"

#include "PsWebServerHandler.generated.h"

class UPsWebServerHandler;
class UPsWebServerWrapper;

/**
 * Native C++ wrapper to connect civet and ue4 
 */
class PSWEBSERVER_API WebServerHandler : public CivetHandler
{
public:
	WebServerHandler();

	bool handlePost(CivetServer* server, struct mg_connection* conn);
	bool handleGet(CivetServer* server, struct mg_connection* conn);

	/** Called from game thread to fill the data */
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
	void ProcessRequest(const FGuid& RequestUniqueId, const FString& RequestData);

	/** Override it if you want to have any data validation level */
	UFUNCTION(BlueprintNativeEvent, Category = "PsWebServer|Handler")
	bool SetResponseData(const FGuid& RequestUniqueId, const FString& ResponseData);

protected:
	/** Interlal civet-based handler for uri */
	WebServerHandler Handler;

private:
	/** Cached handler URI */
	FString HandlerURI;

	/** Weak pointer to server used for handler binding */
	TWeakObjectPtr<UPsWebServerWrapper> Wrapper;
};

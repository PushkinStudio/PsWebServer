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

UCLASS()
class UPsWebServerHandler : public UObject
{
	GENERATED_UCLASS_BODY()

public:
	//~ Begin UObject Interface
	virtual void BeginDestroy() override;
	//~ End UObject Interface

	bool BindHandler(UPsWebServerWrapper* ServerWrapper, const FString& URI);

	virtual void ProcessPost();

private:
	/** Interlal civet-based handler for uri */
	WebServerHandler Handler;

	/** Cached handler URI */
	FString HandlerURI;

	/** Weak pointer to server used for handler binding */
	TWeakObjectPtr<UPsWebServerWrapper> Wrapper;
};

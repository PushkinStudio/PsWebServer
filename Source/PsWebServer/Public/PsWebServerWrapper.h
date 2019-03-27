// Copyright 2015-2019 Mail.Ru Group. All Rights Reserved.

#pragma once

#include "CoreUObject.h"
#include "TimerManager.h"

#include "PsWebServerWrapper.generated.h"

#if WITH_CIVET
#include "CivetServer.h"
#endif

class UPsWebServerHandler;

/**
 * CivetWeb server wrapper
 *
 * Based on https://github.com/civetweb/civetweb/commit/b21ca4a0a6b54c1ee1223be1f6a823640f1b2c50 commit
 */
UCLASS(BlueprintType)
class PSWEBSERVER_API UPsWebServerWrapper : public UObject
{
	GENERATED_UCLASS_BODY()

	//~ Begin UObject Interface
	virtual void BeginDestroy() override;
	//~ End UObject Interface

public:
	/** Start server from civetweb examples */
	UFUNCTION(BlueprintCallable, Category = "PsWebServer|Test")
	void StartExampleServer();

	/** Start web server */
	UFUNCTION(BlueprintCallable, Category = "PsWebServer")
	void StartServer();

	/** Stop web server and unbind all handlers */
	UFUNCTION(BlueprintCallable, Category = "PsWebServer")
	void StopServer();

	/** Add new handler to process URI */
	UFUNCTION(BlueprintCallable, Category = "PsWebServer")
	bool AddHandler(UPsWebServerHandler* Handler, const FString& URI);

	/** Remove handler from URI */
	UFUNCTION(BlueprintCallable, Category = "PsWebServer")
	bool RemoveHandler(const FString& URI);

protected:
	virtual void ForceGCTimer();

protected:
	FTimerHandle TimerHandle_ForceGCTimer;

private:
	/** All added handlers <URI, Handler> */
	UPROPERTY()
	TMap<FString, UPsWebServerHandler*> BinnedHandlers;

#if WITH_CIVET
private:
	/** Native CivetWeb C++ wrapper */
	TUniquePtr<CivetServer> Server;
	friend class UPsWebServerHandler;
#endif
};

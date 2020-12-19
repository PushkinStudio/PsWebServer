// Copyright 2015-2020 Mail.Ru Group. All Rights Reserved.

#pragma once

#include "CoreUObject.h"
#include "Engine/EngineTypes.h"

#include "PsWebServer.generated.h"

class UPsWebServerHandler;
class CivetServer;

/**
 * CivetWeb server wrapper
 *
 * Based on https://github.com/civetweb/civetweb/commit/b21ca4a0a6b54c1ee1223be1f6a823640f1b2c50 commit
 */
UCLASS(BlueprintType)
class PSWEBSERVER_API UPsWebServer : public UObject
{
	GENERATED_BODY()

	//~ Begin UObject Interface
	virtual void BeginDestroy() override;
	//~ End UObject Interface

public:
	/** Start web server */
	UFUNCTION(BlueprintCallable, Category = "PsWebServer")
	void StartServer();

	/** Stop web server and unbind all handlers */
	UFUNCTION(BlueprintCallable, Category = "PsWebServer")
	void StopServer();

	/** Whether the server is running */
	UFUNCTION(BlueprintPure, Category = "PsWebServer")
	bool IsRunning() const;

	/** Add new handler to process URI */
	UFUNCTION(BlueprintCallable, Category = "PsWebServer")
	bool AddHandler(UPsWebServerHandler* Handler, const FString& URI);

	/** Remove handler from URI */
	UFUNCTION(BlueprintCallable, Category = "PsWebServer")
	bool RemoveHandler(const FString& URI);

protected:
	/** Force gc timer callback */
	virtual void ForceGCTimer();

	/** Force gc timer handle */
	FTimerHandle TimerHandle_ForceGCTimer;

private:
	/** All added handlers <URI, Handler> */
	UPROPERTY()
	TMap<FString, UPsWebServerHandler*> BinnedHandlers;

#if WITH_CIVET
private:
	/** Pointer to the native CivetWeb implemetation type */
	struct CivetServerDeleter
	{
		void operator()(CivetServer*) const;
	};
	using FPimpl = TUniquePtr<CivetServer, CivetServerDeleter>;

	/** Pointer to the native CivetWeb implemetation instance */
	FPimpl Impl;
#endif
};

// Copyright 2015-2019 Mail.Ru Group. All Rights Reserved.

#pragma once

#include "CoreUObject.h"

#include "PsWebServerWrapper.generated.h"

class CivetServer;

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

	/** Internal server getter */
	CivetServer* GetServer();

private:
	/** Native CivetWeb C++ wrapper */
	CivetServer* Server;
};

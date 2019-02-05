// Copyright 2015-2019 Mail.Ru Group. All Rights Reserved.

#pragma once

#include "CoreUObject.h"

#include "civetweb/include/CivetServer.h"
#include "civetweb/include/civetweb.h"

#include "PsWebServerWrapper.generated.h"

/**
 * CivetWeb server wrapper
 *
 * Based on https://github.com/civetweb/civetweb/commit/b21ca4a0a6b54c1ee1223be1f6a823640f1b2c50 commit
 */
UCLASS()
class PSWEBSERVER_API UPsWebServerWrapper : public UObject
{
	GENERATED_UCLASS_BODY()

public:
	/** Stop web server and unbind all handlers */
	UFUNCTION(BlueprintCallable, Category = "PsCivetWebServer")
	void StopServer();

private:
	/** Native CivetWeb C++ wrapper */
	CivetServer* Server;
};

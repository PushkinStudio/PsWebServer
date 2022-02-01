// Copyright 2015-2022 MY.GAMES. All Rights Reserved.

#pragma once

#include "PsWebServerSettings.generated.h"

UCLASS(config = Engine, defaultconfig)
class UPsWebServerSettings : public UObject
{
	GENERATED_UCLASS_BODY()

	/** Reads settings from config before the UObject system is setup to do this by itself */
	void Load();

	/** Server address to run on. If ServerPort set to 0, ServerAddress can contain string with port (IP:Port) */
	UPROPERTY(Config, EditAnywhere, Category = "Server Address")
	FString ServerAddress;

	/** Server port to run on. If set to 0, ServerAddress can contain string with port (IP:Port) */
	UPROPERTY(Config, EditAnywhere, Category = "Server Address", meta = (ClampMin = 0, ClampMax = 65535))
	int32 ServerPort;

	/** If enabled, server will allow to reuse TCP connections. */
	UPROPERTY(Config, EditAnywhere, Category = "Server Settings")
	bool bEnableKeepAlive;

	/** Keep alive timeout between requests (msec). Used if EnableKeepAlive = true */
	UPROPERTY(Config, EditAnywhere, Category = "Server Settings", meta = (EditCondition = "bEnableKeepAlive", ClampMin = 500, ClampMax = 60000))
	int32 KeepAliveTimeout;

	/** Timeout in msec for the entire http request to complete. */
	UPROPERTY(Config, EditAnywhere, Category = "Server Settings", meta = (ClampMin = 0, ClampMax = 60000))
	int32 RequestTimeout;

	/** Number of worker threads. CivetWeb handles each incoming connection in a separate thread. Therefore,
	 * the value of this option is effectively the number of concurrent HTTP connections CivetWeb can handle. */
	UPROPERTY(Config, EditAnywhere, Category = "Server Settings", meta = (ClampMin = 1, ClampMax = 250))
	int32 NumTreads;

	/** Force garbage collection time (msec). If == 0 gc won't be triggered manually. */
	UPROPERTY(Config, EditAnywhere, Category = "Server Settings", meta = (ClampMin = 0, ClampMax = 60000))
	int32 ForceGCTime;
};

// Copyright 2015-2019 Mail.Ru Group. All Rights Reserved.

#pragma once

#include "PsWebServerSettings.generated.h"

UCLASS(config = Engine, defaultconfig)
class UPsWebServerSettings : public UObject
{
	GENERATED_UCLASS_BODY()

	/** Reads settings from config before the UObject system is setup to do this by itself */
	void Load();

	/** Server address to run on. If ServerPort set to 0, ServerAddress can contain string with port (IP:Port) */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Server Address")
	FString ServerAddress;

	/** Server port to run on. If set to 0, ServerAddress can contain string with port (IP:Port) */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Server Address", meta = (ClampMin = 0, ClampMax = 65535))
	int32 ServerPort;

	/** If enabled, server will allow to reuse TCP connections. */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Server Settings")
	bool bEnableKeepAlive;

	/** Keep alive timeout between requests (msec). Used if EnableKeepAlive = true */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Server Settings", meta = (EditCondition = "bEnableKeepAlive", ClampMin = 500, ClampMax = 60000))
	int32 KeepAliveTimeout;

	/** Timeout in msec for the entire http request to complete. */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Server Settings", meta = (ClampMin = 0, ClampMax = 5000))
	int32 RequestTimeout;
};

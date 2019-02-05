// Copyright 2015-2019 Mail.Ru Group. All Rights Reserved.

#include "PsWebServerSettings.h"

#include "PsWebServerDefines.h"

#include "Misc/CommandLine.h"
#include "Misc/ConfigCacheIni.h"

UPsWebServerSettings::UPsWebServerSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ServerAddress = FString("localhost");
	ServerPort = 2450;

	bEnableKeepAlive = false;
	KeepAliveTimeout = 5000;
	RequestTimeout = 1000;
}

void UPsWebServerSettings::Load()
{
	const FString ConfigSection = FString("/Script/PsWebServer.PsWebServerSettings");

	if (!FParse::Value(FCommandLine::Get(), TEXT("ServerAddress"), ServerAddress) && ServerAddress.Len() == 0)
	{
		GConfig->GetString(*ConfigSection, TEXT("ServerAddress"), ServerAddress, GEngineIni);
	}

	if (!FParse::Value(FCommandLine::Get(), TEXT("ServerPort"), ServerPort))
	{
		GConfig->GetInt(*ConfigSection, TEXT("ServerPort"), ServerPort, GEngineIni);
	}

	if (!FParse::Bool(FCommandLine::Get(), TEXT("bEnableKeepAlive"), bEnableKeepAlive))
	{
		GConfig->GetBool(*ConfigSection, TEXT("bEnableKeepAlive"), bEnableKeepAlive, GEngineIni);
	}

	if (!FParse::Value(FCommandLine::Get(), TEXT("KeepAliveTimeout"), KeepAliveTimeout))
	{
		GConfig->GetInt(*ConfigSection, TEXT("KeepAliveTimeout"), KeepAliveTimeout, GEngineIni);
	}

	if (!FParse::Value(FCommandLine::Get(), TEXT("RequestTimeout"), RequestTimeout))
	{
		GConfig->GetInt(*ConfigSection, TEXT("RequestTimeout"), RequestTimeout, GEngineIni);
	}
}

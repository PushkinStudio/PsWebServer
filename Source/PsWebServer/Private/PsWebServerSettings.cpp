// Copyright 2015-2023 MY.GAMES. All Rights Reserved.

#include "PsWebServerSettings.h"

#include "Misc/CommandLine.h"
#include "Misc/ConfigCacheIni.h"

UPsWebServerSettings::UPsWebServerSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ServerAddress = FString("127.0.0.1");
	ServerPort = 2050;

	bEnableKeepAlive = false;
	KeepAliveTimeout = 5000;
	RequestTimeout = 1000;

	NumTreads = 50;

	ForceGCTime = 5000;
}

void UPsWebServerSettings::Load()
{
	const FString ConfigSection = FString("/Script/PsWebServer.PsWebServerSettings");

	if (!FParse::Value(FCommandLine::Get(), TEXT("ServerAddress="), ServerAddress) && ServerAddress.Len() == 0)
	{
		GConfig->GetString(*ConfigSection, TEXT("ServerAddress"), ServerAddress, GEngineIni);
	}

	if (!FParse::Value(FCommandLine::Get(), TEXT("ServerPort="), ServerPort))
	{
		GConfig->GetInt(*ConfigSection, TEXT("ServerPort"), ServerPort, GEngineIni);
	}

	if (!FParse::Bool(FCommandLine::Get(), TEXT("bEnableKeepAlive="), bEnableKeepAlive))
	{
		GConfig->GetBool(*ConfigSection, TEXT("bEnableKeepAlive"), bEnableKeepAlive, GEngineIni);
	}

	if (!FParse::Value(FCommandLine::Get(), TEXT("KeepAliveTimeout="), KeepAliveTimeout))
	{
		GConfig->GetInt(*ConfigSection, TEXT("KeepAliveTimeout"), KeepAliveTimeout, GEngineIni);
	}

	if (!FParse::Value(FCommandLine::Get(), TEXT("RequestTimeout="), RequestTimeout))
	{
		GConfig->GetInt(*ConfigSection, TEXT("RequestTimeout"), RequestTimeout, GEngineIni);
	}

	if (!FParse::Value(FCommandLine::Get(), TEXT("NumTreads="), NumTreads))
	{
		GConfig->GetInt(*ConfigSection, TEXT("NumTreads"), NumTreads, GEngineIni);
	}

	if (!FParse::Value(FCommandLine::Get(), TEXT("ForceGCTime="), ForceGCTime))
	{
		GConfig->GetInt(*ConfigSection, TEXT("ForceGCTime"), ForceGCTime, GEngineIni);
	}
}

// Copyright 2015-2021 Mail.Ru Group. All Rights Reserved.

#include "PsWebServerLibrary.h"

#include "PsWebServer.h"
#include "PsWebServerDefines.h"

UPsWebServer* UPsWebServerLibrary::ConstructWebServer(UObject* WorldContextObject)
{
	// Slowly check we have existing instance to prevent multiple server instances
	for (TObjectIterator<UPsWebServer> It; It; ++It)
	{
		UPsWebServer* Current = *It;
		if (Current->IsValidLowLevel())
		{
			UE_LOG(LogPwsAll, Error, TEXT("%s: Existing server wrapper is found. Please check you're using it right."), *PS_FUNC_LINE);
			return Current;
		}
	}

	return NewObject<UPsWebServer>(WorldContextObject);
}

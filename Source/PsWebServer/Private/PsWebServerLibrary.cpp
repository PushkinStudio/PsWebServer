// Copyright 2015-2019 Mail.Ru Group. All Rights Reserved.

#include "PsWebServerLibrary.h"

#include "PsWebServerDefines.h"
#include "PsWebServerWrapper.h"

UPsWebServerLibrary::UPsWebServerLibrary(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UPsWebServerWrapper* UPsWebServerLibrary::ConstructWebServerWrapper()
{
	// Slowly check we have existing instance to prevent multiple server instances
	for (TObjectIterator<UPsWebServerWrapper> It; It; ++It)
	{
		UPsWebServerWrapper* CurrentWrapper = *It;
		if (CurrentWrapper->IsValidLowLevel())
		{
			UE_LOG(LogPwsAll, Error, TEXT("%s: Existing server wrapper is found. Please check you're using it right."), *PS_FUNC_LINE);
			return CurrentWrapper;
		}
	}

	return NewObject<UPsWebServerWrapper>();
}

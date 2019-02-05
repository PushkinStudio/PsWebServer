// Copyright 2015-2019 Mail.Ru Group. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"

#include "PsWebServerLibrary.generated.h"

class UPsWebServerWrapper;

UCLASS()
class UPsWebServerLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_UCLASS_BODY()

	/** Create new web server wrapper */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Create Web Server Wrapper"), Category = "PsWebServer")
	static UPsWebServerWrapper* ConstructWebServerWrapper();
};

// Copyright 2015-2019 Mail.Ru Group. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"

#include "PsWebServerLibrary.generated.h"

class UPsWebServerWrapper;

UCLASS()
class UPsWebServerLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_UCLASS_BODY()

	/** Web server wrapper accessor */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PsWebServer")
	static UPsWebServerWrapper* GetWebServer();

};

// Copyright 2015-2019 Mail.Ru Group. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"

#include "PsWebServerLibrary.generated.h"

class UPsWebServer;

UCLASS()
class PSWEBSERVER_API UPsWebServerLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Create a new web server instance */
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject", DisplayName = "Create Web Server"), Category = "PsWebServer")
	static UPsWebServer* ConstructWebServer(UObject* WorldContextObject);
};

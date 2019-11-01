# PsWebServer

[![statusIcon](https://teamcity.ufna.dev/app/rest/builds/buildType:(id:Pushkin_PsWebServer_ClangFormatCheck)/statusIcon.svg)](https://teamcity.ufna.dev/viewType.html?buildTypeId=Pushkin_PsWebServer_ClangFormatCheck&guest=1)

Civet web server integration plugin for Unreal Engine 4

# HowTo

Web server usage (f.e. in AMyGameMode::BeginPlay()))

```cpp
// Launch web server
WebServer = NewObject<UPsWebServerWrapper>(this);
WebServer->StartServer();

// Create and register api handler
auto PingHandler = NewObject<UMyServerPingHandler>(this);
PingHandler->SetHeader(TEXT("Server"), TEXT("MyServer/") + MyGI->GetGameVersion());     // Optional header set
WebServer->AddHandler(PingHandler, TEXT("/api/ping"));
```

If uses `UMyServerPingHandler` class that defined as:

```cpp
#pragma once

#include "PsWebServerHandler.h"

#include "MyServerPingHandler.generated.h"

UCLASS()
class UMyServerPingHandler : public UPsWebServerHandler
{
	GENERATED_BODY()

public:
	/** Override to implement your custom logic of request processing */
	virtual void ProcessRequest_Implementation(const FGuid& RequestUniqueId, const FString& RequestData) override;
};

```

And its definition:

```cpp
#include "MyServerPingHandler.h"

#include "PsWebServerDefines.h"
#include "VaRestJsonObject.h"

/*
 * Check that request has valid json encoded body and return its copy in response
 */
void UMyServerPingHandler::ProcessRequest_Implementation(const FGuid& RequestUniqueId, const FString& RequestData)
{
	// Validate json format with VaRest
	UVaRestJsonObject* JsonTemp = UVaRestJsonObject::ConstructJsonObject(this);
	if (JsonTemp->DecodeJson(RequestData))
	{
		ProcessRequestFinish(RequestUniqueId, FString::Printf(TEXT("{\"request_data\":%s}"), *RequestData));
		return;
	}

	UE_LOG(LogMyGame, Error, TEXT("%s: can't validate data as json one: %s"), *PS_FUNC_LINE, *RequestData);

	const FString ErrorStr = TEXT(R"({"error":"1000","message":"Request data is not a valid json object"})");
	ProcessRequestFinish(RequestUniqueId, ErrorStr);
}
```

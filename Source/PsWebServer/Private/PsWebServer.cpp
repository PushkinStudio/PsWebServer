// Copyright 2015-2020 Mail.Ru Group. All Rights Reserved.

#include "PsWebServer.h"

#include "PsWebServerDefines.h"
#include "PsWebServerHandler.h"
#include "PsWebServerPlugin.h"
#include "PsWebServerSettings.h"

#include "Engine/Engine.h"
#include "Engine/World.h"

#if WITH_CIVET
#include "CivetServer.h"

void UPsWebServer::CivetServerDeleter::operator()(CivetServer* Ptr) const
{
	delete Ptr;
}
#endif // WITH_CIVET

void UPsWebServer::BeginDestroy()
{
	StopServer();

	Super::BeginDestroy();
}

void UPsWebServer::StartServer()
{
#if WITH_CIVET
	const UPsWebServerSettings* ServerSettings = FPsWebServerModule::Get().GetSettings();

	FString ServerURL = ServerSettings->ServerAddress;
	if (ServerSettings->ServerPort != 0)
	{
		ServerURL.Append(TEXT(":") + FString::FromInt(ServerSettings->ServerPort));
	}

	std::vector<std::string> Options;
	Options.push_back("listening_ports");
	Options.push_back(TCHAR_TO_ANSI(*ServerURL));
	Options.push_back("num_threads");
	Options.push_back(TCHAR_TO_ANSI(*FString::FromInt(ServerSettings->NumTreads)));

	if (ServerSettings->bEnableKeepAlive)
	{
		Options.push_back("enable_keep_alive");
		Options.push_back("yes");
		Options.push_back("keep_alive_timeout_ms");
		Options.push_back(TCHAR_TO_ANSI(*FString::FromInt(ServerSettings->KeepAliveTimeout)));
	}
	else
	{
		Options.push_back("enable_keep_alive");
		Options.push_back("no");
		Options.push_back("keep_alive_timeout_ms");
		Options.push_back("0");
	}

	// Stop server first if one already exists
	if (IsRunning())
	{
		UE_LOG(LogPwsAll, Warning, TEXT("%s: Existing CivetWeb server instance found, stop it now"), *PS_FUNC_LINE);
		StopServer();
	}

	// Run new civet instance
	Impl = FPimpl{new CivetServer{Options}};

	// Check that we're really running now
	if (Impl->getContext() == nullptr)
	{
		UE_LOG(LogPwsAll, Fatal, TEXT("%s: Failed to run CivetWeb server instance"), *PS_FUNC_LINE);
	}

	UE_LOG(LogPwsAll, Log, TEXT("%s: CivetWeb server instance started on: %s"), *PS_FUNC_LINE, *ServerURL);

	// Setup ForceGC timer
	if (ServerSettings->ForceGCTime > 0)
	{
		if (GetOuter() && GetOuter()->GetWorld())
		{
			GetOuter()->GetWorld()->GetTimerManager().SetTimer(TimerHandle_ForceGCTimer, this, &UPsWebServer::ForceGCTimer, (float)ServerSettings->ForceGCTime / 1000, true);
			UE_LOG(LogPwsAll, Log, TEXT("%s: ForceGC timer started with period: %d (msec)"), *PS_FUNC_LINE, ServerSettings->ForceGCTime);
		}
		else
		{
			UE_LOG(LogPwsAll, Error, TEXT("%s: No valid world found to start ForceGC timer"), *PS_FUNC_LINE);
		}
	}
#else
	UE_LOG(LogPwsAll, Error, TEXT("%s: Civet is not supported"), *PS_FUNC_LINE);
#endif // WITH_CIVET
}

void UPsWebServer::StopServer()
{
#if WITH_CIVET
	if (Impl)
	{
		Impl->close();
		Impl = nullptr;

		UE_LOG(LogPwsAll, Log, TEXT("%s: CivetWeb server instance stopped"), *PS_FUNC_LINE);
	}

	if (GetOuter() && GetOuter()->GetWorld())
	{
		GetOuter()->GetWorld()->GetTimerManager().ClearTimer(TimerHandle_ForceGCTimer);
	}
	else
	{
		UE_LOG(LogPwsAll, Log, TEXT("%s: No valid world found to stop ForceGC timer. It's okay on app shutdown."), *PS_FUNC_LINE);
	}
#endif // WITH_CIVET
}

bool UPsWebServer::IsRunning() const
{
#if WITH_CIVET
	return Impl.IsValid();
#else
	return false;
#endif // WITH_CIVET
}

bool UPsWebServer::AddHandler(UPsWebServerHandler* Handler, const FString& URI)
{
#if WITH_CIVET
	if (URI.IsEmpty())
	{
		UE_LOG(LogPwsAll, Error, TEXT("%s: Can't add hadler: URI is empty"), *PS_FUNC_LINE);
		return false;
	}

	if (!IsRunning())
	{
		UE_LOG(LogPwsAll, Error, TEXT("%s: Can't add hadler: you should run server first"), *PS_FUNC_LINE);
		return false;
	}

	// Remove previous handler if we had one
	RemoveHandler(URI);

	if (Handler->BindHandler(this, URI, Impl.Get()))
	{
		BinnedHandlers.Emplace(URI, Handler);

		UE_LOG(LogPwsAll, Log, TEXT("%s: Handler successfully added for URI: %s"), *PS_FUNC_LINE, *URI);
		return true;
	}
#endif // WITH_CIVET

	return false;
}

bool UPsWebServer::RemoveHandler(const FString& URI)
{
#if WITH_CIVET
	if (Impl)
	{
		if (BinnedHandlers.Contains(URI))
		{
			BinnedHandlers.Remove(URI);
		}

		// Remove handler from civet anyway
		Impl->removeHandler(TCHAR_TO_ANSI(*URI));

		return true;
	}
#endif // WITH_CIVET

	return false;
}

void UPsWebServer::ForceGCTimer()
{
	UE_LOG(LogPwsAll, Log, TEXT("%s: Force garbage collection"), *PS_FUNC_LINE);
	GEngine->ForceGarbageCollection(true);
}

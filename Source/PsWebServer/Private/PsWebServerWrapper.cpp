// Copyright 2015-2019 Mail.Ru Group. All Rights Reserved.

#include "PsWebServerWrapper.h"

#include "PsWebServerDefines.h"
#include "PsWebServerSettings.h"

#include "civetweb/include/CivetServer.h"
#include "civetweb/include/civetweb.h"

// CivetWeb Example
#define DOCUMENT_ROOT "."
#define PORT "2050"
#define EXAMPLE_URI "/example"
#define EXIT_URI "/exit"

class ExampleHandler : public CivetHandler
{
public:
	bool
	handleGet(CivetServer* server, struct mg_connection* conn)
	{
		mg_printf(conn,
			"HTTP/1.1 200 OK\r\nContent-Type: "
			"text/html\r\nConnection: close\r\n\r\n");
		mg_printf(conn, "<html><body>\r\n");
		mg_printf(conn,
			"<h2>This is an example text from a C++ handler</h2>\r\n");
		mg_printf(conn,
			"<p>To see a page from the A handler <a "
			"href=\"a\">click here</a></p>\r\n");
		mg_printf(conn,
			"<form action=\"a\" method=\"get\">"
			"To see a page from the A handler with a parameter "
			"<input type=\"submit\" value=\"click here\" "
			"name=\"param\" \\> (GET)</form>\r\n");
		mg_printf(conn,
			"<form action=\"a\" method=\"post\">"
			"To see a page from the A handler with a parameter "
			"<input type=\"submit\" value=\"click here\" "
			"name=\"param\" \\> (POST)</form>\r\n");
		mg_printf(conn,
			"<p>To see a page from the A/B handler <a "
			"href=\"a/b\">click here</a></p>\r\n");
		mg_printf(conn,
			"<p>To see a page from the *.foo handler <a "
			"href=\"xy.foo\">click here</a></p>\r\n");
		mg_printf(conn,
			"<p>To see a page from the WebSocket handler <a "
			"href=\"ws\">click here</a></p>\r\n");
		mg_printf(conn,
			"<p>To exit <a href=\"%s\">click here</a></p>\r\n",
			EXIT_URI);
		mg_printf(conn, "</body></html>\r\n");
		return true;
	}
};
// CivetWeb Example END

UPsWebServerWrapper::UPsWebServerWrapper(const class FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	Server = nullptr;
}

void UPsWebServerWrapper::BeginDestroy()
{
	StopServer();

	Super::BeginDestroy();
}

void UPsWebServerWrapper::StartExampleServer()
{
	const char* options[] = {"document_root", DOCUMENT_ROOT, "listening_ports", PORT, 0};

	std::vector<std::string> cpp_options;
	for (int i = 0; i < (sizeof(options) / sizeof(options[0]) - 1); i++)
	{
		cpp_options.push_back(options[i]);
	}

	if (Server)
	{
		UE_LOG(LogPwsAll, Error, TEXT("%s: Current server is not null!"), *PS_FUNC_LINE);
		StopServer();
	}

	// CivetServer server(options); // <-- C style start
	Server = new CivetServer(cpp_options); // <-- C++ style start

	static ExampleHandler h_ex;
	Server->addHandler(EXAMPLE_URI, h_ex);

	printf("Run example at http://localhost:%s%s\n", PORT, EXAMPLE_URI);
}

void UPsWebServerWrapper::StartServer()
{
	const UPsWebServerSettings* ServerSettings = GetDefault<UPsWebServerSettings>();
	check(ServerSettings);

	FString ServerURL = ServerSettings->ServerAddress;
	if (ServerSettings->ServerPort != 0)
	{
		ServerURL.Append(TEXT(":") + FString::FromInt(ServerSettings->ServerPort));
	}

	std::vector<std::string> cpp_options;
	cpp_options.push_back("listening_ports");
	cpp_options.push_back(TCHAR_TO_ANSI(*ServerURL));

	if (ServerSettings->bEnableKeepAlive)
	{
		cpp_options.push_back("enable_keep_alive");
		cpp_options.push_back("yes");
		cpp_options.push_back("keep_alive_timeout_ms");
		cpp_options.push_back(TCHAR_TO_ANSI(*FString::FromInt(ServerSettings->KeepAliveTimeout)));
	}
	else
	{
		cpp_options.push_back("enable_keep_alive");
		cpp_options.push_back("no");
		cpp_options.push_back("keep_alive_timeout_ms");
		cpp_options.push_back("0");
	}

	// Stop server first if one already exists
	if (Server)
	{
		UE_LOG(LogPwsAll, Warning, TEXT("%s: Existing CivetWeb server instance found, stop it now"), *PS_FUNC_LINE);
		StopServer();
	}

	// Run new civet instance
	Server = new CivetServer(cpp_options);

	// Check that we're really running now
	if (Server->getContext() == nullptr)
	{
		UE_LOG(LogPwsAll, Fatal, TEXT("%s: Failed to run CivetWeb server instance"), *PS_FUNC_LINE);
	}

	UE_LOG(LogPwsAll, Log, TEXT("%s: CivetWeb server instance started on: %s"), *PS_FUNC_LINE, *ServerURL);
}

void UPsWebServerWrapper::StopServer()
{
	if (Server)
	{
		Server->close();

		delete Server;
		Server = nullptr;

		UE_LOG(LogPwsAll, Log, TEXT("%s: CivetWeb server instance stopped"), *PS_FUNC_LINE);
	}
}

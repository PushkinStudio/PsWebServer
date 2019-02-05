// Copyright 2015-2019 Mail.Ru Group. All Rights Reserved.

#include "PsWebServerWrapper.h"

#include "PsWebServerDefines.h"

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

void UPsWebServerWrapper::StopServer()
{
	if (Server)
	{
		Server->close();

		delete Server;
		Server = nullptr;
	}
}

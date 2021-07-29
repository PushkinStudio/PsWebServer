// Copyright 2015-2021 Mail.Ru Group. All Rights Reserved.

#pragma once

#if !WITH_CIVET
#error "Civet is not supported"
#endif

#include "CivetServer.h"

#include "CoreMinimal.h"

namespace PsWebServerUtils
{

/** Read post data from the connection */
FString GetPostData(mg_connection* RequestConnection);

} // namespace PsWebServerUtils

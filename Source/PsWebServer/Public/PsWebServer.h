// Copyright 2015-2019 Mail.Ru Group. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

class FPsWebServerModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};

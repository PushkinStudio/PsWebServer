// Copyright 2015-2019 Mail.Ru Group. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Logging/LogCategory.h"
#include "Logging/LogMacros.h"
#include "Logging/LogVerbosity.h"

DECLARE_LOG_CATEGORY_EXTERN(LogPwsAll, Log, All);

#define PS_FUNC (FString(__FUNCTION__))				 // Current Class Name + Function Name where this is called
#define PS_LINE (FString::FromInt(__LINE__))		 // Current Line Number in the code where this is called
#define PS_FUNC_LINE (PS_FUNC + "(" + PS_LINE + ")") // Current Class and Line Number where this is called!

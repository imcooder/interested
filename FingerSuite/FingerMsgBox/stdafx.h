// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#pragma once

// Change this value to use different versions
#define WINVER 0x0420

#define _WIN32_WCE_AYGSHELL 1

#include <atlbase.h>
#include <atlapp.h>

extern CAppModule _Module;

#include <atlwin.h>

#include <atlapp.h>
#include <atlmisc.h>

#include "..\Common\fngrbufdc.h"
#include "..\cximage\ximage.h"
#include "..\Common\ext\SimpleIni.h"

#include <aygshell.h>
#pragma comment(lib, "aygshell.lib")


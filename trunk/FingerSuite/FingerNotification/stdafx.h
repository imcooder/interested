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
#include <atlmisc.h>
#include <atlframe.h>
#include <atlscrl.h>
#include <atlctrls.h>
#include <atlwince.h>

#define WM_PRINTCLIENT WM_PAINT 
#include <atlgdix.h>

#include <atldlgs.h>

#include "..\Common\fngrbufdc.h"
#include "..\Common\fngrscrl.h"
#include "..\cximage\ximage.h"
#include "..\Common\fngrbtn.h"


#include "..\Common\ext\SimpleIni.h"

#include "InterceptEngine.h"
#include "Commons.h"

#include "..\Common\log\logger.h"
#include "..\Common\utils.h"


#include <aygshell.h>
#pragma comment(lib, "aygshell.lib")

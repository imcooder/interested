//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this sample source code is subject to the terms of the Microsoft
// license agreement under which you licensed this sample source code. If
// you did not accept the terms of the license agreement, you are not
// authorized to use this sample source code. For the terms of the license,
// please see the license agreement between you and Microsoft or, if applicable,
// see the LICENSE.RTF on your install media or the root of your tools installation.
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES OR INDEMNITIES.
//
// ************************************************************
// setup.h
// 
// Declaring types and functions...equivalent to the standard 
// ce_setup.h.
//
//
// ************************************************************

#pragma once

#ifdef __cplusplus
extern "C" {
#endif // _cplusplus

// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the SETUP_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// SETUP_API functions as being imported from a DLL, wheras this DLL sees symbols
// defined with this macro as being exported.
#ifdef SETUP_EXPORTS
#define SETUP_API __declspec(dllexport)
#else
#define SETUP_API __declspec(dllimport)
#endif

#define REGISTER_WAIT_TIME  60000   // in ms

//
// Install_Init
//
// @comm    Called before any part of the application is installed
//
typedef enum
{
    codeINSTALL_INIT_CONTINUE  = 0,     // @comm Continue with the installation
    codeINSTALL_INIT_CANCEL             // @comm Immediately cancel the installation
}
codeINSTALL_INIT;

SETUP_API codeINSTALL_INIT Install_Init(
    HWND        hwndParent,
    BOOL        fFirstCall,     // is this the first time this function is being called?
    BOOL        fPreviouslyInstalled,
    LPCTSTR     pszInstallDir
);


//
// Install_Exit
//
// @comm    Called after the application is installed
//
typedef enum
{
    codeINSTALL_EXIT_DONE       = 0,    // @comm Exit the installation successfully
    codeINSTALL_EXIT_UNINSTALL          // @comm Uninstall the application before exiting the installation
}
codeINSTALL_EXIT;

SETUP_API codeINSTALL_EXIT Install_Exit(
    HWND    hwndParent,
    LPCTSTR pszInstallDir,      // final install directory
    WORD    cFailedDirs,
    WORD    cFailedFiles,
    WORD    cFailedRegKeys,
    WORD    cFailedRegVals,
    WORD    cFailedShortcuts
);


//
// Uninstall_Init
//
// @comm    Called before the application is uninstalled
//
typedef enum
{
    codeUNINSTALL_INIT_CONTINUE = 0,    // @comm Continue with the uninstallation
    codeUNINSTALL_INIT_CANCEL           // @comm Immediately cancel the uninstallation
}
codeUNINSTALL_INIT;

SETUP_API codeUNINSTALL_INIT Uninstall_Init(
    HWND        hwndParent,
    LPCTSTR     pszInstallDir
);


//
// Uninstall_Exit
//
// @comm    Called after the application is uninstalled
//
typedef enum
{
    codeUNINSTALL_EXIT_DONE     = 0     // @comm Exit the uninstallation successfully
}
codeUNINSTALL_EXIT;

SETUP_API codeUNINSTALL_EXIT Uninstall_Exit(
    HWND    hwndParent
);

#ifdef __cplusplus
};
#endif // _cplusplus

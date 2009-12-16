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
// setup.cpp
// 
// Implementation of DllMain and setup functions
//
//
// ************************************************************

#include "stdafx.h"

#define ARRAYSIZE(x) (sizeof(x)/sizeof((x)[0]))

HINSTANCE g_hinstModule;

// dbg
HWND g_hwndParent;
WCHAR g_szDbg[MAX_PATH];
// fine dbg

DWORD CheckRealDPI(HWND hwndParent)
{
    DWORD nSystemDPI = 0;
    {
        TCHAR szGetRealDPI[MAX_PATH + 40];
        PROCESS_INFORMATION pi;
        
        ::StringCchCat(szGetRealDPI, ARRAYSIZE(szGetRealDPI), _T("\\Application Data\\FingerSuite\\GetRealDPI.EXE")); 
        ::CreateProcess(szGetRealDPI, NULL, NULL, NULL, NULL, 0, NULL, NULL, NULL, &pi);
        ::WaitForSingleObject(pi.hProcess, 10000);
        ::GetExitCodeProcess(pi.hProcess, &nSystemDPI);
        ::CloseHandle(pi.hProcess);
    }

	return nSystemDPI;
}

void DeletePngs(DWORD nSystemDPI, LPCTSTR pszInstallDir, LPCTSTR pszAppName)
{
	WCHAR szSearch[MAX_PATH + 40];
	if (nSystemDPI == 96)
	{
		wsprintf(szSearch, L"%s\\%s\\skins\\default\\*_vga.png", pszInstallDir, pszAppName);
	}
	else if (nSystemDPI == 192)
	{
		wsprintf(szSearch, L"%s\\%s\\skins\\default\\*_qvga.png", pszInstallDir, pszAppName);
	}
	else
		return;

	WIN32_FIND_DATA fd;
	WCHAR szFileName[MAX_PATH + 40];
	HANDLE hFind = FindFirstFile(szSearch, &fd);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			wsprintf(szFileName, L"%s\\%s\\skins\\default\\%s", pszInstallDir, pszAppName, fd.cFileName);
			DeleteFile(szFileName);
		} while (FindNextFile(hFind, &fd));
	}
}

void DeleteMUIs(DWORD nLangID, LPCTSTR pszInstallDir, LPCTSTR pszAppName)
{
	WCHAR szSearch[MAX_PATH + 40];
	wsprintf(szSearch, L"%s\\%s\\%s.exe.%04x.mui", pszInstallDir, pszAppName, pszAppName, nLangID);

	//wsprintf(g_szDbg, L"mui=%s", szSearch);
	//MessageBox(g_hwndParent, g_szDbg, L"", MB_ICONINFORMATION);

	WIN32_FIND_DATA fd;
	HANDLE hFind = FindFirstFile(szSearch, &fd);
	if (hFind == INVALID_HANDLE_VALUE)
	{
		// default to english
		WCHAR szEnglishMUI[MAX_PATH + 40];
		wsprintf(szEnglishMUI, L"%s\\%s\\%s.exe.0409.mui", pszInstallDir, pszAppName, pszAppName);

		CopyFile(szEnglishMUI, szSearch, FALSE);
	}
}


BOOL APIENTRY DllMain(
    HANDLE hModule, 
    DWORD  ul_reason_for_call, 
    LPVOID lpReserved
    )
{
    switch (ul_reason_for_call)
    {
        case DLL_PROCESS_ATTACH:
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
        case DLL_PROCESS_DETACH:
            g_hinstModule = (HINSTANCE)hModule;
            break;
    }
    return TRUE;
}


// **************************************************************************
// Function Name: Install_Init
// 
// Purpose: processes the push message.
//
// Arguments:
//    IN HWND hwndParent  handle to the parent window
//    IN BOOL fFirstCall  indicates that this is the first time this function is being called
//    IN BOOL fPreviouslyInstalled  indicates that the current application is already installed
//    IN LPCTSTR pszInstallDir  name of the user-selected install directory of the application
//
// Return Values:
//    codeINSTALL_INIT
//    returns install status
//
// Description:  
//    The Install_Init function is called before installation begins.
//    User will be prompted to confirm installation.
// **************************************************************************
SETUP_API codeINSTALL_INIT Install_Init(
    HWND        hwndParent,
    BOOL        fFirstCall,     // is this the first time this function is being called?
    BOOL        fPreviouslyInstalled,
    LPCTSTR     pszInstallDir
    )
{
	//WCHAR szKeyName[] = L"Software\\FingerMenu";
	//RegDeleteKey(HKEY_LOCAL_MACHINE, szKeyName);
	return codeINSTALL_INIT_CONTINUE;
}


// **************************************************************************
// Function Name: Install_Exit
// 
// Purpose: processes the push message.
//
// Arguments:
//    IN HWND hwndParent  handle to the parent window
//    IN LPCTSTR pszInstallDir  name of the user-selected install directory of the application
//
// Return Values:
//    codeINSTALL_EXIT
//    returns install status
//
// Description:  
//    Register query client with the PushRouter as part of installation.
//    Only the first two parameters really count.
// **************************************************************************
SETUP_API codeINSTALL_EXIT Install_Exit(
    HWND    hwndParent,
    LPCTSTR pszInstallDir,      // final install directory
    WORD    cFailedDirs,
    WORD    cFailedFiles,
    WORD    cFailedRegKeys,
    WORD    cFailedRegVals,
    WORD    cFailedShortcuts
    )
{
	//WCHAR szStartupFile[] = L"\\Windows\\Startup\\FingerMenu.lnk";
	//if (MessageBox(hwndParent, L"Autostart FingerMenu on boot?", L"Autostart", MB_ICONINFORMATION | MB_YESNO) == IDNO)
	//{
	//	DeleteFile(szStartupFile);
	//}
	g_hwndParent = hwndParent;


	DWORD nSystemDPI = CheckRealDPI(hwndParent);
	
	// delete useless pngs
	WCHAR szProgramFiles[MAX_PATH];
	SHGetSpecialFolderPath(hwndParent, szProgramFiles, CSIDL_PROGRAM_FILES, FALSE);
	DeletePngs(nSystemDPI, szProgramFiles, L"FingerMenu");
	DeletePngs(nSystemDPI, szProgramFiles, L"FingerMsgbox");

	// delete useless languages
	LANGID lLang = GetSystemDefaultUILanguage();
	DeleteMUIs(lLang, szProgramFiles, L"FingerMenu");
	DeleteMUIs(lLang, szProgramFiles, L"FingerMsgbox");

	// TODO set locales
    return codeINSTALL_EXIT_DONE;
}


// **************************************************************************
// Function Name: Uninstall_Init
// 
// Purpose: processes the push message.
//
// Arguments:
//    IN HWND hwndParent  handle to the parent window
//    IN LPCTSTR pszInstallDir  name of the user-selected install directory of the application
//
// Return Values:
//    codeUNINSTALL_INIT
//    returns uninstall status
//
// Description:  
//    Query the device data using the query xml in the push message,
//    and send the query results back to the server.
// **************************************************************************
SETUP_API codeUNINSTALL_INIT Uninstall_Init(
    HWND        hwndParent,
    LPCTSTR     pszInstallDir
    )
{
	// close programs
	HWND hWndDest = ::FindWindow(L"FINGER_MENU", NULL);
	if (hWndDest != NULL)
	{
		::PostMessage(hWndDest, WM_CLOSE, 0, 0);
	}

	hWndDest = ::FindWindow(L"FINGER_MSGBOX", NULL);
	if (hWndDest != NULL)
	{
		::PostMessage(hWndDest, WM_CLOSE, 0, 0);
	}

	return codeUNINSTALL_INIT_CONTINUE;
}


// **************************************************************************
// Function Name: Uninstall_Exit
// 
// Purpose: processes the push message.
//
// Arguments:
//    IN HWND hwndParent  handle to the parent window
//
// Return Values:
//    codeUNINSTALL_EXIT
//    returns uninstall status
//
// Description:  
//    Query the device data using the query xml in the push message,
//    and send the query results back to the server.
// **************************************************************************
SETUP_API codeUNINSTALL_EXIT Uninstall_Exit(
    HWND    hwndParent
    )
{
	WCHAR szStartup[MAX_PATH];
	SHGetSpecialFolderPath(hwndParent, szStartup, CSIDL_STARTUP, FALSE);

	WCHAR szLnk[MAX_PATH + 40];

	wsprintf(szLnk, L"%s\\FingerMenu.lnk", szStartup);
	DeleteFile(szLnk);

	wsprintf(szLnk, L"%s\\FingerMsgbox.lnk", szStartup);
	DeleteFile(szLnk);

    return codeUNINSTALL_EXIT_DONE;
}




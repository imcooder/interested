// FingerSuiteCPL.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "..\Common\atlcplapplet.h"
#include "resource.h"
#include "Commons.h"

static UINT UWM_UPDATECONFIGURATION = ::RegisterWindowMessage(UWM_UPDATECONFIGURATION_MSG);

#include "FingerMenuCPLDlg.h"
#include "AboutDlg.h"



class CFingerSuiteApplet : public CCPlAppletBase<CFingerSuiteApplet>
{
public:
	CFingerMenuCPLDlg1 dlg1;
	CFingerMenuCPLDlg2 dlg2;
	CFingerMenuCPLDlg3 dlg3;
	CFingerMenuCPLDlg4 dlg4;
	CFingerMenuCPLDlg5 dlg5;
	CFingerMenuCPLDlg6 dlg6;
	CFingerMenuCPLDlg7 dlg7;
	CFingerMenuCPLDlg8 dlg8;
	CAboutDlg dlgAbout;

	BOOL ShowApplet(HWND hWnd, LONG_PTR /*lData*/, LPCTSTR pstrCommand)
	{
		BOOL bWithFingerMenu = FALSE;
		CString szPath;
		CString szProgramFilesFolder;
		WCHAR szInstallDir[MAX_PATH];
		WCHAR szValue[MAX_PATH];
		ZeroMemory(szValue, sizeof(szValue));
		if (SHGetSpecialFolderPath(NULL, szValue, CSIDL_PROGRAM_FILES, FALSE))
		{
			szProgramFilesFolder.Format(L"%s", szValue);
		}
		if (RegistryGetString(HKEY_LOCAL_MACHINE, L"Software\\FingerMenu", L"InstallDir", szInstallDir, MAX_PATH) == S_OK)
		{
			szPath.Format(L"%s\\FingerMenu.exe", szInstallDir);
		}
		else
		{
			szPath = szProgramFilesFolder + L"\\FingerMenu\\FingerMenu.exe";
		}
		
		CFindFile finder; 
		if (finder.FindFile(szPath))
		{
			bWithFingerMenu = TRUE;
		}



		dlg1.SetTitle(L"Startup");
		dlg1.m_bWithFingerMenu = bWithFingerMenu;
		if (bWithFingerMenu)
		{
			dlg2.SetTitle(L"Menu options");
			dlg3.SetTitle(L"Menu exclusions");
			dlg7.SetTitle(L"Menu wnd exclusions");
		}
		dlg5.SetTitle(L"Msgbox options");
		dlg6.SetTitle(L"Msgbox exclusions");
		dlg8.SetTitle(L"Msgbox wnd exclusions");
		dlg4.SetTitle(L"Skins");
		dlg4.m_bWithFingerMenu = bWithFingerMenu;
		dlgAbout.SetTitle(L"About");

		// about box
		CString strCredits = "\n\n"
			    "\tFingerMenu v1.12\n\n"
				"\tFingerMsgBox v1.01\n\n"
				"\rdeveloped by:\n"
				"Francesco Carlucci\n"
				"<francarl@gmail.com>\n"
				"\n\n"
				"http://forum.xda-developers.com/\n"
				"                showthread.php?t=459125\n";

		dlgAbout.SetCredits(strCredits);

		CPropertySheet sheet;
		sheet.AddPage(dlg1);
		if (bWithFingerMenu)
		{
			sheet.AddPage(dlg2);
			sheet.AddPage(dlg3);
			sheet.AddPage(dlg7);
		}
		sheet.AddPage(dlg5);
		sheet.AddPage(dlg6);
		sheet.AddPage(dlg8);
		sheet.AddPage(dlg4);
		sheet.AddPage(dlgAbout);
		sheet.SetActivePage(_ttol(pstrCommand));
		if (IDOK == sheet.DoModal(hWnd))
		{
			ReloadConfiguration();
		}

		return TRUE;
	}

private:

	void ReloadConfiguration()
	{
		HWND hWndDest = ::FindWindow(L"FINGER_MENU", NULL);
		if (hWndDest != NULL)
		{
			::PostMessage(hWndDest, UWM_UPDATECONFIGURATION, 0, 0);
			/*
			::PostMessage(hWndDest, WM_CLOSE, 0, 0);
			Sleep(2000);

			CString strApp = L"\\Program Files\\FingerMenu\\FingerMenu.exe";
			SHELLEXECUTEINFO sei;
			memset(&sei, 0, sizeof(sei));
	    
			sei.cbSize       = sizeof(sei);
			sei.fMask        = 0;                
			sei.hwnd         = 0;                
			sei.lpVerb       = L"open";                      // Operation to perform
			sei.lpFile       = strApp;                 // Application name
			sei.lpParameters = NULL;                        // Additional parameters
			sei.lpDirectory  = 0;                           // Default directory
			sei.nShow        = SW_SHOW;
			sei.hInstApp     = 0;
	    
			ShellExecuteEx(&sei);
			*/
		}

		hWndDest = ::FindWindow(L"FINGER_MSGBOX", NULL);
		if (hWndDest != NULL)
		{
			::PostMessage(hWndDest, UWM_UPDATECONFIGURATION, 0, 0);
			/*
			::PostMessage(hWndDest, WM_CLOSE, 0, 0);
			Sleep(2000);

			CString strApp = L"\\Program Files\\FingerMsgbox\\FingerMsgbox.exe";
			SHELLEXECUTEINFO sei;
			memset(&sei, 0, sizeof(sei));
	    
			sei.cbSize       = sizeof(sei);
			sei.fMask        = 0;                
			sei.hwnd         = 0;                
			sei.lpVerb       = L"open";                      // Operation to perform
			sei.lpFile       = strApp;                 // Application name
			sei.lpParameters = NULL;                        // Additional parameters
			sei.lpDirectory  = 0;                           // Default directory
			sei.nShow        = SW_SHOW;
			sei.hInstApp     = 0;
	    
			ShellExecuteEx(&sei);
			*/
		}

	}


};


BEGIN_CPLAPPLET_MAP()
	CPLAPPLET_ENTRY(CFingerSuiteApplet, "FingerSuiteApplet", FingerSuiteApplet)
END_CPLAPPLET_MAP()


CAppModule _Module;
CCPlAppletModule _Applets;


BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
   if( ul_reason_for_call == DLL_PROCESS_ATTACH ) _Module.Init(NULL, (HMODULE)hModule);
   if( ul_reason_for_call == DLL_PROCESS_DETACH ) _Module.Term();
   return TRUE;
}

extern "C" __declspec(dllexport) LONG APIENTRY CPlApplet(HWND hwndCPl, UINT msg, LPARAM lParam1, LPARAM lParam2)
{
   return _Applets.CPlApplet(hwndCPl, msg, lParam1, lParam2);
}


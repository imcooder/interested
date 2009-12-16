#include "stdafx.h"
#include <windows.h>
#include "hook\SysDecls.h" 
#include "hook\undocWM5.h"
#include "InterceptEngine.h"
#include <tlhelp32.h>
#include "..\Common\log\logger.h"
#include "..\Common\utils.h"

// globals
HINSTANCE   g_hInst = NULL; 
UINT        UWM_INTERCEPT_MENU;
UINT        UWM_INTERCEPT_MSGBOX;
UINT        UWM_INTERCEPT_NOTIF;
UINT        UWM_NOTIF_POPUP;
HWND        g_hWndHHTaskBar = 0;

#pragma data_seg("SH_DATA")
// for SH_WMGR apiset replacement
CINFO *   m_pNewWmgrApiSet = NULL;
CINFO *   m_pOldWmgrApiSet = NULL;
// end SH_WMGR
// for SH_SHELL apiset replacement
CINFO *   m_pNewShellApiSet = NULL;
CINFO *   m_pOldShellApiSet = NULL;
// end SH_SHELL

HWND      g_hWndServer = 0;
HWND      g_hWndServerMsgBox = 0;
HWND      g_hWndServerNotif = 0;

HANDLE    m_hDestProcess = NULL;
HINSTANCE m_hDllInst = 0;

BOOL      g_bTrackPopupMenuExResult = FALSE;
int       g_iMsgBoxResult = 0;

PFNVOID   g_pOrigTrackPopupMenuEx = NULL;
PFNVOID   g_pOrigMessageBoxW = NULL;

PFNVOID   g_pOrigSHNotificationAddII = NULL;
PFNVOID   g_pOrigSHNotificationUpdateII = NULL;
PFNVOID   g_pOrigSHNotificationRemoveII = NULL;

#pragma data_seg()
#pragma comment( linker, "/SECTION:SH_DATA,SRW" )



__declspec(dllexport) 
HANDLE CreateWaitEvent(LPTSTR szEvt)
{
	HANDLE hEvt = CreateEvent(NULL, TRUE, FALSE, szEvt);
	if ( (hEvt != NULL) && (GetLastError() == ERROR_ALREADY_EXISTS) )
		ResetEvent(hEvt);
	return hEvt;
}

__declspec(dllexport) 
BOOL ResetWaitEvent(LPTSTR szEvt)
{
	HANDLE hEvt = OpenEvent(EVENT_ALL_ACCESS, FALSE, szEvt);
	if (hEvt != NULL)
	{
		BOOL bRes = ResetEvent(hEvt);
		CloseHandle(hEvt);
		return bRes;
	}
	
	return FALSE;
}

__declspec(dllexport) 
BOOL SignalWaitEvent(LPTSTR szEvt)
{
	BOOL bRes = FALSE;
	//wprintf(L"SignalWaitEvent...");

	HANDLE hEvt = OpenEvent(EVENT_ALL_ACCESS, FALSE, szEvt);
	if (hEvt != NULL)
	{
		bRes = SetEvent(hEvt);
		CloseHandle(hEvt);
	}
	//wprintf(L"...done\n");	
	return bRes;
}


/*-------------------------------------------------------------
   FUNCTION: HookTrackPopupMenuEx
   PURPOSE:  Interceptor for TrackPopupMenuEx API
   Parameters and return value are identical to the documented
   HookTrackPopupMenuEx function.
-------------------------------------------------------------*/
BOOL HookTrackPopupMenuEx
(
        HMENU       p_hmenu,
        UINT        p_uFlags,
        int         p_x,
        int         p_y,
        HWND        p_hWnd,
        LPTPMPARAMS p_lptpm
)
{
	BOOL bResult = FALSE;

	if (!(IsWindow(g_hWndServer)))
		goto normal;

	if (p_hWnd == g_hWndHHTaskBar)
		goto normal;

	UINT msgToBeSent = UWM_INTERCEPT_MENU;
	//if (p_uFlags & TPM_RETURNCMD)

	TRACKPOPUPMENUINFO info;
	info.hMenu = p_hmenu;
	info.hWnd = p_hWnd;
	info.uFlags = p_uFlags;

	BOOL bInterceptRes =  (BOOL)SendMessageRemote(g_hWndServer, msgToBeSent, (WPARAM)0, (LPARAM)&info, sizeof(TRACKPOPUPMENUINFO));

	if (bInterceptRes) 
	{
		goto normal;
	}

	HANDLE arr[1];
	arr[0] = CreateWaitEvent(EVT_FNGRMENU);
	
	DWORD dwRes;
	MSG msg;
	do
	{
		dwRes = MsgWaitForMultipleObjectsEx(1, arr, INFINITE, QS_ALLINPUT, 0);
		if (dwRes != WAIT_OBJECT_0 + 0)
		{
			if (GetMessage( &msg, NULL, 0, 0 ))
			{
				TranslateMessage( &msg );
				DispatchMessage( &msg );
			}
		}
	}
	while (dwRes != WAIT_OBJECT_0 + 0);

	CloseHandle(arr[0]);


	if (g_bTrackPopupMenuExResult == -1)
	{
		goto normal;
	}

	if (p_uFlags & TPM_RETURNCMD)
	{
		// calling process should call SetTrackPopupMenuExResult
		return g_bTrackPopupMenuExResult; 
	}
	else
		return bResult;

normal:
	
    if(g_pOrigTrackPopupMenuEx)
    {
        bResult = ((BOOL (*)(
            HMENU, UINT, int, int,
            HWND, LPTPMPARAMS))g_pOrigTrackPopupMenuEx)
            (
				p_hmenu,
				p_uFlags,
				p_x,
				p_y,
				p_hWnd,
				p_lptpm
            );
    }//if(g_pOrigTrackPopupMenuEx)
    
    return bResult;
}//HANDLE HookTrackPopupMenuEx

/*-------------------------------------------------------------
BOOL HookTrackPopupMenuEx
(
        HMENU       p_hmenu,
        UINT        p_uFlags,
        int         p_x,
        int         p_y,
        HWND        p_hWnd,
        LPTPMPARAMS p_lptpm
)
{
	BOOL bResult = FALSE;

	if (!(IsWindow(g_hWndServer)))
		goto normal;

	if (p_hWnd == g_hWndHHTaskBar)
		goto normal;

	UINT msg = 0;
	if (p_uFlags & TPM_RETURNCMD)
		msg = UWM_INTERCEPT_MENU2;
	else
		msg = UWM_INTERCEPT_MENU1;

	BOOL bInterceptRes =  (BOOL)SendMessage(g_hWndServer, msg, (WPARAM)p_hmenu, (LPARAM)p_hWnd);

	if (bInterceptRes) 
	{
		goto normal;
	}

	HANDLE hEvt = CreateWaitEvent(EVT_FNGRMENU);
	DWORD dwRes = WaitForSingleObject(hEvt, INFINITE);
	if (dwRes == WAIT_OBJECT_0)
	{

	}

	CloseHandle(hEvt);

	if (g_bTrackPopupMenuExResult == -1)
	{
		goto normal;
	}

	if (p_uFlags & TPM_RETURNCMD)
	{
		// calling process should call SetTrackPopupMenuExResult
		return g_bTrackPopupMenuExResult; 
	}
	else
		return bResult;

normal:
	
    if(g_pOrigTrackPopupMenuEx)
    {
        bResult = ((BOOL (*)(
            HMENU, UINT, int, int,
            HWND, LPTPMPARAMS))g_pOrigTrackPopupMenuEx)
            (
				p_hmenu,
				p_uFlags,
				p_x,
				p_y,
				p_hWnd,
				p_lptpm
            );
    }//if(g_pOrigTrackPopupMenuEx)
    
    return bResult;
}

int HookMessageBoxW(
                HWND    hwnd,
        const   WCHAR*  szText,
        const   WCHAR*  szCaption,
                UINT    uType,
                MSG*    pMsg
        )
{
	int iResult = 0;

	if (!(IsWindow(g_hWndServerMsgBox)))
		goto normal;

	LPMSGBOXINFO info = (MSGBOXINFO*)g_sharedMsgBoxMem.GetValue();

	ZeroMemory(info, sizeof(MSGBOXINFO));
	wsprintf(info->szCaption, L"%s", szCaption);
	wsprintf(info->szText, L"%s", szText);

	//g_sharedMsgBoxMem.SetValue(&info, sizeof(info));

	BOOL bInterceptRes =  (BOOL)SendMessage(g_hWndServerMsgBox, UWM_INTERCEPT_MSGBOX, (WPARAM)hwnd, (LPARAM)uType);

	if (bInterceptRes) 
	{
		goto normal;
	}

	HANDLE hEvt = CreateWaitEvent(EVT_FNGRMSGBOX);
	DWORD dwRes = WaitForSingleObject(hEvt, INFINITE);
	if (dwRes == WAIT_OBJECT_0)
	{
		if (g_iMsgBoxResult == -1)
		{
			goto normal;
		}

		return g_iMsgBoxResult;
	}
normal:
	
	if (g_pOrigMessageBoxW)
    {
		iResult = ((int (*)(HWND, 
			                const WCHAR*, 
						    const WCHAR*, 
						    UINT, 
						    MSG*))g_pOrigMessageBoxW)
            (
              hwnd,
              szText,
              szCaption,
              uType,
              pMsg
            );
    }//if(g_pOrigMessageBoxW)
    
    return iResult;
}
-------------------------------------------------------------*/


int HookMessageBoxW(
                HWND    hwnd,
        const   WCHAR*  szText,
        const   WCHAR*  szCaption,
                UINT    uType,
                MSG*    pMsg
        )
{
	int iResult = 0;

	if (!(IsWindow(g_hWndServerMsgBox)))
		goto normal;

	MSGBOXINFO info;
	info.hWnd = hwnd;
	info.uType = uType;
	wsprintf(info.szCaption, L"%s", szCaption);
	wsprintf(info.szText, L"%s", szText);

	BOOL bInterceptRes =  (BOOL)SendMessageRemote(g_hWndServerMsgBox, UWM_INTERCEPT_MSGBOX, (WPARAM)0, (LPARAM)&info, sizeof(MSGBOXINFO));

	if (bInterceptRes) 
	{
		goto normal;
	}

	HANDLE arr[1];
	arr[0] = CreateWaitEvent(EVT_FNGRMSGBOX);
	
	DWORD dwRes;
	do
	{
		dwRes = MsgWaitForMultipleObjectsEx(1, arr, INFINITE, QS_ALLINPUT, 0);
		if (dwRes != WAIT_OBJECT_0 + 0)
		{
			MSG msg;
			if (GetMessage( &msg, NULL, 0, 0 ))
			{
				TranslateMessage( &msg );
				DispatchMessage( &msg );
			}
		}
	}
	while (dwRes != WAIT_OBJECT_0 + 0);

	CloseHandle(arr[0]);

	if (dwRes == WAIT_OBJECT_0)
	{
		if (g_iMsgBoxResult == -1)
		{
			goto normal;
		}

		return g_iMsgBoxResult;
	}




normal:
	
	if (g_pOrigMessageBoxW)
    {
		iResult = ((int (*)(HWND, 
			                const WCHAR*, 
						    const WCHAR*, 
						    UINT, 
						    MSG*))g_pOrigMessageBoxW)
            (
              hwnd,
              szText,
              szCaption,
              uType,
              pMsg
            );
    }//if(g_pOrigMessageBoxW)
    
    return iResult;
}




/*-------------------------------------------------------------------
   FUNCTION: DllMain
   PURPOSE: Regular DLL initialization point. Used to initialize
    per-process data structures, such as the tracing engine
-------------------------------------------------------------------*/
BOOL WINAPI DllMain(HANDLE hInstance, DWORD dwReason, 
                    LPVOID /*lpReserved*/)
{
    TCHAR l_szPath[_MAX_PATH];
    *l_szPath = 0;
    GetModuleFileName(NULL, l_szPath, _MAX_PATH);
    

    static long s_lLoadCount = 0;

	switch (dwReason)
	{
		case DLL_PROCESS_ATTACH:
        {
            g_hInst = (HINSTANCE)hInstance;

			UWM_INTERCEPT_MENU = RegisterWindowMessage(UWM_INTERCEPT_MENU_MSG);
			UWM_INTERCEPT_MSGBOX = RegisterWindowMessage(UWM_INTERCEPT_MSGBOX_MSG);
			UWM_INTERCEPT_NOTIF = RegisterWindowMessage(UWM_INTERCEPT_NOTIF_MSG);
			UWM_NOTIF_POPUP = RegisterWindowMessage(UWM_NOTIF_POPUP_MSG);

			g_hWndHHTaskBar = FindWindow(L"HHTaskBar", NULL);

            if(InterlockedIncrement(&s_lLoadCount) == 1)
            {
                DWORD l_dwOldProtect;
				BOOL bRes = VirtualProtect(&m_pNewWmgrApiSet, sizeof(m_pNewWmgrApiSet),
                    PAGE_READWRITE | PAGE_NOCACHE, &l_dwOldProtect);
            }
			
        }
        break;
        case DLL_PROCESS_DETACH:
        {
            if(InterlockedDecrement(&s_lLoadCount) == 0)
            {
            }

        }
        break;
    }
    return TRUE;
}//BOOL WINAPI DllMain


__declspec(dllexport) 
BOOL SetHWndServer(HWND hWndServer)
{
	g_hWndServer = hWndServer;
	return TRUE;
}

__declspec(dllexport) 
BOOL SetHWndServerMsgBox(HWND hWndServer)
{
	g_hWndServerMsgBox = hWndServer;
	return TRUE;
}

__declspec(dllexport) 
BOOL SetHWndServerNotif(HWND hWndServer)
{
	g_hWndServerNotif = hWndServer;
	return TRUE;
}


/*-------------------------------------------------------------------
   FUNCTION: ProcessAddress
   PURPOSE:  
   returns an address of memory slot for the given process index.
   PARAMETERS:
    BYTE p_byProcNum - process number (slot index) between 0 and 31
   RETURNS:
    Address of the memory slot.
-------------------------------------------------------------------*/
inline DWORD ProcessAddress(BYTE p_byProcNum)
{
    return 0x02000000 * (p_byProcNum+1);
}

/*-------------------------------------------------------------------
   FUNCTION: ConvertAddr
   PURPOSE:  
    ConvertAddr does the same as MapPtrToProcess - maps an address in
    slot 0 to the address in the slot of the given process. Unlike 
    MapPtrToProcess, which accepts process handle, ConvertAddr uses
    undocumented PROCESS structure.
   PARAMETERS:
    LPVOID p_pAddr - address to convert
    PPROCESS p_pProcess - internal kernel Process structure
   RETURNS:
    Address mapped to the slot of the given process
-------------------------------------------------------------------*/
LPVOID ConvertAddr(LPVOID p_pAddr, PPROCESS p_pProcess)
{
    if( ((DWORD)p_pAddr) < 0x2000000 && p_pProcess)
    {//Slot 0 and process is not the kernel
        LPVOID l_pOld = p_pAddr;
        BYTE l_byProcNum = 
            *(((LPBYTE)p_pProcess) + PROCESS_NUM_OFFSET);
        p_pAddr = (LPVOID) (((DWORD)p_pAddr) + 
            ProcessAddress(l_byProcNum));
    }
    return p_pAddr;
}

int FilterException(LPEXCEPTION_POINTERS pEx)
{
	DWORD exCode = pEx->ExceptionRecord->ExceptionCode;
	BOOL bRes = (exCode == EXCEPTION_ACCESS_VIOLATION);
	return EXCEPTION_EXECUTE_HANDLER;
}

void * AllocateMemInKernelProc(int p_iSize)
{
	LPVOID pAllocated = NULL;
	// find process id of nk.exe
	HANDLE snapShot = INVALID_HANDLE_VALUE;
	DWORD dwNKProcessId = 0;
	__try
	{
	    snapShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS | TH32CS_SNAPNOHEAPS, 0);
	    if (snapShot != INVALID_HANDLE_VALUE)
		{
			// Build new list
			PROCESSENTRY32 processEntry;
			processEntry.dwSize = sizeof(PROCESSENTRY32);
			BOOL ret = Process32First(snapShot, &processEntry);
			while (ret == TRUE)
			{
				if (lstrcmpi(processEntry.szExeFile, L"nk.exe") == 0)
				{
					dwNKProcessId = processEntry.th32ProcessID;
					break;
				}
				ret = Process32Next(snapShot, &processEntry);
			}
			CloseToolhelp32Snapshot(snapShot);
		}
	} __except (EXCEPTION_EXECUTE_HANDLER)
	{
		if (snapShot != INVALID_HANDLE_VALUE)
		{
			CloseToolhelp32Snapshot(snapShot);
		}
		return NULL;
	}

	HANDLE hNKProcess = OpenProcess(0, FALSE, dwNKProcessId);
	if (hNKProcess == NULL)
		return NULL;

	HINSTANCE hCoreDll = LoadLibrary(_T("COREDLL"));
	CALLBACKINFO cbi;
	cbi.m_hDestinationProcessHandle = hNKProcess;
	cbi.m_pFunction = (FARPROC)MapPtrToProcess(GetProcAddress(hCoreDll, L"VirtualAlloc"), hNKProcess);
	cbi.m_pFirstArgument = (LPVOID)0; 
	DWORD dwParam2 = p_iSize;
    DWORD dwParam3 = MEM_COMMIT;
	DWORD dwParam4 = PAGE_EXECUTE_READWRITE;
	DWORD dwPtr = PerformCallBack4(&cbi, dwParam2, dwParam3, dwParam4); //returns 1 if correctly executed	

	pAllocated = MapPtrToProcess( (LPVOID)dwPtr, hNKProcess);
	CloseHandle(hNKProcess);
	return pAllocated;
}

__declspec(dllexport) 
BOOL InstallHook()
{
    static long s_lCount = 0;
    if (InterlockedIncrement(&s_lCount) > 1)
    {
		// no need to install again
		return TRUE;
	}

	BOOL bResult = TRUE;

	if (m_hDestProcess == NULL)
	{
		int iAPISetId = SH_WMGR; 
		DWORD dwOldPermissions = 0;
		SetKMode(TRUE);
		dwOldPermissions = SetProcPermissions(-1);    
		__try
		{
			CINFO ** pSystemAPISets = (CINFO**)(UserKInfo[KINX_APISETS]);
			m_hDestProcess = pSystemAPISets[iAPISetId]->m_pProcessServer->hProc;

			CALLBACKINFO cbi;
			ZeroMemory(&cbi, sizeof(CALLBACKINFO));
			cbi.m_hDestinationProcessHandle = m_hDestProcess;
			cbi.m_pFunction = (FARPROC)MapPtrToProcess(GetProcAddress(GetModuleHandle(L"COREDLL"), L"LoadLibraryW"), m_hDestProcess);
			cbi.m_pFirstArgument = (LPVOID)MapPtrToProcess(L"\\Windows\\FingerSuiteDll.dll", GetCurrentProcess());
			m_hDllInst = (HINSTANCE)PerformCallBack4(&cbi, 0,0,0); //returns the HINSTANCE from LoadLibraryW

			Sleep(1000);

			ZeroMemory(&cbi, sizeof(CALLBACKINFO));
			cbi.m_hDestinationProcessHandle = m_hDestProcess;
			cbi.m_pFunction = (FARPROC)MapPtrToProcess(GetProcAddress(m_hDllInst, L"StartHookOnServer"), m_hDestProcess);
			cbi.m_pFirstArgument = NULL; 
			DWORD dw = PerformCallBack4(&cbi, 0,0,0); //returns 1 if correctly executed		

			Sleep(1000);
		}
		__except(FilterException(GetExceptionInformation()))
		{
			bResult = FALSE;
		}
		if(dwOldPermissions)
		{
			SetProcPermissions(dwOldPermissions);
		}
		SetKMode(FALSE);
	}
	return bResult;
}

__declspec(dllexport) 
BOOL RemoveHook()
{
    static long s_lCount = 0;
    if (InterlockedIncrement(&s_lCount) > 1)
    {
		// no need to remove now
		return TRUE;
	}

	BOOL bResult = TRUE;

	if (m_hDestProcess != NULL)
	{
		int iAPISetId = SH_WMGR; 
		DWORD dwOldPermissions = 0;
		SetKMode(TRUE);
		dwOldPermissions = SetProcPermissions(-1);    
		__try
		{
			CALLBACKINFO cbi;
			ZeroMemory(&cbi, sizeof(CALLBACKINFO));
			cbi.m_hDestinationProcessHandle = m_hDestProcess;
			cbi.m_pFunction = (FARPROC)MapPtrToProcess(GetProcAddress(m_hDllInst, L"StopHookOnServer"), m_hDestProcess);
			cbi.m_pFirstArgument = NULL;
			BOOL bRes = (BOOL)PerformCallBack4(&cbi, 0,0,0); //returns the HINSTANCE from LoadLibraryW

			Sleep(1000);

			ZeroMemory(&cbi, sizeof(CALLBACKINFO));
			cbi.m_hDestinationProcessHandle = m_hDestProcess;
			cbi.m_pFunction = (FARPROC)MapPtrToProcess(GetProcAddress(GetModuleHandle(L"COREDLL"), L"FreeLibraryW"), m_hDestProcess);
			cbi.m_pFirstArgument = m_hDllInst;
			bRes = (BOOL)PerformCallBack4(&cbi, 0,0,0); //returns the HINSTANCE from LoadLibraryW

			Sleep(1000);
		}				
		__except(FilterException(GetExceptionInformation()))
		{
			bResult = FALSE;
		}
		if(dwOldPermissions)
		{
			SetProcPermissions(dwOldPermissions);
		}
		SetKMode(FALSE);
	}
	
	StopHookOnSIP();


	return bResult;
}



__declspec(dllexport) 
BOOL StartHookOnServer()
{
	BOOL bResult = TRUE;

	int iAPISetId = SH_WMGR; 
    int iMethodIndex = WMGR_TrackPopupMenuEx;
	int iMethodIndex2 = WMGR_MessageBoxW;

    //HANDLE hCurProc = GetCurrentProcess();
	CINFO * pOrigApiSet; pOrigApiSet = NULL;
	
	DWORD dwOldPermissions = 0;
    SetKMode(TRUE);
    dwOldPermissions = SetProcPermissions(-1);    
	__try
	{
		CINFO ** pSystemAPISets = (CINFO**)(UserKInfo[KINX_APISETS]);
		pOrigApiSet = pSystemAPISets[iAPISetId];

		PFNVOID * pHookMthds = NULL;
		DWORD   * pdwHookSignatures = NULL;
		PFNVOID * pOrigMethods = (PFNVOID *)ConvertAddr(pOrigApiSet->m_ppMethods, pOrigApiSet->m_pProcessServer);
		DWORD   * pOrigSignatures = (DWORD *)ConvertAddr(pOrigApiSet->m_pdwMethodSignatures, pOrigApiSet->m_pProcessServer);

		m_pNewWmgrApiSet = (CINFO *)AllocateMemInKernelProc(sizeof(CINFO));

		int iNewNumMethods = pOrigApiSet->m_wNumMethods;


		// duplicate method table
		int iSize = (sizeof(PFNVOID) * iNewNumMethods + sizeof(DWORD) * iNewNumMethods);
		LPBYTE pRemoteMem = (LPBYTE)AllocateMemInKernelProc(iSize);

		pHookMthds = (PFNVOID *)pRemoteMem;
		pdwHookSignatures = (DWORD*)(pRemoteMem + sizeof(PFNVOID)*iNewNumMethods);
		
		for (int i = 0; i < iNewNumMethods; i++)
		{
			pHookMthds[i] = pOrigMethods[i];
			pdwHookSignatures[i] = pOrigSignatures? pOrigSignatures[i] : FNSIG0();
		}

		memcpy(m_pNewWmgrApiSet, pOrigApiSet, sizeof(CINFO));

		//But replace the method and signature tables:
		m_pNewWmgrApiSet->m_ppMethods = pHookMthds;
		m_pNewWmgrApiSet->m_pdwMethodSignatures = pdwHookSignatures;
		m_pNewWmgrApiSet->m_wNumMethods = (WORD)iNewNumMethods;

		// replace pointer with hooked procedure
		g_pOrigTrackPopupMenuEx = ((PFNVOID *)m_pNewWmgrApiSet->m_ppMethods)[iMethodIndex];
		((PFNVOID *)m_pNewWmgrApiSet->m_ppMethods)[iMethodIndex] = (PFNVOID)HookTrackPopupMenuEx;


		g_pOrigMessageBoxW = ((PFNVOID *)m_pNewWmgrApiSet->m_ppMethods)[iMethodIndex2];
		((PFNVOID *)m_pNewWmgrApiSet->m_ppMethods)[iMethodIndex2] = (PFNVOID)HookMessageBoxW;


		//And finally, replace the pointer to the original CINFO
		//by the pointer to ours.
		m_pOldWmgrApiSet = pSystemAPISets[iAPISetId];
		pSystemAPISets[iAPISetId] = m_pNewWmgrApiSet;

	}
	__except(FilterException(GetExceptionInformation()))
	{
		bResult = FALSE;
	}
    if(dwOldPermissions)
    {
        SetProcPermissions(dwOldPermissions);
    }
    SetKMode(FALSE);

	return bResult;
}

__declspec(dllexport) 
BOOL StopHookOnServer()
{
	BOOL bResult = TRUE;

	int iAPISetId = SH_WMGR; 
	DWORD dwOldPermissions = 0;
    SetKMode(TRUE);
    dwOldPermissions = SetProcPermissions(-1);    
	__try
	{
		CINFO ** pSystemAPISets = (CINFO**)(UserKInfo[KINX_APISETS]);
		//restore the pointer to the original CINFO
		if (m_pOldWmgrApiSet != NULL)
		{
			pSystemAPISets[iAPISetId] = m_pOldWmgrApiSet;

			// TODO 
			// free mem allocated for m_pNewApiSet
		}

	}
	__except(FilterException(GetExceptionInformation()))
	{
		bResult = FALSE;
	}
    if(dwOldPermissions)
    {
        SetProcPermissions(dwOldPermissions);
    }
    SetKMode(FALSE);

	return bResult;
}


__declspec(dllexport) 
void SetMsgBoxResult(int iResult)
{
	g_iMsgBoxResult = iResult;
}


__declspec(dllexport) 
void SetTrackPopupMenuExResult(BOOL bResult)
{
	g_bTrackPopupMenuExResult = bResult;
}

/// SHELL ////////////////
//////////////////////////

LRESULT UpdateBuffer(LPNOTIFICATIONINFO pInfo, const CLSID *pclsid, DWORD dwID, DWORD grfFlagsOriginal)
{
	SHNOTIFICATIONDATA shnd;
	ZeroMemory(&shnd, sizeof(SHNOTIFICATIONDATA));
	shnd.cbStruct = sizeof(SHNOTIFICATIONDATA);
	LRESULT lRes = SHNotificationGetData(pclsid, dwID, &shnd);


	// notification data
	if (lRes == ERROR_SUCCESS)
	{
		ZeroMemory(pInfo, sizeof(NOTIFICATIONINFO));
		memcpy(&pInfo->nd, &shnd, sizeof(SHNOTIFICATIONDATA));

		pInfo->grfFlagsOriginal = grfFlagsOriginal;

		// strings
		wsprintf(pInfo->szTitle, L"%s", shnd.pszTitle);
		wsprintf(pInfo->szHTML, L"%s", shnd.pszHTML);
		if (shnd.pszHTML) 
			free((LPVOID)shnd.pszHTML);
		shnd.pszHTML = NULL;
		if (shnd.pszTitle) 
			free((LPVOID)shnd.pszTitle);
		shnd.pszTitle = NULL;
		
		// adjust pointers
		pInfo->nd.pszTitle = pInfo->szTitle;
		pInfo->nd.pszHTML = pInfo->szHTML;

		// menu
		if (shnd.grfFlags & SHNF_HASMENU)
		{
			memcpy(pInfo->rgskc, shnd.skm.prgskc, sizeof(SOFTKEYMENU) * shnd.skm.cskc);
			// adjust pointers
			pInfo->nd.skm.prgskc = pInfo->rgskc;
		}
		else
		{
			if (shnd.rgskn[0].pszTitle != NULL) wsprintf(pInfo->sk1Title, L"%s", shnd.rgskn[0].pszTitle);
			if (shnd.rgskn[1].pszTitle != NULL) wsprintf(pInfo->sk2Title, L"%s", shnd.rgskn[1].pszTitle);

			// adjust pointers
			pInfo->nd.rgskn[0].pszTitle = pInfo->sk1Title;
			pInfo->nd.rgskn[1].pszTitle = pInfo->sk2Title;
		}
	}
	return lRes;
}


LRESULT WINAPI HookSHNotificationAddII( SHNOTIFICATIONDATA *pndAdd, DWORD cbData, LPTSTR pszTitle, LPTSTR pszHTML )
{
	LRESULT lResult = ERROR_INVALID_PARAMETER;
	NOTIFICATIONINFO info;

	DWORD grfFlagsOriginal = pndAdd->grfFlags;

	BOOL bIsManaged = IsManagedNotification(pndAdd->clsid);
	

	if ((IsWindow(g_hWndServerNotif)) && (pndAdd->npPriority & SHNP_INFORM) && (bIsManaged))
	{
		//modifica parametri pndAdd 
		pndAdd->grfFlags |= SHNF_STRAIGHTTOTRAY;
		pndAdd->grfFlags &= (~SHNF_FORCEMESSAGE);
	}

    if (g_pOrigSHNotificationAddII)
    {
        	lResult = ((LRESULT (*)(
				SHNOTIFICATIONDATA *, DWORD, LPTSTR, LPTSTR))g_pOrigSHNotificationAddII)
				(
					pndAdd,
					cbData,
					pszTitle,
					pszHTML
				);
    } //if (g_pOrigSHNotificationAddII)

	if (lResult != ERROR_SUCCESS)
		goto normal;

	if (pndAdd->npPriority == SHNP_ICONIC)
		goto normal;

	if (!(IsWindow(g_hWndServerNotif)))
		goto normal;
	
	lResult = UpdateBuffer(&info, &pndAdd->clsid, pndAdd->dwID, grfFlagsOriginal);
	
	info.isManaged = bIsManaged;

	BOOL bInterceptRes =  (BOOL)SendMessageRemote(g_hWndServerNotif, UWM_INTERCEPT_NOTIF, NOTIF_ADD, (LPARAM)&info, sizeof(NOTIFICATIONINFO));

	if (bInterceptRes) 
	{
		goto normal;
	}

normal:
	return lResult;
}


LRESULT WINAPI HookSHNotificationUpdateII(DWORD grnumUpdateMask, SHNOTIFICATIONDATA *pndNew, DWORD cbData, LPTSTR pszTitle, LPTSTR pszHTML)
{
	LRESULT lResult = ERROR_INVALID_PARAMETER;

	NOTIFICATIONINFO info;

	DWORD grfFlagsOriginal = pndNew->grfFlags;

	BOOL bIsManaged = IsManagedNotification(pndNew->clsid);

	if (IsWindow(g_hWndServerNotif) && (bIsManaged))
	{
		pndNew->grfFlags |= SHNF_STRAIGHTTOTRAY;
		pndNew->grfFlags &= (~SHNF_FORCEMESSAGE);
	}

	if (g_pOrigSHNotificationUpdateII)
    {
        	lResult = ((LRESULT (*)(
				DWORD, SHNOTIFICATIONDATA *, DWORD, LPTSTR, LPTSTR))g_pOrigSHNotificationUpdateII)
				(
				    grnumUpdateMask,
					pndNew,
					cbData,
					pszTitle,
					pszHTML
				);
    } //if (g_pOrigSHNotificationUpdateII)

	if (lResult != ERROR_SUCCESS)
		goto normal;

	if (!(IsWindow(g_hWndServerNotif)))
		goto normal;

	lResult = UpdateBuffer(&info, &pndNew->clsid, pndNew->dwID, grfFlagsOriginal);

	if (lResult != ERROR_SUCCESS)
		goto normal;

	info.isManaged = bIsManaged;

	BOOL bInterceptRes =  (BOOL)SendMessageRemote(g_hWndServerNotif, UWM_INTERCEPT_NOTIF, NOTIF_UPD, (LPARAM)&info, sizeof(NOTIFICATIONINFO));

	if (bInterceptRes) 
	{
		goto normal;
	}

normal:
    return lResult;
}


LRESULT WINAPI HookSHNotificationRemoveII(const CLSID *pclsid, DWORD cbCLSID, DWORD dwID)
{
	LRESULT lResult = ERROR_INVALID_PARAMETER;

	if (IsWindow(g_hWndServerNotif))
	{
	}

	NOTIFICATIONINFO info;
	ZeroMemory(&info, sizeof(NOTIFICATIONINFO));
	memcpy(&info.nd.clsid, pclsid, sizeof(CLSID));
	info.nd.dwID = dwID;

	if (g_pOrigSHNotificationRemoveII)
    {
        	lResult = ((LRESULT (*)(
				const CLSID *, DWORD, DWORD))g_pOrigSHNotificationRemoveII)
				(
					pclsid, 
					cbCLSID,
					dwID
				);
    } //if (g_pOrigSHNotificationRemoveII)

	if (lResult != ERROR_SUCCESS)
		goto normal;

	if (!(IsWindow(g_hWndServerNotif)))
		goto normal;

	BOOL bInterceptRes =  (BOOL)SendMessageRemote(g_hWndServerNotif, UWM_INTERCEPT_NOTIF, NOTIF_DEL, (LPARAM)&info, sizeof(NOTIFICATIONINFO));

	if (bInterceptRes) 
	{
		goto normal;
	}


normal:
    return lResult;
}

// NON SERVE PIU
/*
BOOL WINAPI HookShell_NotifyIconI(DWORD dwMsg, PNOTIFYICONDATA pNID, DWORD cbNID)
{
	LOG(L"Start HookShell_NotifyIconI ...\n");

	BOOL bResult = FALSE;

	LPNOTIFICATIONINFO info = NULL;

	if (IsWindow(g_hWndServerNotif))
	{
		info = (NOTIFICATIONINFO*)g_sharedNotifMem.GetValue();
		ZeroMemory(info, sizeof(NOTIFICATIONINFO));
		info->dwType = NTF_TYPE_NOTIFYICONDATA;
		info->info2.dwMessage = dwMsg;
		memcpy(&info->info2.nid, pNID, sizeof(PNOTIFYICONDATA));

		//modifica parametri pndAdd 
		//pndAdd->npPriority = SHNP_ICONIC;
	}

	if (g_pOrigShell_NotifyIconI)
    {
        	bResult = ((BOOL (*)(
				DWORD, PNOTIFYICONDATA, DWORD))g_pOrigShell_NotifyIconI)
				(
					dwMsg, 
					pNID, 
					cbNID
				);
    } //if (g_pOrigShell_NotifyIconI)

	if (bResult == FALSE)
		goto normal;

	if (!(IsWindow(g_hWndServerNotif)))
		goto normal;

	BOOL bInterceptRes =  (BOOL)SendMessage(g_hWndServerNotif, UWM_INTERCEPT_SHELLNOTIFICON, 0, 0);

	if (bInterceptRes) 
	{
		goto normal;
	}


normal:
    LOG(L"End HookShell_NotifyIconI .....\n");

    return bResult;
}
*/


__declspec(dllexport) 
BOOL InstallHookOnShell()
{
    static long s_lCount = 0;
    if (InterlockedIncrement(&s_lCount) > 1)
    {
		// no need to install again
		return TRUE;
	}

	BOOL bResult = TRUE;
	HANDLE    hDestProcess = NULL;
	HINSTANCE hDllInst = 0;

	int iAPISetId = SH_SHELL; 
	DWORD dwOldPermissions = 0;
	SetKMode(TRUE);
	dwOldPermissions = SetProcPermissions(-1);    
	__try
	{
		CINFO ** pSystemAPISets = (CINFO**)(UserKInfo[KINX_APISETS]);
		hDestProcess = pSystemAPISets[iAPISetId]->m_pProcessServer->hProc; // should be shell32.exe

		CALLBACKINFO cbi;
		ZeroMemory(&cbi, sizeof(CALLBACKINFO));
		cbi.m_hDestinationProcessHandle = hDestProcess;
		cbi.m_pFunction = (FARPROC)MapPtrToProcess(GetProcAddress(GetModuleHandle(L"COREDLL"), L"LoadLibraryW"), hDestProcess);
		cbi.m_pFirstArgument = (LPVOID)MapPtrToProcess(L"\\Windows\\FingerSuiteDll.dll", GetCurrentProcess());
		hDllInst = (HINSTANCE)PerformCallBack4(&cbi, 0,0,0); //returns the HINSTANCE from LoadLibraryW

		Sleep(1000);

		ZeroMemory(&cbi, sizeof(CALLBACKINFO));
		cbi.m_hDestinationProcessHandle = hDestProcess;
		cbi.m_pFunction = (FARPROC)MapPtrToProcess(GetProcAddress(hDllInst, L"StartHookOnShellServer"), hDestProcess);
		cbi.m_pFirstArgument = NULL; 
		DWORD dw = PerformCallBack4(&cbi, 0,0,0); //returns 1 if correctly executed		

		Sleep(1000);
	}
	__except(FilterException(GetExceptionInformation()))
	{
		bResult = FALSE;
	}
	if(dwOldPermissions)
	{
		SetProcPermissions(dwOldPermissions);
	}
	SetKMode(FALSE);

	return bResult;
}



__declspec(dllexport) 
BOOL StartHookOnShellServer()
{
	BOOL bResult = TRUE;

	int iAPISetId = SH_SHELL; 
    int iMethodIndex = SHELL_SHNotificationAddII;
	int iMethodIndex2 = SHELL_SHNotificationUpdateII;
	int iMethodIndex3 = SHELL_SHNotificationRemoveII;

    //HANDLE hCurProc = GetCurrentProcess();
	CINFO * pOrigApiSet; pOrigApiSet = NULL;
	
	DWORD dwOldPermissions = 0;
    SetKMode(TRUE);
    dwOldPermissions = SetProcPermissions(-1);    
	__try
	{
		CINFO ** pSystemAPISets = (CINFO**)(UserKInfo[KINX_APISETS]);
		pOrigApiSet = pSystemAPISets[iAPISetId];

		PFNVOID * pHookMthds = NULL;
		DWORD   * pdwHookSignatures = NULL;
		PFNVOID * pOrigMethods = (PFNVOID *)ConvertAddr(pOrigApiSet->m_ppMethods, pOrigApiSet->m_pProcessServer);
		DWORD   * pOrigSignatures = (DWORD *)ConvertAddr(pOrigApiSet->m_pdwMethodSignatures, pOrigApiSet->m_pProcessServer);

		m_pNewShellApiSet = (CINFO *)AllocateMemInKernelProc(sizeof(CINFO));

		int iNewNumMethods = pOrigApiSet->m_wNumMethods;


		// duplicate method table
		int iSize = (sizeof(PFNVOID) * iNewNumMethods + sizeof(DWORD) * iNewNumMethods);
		LPBYTE pRemoteMem = (LPBYTE)AllocateMemInKernelProc(iSize);

		pHookMthds = (PFNVOID *)pRemoteMem;
		pdwHookSignatures = (DWORD*)(pRemoteMem + sizeof(PFNVOID)*iNewNumMethods);
		
		for (int i = 0; i < iNewNumMethods; i++)
		{
			pHookMthds[i] = pOrigMethods[i];
			pdwHookSignatures[i] = pOrigSignatures? pOrigSignatures[i] : FNSIG0();
		}

		memcpy(m_pNewShellApiSet, pOrigApiSet, sizeof(CINFO));

		//But replace the method and signature tables:
		m_pNewShellApiSet->m_ppMethods = pHookMthds;
		m_pNewShellApiSet->m_pdwMethodSignatures = pdwHookSignatures;
		m_pNewShellApiSet->m_wNumMethods = (WORD)iNewNumMethods;

		// replace pointer with hooked procedure
		g_pOrigSHNotificationAddII = ((PFNVOID *)m_pNewShellApiSet->m_ppMethods)[iMethodIndex];
		((PFNVOID *)m_pNewShellApiSet->m_ppMethods)[iMethodIndex] = (PFNVOID)HookSHNotificationAddII;


		g_pOrigSHNotificationUpdateII = ((PFNVOID *)m_pNewShellApiSet->m_ppMethods)[iMethodIndex2];
		((PFNVOID *)m_pNewShellApiSet->m_ppMethods)[iMethodIndex2] = (PFNVOID)HookSHNotificationUpdateII;

		g_pOrigSHNotificationRemoveII = ((PFNVOID *)m_pNewShellApiSet->m_ppMethods)[iMethodIndex3];
		((PFNVOID *)m_pNewShellApiSet->m_ppMethods)[iMethodIndex3] = (PFNVOID)HookSHNotificationRemoveII;

		//And finally, replace the pointer to the original CINFO
		//by the pointer to ours.
		m_pOldShellApiSet = pSystemAPISets[iAPISetId];
		pSystemAPISets[iAPISetId] = m_pNewShellApiSet;

	}
	__except(FilterException(GetExceptionInformation()))
	{
		bResult = FALSE;
	}
    if(dwOldPermissions)
    {
        SetProcPermissions(dwOldPermissions);
    }
    SetKMode(FALSE);
	return bResult;
}

/////////////////////////////////// SIP BUTTON HOOK //////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
////// SIP Hook
LONG oldSIPBtnWindowProc;
DWORD dwSIPBtnStartTime;
DWORD dwSIPBtnInterval;

LRESULT CALLBACK newSIPBtnWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) 
{
	//LOG(L"Interceptiong SIP Button message...\n");
	BOOL bStartSIPMenu = FALSE;

	if (!(IsWindow(g_hWndServer)))
		goto normal;

	switch (uMsg){
	case WM_LBUTTONDOWN:
		dwSIPBtnStartTime = ::GetTickCount();
		break;
	case WM_LBUTTONUP:
		if ((::GetTickCount() - dwSIPBtnStartTime) > dwSIPBtnInterval)
			bStartSIPMenu = TRUE;
		break;
	default:
		break;
	}

normal:

	LRESULT lResult = CallWindowProc((WNDPROC)oldSIPBtnWindowProc, hwnd, uMsg, wParam, lParam);
	if (bStartSIPMenu)
	{
		RECT rc; ::GetClientRect(hwnd, &rc);
		LPARAM lParam = ((rc.bottom / 2) << 16) + (rc.right - 10);
		::PostMessage(hwnd, WM_LBUTTONDOWN, (WPARAM)0x00000001, lParam);
	}

	return lResult;
}

__declspec(dllexport) 
BOOL StartHookOnSIP()
{
	BOOL bResult = TRUE;
	HWND hWndSIPButton = ::FindWindow(L"MS_SIPBUTTON", NULL);
	if (hWndSIPButton != NULL)
	{
		HWND hWndToolbar = ::GetWindow(hWndSIPButton, GW_CHILD);
		if (hWndToolbar != NULL)
		{
			oldSIPBtnWindowProc = ::GetWindowLong(hWndToolbar, GWL_WNDPROC);
			if (::SetWindowLong(hWndToolbar, GWL_WNDPROC, (LONG)newSIPBtnWindowProc) == 0)
			{
				LOG(L"Errore StartHookOnSIP\n");
				return FALSE;
			}
		}
	}

	if (RegistryGetDWORD(HKEY_LOCAL_MACHINE, L"Software\\FingerMenu", L"SIPBtnPressInterval", &dwSIPBtnInterval) != S_OK)
		dwSIPBtnInterval = 500;

	LOG(L"StartHookOnSIP OK\n");
	return TRUE;
}

__declspec(dllexport) 
BOOL StopHookOnSIP()
{
	BOOL bResult = TRUE;
	HWND hWndSIPButton = ::FindWindow(L"MS_SIPBUTTON", NULL);
	if (hWndSIPButton != NULL)
	{
		HWND hWndToolbar = ::GetWindow(hWndSIPButton, GW_CHILD);
		if (hWndToolbar != NULL)
		{
			if (oldSIPBtnWindowProc != NULL)
				::SetWindowLong(hWndToolbar, GWL_WNDPROC, (LONG)oldSIPBtnWindowProc);
		}
	}

	LOG(L"StopHookOnSIP OK\n");
	return TRUE;
}


__declspec(dllexport) 
BOOL InstallHookOnSIP()
{
    static long s_lCount = 0;
    if (InterlockedIncrement(&s_lCount) > 1)
    {
		// no need to install again
		return TRUE;
	}

	BOOL bResult = TRUE;
	HANDLE    hDestProcess = NULL;
	HINSTANCE hDllInst = 0;

	DWORD dwOldPermissions = 0;
	SetKMode(TRUE);
	dwOldPermissions = SetProcPermissions(-1);    
	__try
	{
		HWND hWndSIPButton = ::FindWindow(L"MS_SIPBUTTON", NULL);
		DWORD dwProcessId; GetWindowThreadProcessId(hWndSIPButton, &dwProcessId);
		hDestProcess = (HANDLE)dwProcessId;

		CALLBACKINFO cbi;
		ZeroMemory(&cbi, sizeof(CALLBACKINFO));
		cbi.m_hDestinationProcessHandle = hDestProcess;
		cbi.m_pFunction = (FARPROC)MapPtrToProcess(GetProcAddress(GetModuleHandle(L"COREDLL"), L"LoadLibraryW"), hDestProcess);
		cbi.m_pFirstArgument = (LPVOID)MapPtrToProcess(L"\\Windows\\FingerSuiteDll.dll", GetCurrentProcess());
		hDllInst = (HINSTANCE)PerformCallBack4(&cbi, 0,0,0); //returns the HINSTANCE from LoadLibraryW

		Sleep(1000);

		ZeroMemory(&cbi, sizeof(CALLBACKINFO));
		cbi.m_hDestinationProcessHandle = hDestProcess;
		cbi.m_pFunction = (FARPROC)MapPtrToProcess(GetProcAddress(hDllInst, L"StartHookOnSIP"), hDestProcess);
		cbi.m_pFirstArgument = NULL; 
		DWORD dw = PerformCallBack4(&cbi, 0,0,0); //returns 1 if correctly executed		

		Sleep(1000);
	}
	__except(FilterException(GetExceptionInformation()))
	{
		bResult = FALSE;
	}
	if(dwOldPermissions)
	{
		SetProcPermissions(dwOldPermissions);
	}
	SetKMode(FALSE);

	return bResult;
}




/////////////////////////////////// TITLEBAR HOOK //////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
typedef enum {QVGA = 320, WQVGA = 400, VGA = 640, WVGA = 800} RESOLUTION;

int scaleFactor = 0;

LONG oldHHTaskbarWindowProc;

LRESULT CALLBACK newHHTaskbarWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) 
{
	switch (uMsg){
	case WM_LBUTTONDOWN:

		if (!(::IsWindow(g_hWndServerNotif)))
			break;

		RECT rc; ::GetClientRect(hwnd, &rc);
		::InflateRect(&rc, -27 * scaleFactor, 0); // only VGA
		POINT pt; pt.x  = LOWORD(lParam); pt.y = HIWORD(lParam);

		if (::PtInRect(&rc, pt))
		{
			::PostMessage(g_hWndServerNotif, UWM_NOTIF_POPUP, 0, 0);
			return 0;
		}
		break;
	default:
		break;
	}
	LRESULT lResult = CallWindowProc((WNDPROC)oldHHTaskbarWindowProc, hwnd, uMsg, wParam, lParam);

	return lResult;
}

__declspec(dllexport) 
BOOL StartHookOnHHTaskBar()
{
	BOOL bResult = TRUE;
	HWND hWndHHTaskbar = ::FindWindow(L"HHTaskBar", NULL);
	if (hWndHHTaskbar != NULL)
	{
		oldHHTaskbarWindowProc = ::GetWindowLong(hWndHHTaskbar, GWL_WNDPROC);
		if (::SetWindowLong(hWndHHTaskbar, GWL_WNDPROC, (LONG)newHHTaskbarWindowProc) == 0)
		{
			LOG(L"Errore StartHookOnHHTaskBar\n");
			return FALSE;
		}
	}

	// detect resolution
	RESOLUTION resolution = QVGA;
	DEVMODE dm;
	::ZeroMemory(&dm, sizeof(DEVMODE));
	dm.dmSize = sizeof(DEVMODE);
	if (EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dm))
	{
		resolution = (RESOLUTION)dm.dmPelsHeight;
	}

	// load icon images
	switch (resolution)
	{
	case QVGA:
	case WQVGA:
		scaleFactor = 1;
		break;
	case VGA:
	case WVGA:
		scaleFactor = 2;
		break;
	}

	LOG(L"Tutto OK StartHookOnHHTaskBar\n");
	return TRUE;
}

__declspec(dllexport) 
BOOL InstallHookOnHHTaskBar()
{
    static long s_lCount = 0;
    if (InterlockedIncrement(&s_lCount) > 1)
    {
		// no need to install again
		return TRUE;
	}

	BOOL bResult = TRUE;
	HANDLE    hDestProcess = NULL;
	HINSTANCE hDllInst = 0;

	DWORD dwOldPermissions = 0;
	SetKMode(TRUE);
	dwOldPermissions = SetProcPermissions(-1);    
	__try
	{
		HWND hWndHHTaskbar = ::FindWindow(L"HHTaskBar", NULL);
		DWORD dwProcessId; GetWindowThreadProcessId(hWndHHTaskbar, &dwProcessId);
		hDestProcess = (HANDLE)dwProcessId;

		CALLBACKINFO cbi;
		ZeroMemory(&cbi, sizeof(CALLBACKINFO));
		cbi.m_hDestinationProcessHandle = hDestProcess;
		cbi.m_pFunction = (FARPROC)MapPtrToProcess(GetProcAddress(GetModuleHandle(L"COREDLL"), L"LoadLibraryW"), hDestProcess);
		cbi.m_pFirstArgument = (LPVOID)MapPtrToProcess(L"\\Windows\\FingerSuiteDll.dll", GetCurrentProcess());
		hDllInst = (HINSTANCE)PerformCallBack4(&cbi, 0,0,0); //returns the HINSTANCE from LoadLibraryW

		Sleep(1000);

		ZeroMemory(&cbi, sizeof(CALLBACKINFO));
		cbi.m_hDestinationProcessHandle = hDestProcess;
		cbi.m_pFunction = (FARPROC)MapPtrToProcess(GetProcAddress(hDllInst, L"StartHookOnHHTaskBar"), hDestProcess);
		cbi.m_pFirstArgument = NULL; 
		DWORD dw = PerformCallBack4(&cbi, 0,0,0); //returns 1 if correctly executed		

		Sleep(1000);
	}
	__except(FilterException(GetExceptionInformation()))
	{
		bResult = FALSE;
	}
	if(dwOldPermissions)
	{
		SetProcPermissions(dwOldPermissions);
	}
	SetKMode(FALSE);

	return bResult;
}


__declspec(dllexport) 
BOOL IsManagedNotification(REFCLSID clsid)
{
	LPOLESTR pOleStr;
	StringFromCLSID(clsid, &pOleStr);

	LOG(L"IsManagedNotification notif = %s\n", (WCHAR*)pOleStr);
	
	BOOL bResult = FALSE;
	for (int i = 0; i < MANAGED_NOTIF_SIZE; i++)
	{
		if ( lstrcmpi(g_szManagedNotificationCLSID[i], (WCHAR*)pOleStr) == 0 )
		{
			bResult = TRUE;
			break;
		}
	}

	CoTaskMemFree(pOleStr);

	return bResult;
}
#ifndef _INTERCEPT_ENGINE_H_
#define _INTERCEPT_ENGINE_H_

//Redefinitions for some CE internal structures and undocumented APIs
#include "hook\SysDecls.h" 

#define UWM_INTERCEPT_MENU_MSG             _T("UWM_INTERCEPT_MENU_MSG-44E531B1_14D3_11d5_A025_006067718D04")
#define UWM_INTERCEPT_MSGBOX_MSG           _T("UWM_INTERCEPT_MSGBOX_MSG-44E531B1_14D3_11d5_A025_006067718D04")
#define UWM_INTERCEPT_NOTIF_MSG            _T("UWM_INTERCEPT_NOTIF_MSG-44E531B1_14D3_11d5_A025_006067718D04")
#define UWM_NOTIF_POPUP_MSG                _T("UWM_NOTIF_POPUP_MSG-44E531B1_14D3_11d5_A025_006067718D04")
#define EVT_FNGRMENU                       _T("FNGRMENUWAITEVT")
#define EVT_FNGRMSGBOX                     _T("FNGRMSGBOXWAITEVT")

#define NOTIF_ADD   1
#define NOTIF_UPD   2
#define NOTIF_DEL   3


/*
struct HookedAPI holds information for one system API, 
which is hooked by us.
*/
struct HookedAPI
{
    BOOL    m_bUsed;
    BOOL    m_bSwapped;
    int     m_iOrigApiSetId;
    CINFO * m_pOrigApiSet;
    CINFO * m_pOurApiSet;
};

struct ProcessInfo
{
    DWORD          dwID;
    DWORD          dwTrust;
    
    HINSTANCE      hSpyDllLoaded;
    WCHAR          szExeFile[_MAX_PATH];  //Points into array m_szExeFile
};

typedef struct tagTRACKPOPUPMENUINFO
{
    HMENU       hMenu;
    UINT        uFlags;
    HWND        hWnd;
} TRACKPOPUPMENUINFO, *LPTRACKPOPUPMENUINFO;

typedef struct tagMSGBOXINFO
{
	HWND        hWnd;
	UINT        uType;
	WCHAR       szCaption[128];
	WCHAR       szText[1024];
} MSGBOXINFO, *LPMSGBOXINFO;

typedef struct tagNOTIFICATIONINFO
{
	SHNOTIFICATIONDATA nd;
	WCHAR szTitle[256];
	WCHAR szHTML[512];
	SOFTKEYCMD rgskc[30];
	WCHAR sk1Title[50];
	WCHAR sk2Title[50];
	DWORD grfFlagsOriginal;
	BOOL isManaged;
} NOTIFICATIONINFO, *LPNOTIFICATIONINFO;

#define MANAGED_NOTIF_SIZE 7

WCHAR g_szManagedNotificationCLSID[MANAGED_NOTIF_SIZE][39] = 
			{
				L"{a877d65b-239c-47a7-9304-0d347f580408}", // new text message
				L"{a877d660-239c-47a7-9304-0d347f580408}", // missed call
				L"{f528c2e3-17da-4248-88dd-38c4cdcf0d20}", // customer experience
				L"{15f11f90-8a5f-454c-89fc-ba9b7aab0cad}", // reminder
				L"{a877d663-239c-47a7-9304-0d347f580408}",  // main battery very low
				L"{1ee011d2-f265-4214-a4ed-f0ea2e827f2a}",  // bluetooth utiliry
				L"{0d3132c4-1298-469c-b2b8-f28ce2d649d0}"  // bluetooth 1
			};


extern "C" __declspec(dllexport) BOOL SetHWndServer(HWND hWndServer);
extern "C" __declspec(dllexport) BOOL SetHWndServerMsgBox(HWND hWndServer);
extern "C" __declspec(dllexport) BOOL StartHookOnServer();
extern "C" __declspec(dllexport) BOOL StartHookOnShellServer();
extern "C" __declspec(dllexport) BOOL StopHookOnServer();
extern "C" __declspec(dllexport) BOOL InstallHook();
extern "C" __declspec(dllexport) BOOL InstallHookOnShell();
extern "C" __declspec(dllexport) BOOL RemoveHook();
extern "C" __declspec(dllexport) BOOL SignalWaitEvent(LPTSTR szEvt);
extern "C" __declspec(dllexport) void SetTrackPopupMenuExResult(int bResult);
extern "C" __declspec(dllexport) void SetMsgBoxResult(int iResult);

extern "C" __declspec(dllexport) BOOL SetHWndServerNotif(HWND hWndServer);

extern "C" __declspec(dllexport) BOOL StartHookOnSIP();
extern "C" __declspec(dllexport) BOOL StopHookOnSIP();
extern "C" __declspec(dllexport) BOOL InstallHookOnSIP();

extern "C" __declspec(dllexport) BOOL StartHookOnHHTaskBar();
extern "C" __declspec(dllexport) BOOL InstallHookOnHHTaskBar();

extern "C" __declspec(dllexport) BOOL IsManagedNotification(REFCLSID clsid);


#endif // _INTERCEPT_ENGINE_H_
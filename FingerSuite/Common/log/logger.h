#ifndef LOGGER_H
#define LOGGER_H

#pragma once

//#include <windows.h>

#ifndef __ATLMISC_H__
	#error logger.h requires atlmisc.h to be included first
#endif


//#ifdef __cplusplus
//extern "C" {
//#endif

//#define NOLOG


#ifndef NOLOG
	//#define LOG LogEventDebug
	#define LOG LogEventFile
#else
	#define LOG //
#endif

#define _INTERNAL_LOG_NAME	L"\\Storage Card\\fingersuite.log"



inline void WriteFileWChar(HANDLE hFile, TCHAR* buf)
{
	char cbuf[1025];
	DWORD w = wcslen(buf);
	w = WideCharToMultiByte(CP_ACP, 0, buf, w, cbuf, sizeof(cbuf), NULL, NULL);
	WriteFile(hFile, cbuf, w, (LPDWORD)&w, NULL);
}


inline void LogEventDebug(LPCTSTR str, ...)
{
	if (str == NULL) return;
	TCHAR buf[1100];
	va_list argptr;
	va_start(argptr, str);
	HRESULT ecode;
	if (FAILED(ecode=StringCchVPrintf(buf, 1100, str, argptr))) 
		StringCchPrintf(buf, 1100, L"StringCchVPrintf error: %d\n", ecode);
	va_end(argptr);
	OutputDebugString(buf);
}

inline void LogEventFile(LPCTSTR str, ...)
{
	static TCHAR* hLogFileName = 0;
	static HANDLE hLogFile = 0;

	if (str == NULL) return;

	if (!hLogFileName)
	{
		TCHAR buf[MAX_PATH+1];
		wcscpy(buf, L"");
		wcscat(buf, _INTERNAL_LOG_NAME);
		hLogFileName = (TCHAR*)malloc(sizeof(TCHAR)*wcslen(buf)+1);
		wcscpy(hLogFileName, buf);
		hLogFile = CreateFile(hLogFileName, GENERIC_WRITE | GENERIC_READ, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
		SetFilePointer(hLogFile, 0, NULL, FILE_END);
		LOG(L"New log begins...\n");
	}
	if (!hLogFile && hLogFileName)
	{
		hLogFile = CreateFile(hLogFileName, GENERIC_WRITE | GENERIC_READ, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
		SetFilePointer(hLogFile, 0, NULL, FILE_END);
	}

	if (hLogFile)
	{
		TCHAR buf[1025];
		SYSTEMTIME stime;

		GetLocalTime(&stime);
		wsprintf(buf, L"%02d-%02d-%02d %02d:%02d:%02d ",
			stime.wYear, stime.wMonth, stime.wDay, stime.wHour, stime.wMinute, stime.wSecond);
		WriteFileWChar(hLogFile, buf);

		va_list argptr;
		va_start(argptr, str);
		HRESULT ecode;
		if (FAILED(ecode=StringCchVPrintf(buf, 1100, str, argptr))) 
			StringCchPrintf(buf, 1100, L"StringCchVPrintf error: %d\n", ecode);
		va_end(argptr);
		WriteFileWChar(hLogFile, buf);

#ifdef LOG_EXCLUSIVE
		FlushFileBuffers(hLogFile);
#else
		CloseHandle(hLogFile);
		hLogFile = 0;
#endif
		OutputDebugString(buf); // log to screen
	}
}




inline void LogEventNull(LPCTSTR str, ...)
{
}



/////////////////////////////////////////////////

inline CString MifMaskString(DWORD f)
{
    CSimpleArray<CString> l;
	if (f&MIIM_CHECKMARKS) l.Add(L"MIIM_CHECKMARKS");
    if (f&MIIM_DATA      ) l.Add(L"MIIM_DATA");
    if (f&MIIM_ID        ) l.Add(L"MIIM_ID");
    if (f&MIIM_STATE     ) l.Add(L"MIIM_STATE");
    if (f&MIIM_SUBMENU   ) l.Add(L"MIIM_SUBMENU");
    if (f&MIIM_TYPE      ) l.Add(L"MIIM_TYPE");
    if (f&MIIM_FULLSTR   ) l.Add(L"MIIM_FULLSTR");

    DWORD all = MIIM_CHECKMARKS|MIIM_DATA|MIIM_ID|MIIM_STATE|MIIM_SUBMENU|MIIM_TYPE|MIIM_FULLSTR;
    if (f&~all) {
		CString a; a.Format(L"MASK_%08lx", f&~all);
		l.Add(a);
    }
	CString b;
	for (int i = 0; i < l.GetSize(); i++)
		b += l[i] + L",";
    return b;
}

inline CString MifTypeString(DWORD f)
{
    CSimpleArray<CString> l;
	if (f&MFT_MENUBARBREAK   ) l.Add(L"MFT_MENUBARBREAK");
    if (f&MFT_MENUBREAK      ) l.Add(L"MFT_MENUBREAK");
    if (f&MFT_OWNERDRAW      ) l.Add(L"MFT_OWNERDRAW");
    if (f&MFT_RADIOCHECK     ) l.Add(L"MFT_RADIOCHECK");
    if (f&MFT_SEPARATOR      ) l.Add(L"MFT_SEPARATOR");
    if (f == MFT_STRING         ) l.Add(L"MFT_STRING"); // MFT_STRING = 0

    DWORD all = MFT_MENUBARBREAK|MFT_MENUBREAK|MFT_OWNERDRAW|MFT_RADIOCHECK|MFT_SEPARATOR|MFT_STRING;
    if (f&~all) {
		CString a; a.Format(L"TYPE_%08lx", f&~all);
		l.Add(a);
    }
	CString b;
	for (int i = 0; i < l.GetSize(); i++)
		b += l[i] + L",";
    return b;
}

inline CString MifStateString(DWORD f)
{
    CSimpleArray<CString> l;
	if (f&MFS_CHECKED      ) l.Add(L"MFS_CHECKED");
    if (f&MFS_ENABLED      ) l.Add(L"MFS_ENABLED");
    if (f&MFS_HILITE       ) l.Add(L"MFS_HILITE");
    if (f&MFS_UNCHECKED    ) l.Add(L"MFS_UNCHECKED");
    if (f&MFS_UNHILITE     ) l.Add(L"MFS_UNHILITE");


    DWORD all = MFS_CHECKED|MFS_ENABLED|MFS_HILITE|MFS_UNCHECKED|MFS_UNHILITE;
    if (f&~all) {
		CString a; a.Format(L"TYPE_%08lx", f&~all);
		l.Add(a);
    }
	CString b;
	for (int i = 0; i < l.GetSize(); i++)
		b += l[i] + L",";
    return b;
}


inline void DUMP_MENUITEMINFO(MENUITEMINFO* mif)
{
	//LOG(L"\n\n");
	if (mif->cbSize != sizeof(MENUITEMINFO))
		LOG(L"NOTE: structsize = 0x%x ( expected 0x%x )\n", mif->cbSize, sizeof(MENUITEMINFO));
	LOG(L"fMask         : %s\n", MifMaskString(mif->fMask));
	LOG(L"fType         : %s\n", MifTypeString(mif->fType));
	LOG(L"fState        : %s\n", MifStateString(mif->fState));
	LOG(L"dwID          : %d (Ox%08lx)\n", mif->wID, mif->wID);
	LOG(L"hSubMenu      : Ox%08lx\n", mif->hSubMenu);
	LOG(L"hbmpChecked   : Ox%08lx\n", mif->hbmpChecked);
	LOG(L"hbmpUnchecked : Ox%08lx\n", mif->hbmpUnchecked);
	LOG(L"dwItemData    : %d (Ox%08lx)\n", mif->dwItemData, mif->dwItemData);
	//if (mif->fType == MFT_STRING)
		LOG(L"dwTypeData    : %s\n", mif->dwTypeData);
	//else
		LOG(L"dwTypeData    : %d (Ox%08lx)\n", mif->dwTypeData, mif->dwTypeData);
	LOG(L"cch           : %d\n", mif->cch);

}


inline CString NmFlagsString(DWORD f)
{
	CString b = ""; 

	if (f == SHNN_LINKSEL     )  b = L"SHNN_LINKSEL";
	if (f == SHNN_DISMISS     )  b = L"SHNN_DISMISS";   
	if (f == SHNN_SHOW        )  b = L"SHNN_SHOW";
	if (f == SHNN_NAVPREV     )  b = L"SHNN_NAVPREV";
	if (f == SHNN_NAVNEXT     )  b = L"SHNN_NAVNEXT";
	if (f == SHNN_ACTIVATE    )  b = L"SHNN_ACTIVATE";
	if (f == SHNN_ICONCLICKED )  b = L"SHNN_ICONCLICKED";
	if (f == SHNN_HOTKEY      )  b = L"SHNN_HOTKEY";

	return b;
}

inline void DUMP_NMSHN(NMSHN* nm)
{
	LOG(L"\n\n");
	LOG(L"hdr.dwID        : %08lx\n", nm->hdr.idFrom);
	LOG(L"hdr.hwndFrom    : %08lx\n", nm->hdr.hwndFrom);
	LOG(L"hdr.code        : %s\n", NmFlagsString(nm->hdr.code));
	LOG(L"lParam          : %08lx\n", nm->lParam);
	
	if (nm->hdr.code == SHNN_LINKSEL)
	{
		LOG(L"dwReturn        : %08lx\n", nm->dwReturn);
		LOG(L"pszLink         : %ls\n", nm->pszLink);
	}
	if (nm->hdr.code == SHNN_DISMISS)
		LOG(L"fTimeout        : %s\n", (nm->fTimeout == FALSE) ? L"FALSE" : L"TRUE");
	if (nm->hdr.code == SHNN_SHOW)
		LOG(L"pt              : (%d,%d)\n", nm->pt.x, nm->pt.y);
	LOG(L"\n\n");
}

inline CString ShnFlagsString(DWORD f)
{
    CSimpleArray<CString> l;
	if (f&SHNF_STRAIGHTTOTRAY) l.Add(L"SHNF_STRAIGHTTOTRAY");
    if (f&SHNF_CRITICAL      ) l.Add(L"SHNF_CRITICAL");
    if (f&SHNF_FORCEMESSAGE  ) l.Add(L"SHNF_FORCEMESSAGE");
    if (f&SHNF_DISPLAYON     ) l.Add(L"SHNF_DISPLAYON");
    if (f&SHNF_SILENT        ) l.Add(L"SHNF_SILENT");
    if (f&SHNF_HASMENU       ) l.Add(L"SHNF_HASMENU");
    if (f&SHNF_TITLETIME     ) l.Add(L"SHNF_TITLETIME");
    if (f&SHNF_SPINNERS      ) l.Add(L"SHNF_SPINNERS");
    if (f&SHNF_ALERTONUPDATE ) l.Add(L"SHNF_ALERTONUPDATE");
    if (f&SHNF_WANTVKTTALK   ) l.Add(L"SHNF_WANTVKTTALK");

    DWORD all=SHNF_STRAIGHTTOTRAY|SHNF_CRITICAL|SHNF_FORCEMESSAGE|SHNF_DISPLAYON|SHNF_SILENT|SHNF_HASMENU|SHNF_TITLETIME|SHNF_SPINNERS|SHNF_ALERTONUPDATE|SHNF_WANTVKTTALK;
    if (f&~all) {
		CString a; a.Format(L"SHNFLAG_%08lx", f&~all);
		l.Add(a);
    }
	CString b;
	for (int i = 0; i < l.GetSize(); i++)
		b += l[i] + L",";
    return b;
}

inline CString SkFlagsString(DWORD f)
{
    CSimpleArray<CString> l;
	if (f & NOTIF_SOFTKEY_FLAGS_DISMISS) l.Add(L"NOTIF_SOFTKEY_FLAGS_DISMISS");
    if (f & NOTIF_SOFTKEY_FLAGS_HIDE) l.Add(L"NOTIF_SOFTKEY_FLAGS_HIDE");
    if (f & NOTIF_SOFTKEY_FLAGS_STAYOPEN) l.Add(L"NOTIF_SOFTKEY_FLAGS_STAYOPEN");
    if (f & NOTIF_SOFTKEY_FLAGS_SUBMIT_FORM) l.Add(L"NOTIF_SOFTKEY_FLAGS_SUBMIT_FORM");
    if (f & NOTIF_SOFTKEY_FLAGS_DISABLED) l.Add(L"NOTIF_SOFTKEY_FLAGS_DISABLED");

	CString b;
	for (int i = 0; i < l.GetSize(); i++)
		b += l[i] + L",";
    return b;
}

inline void DUMP_SHN(SHNOTIFICATIONDATA* shn)
{
	LOG(L"\n\n");
	if (shn->cbStruct!=sizeof(*shn))
        LOG(L"NOTE: structsize = 0x%x ( expected 0x%x )\n", shn->cbStruct, sizeof(*shn));
    LOG(L"dwId      : %08lx\n", shn->dwID);
    LOG(L"npPriority: %s\n", shn->npPriority==SHNP_INFORM ? L"INFORM" : shn->npPriority==SHNP_ICONIC ? L"ICONIC" : L"<unknown>");
    LOG(L"csDuration: %d\n", shn->csDuration);
    LOG(L"hicon     : %08lx\n", shn->hicon);
    LOG(L"grfFlags  : %s\n", ShnFlagsString(shn->grfFlags));
    LOG(L"clsid     : %08lx-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x\n", 
            shn->clsid.Data1, shn->clsid.Data2, shn->clsid.Data3,
            shn->clsid.Data4[0], shn->clsid.Data4[1], shn->clsid.Data4[2], shn->clsid.Data4[3], shn->clsid.Data4[4], shn->clsid.Data4[5], shn->clsid.Data4[6], shn->clsid.Data4[7]);
    LOG(L"hwndSink  : %08lx\n", shn->hwndSink);
    LOG(L"pszHTML   : %s\n", shn->pszHTML);
    LOG(L"pszTitle  : %s\n", shn->pszTitle);
    LOG(L"lParam    : %08lx\n", shn->lParam);
    if (shn->grfFlags & SHNF_HASMENU) {
        LOG(L"skm       : \n");
        LOG(L"    hMenu    : %08lx\n", shn->skm.hMenu);
        LOG(L"    cskc     : %d\n", shn->skm.cskc);
        for (DWORD i=0 ; i<shn->skm.cskc ; i++)
            LOG(L"    prgskc[%2d]   : %d %s\n", i, shn->skm.prgskc[i].wpCmd, SkFlagsString(shn->skm.prgskc[i].grfFlags));

		int i = 0;
		MENUITEMINFO mii;
		WCHAR szLabel[256];
		while (1)
		{
			ZeroMemory(&mii, sizeof(MENUITEMINFO));
			mii.cbSize = sizeof(MENUITEMINFO);
			mii.fMask = MIIM_CHECKMARKS | MIIM_DATA | MIIM_ID | MIIM_STATE | MIIM_SUBMENU | MIIM_TYPE | MIIM_FULLSTR;
			mii.dwTypeData = (LPTSTR)szLabel;
			mii.cch = 256;
			if (GetMenuItemInfo(shn->skm.hMenu, i, TRUE, &mii))
			{
				LOG(L"----- menuiteminfo------\n");
				DUMP_MENUITEMINFO(&mii);    
				LOG(L"------------------------\n");

				if (mii.hSubMenu)
				{
					int j = 0;
					MENUITEMINFO mii2;
					WCHAR szLabel2[256];
					while (1) 
					{
						ZeroMemory(&mii2, sizeof(MENUITEMINFO));
						mii2.cbSize = sizeof(MENUITEMINFO);
						mii2.fMask = MIIM_CHECKMARKS | MIIM_DATA | MIIM_ID | MIIM_STATE | MIIM_SUBMENU | MIIM_TYPE | MIIM_FULLSTR;
						mii2.dwTypeData = (LPTSTR)szLabel2;
						mii2.cch = 256;
						if (GetMenuItemInfo(mii.hSubMenu, j, TRUE, &mii2))
						{
							LOG(L"----- sub menuiteminfo------\n");
							DUMP_MENUITEMINFO(&mii2);    
							LOG(L"----------------------------\n");
						}
						else
							break;
						j++;
					}
				}
			}
			else
				break;
			i++;
		}
    }
    else 
	{
        for (int i=0 ; i<2 ; i++) {
            LOG(L"rgskn%d    : \n", i);
            LOG(L"    pszTitle    : %s\n", shn->rgskn[i].pszTitle);
            LOG(L"    wpCmd       : %d\n", shn->rgskn[i].skc.wpCmd);
            LOG(L"    grfFlags    : %s\n", SkFlagsString(shn->rgskn[i].skc.grfFlags));
        }
    }
    LOG(L"pszTodaySK: %ls\n",   shn->pszTodaySK);
    LOG(L"pszTodayExec: %ls\n", shn->pszTodayExec);

}





//#ifdef __cplusplus
//}
//#endif

#endif // LOGGER_H
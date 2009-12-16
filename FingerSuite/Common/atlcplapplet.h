#ifndef __ATLCPLAPPLET_H__
#define __ATLCPLAPPLET_H__

/////////////////////////////////////////////////////////////////////////////
// atlcplapplet.h - Control Panel applet
//
// Written by Bjarke Viksoe (bjarke@viksoe.dk)
// Copyright (c) 2007 Bjarke Viksoe.
//
// This code may be used in compiled form in any way you desire. This
// source file may be redistributed by any means PROVIDING it is 
// not sold for profit without the authors written consent, and 
// providing that this notice and the authors name is included. 
//
// This file is provided "as is" with no expressed or implied warranty.
// The author accepts no liability if it causes any damage to you or your
// computer whatsoever. It's free, so don't hassle me about it.
//
// Beware of bugs.
//

#pragma once

#ifndef __cplusplus
   #error ATL requires C++ compilation (use a .cpp suffix)
#endif

#ifndef __ATLAPP_H__
   #error atlcplapplet.h requires atlapp.h to be included first
#endif


#include <cpl.h>


typedef struct tagATLCPLAPPLETREG
{
   int iIndex;
   BOOL bOnlyCurrentUser;
   DWORD dwCategory;
   DWORD dwRunLevel;
   UINT uTaskRegID;
} ATLCPLAPPLETREG;


#define DECLARE_APPLET_CATEGORY(x)  enum { APPLET_CATEGORY = x };

#define DECLARE_APPLET_RUNLEVEL(x)  enum { APPLET_RUNLEVEL = x };

#define DECLARE_APPLET_RESOURCEID(x)  enum { APPLET_TASK_REGISTRYID = x };


typedef struct tagATLCPLAPPLETINFO
{
   LPCTSTR pstrCanonicalName;
   UINT uNameRes;
   UINT uInfoRes;
   UINT uTaskRes;
   UINT uKeywordsRes;
   UINT uIconRes;
   BOOL bDynamic;
   DWORD dwFlags;
   ULONG_PTR lData;
   DWORD dwHelpContext;
   HRESULT (*pfnUpdateRegistry)(int iIndex, BOOL bRegister);
   BOOL (*pfnCreateInstance)(HWND hWnd, LONG_PTR lData, LPCTSTR pstrData);
   VOID (*pfnGetAppletInfo)(int iLevel, tagATLCPLAPPLETINFO* pInfo);
} ATLCPLAPPLETINFO;

extern ATLCPLAPPLETINFO _pAtlApplets[];


#define BEGIN_CPLAPPLET_MAP()  \
   ATLCPLAPPLETINFO _pAtlApplets[] = {

#define CPLAPPLET_ENTRY(applclass, cname, name)  \
   { _T(cname), IDS_##name, IDS_Description_##name, IDS_Task_##name, IDS_Keywords_##name, IDI_##name, FALSE, 0, 0L, 0, &applclass::UpdateRegistry, &applclass::CreateInstance, &applclass::GetAppletInfo },

#define CPLAPPLET_ENTRY_EX(applclass, cname, name, info, task, keywords, icon, flags, data, helpctx)  \
   { _T(cname), name, info, task, keywords, icon, TRUE, flags, data, helpctx, &applclass::UpdateRegistry, &applclass::CreateInstance, &applclass::GetAppletInfo },

#define CPLAPPLET_ENTRY_CLASS(applclass)  \
   { NULL, CPL_DYNAMIC_RES, CPL_DYNAMIC_RES, 0, 0, CPL_DYNAMIC_RES, TRUE, 0, 0L, 0, &applclass::UpdateRegistry, &applclass::CreateInstance, &applclass::GetAppletInfo },

#define END_CPLAPPLET_MAP()  \
   { 0 } };


typedef enum CPLAPPLETINFO
{
   CPLINFO_NAME = 1,
   CPLINFO_INFO,
   CPLINFO_ICON,
   CPLINFO_DATA,
   CPLINFO_HELPCONTEXT,
};


template< class T >
class CCPlAppletBase
{
public:
   enum { APPLET_CATEGORY = 0x05 };
   enum { APPLET_RUNLEVEL = 0x00 };
   enum { APPLET_TASK_REGISTRYID = 0 };



   static HRESULT UpdateRegistry(int iIndex, BOOL bRegister)
   {
      ATLCPLAPPLETREG RegInfo = { iIndex, FALSE, T::APPLET_CATEGORY, T::APPLET_RUNLEVEL, T::APPLET_TASK_REGISTRYID };
      return _Applets.UpdateRegistry(bRegister, &RegInfo);
   }


   static BOOL CreateInstance(HWND hWnd, LONG_PTR lData, LPCTSTR pstrData)
   {
      T* pObject = new T();
      if( pObject == NULL ) return FALSE;
      BOOL bRes = pObject->ShowApplet(hWnd, lData, pstrData);
      delete pObject;
      return bRes;
   }

   static VOID GetAppletInfo(int /*iLevel*/, ATLCPLAPPLETINFO* /*pInfo*/)
   {
   }
};


class CCPlAppletModule
{
public:
   HWND m_hwndCPl;

   CCPlAppletModule() : m_hwndCPl(NULL)
   {
   }

   LONG CPlApplet(HWND hwndCPl, UINT msg, LPARAM lParam1, LPARAM lParam2)
   {
      m_hwndCPl = hwndCPl;
      switch( msg ) {
      case CPL_INIT:
         CPlApplet_Init();
         return 1;
      case CPL_EXIT:
         CPlApplet_Exit();
         return 0;
      case CPL_GETCOUNT:         
         return CPlApplet_GetCount();
      case CPL_DBLCLK:
         CPlApplet_DblClick((int) lParam1, hwndCPl);
         return 0;
#ifndef _WIN32_WCE
      case CPL_INQUIRE:
         CPlApplet_Inquire((int) lParam1, (CPLINFO*) lParam2);
         return 0;
#endif // !_WIN32_WCE
      case CPL_NEWINQUIRE:
         CPlApplet_NewInquire((int) lParam1, (NEWCPLINFO*) lParam2);
         return 0;
#ifndef _WIN32_WCE
#if (WINVER >= 0x0400)
      case CPL_STARTWPARMSA:
         {
#ifdef _UNICODE
            WCHAR wszCommand[100] = { 0 };
            ::MultiByteToWideChar(CP_ACP, 0, (LPCSTR) lParam2, -1, wszCommand, (sizeof(wszCommand) / sizeof(WCHAR)) -1);
            LPCWSTR pstrCommand = wszCommand;
#else
            LPCSTR pstrCommand = (LPCSTR) lParam2;
#endif  // _UNICODE
            return (LONG) CPlApplet_StartWithParams((int) lParam1, hwndCPl, pstrCommand);
         }
      case CPL_STARTWPARMSW:
         {
#ifdef _UNICODE
            LPCWSTR pstrCommand = (LPCWSTR) lParam2;
#else
            CHAR szCommand[200] = { 0 };
            ::WideCharToMultiByte(CP_ACP, 0, (LPCWSTR) lParam2, -1, szCommand, sizeof(szCommand) - 1, NULL, NULL);
            LPCSTR pstrCommand = szCommand;
#endif // _UNICODE
            return (LONG) CPlApplet_StartWithParams((int) lParam1, hwndCPl, pstrCommand);
         }
#endif // (WINVER >= 0x0400)
#endif // !_WIN32_WCE
      case CPL_STOP:
         CPlApplet_Stop((int) lParam1);
         return 0;
      }
      return 0;
   }

   HRESULT RegisterApplets()
   {
      HRESULT Hr = S_OK;
#ifndef _WIN32_WCE
      int iIndex = 0;
      for( const ATLCPLAPPLETINFO* pObjects = _pAtlApplets; (*pObjects).pfnCreateInstance != NULL; pObjects++ ) {
         if( FAILED( Hr = pObjects->pfnUpdateRegistry(iIndex++, TRUE) ) ) return Hr;
      }
#endif // !_WIN32_WCE
      return Hr;
   }

   HRESULT UnregisterApplets()
   {
      HRESULT Hr = S_OK, HrRes = S_OK;
#ifndef _WIN32_WCE
      int iIndex = 0;
      for( const ATLCPLAPPLETINFO* pObjects = _pAtlApplets; (*pObjects).pfnCreateInstance != NULL; pObjects++ ) {
         if( FAILED( Hr = pObjects->pfnUpdateRegistry(iIndex++, FALSE) ) ) HrRes = Hr;
      }
#endif // !_WIN32_WCE
      return HrRes;
   }

   HWND GetOwnerHWND()
   {
      return m_hwndCPl;
   }

   BOOL ChangeAppletInfo(int iIndex, CPLAPPLETINFO Type, UINT uResID)
   {
      ATLASSERT(iIndex>=0 && iIndex<CPlApplet_GetCount());
      if( iIndex < 0 || iIndex >= CPlApplet_GetCount() ) return FALSE;
      switch( Type ) {
      case CPLINFO_NAME:         _pAtlApplets[iIndex].uNameRes = uResID; break;
      case CPLINFO_INFO:         _pAtlApplets[iIndex].uInfoRes = uResID; break;
      case CPLINFO_ICON:         _pAtlApplets[iIndex].uIconRes = uResID; break;
      case CPLINFO_DATA:         _pAtlApplets[iIndex].lData = (LONG_PTR) uResID; break;
      case CPLINFO_HELPCONTEXT:  _pAtlApplets[iIndex].dwHelpContext = uResID; break;
      default: return FALSE;
      }
      _pAtlApplets[iIndex].bDynamic = TRUE;
      return TRUE;     
   }

   HRESULT UpdateRegistry(BOOL bRegister, ATLCPLAPPLETREG* pRegInfo)
   {
#ifndef _WIN32_WCE
      OSVERSIONINFO osvi = { 0 };;
      osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
      ::GetVersionEx(&osvi);
      bool bIsVista = (osvi.dwMajorVersion >= 6);
      bool bIsWin98 = ((osvi.dwPlatformId != VER_PLATFORM_WIN32_WINDOWS && osvi.dwMajorVersion >= 5) || (osvi.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS && (osvi.dwMinorVersion >= 90 || osvi.dwMajorVersion > 4)));
      bool bIsWinXP = ((osvi.dwPlatformId != VER_PLATFORM_WIN32_WINDOWS) && ((osvi.dwMajorVersion > 5) || ((osvi.dwMajorVersion == 5) && (osvi.dwMinorVersion >= 1))));

      TCHAR szDllFile[MAX_PATH] = { 0 };
      ::GetModuleFileName(ModuleHelper::GetModuleInstance(), szDllFile, MAX_PATH);

      ATLCPLAPPLETINFO Info = _pAtlApplets[pRegInfo->iIndex];
      _pAtlApplets[pRegInfo->iIndex].pfnGetAppletInfo(1, &Info);

      // Create a friendly name for the dll filename. No spaces and funny characters.
      LPCTSTR pSep = _FindLastOf(szDllFile, '\\');
      if( pSep == NULL ) return E_FAIL;
      TCHAR szDllName[MAX_PATH] = { 0 };
      ::lstrcpy(szDllName, pSep + 1);
      TCHAR szFriendlyName[MAX_PATH] = { 0 };
      ::lstrcpy(szFriendlyName, pSep + 1);
      for( LPTSTR p = szFriendlyName; *p != '\0'; p = ::CharNext(p) ) {
         if( *p == '.' ) { *p = '\0'; break; }
         if( !IsCharAlphaNumeric(*p) ) *p = '_';
      }

      HRESULT Hr = S_OK;
      if( bIsVista )
      {
         HKEY hKeyRoot = HKEY_LOCAL_MACHINE;
         if( pRegInfo->bOnlyCurrentUser ) hKeyRoot = HKEY_CURRENT_USER;
         CRegKey reg;         
         LONG lRes = reg.Create(hKeyRoot, _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Control Panel\\Cpls"));
         if( lRes != ERROR_SUCCESS ) return HRESULT_FROM_WIN32(lRes);
         if( bRegister ) {
            ::RegSetValueEx(reg, szFriendlyName, NULL, REG_EXPAND_SZ, (CONST BYTE*) szDllFile, (lstrlen(szDllFile) + 1) * sizeof(TCHAR));
            if( pRegInfo->dwRunLevel > 0 ) {
               CRegKey regSub;
               if( regSub.Create(reg, szDllName) == ERROR_SUCCESS ) {
                  ::RegSetValueEx(regSub, _T("RunLevel"), NULL, REG_DWORD, (CONST BYTE*) &pRegInfo->dwRunLevel, sizeof(DWORD));
                  regSub.Close();
               }
            }
         }
         else {
            reg.DeleteValue(szFriendlyName);
            reg.DeleteSubKey(szDllName);
         }
         reg.Close();
         
         // Register canonical name
         lRes = reg.Open(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Control Panel\\Extended Properties\\System.ApplicationName"), KEY_ALL_ACCESS);
         if( lRes != ERROR_SUCCESS ) return HRESULT_FROM_WIN32(lRes);
         if( bRegister ) {
              ::RegSetValueEx(reg, szDllFile, NULL, REG_SZ, (CONST BYTE*) Info.pstrCanonicalName, (lstrlen(Info.pstrCanonicalName) + 1) * sizeof(TCHAR));
         }
         else {
            reg.DeleteValue(szDllFile);
         }
         reg.Close();

         TCHAR szGUID[50] = { 0 };
         DWORD dwCategory = pRegInfo->dwCategory;
         TCHAR szTaskFileUrl[MAX_PATH + 20] = { 0 };

         if( pRegInfo->uTaskRegID == 0 )
         {
            GUID AppGUID, TaskGUID;
            ::CoCreateGuid(&AppGUID);
            ::CoCreateGuid(&TaskGUID);
            WCHAR wszGUID[50], wszTaskGUID[50];
            ::StringFromGUID2(AppGUID, wszGUID, 50);
            ::StringFromGUID2(TaskGUID, wszTaskGUID, 50);
            WCHAR wszBuffer[1025] = { 0 };
            ::wsprintfW(wszBuffer, 
               L"\xfeff<?xml version=\"1.0\" encoding=\"UTF-16\" ?>\n"
               L"<applications xmlns=\"http://schemas.microsoft.com/windows/cpltasks/v1\" xmlns:sh=\"http://schemas.microsoft.com/windows/tasks/v1\">"
               L"<application id=\"%s\">"
               L"<sh:task id=\"%s\" needsElevation=\"%s\">"
#ifdef _UNICODE
               L"<sh:name>@%s,-%ld</sh:name>"
               L"<sh:keywords>@%s,-%ld</sh:keywords>"
               L"<sh:command>%%systemroot%%\\system32\\control.exe /name %s</sh:command>"
#else
               L"<sh:name>@%hs,-%ld</sh:name>"
               L"<sh:keywords>@%hs,-%ld</sh:keywords>"
               L"<sh:command>%%systemroot%%\\system32\\control.exe /name %hs</sh:command>"
#endif // _UNICODE
               L"</sh:task>"
               L"<category id=\"%ld\">"
               L"<sh:task idref=\"%s\"/>"
               L"</category>"
               L"</application>"
               L"</applications>",
               wszGUID,
               wszTaskGUID,
               pRegInfo->dwRunLevel == 0 ? L"false" : L"true",
               szDllName, Info.uTaskRes,
               szDllName, Info.uKeywordsRes,
               Info.pstrCanonicalName,
               pRegInfo->dwCategory,
               wszTaskGUID);
            ::wsprintf(szGUID, _T("%ws"), wszGUID);
            ::lstrcpy(szTaskFileUrl, szDllFile);
            ::wsprintf(_FindLastOf(szTaskFileUrl, '.'), _T("_cpl%d.xml"), pRegInfo->iIndex + 1);
            if( bRegister ) {
               HANDLE hFile = ::CreateFile(szTaskFileUrl, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
               if( hFile == INVALID_HANDLE_VALUE ) return HRESULT_FROM_WIN32(::GetLastError());
               DWORD dwWritten = 0;
               ::WriteFile(hFile, wszBuffer, lstrlenW(wszBuffer) * sizeof(WCHAR), &dwWritten, NULL);
               ::CloseHandle(hFile);
            }
            else {
               ::DeleteFile(szTaskFileUrl);
            }
         }
         else
         {
            // Load the XML resource.
            // Here we assume it is UTF-8 encoded (or latin-1) and embedded in the "XML" resource
            // type section.
            HRSRC hResource = ::FindResource(ModuleHelper::GetResourceInstance(), MAKEINTRESOURCE(pRegInfo->uTaskRegID), _T("XML"));
            if( hResource == NULL ) return TYPE_E_FIELDNOTFOUND;
            HGLOBAL hGlobal = ::LoadResource(ModuleHelper::GetResourceInstance(), hResource);
            if( hGlobal == NULL ) return E_OUTOFMEMORY;
            DWORD dwSize = ::SizeofResource(ModuleHelper::GetResourceInstance(), hResource);
            LPVOID pVoid = ::LockResource(hGlobal);
            LPSTR pstr = (LPSTR) malloc(dwSize + 1);
            memcpy(pstr, pVoid, dwSize);
            FreeResource(hGlobal);
            pstr[dwSize] = '\0';
            LPSTR pstrCategory = strstr(pstr, "<category id=\"");
            if( pstrCategory != NULL ) dwCategory = (DWORD) atol(pstrCategory + 14);
            LPSTR pstrGUID = strstr(pstr, "<application id=\"");
            if( pstrGUID == NULL ) return TYPE_E_ELEMENTNOTFOUND;
            BYTE bGUID[50] = { 0 };
            memcpy(bGUID, pstrGUID + 17, 38);
            ::wsprintf(szGUID, _T("%hs"), bGUID);
            ::wsprintf(szTaskFileUrl, _T("%s,-%ld"), szDllFile, pRegInfo->uTaskRegID);
            free(pstr);
         }

         // Set the ApplicationName (canonical name)
         lRes = reg.Open(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Control Panel\\Extended Properties\\System.Software.AppId"), KEY_ALL_ACCESS);
         if( lRes != ERROR_SUCCESS ) return HRESULT_FROM_WIN32(lRes);
         if( bRegister ) {
              ::RegSetValueEx(reg, szDllFile, NULL, REG_SZ, (CONST BYTE*) szGUID, (lstrlen(szGUID) + 1) * sizeof(TCHAR));
         }
         else {
            reg.DeleteValue(szDllFile);
         }
         reg.Close();
         
         // Set the TaskFileUrl
         lRes = reg.Open(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Control Panel\\Extended Properties\\System.Software.TasksFileUrl"), KEY_ALL_ACCESS);
         if( lRes != ERROR_SUCCESS ) return HRESULT_FROM_WIN32(lRes);
         if( bRegister ) {
              ::RegSetValueEx(reg, szDllFile, NULL, REG_SZ, (CONST BYTE*) szTaskFileUrl, (lstrlen(szTaskFileUrl) + 1) * sizeof(TCHAR));
         }
         else {
            reg.DeleteValue(szDllFile);
         }
         reg.Close();

         // Set the Category
         lRes = reg.Open(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Control Panel\\Extended Properties\\System.ControlPanel.Category"), KEY_ALL_ACCESS);
         if( lRes != ERROR_SUCCESS ) return HRESULT_FROM_WIN32(lRes);
         if( bRegister ) {
            ::RegSetValueEx(reg, szDllFile, NULL, REG_DWORD, (CONST BYTE*) &dwCategory, sizeof(DWORD));
         }
         else {
            reg.DeleteValue(szDllFile);
         }
         reg.Close();
      }
      else if( bIsWin98 )
      {
         HKEY hKeyRoot = HKEY_LOCAL_MACHINE;
         if( pRegInfo->bOnlyCurrentUser ) hKeyRoot = HKEY_CURRENT_USER;
         CRegKey reg;
         LONG lRes = reg.Create(hKeyRoot, _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Control Panel\\Cpls"));
         if( lRes != ERROR_SUCCESS ) return HRESULT_FROM_WIN32(lRes);
         if( bRegister ) {
            ::RegSetValueEx(reg, szFriendlyName, NULL, REG_EXPAND_SZ, (CONST BYTE*) szDllFile, (lstrlen(szDllFile) + 1) * sizeof(TCHAR));
         }
         else {
            reg.DeleteValue(szFriendlyName);
         }
         reg.Close();

         // Set the Category on Windows XP
         if( bIsWinXP ) {
            lRes = reg.Create(HKEY_LOCAL_MACHINE, _T("Software\\Microsoft\\Windows\\CurrentVersion\\Control Panel\\Extended Properties\\{305CA226-D286-468e-B848-2B2E8E697B74} 2"));
            if( lRes == ERROR_SUCCESS ) {
               if( bRegister ) {
                  ::RegSetValueEx(reg, szDllFile, NULL, REG_DWORD, (CONST BYTE*) &pRegInfo->dwCategory, sizeof(DWORD));
               }
               else {
                  reg.DeleteValue(szDllFile);
               }
            }
         }
      }
      else
      {
        BOOL bRes = ::WritePrivateProfileString(_T("MMCPL"), szFriendlyName, (bRegister ? szDllFile : NULL), _T("control.ini"));    
        if( !bRes ) Hr = HRESULT_FROM_WIN32(::GetLastError());
      }
       return Hr;
#else
	   return S_OK;
#endif // !_WIN32_WCE

   }

   void CPlApplet_Init()
   {
   }

   void CPlApplet_Exit()
   {
   }

   void CPlApplet_Stop(int iIndex)
   {
      ATLASSERT(iIndex>=0 && iIndex<CPlApplet_GetCount());
      if( iIndex < 0 || iIndex >= CPlApplet_GetCount() ) return;
   }

   void CPlApplet_DblClick(int iIndex, HWND hWnd)
   {
      ATLASSERT(iIndex>=0 && iIndex<CPlApplet_GetCount());
      if( iIndex < 0 || iIndex >= CPlApplet_GetCount() ) return;
      LONG_PTR lData = _pAtlApplets[iIndex].lData;
      _pAtlApplets[iIndex].pfnCreateInstance(hWnd, lData, _T(""));
   }

#ifndef _WIN32_WCE
   BOOL CPlApplet_StartWithParams(int iIndex, HWND hWnd, LPCTSTR pstrText)
   {
      ATLASSERT(iIndex>=0 && iIndex<CPlApplet_GetCount());
      if( iIndex < 0 || iIndex >= CPlApplet_GetCount() ) return FALSE;
      LONG_PTR lData = _pAtlApplets[iIndex].lData;
      return _pAtlApplets[iIndex].pfnCreateInstance(hWnd, lData, pstrText);
   }
#endif // !_WIN32_WCE

   LONG CPlApplet_GetCount()
   {
      LONG nCount = 0;
      const ATLCPLAPPLETINFO* pObjects = _pAtlApplets;
      while( (*pObjects).pfnCreateInstance != NULL ) pObjects++, nCount++;
      return nCount;
   }

#ifndef _WIN32_WCE
   void CPlApplet_Inquire(int iIndex, CPLINFO* pInfo)
   {
      ATLASSERT(iIndex>=0 && iIndex<CPlApplet_GetCount());
      if( iIndex < 0 || iIndex >= CPlApplet_GetCount() ) return;
      _pAtlApplets[iIndex].pfnGetAppletInfo(0, &_pAtlApplets[iIndex]);
      pInfo->idIcon = _pAtlApplets[iIndex].uIconRes;
      pInfo->idInfo = _pAtlApplets[iIndex].uInfoRes;
      pInfo->idName = _pAtlApplets[iIndex].uNameRes;
      pInfo->lData = _pAtlApplets[iIndex].lData;
   }
#endif // !_WIN32_WCE

   void CPlApplet_NewInquire(int iIndex, NEWCPLINFO* pInfo)
   {
      ATLASSERT(iIndex>=0 && iIndex<CPlApplet_GetCount());
      if( iIndex < 0 || iIndex >= CPlApplet_GetCount() ) return;
      _pAtlApplets[iIndex].pfnGetAppletInfo(1, &_pAtlApplets[iIndex]);
      ::ZeroMemory(pInfo, sizeof(NEWCPLINFO));
      pInfo->dwSize = sizeof(NEWCPLINFO);
      ::LoadString(ModuleHelper::GetResourceInstance(), _pAtlApplets[iIndex].uNameRes, pInfo->szName, sizeof(pInfo->szName) / sizeof(TCHAR));
      ::LoadString(ModuleHelper::GetResourceInstance(), _pAtlApplets[iIndex].uInfoRes, pInfo->szInfo, sizeof(pInfo->szInfo) / sizeof(TCHAR));
      pInfo->hIcon = ::LoadIcon(ModuleHelper::GetResourceInstance(), MAKEINTRESOURCE(_pAtlApplets[iIndex].uIconRes));
      pInfo->dwFlags = _pAtlApplets[iIndex].dwFlags;
      pInfo->dwHelpContext = _pAtlApplets[iIndex].dwHelpContext;
      pInfo->lData = _pAtlApplets[iIndex].lData;
   }

   LPTSTR _FindLastOf(LPTSTR pstr, TCHAR ch) const
   {
      LPTSTR pSep = NULL;
      for( LPTSTR p = pstr; *p != '\0'; p = ::CharNext(p) ) {
         if( *p == ch ) pSep = p;
      }
      return pSep;
   }
};

extern CCPlAppletModule _Applets;



#endif __ATLCPLAPPLET_H__

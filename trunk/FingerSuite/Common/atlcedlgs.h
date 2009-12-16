#ifndef __ATLCEDLGS_H__
#define __ATLCEDLGS_H__

#pragma once

#ifndef __cplusplus
   #error ATL requires C++ compilation (use a .cpp suffix)
#endif

#ifndef __ATLDLGS_H__
   #error atlcedlgs.h requires atldlgs.h to be included first
#endif

#ifndef _WIN32_WCE
   #error atlcedlgs.h compiles under Windows CE only
#endif


/////////////////////////////////////////////////////////////////////////////
// Classes in this file
//
// CCePropertyPageImpl
// CCePropertySheet
//

template< class T, class TBase = CPropertySheetWindow >
class ATL_NO_VTABLE CCePropertySheetImpl : public CPropertySheetImpl< T, TBase >
{
public:
   // Construction/Destruction

   CCePropertySheetImpl(ATL::_U_STRINGorID caption = (LPCTSTR) NULL, UINT uStartPage = 0, HWND hWndParent = NULL)
   {
      ::ZeroMemory(&m_psh, sizeof(PROPSHEETHEADER));
      m_psh.dwSize = sizeof(PROPSHEETHEADER);
      m_psh.dwFlags = PSH_USECALLBACK | PSH_MAXIMIZE;
#if (_ATL_VER >= 0x0700)
      m_psh.hInstance = ATL::_AtlBaseModule.GetResourceInstance();
#else //!(_ATL_VER >= 0x0700)
      m_psh.hInstance = _Module.GetResourceInstance();
#endif //!(_ATL_VER >= 0x0700)
      m_psh.phpage = NULL;   // will be set later
      m_psh.nPages = 0;      // will be set later
      m_psh.pszCaption = caption.m_lpstr;
      m_psh.nStartPage = uStartPage;
      m_psh.hwndParent = hWndParent;   // if NULL, will be set in DoModal/Create
      m_psh.pfnCallback = T::PropSheetCallback;
   }
   static int CALLBACK PropSheetCallback(HWND hWnd, UINT uMsg, LPARAM lParam)
   {
      if( uMsg == PSCB_INITIALIZED )
      {
         ATLASSERT(hWnd != NULL);
#if (_ATL_VER >= 0x0700)
         T* pT = (T*) ATL::_AtlWinModule.ExtractCreateWndData();
#else //!(_ATL_VER >= 0x0700)
         T* pT = (T*) _Module.ExtractCreateWndData();
#endif //!(_ATL_VER >= 0x0700)
         // Subclass the sheet window
         pT->SubclassWindow(hWnd);
         // Remove page handles array
         pT->_CleanUpPages();
         // Display empty menubar
         SHMENUBARINFO cbi = { 0 };
         cbi.cbSize = sizeof(SHMENUBARINFO);
         cbi.hwndParent = hWnd;
         cbi.dwFlags = SHCMBF_EMPTYBAR;
         ::SHCreateMenuBar( &cbi );
      }
      else if( uMsg == PSCB_GETVERSION )
      {
         return COMCTL32_VERSION;
      }
      else if( uMsg == PSCB_GETTITLE )
      {
         T::OnGetTitle((LPTSTR)lParam);
      }
      else if( uMsg = PSCB_GETLINKTEXT )
      {
         T::OnGetLinkText((LPTSTR)lParam);
      }
      return 0;
   }
   static void OnGetTitle(LPTSTR pstrTitle)
   {
   }
   static void OnGetLinkText(LPTSTR pstrText)
   {
   }
};

// for non-customized sheets
class CCePropertySheet : public CCePropertySheetImpl<CCePropertySheet>
{
public:
   CCePropertySheet(ATL::_U_STRINGorID caption = (LPCTSTR) NULL, UINT uStartPage = 0, HWND hWndParent = NULL)
      : CCePropertySheetImpl<CCePropertySheet>(caption, uStartPage, hWndParent)
   { 
   }

   BEGIN_MSG_MAP(CCePropertySheet)
      MESSAGE_HANDLER(WM_COMMAND, CCePropertySheetImpl<CCePropertySheet>::OnCommand)
   END_MSG_MAP()
};


#endif // __ATLCEDLGS_H__

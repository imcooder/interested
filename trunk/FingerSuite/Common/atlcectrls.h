
#ifndef __ATLCECTRLS_H__
#define __ATLCECTRLS_H__

#pragma once

#ifndef __cplusplus
   #error ATL requires C++ compilation (use a .cpp suffix)
#endif

#ifndef __ATLCTRLS_H__
   #error atlcectrls.h requires atlctrls.h to be included first
#endif

#ifndef _WIN32_WCE
   #error atlcectrls.h compiles under Windows CE only
#endif

#if (_WIN32_WCE < 400)
   #error This file requires a newer version of the platform.
#endif

#include <htmlctrl.h>
#pragma comment(lib, "htmlview.lib")

#include <richink.h>
#pragma comment(lib, "richink.lib")

#include <inkx.h>
#pragma comment(lib, "inkx.lib")

#include <voicectl.h>
#pragma comment(lib, "voicectl.lib")

#include <doclist.h>
#pragma comment(lib, "doclist.lib")


///////////////////////////////////////////////////////////////////////////////
// Classes in this file:
//
// CHtmlCtrlT<TBase> - CHtmlCtrl
// CRichInkCtrlT<TBase> - CRichInkCtrl
// CInkXCtrlT<TBase> - CInkXCtrl
// CVoiceRecorderCtrlT<TBase> - CVoiceRecorderCtrl
// CDocListCtrlT<TBase> - CDocListCtrl
// CCapEdit<TBase> - CCapEdit
// CTTStatic<TBase> - CTTStatic
// CTTButton<TBase> - CTTButton
//

namespace WTL
{

// These are wrapper classes for the Windows CE 4.2 controls
// To implement a window based on a control, use following:
// Example: Implementing a window based on a list box
//
// class CMyHtml : CWindowImpl<CMyHtml, CHtmlCtrl>
// {
// public:
//      BEGIN_MSG_MAP(CMyHtml)
//          // put your message handler entries here
//      END_MSG_MAP()
// };


///////////////////////////////////////////////////////////////////////////////
// CHtmlCtrl

template< class TBase >
class CHtmlCtrlT : public TBase
{
public:
   // Constructors
   
   CHtmlCtrlT(HWND hWnd = NULL) : TBase(hWnd)
   { 
   }

   CHtmlCtrlT< TBase >& operator =(HWND hWnd)
   {
      m_hWnd = hWnd;
      return *this;
   }

   HWND Create(HWND hWndParent, ATL::_U_RECT rect = NULL, LPCTSTR szWindowName = NULL,
         DWORD dwStyle = 0, DWORD dwExStyle = 0,
         ATL::_U_MENUorID MenuOrID = 0U, LPVOID lpCreateParam = NULL)
   {
      HWND hWnd = TBase::Create(GetWndClassName(), hWndParent, rect.m_lpRect, szWindowName, dwStyle, dwExStyle, MenuOrID.m_hMenu, lpCreateParam);
      ATLASSERT(hWnd!=NULL); // Did you remember to call InitHTMLControl(hInstance) ??
      return hWnd;
   }

   // Attributes

   static LPCTSTR GetWndClassName()
   {
      return WC_HTML;
   }

   void AddStyle(LPCWSTR pszStyle)
   {
      ATLASSERT(::IsWindow(m_hWnd));
      ::SendMessage(m_hWnd, DTM_ADDSTYLE, 0, (LPARAM) pszStyle);
   }

   void AddText(BOOL bPlainText, LPCSTR pszText)
   {
      ATLASSERT(::IsWindow(m_hWnd));
      ::SendMessage(m_hWnd, DTM_ADDTEXT, (WPARAM) bPlainText, (LPARAM) pszText);
   }

   void AddHTML(LPCSTR pszHTML)
   {
      ATLASSERT(::IsWindow(m_hWnd));
      ::SendMessage(m_hWnd, DTM_ADDTEXT, (WPARAM) FALSE, (LPARAM) pszHTML);
   }

   void AddText(BOOL bPlainText, LPCWSTR pszText)
   {
      ATLASSERT(::IsWindow(m_hWnd));
      ::SendMessage(m_hWnd, DTM_ADDTEXTW, (WPARAM) bPlainText, (LPARAM) pszText);
   }

   void AddHTML(LPCWSTR pszHTML)
   {
      ATLASSERT(::IsWindow(m_hWnd));
      ::SendMessage(m_hWnd, DTM_ADDTEXTW, (WPARAM) FALSE, (LPARAM) pszHTML);
   }

   void Anchor(LPCSTR pszAnchor)
   {
      ATLASSERT(::IsWindow(m_hWnd));
      ::SendMessage(m_hWnd, DTM_ANCHOR, 0, (LPARAM) pszAnchor);
   }

   void Anchor(LPCWSTR pszAnchor)
   {
      ATLASSERT(::IsWindow(m_hWnd));
      ::SendMessage(m_hWnd, DTM_ANCHORW, 0, (LPARAM) pszAnchor);
   }

   void GetBrowserDispatch(IDispatch** ppDispatch)
   {
      ATLASSERT(::IsWindow(m_hWnd));
      ATLASSERT(ppDispatch);
      ATLASSERT(*ppDispatch==NULL);
      ::SendMessage(m_hWnd, DTM_BROWSERDISPATCH, 0, (LPARAM) ppDispatch);
   }

   void Clear()
   {
      ATLASSERT(::IsWindow(m_hWnd));
      ::SendMessage(m_hWnd, DTM_CLEAR, 0, 0L);
   }

   void EnableClearType(BOOL bEnable = TRUE)
   {
      ATLASSERT(::IsWindow(m_hWnd));
      ::SendMessage(m_hWnd, DTM_ENABLECLEARTYPE, 0, (LPARAM) bEnable);
   }

   void EnableContextMenu(BOOL bEnable = TRUE)
   {
      ATLASSERT(::IsWindow(m_hWnd));
      ::SendMessage(m_hWnd, DTM_ENABLECONTEXTMENU, 0, (LPARAM) bEnable);
   }

   void EnableScripting(BOOL bEnable = TRUE)
   {
      ATLASSERT(::IsWindow(m_hWnd));
      ::SendMessage(m_hWnd, DTM_ENABLESCRIPTING, 0, (LPARAM) bEnable);
   }

   void EnableShrink(BOOL bEnable = TRUE)
   {
      ATLASSERT(::IsWindow(m_hWnd));
      ::SendMessage(m_hWnd, DTM_ENABLESHRINK, 0, (LPARAM) bEnable);
   }

   void EndOfSource()
   {
      ATLASSERT(::IsWindow(m_hWnd));
      ::SendMessage(m_hWnd, DTM_ENDOFSOURCE, 0, 0L);
   }

   void ImageFail(DWORD dwCookie)
   {
      ATLASSERT(::IsWindow(m_hWnd));
      ::SendMessage(m_hWnd, DTM_IMAGEFAIL, 0, (LPARAM) dwCookie);
   }

   int GetLayoutHeight() const
   {
      ATLASSERT(::IsWindow(m_hWnd));
      return (int) ::SendMessage(m_hWnd, DTM_LAYOUTHEIGHT, 0, 0L);
   }

   int GetLayoutWidth() const
   {
      ATLASSERT(::IsWindow(m_hWnd));
      return (int) ::SendMessage(m_hWnd, DTM_LAYOUTWIDTH, 0, 0L);
   }

   void Navigate(LPCTSTR pstrURL, UINT uFlags = 0)
   {
      ATLASSERT(::IsWindow(m_hWnd));
      ATLASSERT(pstrURL);
      ::SendMessage(m_hWnd, DTM_NAVIGATE, (WPARAM) uFlags, (LPARAM) pstrURL);
   }

   void SelectAll()
   {
      ATLASSERT(::IsWindow(m_hWnd));
      ::SendMessage(m_hWnd, DTM_SELECTALL, 0, 0L);
   }

   void SetImage(INLINEIMAGEINFO* pImageInfo)
   {
      ATLASSERT(::IsWindow(m_hWnd));
      ATLASSERT(pImageInfo);
      ::SendMessage(m_hWnd, DTM_SETIMAGE, 0, (LPARAM) pImageInfo);
   }

   void ZoomLevel(int iLevel)
   {
      ATLASSERT(::IsWindow(m_hWnd));
      ::SendMessage(m_hWnd, DTM_ZOOMLEVEL, 0, (LPARAM) iLevel);
   }
   
   void Stop()
   {
      ATLASSERT(::IsWindow(m_hWnd));
      ::SendMessage(m_hWnd, DTM_STOP, 0, 0L);
   }

   void GetScriptDispatch(IDispatch** ppDispatch)
   {
      ATLASSERT(::IsWindow(m_hWnd));
      ATLASSERT(ppDispatch);
      ATLASSERT(*ppDispatch==NULL);
      ::SendMessage(m_hWnd, DTM_SCRIPTDISPATCH, 0, (LPARAM) ppDispatch);
   }
};

typedef CHtmlCtrlT<ATL::CWindow> CHtmlCtrl;


///////////////////////////////////////////////////////////////////////////////
// CRichInkCtrl

template< class TBase >
class CRichInkCtrlT : public TBase
{
public:
   // Constructors
   
   CRichInkCtrlT(HWND hWnd = NULL) : TBase(hWnd)
   { 
   }

   CRichInkCtrlT< TBase >& operator =(HWND hWnd)
   {
      m_hWnd = hWnd;
      return *this;
   }

   HWND Create(HWND hWndParent, ATL::_U_RECT rect = NULL, LPCTSTR szWindowName = NULL,
         DWORD dwStyle = 0, DWORD dwExStyle = 0,
         ATL::_U_MENUorID MenuOrID = 0U, LPVOID lpCreateParam = NULL)
   {
      HWND hWnd = TBase::Create(GetWndClassName(), hWndParent, rect.m_lpRect, szWindowName, dwStyle, dwExStyle, MenuOrID.m_hMenu, lpCreateParam);
      ATLASSERT(hWnd!=NULL); // Did you remember to call InitRichInkDLL() ??
      return hWnd;
   }

   // Attributes

   static LPCTSTR GetWndClassName()
   {
      return WC_RICHINK;
   }

   BOOL CanPaste(UINT uFormat = 0) const
   {
      ATLASSERT(::IsWindow(m_hWnd));
      return (BOOL) ::SendMessage(m_hWnd, EM_CANPASTE, (WPARAM) uFormat, 0L);
   }

   BOOL CanRedo() const
   {
      ATLASSERT(::IsWindow(m_hWnd));
      return (BOOL) ::SendMessage(m_hWnd, EM_CANREDO, 0, 0L);
   }

   BOOL CanUndo() const
   {
      ATLASSERT(::IsWindow(m_hWnd));
      return (BOOL) ::SendMessage(m_hWnd, EM_CANUNDO, 0, 0L);
   }

   void ClearAll(BOOL bRepaint = TRUE) const
   {
      ATLASSERT(::IsWindow(m_hWnd));
      ::SendMessage(m_hWnd, EM_CLEARALL, (WPARAM) bRepaint, 0L);
   }

   BOOL GetModify() const
   {
      ATLASSERT(::IsWindow(m_hWnd));
      return (BOOL) ::SendMessage(m_hWnd, EM_GETMODIFY, 0, 0L);
   }

   UINT GetPageStyle() const
   {
      ATLASSERT(::IsWindow(m_hWnd));
      return (UINT) ::SendMessage(m_hWnd, EM_GETPAGESTYLE, 0, 0L);
   }

   UINT GetPenMode() const
   {
      ATLASSERT(::IsWindow(m_hWnd));
      return (UINT) ::SendMessage(m_hWnd, EM_GETPENMODE, 0, 0L);
   }

   UINT GetViewStyle() const
   {
      ATLASSERT(::IsWindow(m_hWnd));
      return (UINT) ::SendMessage(m_hWnd, EM_GETVIEW, 0, 0L);
   }

   UINT GetWrapMode() const
   {
      ATLASSERT(::IsWindow(m_hWnd));
      return (UINT) ::SendMessage(m_hWnd, EM_GETWRAPMODE, 0, 0L);
   }

   UINT GetZoomPercent() const
   {
      ATLASSERT(::IsWindow(m_hWnd));
      return (UINT) ::SendMessage(m_hWnd, EM_GETZOOMPERCENT, 0, 0L);
   }

   void InsertLinks(LPWSTR lpString, int cchLength = -1)
   {
      ATLASSERT(::IsWindow(m_hWnd));
      if( cchLength == -1 ) strLength = lstrlen(lpString);
      ::SendMessage(m_hWnd, EM_INSERTLINKS, (WPARAM) cchLength, (LPARAM) lpString);
   }

   void RedoEvent()
   {
      ATLASSERT(::IsWindow(m_hWnd));
      ::SendMessage(m_hWnd, EM_REDOEVENT, 0, 0L);
   }

   UINT SetInkLayer(UINT uLayer)
   {
      ATLASSERT(::IsWindow(m_hWnd));
      return (UINT) ::SendMessage(m_hWnd, EM_SETINKLAYER, (WPARAM) uLayer, 0L);
   }

   void SetPageStyle(UINT uStyle)
   {
      ATLASSERT(::IsWindow(m_hWnd));
      ::SendMessage(m_hWnd, EM_SETPAGESTYLE, (WPARAM) uStyle, 0L);
   }

   void SetPenMode(UINT uMode)
   {
      ATLASSERT(::IsWindow(m_hWnd));
      ::SendMessage(m_hWnd, EM_SETPENMODE, (WPARAM) uMode, 0L);
   }

   void SetViewStyle(UINT uStyle)
   {
      ATLASSERT(::IsWindow(m_hWnd));
      ::SendMessage(m_hWnd, EM_SETVIEW, (WPARAM) uStyle, 0L);
   }

   void SetViewAttributes(VIEWATTRIBUTES* pAttribs)
   {
      ATLASSERT(::IsWindow(m_hWnd));
      ATLASSERT(pAttribs);
      ::SendMessage(m_hWnd, EM_SETVIEWATTRIBUTES, 0, (LPARAM) pAttribs);
   }

   void SetWrapMode(UINT uMode)
   {
      ATLASSERT(::IsWindow(m_hWnd));
      ::SendMessage(m_hWnd, EM_SETWRAPMODE, (WPARAM) uMode, 0L);
   }

   void SetZoomPercent(UINT uPercent)
   {
      ATLASSERT(::IsWindow(m_hWnd));
      ::SendMessage(m_hWnd, EM_SETZOOMPERCENT, (WPARAM) uPercent, 0L);
   }

   LONG StreamIn(UINT uFormat, EDITSTREAM& es)
   {
      ATLASSERT(::IsWindow(m_hWnd));
      return (LONG) ::SendMessage(m_hWnd, EM_STREAMIN, (WPARAM) uFormat, (LPARAM) &es);
   }

   LONG StreamOut(UINT uFormat, EDITSTREAM& es)
   {
      ATLASSERT(::IsWindow(m_hWnd));
      return (LONG) ::SendMessage(m_hWnd, EM_STREAMOUT, (WPARAM) uFormat, (LPARAM) &es);
   }

   void UndoEvent()
   {
      ATLASSERT(::IsWindow(m_hWnd));
      ::SendMessage(m_hWnd, EM_UNDOEVENT, 0, 0L);
   }

   // Standard EM_xxx messages

   DWORD GetSel() const
   {
      ATLASSERT(::IsWindow(m_hWnd));
      return (DWORD) ::SendMessage(m_hWnd, EM_GETSEL, 0, 0L);
   }

   void GetSel(int& nStartChar, int& nEndChar) const
   {
      ATLASSERT(::IsWindow(m_hWnd));
      ::SendMessage(m_hWnd, EM_GETSEL, (WPARAM) &nStartChar, (LPARAM) &nEndChar);
   }

   void ReplaceSel(LPCTSTR lpszNewText, BOOL bCanUndo = FALSE)
   {
      ATLASSERT(::IsWindow(m_hWnd));
      ::SendMessage(m_hWnd, EM_REPLACESEL, (WPARAM) bCanUndo, (LPARAM) lpszNewText);
   }

   void SetModify(BOOL bModified = TRUE)
   {
      ATLASSERT(::IsWindow(m_hWnd));
      ::SendMessage(m_hWnd, EM_SETMODIFY, (WPARAM) bModified, 0L);
   }

   int GetTextLength() const
   {
      ATLASSERT(::IsWindow(m_hWnd));
      return (int) ::SendMessage(m_hWnd, WM_GETTEXTLENGTH, 0, 0L);
   }

   // Clipboard operations
   
   void Clear()
   {
      ATLASSERT(::IsWindow(m_hWnd));
      ::SendMessage(m_hWnd, WM_CLEAR, 0, 0L);
   }

   void Copy()
   {
      ATLASSERT(::IsWindow(m_hWnd));
      ::SendMessage(m_hWnd, WM_COPY, 0, 0L);
   }

   void Cut()
   {
      ATLASSERT(::IsWindow(m_hWnd));
      ::SendMessage(m_hWnd, WM_CUT, 0, 0L);
   }

   void Paste()
   {
      ATLASSERT(::IsWindow(m_hWnd));
      ::SendMessage(m_hWnd, WM_PASTE, 0, 0L);
   }
};

typedef CRichInkCtrlT<ATL::CWindow> CRichInkCtrl;


///////////////////////////////////////////////////////////////////////////////
// CInkXCtrl

template< class TBase >
class CInkXCtrlT : public TBase
{
public:
   // Constructors
   
   CInkXCtrlT(HWND hWnd = NULL) : TBase(hWnd)
   { 
   }

   CInkXCtrlT< TBase >& operator =(HWND hWnd)
   {
      m_hWnd = hWnd;
      return *this;
   }

   HWND Create(HWND hWndParent, ATL::_U_RECT rect = NULL, LPCTSTR szWindowName = NULL,
         DWORD dwStyle = 0, DWORD dwExStyle = 0,
         ATL::_U_MENUorID MenuOrID = 0U, LPVOID lpCreateParam = NULL)
   {
      HWND hWnd = TBase::Create(GetWndClassName(), hWndParent, rect.m_lpRect, szWindowName, dwStyle, dwExStyle, MenuOrID.m_hMenu, lpCreateParam);
      ATLASSERT(hWnd!=NULL); // Did you remember to call InitInkX() ??
      return hWnd;
   }

   // Attributes

   static LPCTSTR GetWndClassName()
   {
      return WC_INKX;
   }

   static UINT GetHotRecordingMessage()
   {
      return ::RegisterWindowMessage(szHotRecording);
   }

   void ClearAll()
   {
      ATLASSERT(::IsWindow(m_hWnd));
      ::SendMessage(m_hWnd, IM_CLEARALL, 0, 0L);
   }

   int GetData(BYTE* lpBuffer, INT cbBuffer) const
   {
      ATLASSERT(::IsWindow(m_hWnd));
      ATLASSERT(lpBuffer);
      return (int) ::SendMessage(m_hWnd, IM_GETDATA, (WPARAM) cbBuffer, (LPARAM) lpBuffer);
   }

   int GetDataLen() const
   {
      ATLASSERT(::IsWindow(m_hWnd));
      return (int) ::SendMessage(m_hWnd, IM_GETDATALEN, 0, 0L);
   }

   CRichInkCtrl GetRichInk() const
   {
      ATLASSERT(::IsWindow(m_hWnd));
      return (HWND) ::SendMessage(m_hWnd, IM_GETRICHINK, 0, 0L);
   }

   BOOL IsRecording() const
   {
      ATLASSERT(::IsWindow(m_hWnd));
      return (BOOL) ::SendMessage(m_hWnd, IM_RECORDING, 0, 0L);
   }

   void ReInit()
   {
      ATLASSERT(::IsWindow(m_hWnd));
      ::SendMessage(m_hWnd, IM_REINIT, 0, 0L);
   }

   void SetData(const BYTE* lpInkData, INT cbInkData)
   {
      ATLASSERT(::IsWindow(m_hWnd));
      ATLASSERT(lpInkData);
      ::SendMessage(m_hWnd, IM_SETDATA, (WPARAM) cbInkData, (LPARAM) lpInkData);
   }

   void VoicePlay()
   {
      ATLASSERT(::IsWindow(m_hWnd));
      ::SendMessage(m_hWnd, IM_VOICE_PLAY, 0, 0L);
   }

   BOOL IsVoicePlaying() const
   {
      ATLASSERT(::IsWindow(m_hWnd));
      return (BOOL) ::SendMessage(m_hWnd, IM_VOICE_PLAYING, 0, 0L);
   }

   BOOL VoiceRecord()
   {
      ATLASSERT(::IsWindow(m_hWnd));
      return (BOOL) ::SendMessage(m_hWnd, IM_VOICE_RECORD, 0, 0L);
   }

   void VoiceStop()
   {
      ATLASSERT(::IsWindow(m_hWnd));
      ::SendMessage(m_hWnd, IM_VOICE_STOP, 0, 0L);
   }

   void ShowVoiceBar(BOOL bShow = TRUE)
   {
      ATLASSERT(::IsWindow(m_hWnd));
      ::SendMessage(m_hWnd, IM_VOICEBAR, (WPARAM) bShow, 0L);
   }
};

typedef CInkXCtrlT<ATL::CWindow> CInkXCtrl;


///////////////////////////////////////////////////////////////////////////////
// CVoiceRecorderCtrl

template< class TBase >
class CVoiceRecorderCtrlT : public TBase
{
public:
   // Constructors
   
   CVoiceRecorderCtrlT(HWND hWnd = NULL) : TBase(hWnd)
   { 
   }

   CVoiceRecorderCtrlT< TBase >& operator =(HWND hWnd)
   {
      m_hWnd = hWnd;
      return *this;
   }

   HWND Create(HWND hWndParent, const POINT pt, LPTSTR pstrFileName,
         UINT nID, DWORD dwStyle = 0)
   {
      ATLASSERT(pstrFileName);
      CM_VOICE_RECORDER cmvr = { 0 };
      cmvr.cb = sizeof(CM_VOICE_RECORDER);
      cmvr.dwStyle = dwStyle;
      cmvr.xPos = pt.x;
      cmvr.yPos = pt.y;
      cmvr.hwndParent = hWndParent;
      cmvr.id = nID;
      cmvr.lpszRecordFileName = pstrFileName;
      m_hWnd = VoiceRecorder_Create(&cmvr);
      return m_hWnd;
   }

   HWND Create(LPCM_VOICE_RECORDER pAttribs)
   {
      ATLASSERT(pAttribs);
      m_hWnd = VoiceRecorder_Create(pAttribs);
      return m_hWnd;
   }

   // Attributes

   void Record()
   {
      ATLASSERT(::IsWindow(m_hWnd));
      ::SendMessage(m_hWnd, VRM_RECORD, 0, 0L);
   }

   void Play()
   {
      ATLASSERT(::IsWindow(m_hWnd));
      ::SendMessage(m_hWnd, VRM_PLAY, 0, 0L);
   }

   void Stop()
   {
      ATLASSERT(::IsWindow(m_hWnd));
      ::SendMessage(m_hWnd, VRM_STOP, 0, 0L);
   }

   void Cancel()
   {
      ATLASSERT(::IsWindow(m_hWnd));
      ::SendMessage(m_hWnd, VRM_CANCEL, 0, 0L);
   }

   void Done()
   {
      ATLASSERT(::IsWindow(m_hWnd));
      ::SendMessage(m_hWnd, VRM_OK, 0, 0L);
   }
};

typedef CVoiceRecorderCtrlT<ATL::CWindow> CVoiceRecorderCtrl;


///////////////////////////////////////////////////////////////////////////////
// CDocListCtrl

template< class TBase >
class CDocListCtrlT : public TBase
{
public:
   // Attributes

   DOCLISTCREATE m_dlc;
   TCHAR m_szPath[MAX_PATH];

   // Constructors
   
   CDocListCtrlT(HWND hWnd = NULL) : TBase(hWnd)
   { 
   }

   CDocListCtrlT< TBase >& operator =(HWND hWnd)
   {
      m_hWnd = hWnd;
      return *this;
   }

   HWND Create(HWND hWndParent, WORD wId, LPCTSTR pszFolder = NULL, LPCTSTR pstrFilter = NULL,
      WORD wFilterIndex = 0, DWORD dwFlags = DLF_SHOWEXTENSION)
   {
      ATLASSERT(pstrFilter); // It seems to need a filter badly!!
      ::ZeroMemory(&m_dlc, sizeof(DOCLISTCREATE));
      ::ZeroMemory(m_szPath, sizeof(m_szPath));
      if( pszFolder ) ::lstrcpyn(m_szPath, pszFolder, MAX_PATH - 1);
      m_dlc.dwStructSize = sizeof(DOCLISTCREATE);
      m_dlc.hwndParent = hWndParent;
      m_dlc.pszFolder = m_szPath;
      m_dlc.pstrFilter = pstrFilter;
      m_dlc.wFilterIndex = wFilterIndex;
      m_dlc.wId = wId;
      m_dlc.dwFlags = dwFlags;
      m_hWnd = DocList_Create(&m_dlc);
      return m_hWnd;
   }

   HWND Create(DOCLISTCREATE* pDlc)
   {
      m_dlc = *pDlc;
      m_hWnd = DocList_Create(&m_dlc);
      return m_hWnd;
   }

   // Attributes

   void DeleteSel()
   {
      ATLASSERT(::IsWindow(m_hWnd));
      ::SendMessage(m_hWnd, DLM_DELETESEL, 0, 0L);
   }

   void DisableUpdates()
   {
      ATLASSERT(::IsWindow(m_hWnd));
      ::SendMessage(m_hWnd, DLM_DISABLEUPDATES, 0, 0L);
   }

   void EnableUpdates()
   {
      ATLASSERT(::IsWindow(m_hWnd));
      ::SendMessage(m_hWnd, DLM_ENABLEUPDATES, 0, 0L);
   }

   int GetFilterIndex() const
   {
      ATLASSERT(::IsWindow(m_hWnd));
      return (int) ::SendMessage(m_hWnd, DLM_GETFILTERINDEX, 0, 0L);
   }

   int GetItemCount() const
   {
      ATLASSERT(::IsWindow(m_hWnd));
      return (int) ::SendMessage(m_hWnd, DLM_GETITEMCOUNT, 0, 0L);
   }

   int GetNextItem(int iIndex, DWORD dwRelation = LVNI_ALL) const
   {
      ATLASSERT(::IsWindow(m_hWnd));
      return (int) ::SendMessage(m_hWnd, DLM_GETNEXTITEM, (WPARAM) iIndex, (LPARAM) dwRelation);
   }

   int GetFirstItem(DWORD dwRelation = LVNI_ALL) const
   {
      ATLASSERT(::IsWindow(m_hWnd));
      return (int) ::SendMessage(m_hWnd, DLM_GETNEXTITEM, (WPARAM) -1, (LPARAM) dwRelation);
   }

   BOOL GetNextWave(int* pIndex) const
   {
      ATLASSERT(::IsWindow(m_hWnd));
      ATLASSERT(pIndex);
      return (BOOL) ::SendMessage(m_hWnd, DLM_GETNEXTWAVE, 0, (LPARAM) pIndex);
   }

   BOOL GetPrevWave(int* pIndex) const
   {
      ATLASSERT(::IsWindow(m_hWnd));
      ATLASSERT(pIndex);
      return (BOOL) ::SendMessage(m_hWnd, DLM_GETPREVWAVE, 0, (LPARAM) pIndex);
   }

   int GetSelCount() const
   {
      ATLASSERT(::IsWindow(m_hWnd));
      return (int) ::SendMessage(m_hWnd, DLM_GETSELCOUNT, 0, 0L);
   }

   BOOL GetSelPathName(LPTSTR pstrPath, int cchMax) const
   {
      ATLASSERT(::IsWindow(m_hWnd));
      ATLASSERT(pstrPath);
      return (BOOL) ::SendMessage(m_hWnd, DLM_GETSELPATHNAME, (WPARAM) cchMax, (LPARAM) pstrPath);
   }

   void ReceiveIR(LPCTSTR pstrPath) const
   {
      ATLASSERT(::IsWindow(m_hWnd));
      ATLASSERT(pstrPath);
      ::SendMessage(m_hWnd, DLM_RECEIVEIR, 0, (LPARAM) pstrPath);
   }

   void Refresh()
   {
      ATLASSERT(::IsWindow(m_hWnd));
      ::SendMessage(m_hWnd, DLM_REFRESH, 0, 0L);
   }

   BOOL RenameMoveSelectedItems()
   {
      ATLASSERT(::IsWindow(m_hWnd));
      return (BOOL) ::SendMessage(m_hWnd, DLM_RENAMEMOVE, 0, 0L);
   }

   int SelectAll()
   {
      ATLASSERT(::IsWindow(m_hWnd));
      return (int) ::SendMessage(m_hWnd, DLM_SELECTALL, 0, 0L);
   }

   HRESULT SelectItem(LPCTSTR pstrPath, BOOL bVisible = TRUE)
   {
      ATLASSERT(::IsWindow(m_hWnd));
      ATLASSERT(pstrPath);
      return (HRESULT) ::SendMessage(m_hWnd, DLM_SELECTITEM, (WPARAM) bVisible, (LPARAM) pstrPath);
   }

   void SendEMail(LPCTSTR pstrAttachment)
   {
      ATLASSERT(::IsWindow(m_hWnd));
      ::SendMessage(m_hWnd, DLM_SENDEMAIL, 0, (LPARAM) pstrAttachment);
   }

   void SendIR(LPCTSTR pstrPath)
   {
      ATLASSERT(::IsWindow(m_hWnd));
      ::SendMessage(m_hWnd, DLM_SENDIR, 0, (LPARM) pstrPath);
   }

   HRESULT SetFilterIndex(int iIndex)
   {
      ATLASSERT(::IsWindow(m_hWnd));
      return (HRESULT) ::SendMessage(m_hWnd, DLM_SETFILTERINDEX, (WPARAM) iIndex, 0L);
   }

   void SetFolder(LPCTSTR pstrPath)
   {
      ATLASSERT(::IsWindow(m_hWnd));
      ATLASSERT(pstrPath);
      ::SendMessage(m_hWnd, DLM_SETFOLDER, 0, (LPARAM) pstrPath);
   }

   BOOL SetItemState(int iIndex, const LVITEM* pItem)
   {
      ATLASSERT(::IsWindow(m_hWnd));
      ATLASSERT(pItem);
      return (BOOL) ::SendMessage(m_hWnd, DLM_SETITEMSTATE, (WPARAM) iIndex, (LPARAM) pItem);
   }

   BOOL SetItemState(int iIndex, UINT uState, UINT uMask)
   {
      ATLASSERT(::IsWindow(m_hWnd));
      LV_ITEM lvi = { 0 };
      lvi.stateMask = uMask;
      lvi.state = uState;
      return (BOOL) ::SendMessage(m_hWnd, DLM_SETITEMSTATE, (WPARAM) iIndex, (LPARAM) pItem);
   }

   void SetOneItem(int iIndex, LPCVOID pPA)
   {
      ATLASSERT(::IsWindow(m_hWnd));
      ::SendMessage(m_hWnd, DLM_SETONEITEM, (WPARAM) iIndex, (LPARAM) pPA);
   }

   void SetSelect(int iIndex)
   {
      ATLASSERT(::IsWindow(m_hWnd));
      ::SendMessage(m_hWnd, DLM_SETSELECT, (WPARAM) iIndex, 0L);
   }

   void SetSelPathName(LPCTSTR pstrPath)
   {
      ATLASSERT(::IsWindow(m_hWnd));
      ATLASSERT(pstrPath);
      ::SendMessage(m_hWnd, DLM_SETSELPATHNAME, 0, (LPARAM) pstrPath);
   }

   BOOL SetSortOrder()
   {
      ATLASSERT(::IsWindow(m_hWnd));
      return (BOOL) ::SendMessage(m_hWnd, DLM_SETSORTORDER, 0, 0L);
   }

   HRESULT Update()
   {
      ATLASSERT(::IsWindow(m_hWnd));
      return (HRESULT) ::SendMessage(m_hWnd, DLM_UPDATE, 0, 0L);
   }

   BOOL ValidateFolder()
   {
      ATLASSERT(::IsWindow(m_hWnd));
      return (BOOL) ::SendMessage(m_hWnd, DLM_VALIDATEFOLDER, 0, 0L);
   }

   // Functions

   BOOL GetFirstSelectedWaveFile(int* pIndex, LPTSTR szPath, const size_t cchPath)
   {
      ATLASSERT(::IsWindow(m_hWnd));
      return DocList_GetFirstSelectedWaveFile(m_hWnd, pIndex, szPath, cchPath);
   }

   BOOL GetNextSelectedWaveFile(int* pIndex, LPTSTR szPath, const size_t cchPath)
   {
      ATLASSERT(::IsWindow(m_hWnd));
      return DocList_GetNextSelectedWaveFile(m_hWnd, pIndex, szPath, cchPath);
   }
};

typedef CDocListCtrlT<ATL::CWindow> CDocListCtrl;


///////////////////////////////////////////////////////////////////////////////
// CCapEdit

template< class TBase >
class CCapEditT : public TBase
{
public:
   // Constructors
   
   CCapEditT(HWND hWnd = NULL) : TBase(hWnd)
   { 
   }

   CCapEditT< TBase >& operator =(HWND hWnd)
   {
      m_hWnd = hWnd;
      return *this;
   }

   HWND Create(HWND hWndParent, ATL::_U_RECT rect = NULL, LPCTSTR szWindowName = NULL,
         DWORD dwStyle = 0, DWORD dwExStyle = 0,
         ATL::_U_MENUorID MenuOrID = 0U, LPVOID lpCreateParam = NULL)
   {
      HWND hWnd = TBase::Create(GetWndClassName(), hWndParent, rect.m_lpRect, szWindowName, dwStyle, dwExStyle, MenuOrID.m_hMenu, lpCreateParam);
      ATLASSERT(hWnd!=NULL); // Did you remember to call SHInitExtraControls() ??
      return hWnd;
   }

   // Attributes

   static LPCTSTR GetWndClassName()
   {
      return WC_CAPEDIT;
   }
};

typedef CCapEditT<WTL::CEdit> CCapEdit;


///////////////////////////////////////////////////////////////////////////////
// CTTStatic

template< class TBase >
class CTTStaticT : public TBase
{
public:
   // Constructors
   
   CTTStaticT(HWND hWnd = NULL) : TBase(hWnd)
   { 
   }

   CTTStaticT< TBase >& operator =(HWND hWnd)
   {
      m_hWnd = hWnd;
      return *this;
   }

   HWND Create(HWND hWndParent, ATL::_U_RECT rect = NULL, LPCTSTR szWindowName = NULL,
         DWORD dwStyle = 0, DWORD dwExStyle = 0,
         ATL::_U_MENUorID MenuOrID = 0U, LPVOID lpCreateParam = NULL)
   {
      HWND hWnd = TBase::Create(GetWndClassName(), hWndParent, rect.m_lpRect, szWindowName, dwStyle, dwExStyle, MenuOrID.m_hMenu, lpCreateParam);
      ATLASSERT(hWnd!=NULL); // Did you remember to call SHInitExtraControls() ??
      return hWnd;
   }

   // Attributes

   static LPCTSTR GetWndClassName()
   {
      return WC_TSTATIC;
   }

   // Operations

   int SetToolTipText(LPCTSTR pstrTipText)
   {
      ATLASSERT(::IsWindow(m_hWnd));
      ATLASSERT(pstrTipText);
      ATLASSERT(lstrlen(pstrTipText)<=253);
      LPTSTR pstr = _alloca((lstrlen(pstrTipText) + 3) * sizeof(TCHAR));
      ::lstrcpy(pstr, _T("~~"));
      ::lstrcat(pstr, pstrTipText);
      return SetWindowText(pstr);
   }
};

typedef CTTStaticT<WTL::CStatic> CTTStatic;


///////////////////////////////////////////////////////////////////////////////
// CTTButton

template< class TBase >
class CTTButtonT : public TBase
{
public:
   // Constructors
   
   CTTButtonT(HWND hWnd = NULL) : TBase(hWnd)
   { 
   }

   CTTButtonT< TBase >& operator =(HWND hWnd)
   {
      m_hWnd = hWnd;
      return *this;
   }

   HWND Create(HWND hWndParent, ATL::_U_RECT rect = NULL, LPCTSTR szWindowName = NULL,
         DWORD dwStyle = 0, DWORD dwExStyle = 0,
         ATL::_U_MENUorID MenuOrID = 0U, LPVOID lpCreateParam = NULL)
   {
      HWND hWnd = TBase::Create(GetWndClassName(), hWndParent, rect.m_lpRect, szWindowName, dwStyle, dwExStyle, MenuOrID.m_hMenu, lpCreateParam);
      ATLASSERT(hWnd!=NULL); // Did you remember to call SHInitExtraControls() ??
      return hWnd;
   }

   // Attributes

   static LPCTSTR GetWndClassName()
   {
      return WC_TBUTTON;
   }

   // Operations

   int SetToolTipText(LPCTSTR pstrTipText)
   {
      ATLASSERT(::IsWindow(m_hWnd));
      ATLASSERT(pstrTipText);
      ATLASSERT(lstrlen(pstrTipText)<=253);
      LPTSTR pstr = _alloca((lstrlen(pstrTipText) + 3) * sizeof(TCHAR));
      ::lstrcpy(pstr, _T("~~"));
      ::lstrcat(pstr, pstrTipText);
      return SetWindowText(pstr);
   }
};

typedef CTTButtonT<WTL::CButton> CTTButton;


}; // namespace


#endif // __ATLCECTRLS_H__

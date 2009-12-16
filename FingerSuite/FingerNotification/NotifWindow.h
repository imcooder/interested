// WTLApp1View.h : interface of the CWTLApp1View class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "InterceptEngine.h"

#include <initguid.h>
//#include <piedocvw.h>
#include <webvw.h>

#define IDT_TIMER_MENU_ANIMATION  10110
#define TMR_MENU_ANIMATION        10

#define BORDER_WIDTH           10
#define UM_STARTANIM    WM_USER + 1 
#define UM_MINIMIZE     WM_USER + 2

#define IDC_HTMLVIEW     10

//#define CUSTOM_CSS _T("INPUT[type='button'] { BORDER-RIGHT: #7b8194 1px solid; PADDING-RIGHT: 4px; BACKGROUND-POSITION: left bottom; BORDER-TOP: #a5a9b6 1px solid; PADDING-LEFT: 4px; FONT-WEIGHT: bold; FONT-SIZE: 9pt! important; BACKGROUND-IMAGE: url(btn_background.gif); PADDING-BOTTOM: 4px; MARGIN: 2px; VERTICAL-ALIGN: middle; BORDER-LEFT: #a5a9b6 1px solid; COLOR: #586073! important; LINE-HEIGHT: 20px; PADDING-TOP: 4px; BORDER-BOTTOM: #7b8194 1px solid; BACKGROUND-REPEAT: repeat-x; FONT-FAMILY: Tahoma; WHITE-SPACE: nowrap; BACKGROUND-COLOR: #ffffff; TEXT-ALIGN: center; TEXT-DECORATION: none }")

#define CUSTOM_CSS _T("INPUT[type='button'] { BORDER: none; PADDING-RIGHT: 4px; BACKGROUND-POSITION: left bottom; PADDING-LEFT: 4px; FONT-WEIGHT: bold; FONT-SIZE: 9pt! important; BACKGROUND-IMAGE: url(fngrnotif_btn_bkg.gif); PADDING-BOTTOM: 4px; MARGIN: 2px; VERTICAL-ALIGN: middle; COLOR: #000000! important; LINE-HEIGHT: 20px; PADDING-TOP: 4px; BACKGROUND-REPEAT: repeat-x; FONT-FAMILY: Tahoma; WHITE-SPACE: nowrap; BACKGROUND-COLOR: #ffffff; TEXT-ALIGN: center; TEXT-DECORATION: none }")

template <class T>
class CNotifWindow : public CWindowImpl<T, CWindow, CControlWinTraits>
{
public:
	DECLARE_WND_CLASS(NULL)

	CNotifWindow ()
	{
	}

	BOOL PreTranslateMessage(MSG* pMsg)
	{
		pMsg;
		return FALSE;
	}

	void SetScaleFactor(int value)
	{
		m_scaleFactor = value;
	}

	int GetScaleFactor()
	{
		return m_scaleFactor;
	}

	void Show()
	{
		ModifyShape();
		StartAnimation();
	}

	BOOL SetNotification(LPNOTIFICATIONINFO pData)
	{
		m_pInfo = pData;

		return TRUE;
	}

	void DoPaint(CBufferedPaintDC /*dc*/)
	{
		// must be implemented in a derived class
		ATLASSERT(FALSE);
	}



	BEGIN_MSG_MAP(CNotifWindow)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackground)
		MESSAGE_HANDLER(WM_TIMER, OnTimer)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER(UM_STARTANIM, OnStartAnim)
		//MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
		//MESSAGE_HANDLER(WM_LBUTTONUP, OnLButtonUp)
		//MESSAGE_HANDLER(WM_KEYDOWN, OnKeyDown)
		FORWARD_NOTIFICATIONS()
	END_MSG_MAP()

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		ModifyTopShape();

		if (m_fCaption != NULL)
		{
			LOGFONT lf; m_fCaption.GetLogFont(&lf);
			lf.lfHeight += 7 * m_scaleFactor;
			m_fCalendar.CreateFontIndirect(&lf);
		}

		return 0;
	}

	LRESULT OnSize(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
	{
		return 0;
	}

	LRESULT OnEraseBackground(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		return 1;
	}

	LRESULT OnPaint(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
	{
		RECT rc; GetClientRect(&rc);
		CBufferedPaintDC dc(m_hWnd, 0);

		RECT rcHead; CopyRect(&rcHead, &rc);
		rcHead.bottom = m_imgHeader.GetHeight();
		m_imgHeader.Draw(dc, rcHead);

		// draw title
		dc.SetBkMode(TRANSPARENT);
		dc.SetTextColor(m_clCaptionText);
		CFont oldFont = dc.SelectFont(m_fCaption);

		UINT uFormat = DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS;

		if (m_pInfo->grfFlagsOriginal & SHNF_TITLETIME)
		{
			WCHAR szTitle[256];
			WCHAR szTime[50];
			::GetTimeFormat(LOCALE_USER_DEFAULT, TIME_NOSECONDS, NULL, NULL, szTime, 12);
			wsprintf(szTitle, L"%s | %s", szTime, m_pInfo->nd.pszTitle);
			rcHead.left += m_imgCalendar.GetWidth();
			dc.DrawText(szTitle, -1, &rcHead, uFormat);

			// draw icon calendar
			WCHAR szMonth[50];
			WCHAR szDay[50];
			::GetDateFormat(LOCALE_USER_DEFAULT, 0, NULL, L"MMM", szMonth, 50);
			::GetDateFormat(LOCALE_USER_DEFAULT, 0, NULL, L"d", szDay, 50);

			m_imgCalendar.Draw(dc);

			RECT rcCal;
			SetRect(&rcCal, 0, 0, m_imgCalendar.GetWidth(), 10 * m_scaleFactor);
			CFont oldCalFont = dc.SelectFont(m_fCalendar);
			dc.DrawText(szMonth, -1, &rcCal, DT_CENTER | DT_BOTTOM | DT_SINGLELINE | DT_END_ELLIPSIS);
			dc.SelectFont(oldCalFont);

			SetRect(&rcCal, 0, 0, m_imgCalendar.GetWidth(), m_imgCalendar.GetHeight() - 5 * m_scaleFactor);
			dc.SetTextColor(RGB(0,0,0));
			dc.DrawText(szDay, -1, &rcCal, DT_CENTER | DT_BOTTOM | DT_SINGLELINE | DT_END_ELLIPSIS);
		}
		else
			dc.DrawText(m_pInfo->nd.pszTitle, -1, &rcHead, uFormat);
		dc.SelectFont(oldFont);
		
		bHandled = FALSE;
		return 0;
	}

	LRESULT OnTimer(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
	{
		if (wParam == IDT_TIMER_MENU_ANIMATION)
		{
			m_iStep = m_iStep / 2;
			RECT animRect; CopyRect(&animRect, &m_animRect);
			animRect.top = animRect.top + m_iStep;
			MoveWindow(&animRect, TRUE);

			if (m_iStep == 0)
			{
				KillTimer(IDT_TIMER_MENU_ANIMATION);
				m_bAnimating = FALSE;
				m_iStep = 0;
			}	

			bHandled = TRUE;
			return 0;
		}

		bHandled = FALSE;
		return 0;
	}

	LRESULT OnStartAnim(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		ModifyShape();
		StartAnimation();
		return 0;
	}
	
	void ModifyTopShape()
	{
		int nUsedRects = 3 * m_scaleFactor;
		m_heightHrgnTop = nUsedRects;

		if (m_hrgnTop != NULL)
		{
			DeleteObject(m_hrgnTop);
			m_hrgnTop = NULL;
		}

		RECT rcParent; GetParent().GetClientRect(&rcParent);
		int iWidth = rcParent.right - 2 * BORDER_WIDTH * m_scaleFactor;

		RGNDATA *pRgnData;
		RECT *pRect;		
		DWORD dwTemp = sizeof(RGNDATAHEADER) + nUsedRects * sizeof(RECT);
		pRgnData = (RGNDATA*)malloc(dwTemp);	
		pRgnData->rdh.dwSize = sizeof(RGNDATAHEADER);
		pRgnData->rdh.iType = RDH_RECTANGLES;
		pRgnData->rdh.nCount = nUsedRects;
		pRgnData->rdh.nRgnSize = 0;
		pRgnData->rdh.rcBound.left = pRgnData->rdh.rcBound.top = 0;
		pRgnData->rdh.rcBound.right = iWidth;
		pRgnData->rdh.rcBound.bottom = nUsedRects;		


		if (nUsedRects == 3)
		{
			// 1st
			pRect = &(((RECT *)&pRgnData->Buffer)[0]);
			pRect->top = 0;
			pRect->bottom = 1;
			pRect->left = 2;
			pRect->right = iWidth - 2;

			// 2nd
			pRect = &(((RECT *)&pRgnData->Buffer)[1]);
			pRect->top = 1;
			pRect->bottom = 2;
			pRect->left = 1;
			pRect->right = iWidth - 1;

			// 3rd
			pRect = &(((RECT *)&pRgnData->Buffer)[2]);
			pRect->top = 2;
			pRect->bottom = 3;
			pRect->left = 0;
			pRect->right = iWidth;
		}
		else if (nUsedRects == 6)
		{
			// 1st
			pRect = &(((RECT *)&pRgnData->Buffer)[0]);
			pRect->top = 0;
			pRect->bottom = 1;
			pRect->left = 5;
			pRect->right = iWidth - 5;

			// 2nd
			pRect = &(((RECT *)&pRgnData->Buffer)[1]);
			pRect->top = 1;
			pRect->bottom = 2;
			pRect->left = 3;
			pRect->right = iWidth - 3;

			// 3rd
			pRect = &(((RECT *)&pRgnData->Buffer)[2]);
			pRect->top = 2;
			pRect->bottom = 3;
			pRect->left = 2;
			pRect->right = iWidth - 2;

			// 4th
			pRect = &(((RECT *)&pRgnData->Buffer)[3]);
			pRect->top = 3;
			pRect->bottom = 4;
			pRect->left = 1;
			pRect->right = iWidth - 1;

			// 5th
			pRect = &(((RECT *)&pRgnData->Buffer)[4]);
			pRect->top = 4;
			pRect->bottom = 5;
			pRect->left = 1;
			pRect->right = iWidth - 1;

			// 6th
			pRect = &(((RECT *)&pRgnData->Buffer)[5]);
			pRect->top = 5;
			pRect->bottom = 6;
			pRect->left = 0;
			pRect->right = iWidth;

		}

		m_hrgnTop = ExtCreateRegion(NULL, dwTemp, pRgnData);
		free(pRgnData);

	}

	void AddStyle()
	{
	}

	void SubmitForm()
	{
		// must be implemented in a derived class
		ATLASSERT(FALSE);
	}


public:
	RECT m_animRect; 
	BOOL m_bAnimating;
	int m_iStep;
	int m_scaleFactor;
	int m_height;
	BOOL m_bMouseIsDown;
	HRGN m_hrgnTop;
	int m_heightHrgnTop;
	int m_iBodyHeight;
	NOTIFICATIONINFO* m_pInfo;
	COLORREF m_clCaptionText;
	CFontHandle m_fCaption;
	CFontHandle m_fCalendar;
	CxImage m_imgCalendar;
	CxImage m_imgHeader;
	BOOL m_bEnableAnimation;

	void ModifyShape()
	{
		m_height = m_iBodyHeight + m_imgHeader.GetHeight(); 

		RECT rcParent; GetParent().GetClientRect(&rcParent);
		int iWidth = rcParent.right - 2 * BORDER_WIDTH * m_scaleFactor;

		HRGN hrgnBottom = ::CreateRectRgn(0, 0, iWidth, m_height - m_heightHrgnTop);
		::OffsetRgn(hrgnBottom, 0, m_heightHrgnTop);

		CombineRgn(hrgnBottom, m_hrgnTop, hrgnBottom, RGN_OR);

		SetWindowRgn(hrgnBottom, FALSE);
	}

	void StartAnimation()
	{
		int m_itemHeight = 5 * m_scaleFactor;
		// window position
		m_bAnimating = (m_bEnableAnimation) ? TRUE : FALSE;
		m_iStep = (m_bEnableAnimation) ? m_itemHeight / 3 : 0;

		RECT rcParent; GetParent().GetClientRect(&rcParent);

		m_animRect.left = BORDER_WIDTH * m_scaleFactor;
		m_animRect.right = rcParent.right - BORDER_WIDTH * m_scaleFactor;
		m_animRect.top = rcParent.bottom - m_height;
		m_animRect.bottom = rcParent.bottom;


		RECT animRect; CopyRect(&animRect, &m_animRect);
		animRect.top = animRect.top + m_iStep;
		MoveWindow(&animRect, TRUE);

		if (m_bEnableAnimation)
			SetTimer(IDT_TIMER_MENU_ANIMATION, TMR_MENU_ANIMATION);
	}



};

class CHTMLNotifWindow : public CNotifWindow<CHTMLNotifWindow>
{
public:
	DECLARE_WND_CLASS(NULL)

	typedef CNotifWindow<CHTMLNotifWindow> Super;

	CHtmlCtrl m_html;

	BOOL SetNotification(LPNOTIFICATIONINFO pData)
	{
		Super::SetNotification(pData);

		m_html.Clear();
		m_html.AddStyle(CUSTOM_CSS);

		RECT rcParent; GetParent().GetClientRect(&rcParent);
		int iWidth = rcParent.right - 2 * BORDER_WIDTH * m_scaleFactor;
		m_html.ResizeClient(iWidth, 0);

		m_html.SendMessage(WM_SETTEXT, 0, (LPARAM)(LPCTSTR)_T(""));
		m_html.AddHTML(m_pInfo->nd.pszHTML);
		m_html.EndOfSource();

		m_iBodyHeight = m_html.GetLayoutHeight() + 25 * m_scaleFactor; 

		return TRUE;
	}

	void SubmitForm()
	{
		IDispatch* pDisp = NULL;
		m_html.GetDocumentDispatch(&pDisp);

		IPIEHTMLDocument *pHTMLDocument = NULL; 
		HRESULT hr = pDisp->QueryInterface(IID_IPIEHTMLDocument, (void**)&pHTMLDocument);

		if (FAILED(hr)) { 
			if (hr == E_NOINTERFACE) 
				LOG(L"Interface doesn't exist\n"); 
			if (hr == E_NOTIMPL) 
				LOG(L"Queryinterface not implemented\n"); 
			return;
		}

		IPIEHTMLElementCollection *pElementCollection = NULL; 
		if ( pHTMLDocument->get_forms ( &pElementCollection ) == S_OK ) 
		{
			if (pElementCollection != NULL)
			{
				long celem = 0;
			
				if ( pElementCollection->get_length( &celem ) == S_OK )
				{
					for ( long i = 0; i < celem; i++ )
					{
						VARIANT vtIndex;
						vtIndex.vt = VT_I4;
						vtIndex.lVal = i;


						IDispatch* pItem = NULL;
						if( pElementCollection->item( vtIndex, vtIndex, (IDispatch**)&pItem ) == S_OK )
						{
							IPIEHTMLFormElement* form = NULL;
							if( pItem->QueryInterface(__uuidof(IPIEHTMLFormElement), (void**)&form ) == S_OK )
							{
								form->put_method((BSTR)L"get");
								hr = form->submit();
								LOG(L"Tutto OK\n");
							}
						}
					}
				}
			}
		}
	}


	BEGIN_MSG_MAP(CHTMLNotifWindow)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER(UM_STARTANIM, OnStartAnim)
		CHAIN_MSG_MAP(Super)
	END_MSG_MAP()

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		// html control
		InitHTMLControl(ModuleHelper::GetModuleInstance());

		m_html.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, IDC_HTMLVIEW);
		m_html.SetFocus();
		m_html.SendMessage(WM_SETTEXT, 0, (LPARAM)(LPCTSTR)_T(""));

		bHandled = FALSE;
		return 0;
	}

	LRESULT OnSize(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
	{
		RECT rc; GetClientRect(&rc);
		rc.top += m_imgHeader.GetHeight();
		m_html.SetWindowPos(0, &rc, SWP_NOZORDER);

		bHandled = FALSE;
		return 0;
	}

};
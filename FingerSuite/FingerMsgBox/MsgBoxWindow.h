// WTLApp1View.h : interface of the CWTLApp1View class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

//#include "..\Common\ext\RegionBuilder.h"

#ifndef __FNGRBTN_H__
	#error MsgBoxWindows.h requires fngrbtn.h to be included first
#endif

#define UM_STARTANIM    WM_USER + 1 
#define UM_MINIMIZE     WM_USER + 2
#define BORDER_WIDTH    8
#define BOTTOM_BORDER_HEIGHT    3
#define LEFT_BORDER_WIDTH       3 
#define MIN_HEIGHT              80


#define IDC_BTN_OK      10001
#define IDC_BTN_CANCEL  10002
#define IDC_BTN_YES     10003
#define IDC_BTN_NO      10004
#define IDC_BTN_ABORT   10005
#define IDC_BTN_IGNORE  10006
#define IDC_BTN_RETRY   10007



class CMsgBoxWindow : public CWindowImpl<CMsgBoxWindow>
{
public:
	DECLARE_WND_CLASS(NULL)

	typedef CWindowImpl<CMsgBoxWindow> Super;

	// controls
	CFingerButton m_btn1;
	CFingerButton m_btn2;
	CFingerButton m_btn3;

	BOOL PreTranslateMessage(MSG* pMsg)
	{
		pMsg;
		return FALSE;
	}

	CMsgBoxWindow ()
	{
		m_iScaleFactor = 1;
	}

	HWND Create(HWND hWndParent)
	{
		m_btn1.SetArrayImageSet(m_arrDynamicImages);
		m_btn2.SetArrayImageSet(m_arrDynamicImages);
		m_btn3.SetArrayImageSet(m_arrDynamicImages);

		return Super::Create(hWndParent, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);  
	}

	void SetScaleFactor(int iScaleFactor)
	{
		m_iScaleFactor = iScaleFactor;
	}

	BOOL SetMsgBox(HWND hOwnrWnd, UINT uType, LPCTSTR pwszCaption, LPCTSTR pwszText)
	{
		m_hOwnrWnd = hOwnrWnd;
		m_uType = uType;

		lstrcpy(m_szCaption, pwszCaption);
		lstrcpy(m_szText, pwszText);

		ModifyShape();
		ModifyButtons();
		return TRUE;
	}

	HWND GetOwnerWnd()
	{
		return m_hOwnrWnd;
	}

	void DoPaint(CDCHandle dc)
	{
		RECT rc; GetClientRect(&rc);
	
		// fill background
		dc.FillSolidRect(&rc, m_clBkg);

		int sx = m_icnInfo.GetWidth();
		int sy = m_icnInfo.GetHeight();

		RECT rcHeader; 
		SetRect(&rcHeader, 0, 0, rc.right, m_imgHeader.GetHeight());
		m_imgHeader.Draw(dc, rcHeader);


		BOOL bIconDrawn = FALSE;
		if ((m_uType & MB_ICONEXCLAMATION) || (m_uType & MB_ICONWARNING))
		{
			m_icnWarning.Draw(dc);
			bIconDrawn = TRUE;
			goto iconend;
		}

		if ((m_uType & MB_ICONINFORMATION) || (m_uType & MB_ICONASTERISK))
		{
			m_icnInfo.Draw(dc);
			bIconDrawn = TRUE;
			goto iconend;
		}

		if (m_uType & MB_ICONQUESTION)
		{
			m_icnQuestion.Draw(dc);
			bIconDrawn = TRUE;
			goto iconend;
		}

		if ((m_uType & MB_ICONSTOP) || (m_uType & MB_ICONERROR) || (m_uType & MB_ICONHAND))
		{
			m_icnStop.Draw(dc);
			bIconDrawn = TRUE;
			goto iconend;
		}


iconend:

		// draw caption
		int border = BORDER_WIDTH * m_iScaleFactor / 2;
		RECT rcCaption;
		SetRect(&rcCaption, ((bIconDrawn) ? sx : border * m_iScaleFactor), 0, rc.right, sy);
		dc.SetBkMode(TRANSPARENT);
		dc.SetTextColor(m_clCaptionText);
		CFont oldFont = dc.SelectFont(m_fCaption);

		UINT uFormat = DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS;
		if (m_bCenterCaption)
		{
			uFormat |= DT_CENTER;
			rcCaption.left -= (bIconDrawn) ? sx : 0;
		}

		dc.DrawText(m_szCaption, -1, &rcCaption, uFormat);
		dc.SelectFont(oldFont);

		// draw text
//		dc.SetBkMode(TRANSPARENT);
		dc.SetBkColor( RGB(255,216,0) );
		dc.SetTextColor(m_clText);
		oldFont = dc.SelectFont(m_fText);
		dc.DrawText(m_szText, -1, &m_rcText, DT_LEFT | DT_WORDBREAK | DT_END_ELLIPSIS);
		dc.SelectFont(oldFont);

		// draw button line (dotted line 1px)
		CPen pen2; pen2.CreatePen(PS_DASH, 1, m_clLine);
		CPen penOld2 = dc.SelectPen(pen2);
		dc.MoveTo(rc.left, rc.bottom - m_arrDynamicImages[0].GetHeight() - BOTTOM_BORDER_HEIGHT * m_iScaleFactor - 2);
		dc.LineTo(rc.right, rc.bottom - m_arrDynamicImages[0].GetHeight() - BOTTOM_BORDER_HEIGHT * m_iScaleFactor - 2);
		dc.SelectPen(penOld2);	

	}


	BEGIN_MSG_MAP(CMsgBoxWindow)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackground)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER(WM_COMMAND, OnCommand)
	END_MSG_MAP()

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		//ModifyShape();
		return 0;
	}

	LRESULT OnSize(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
	{
		if(wParam != SIZE_MINIMIZED)
		{
			RECT rc; GetClientRect(&rc);
		}

		bHandled = FALSE;
		return 0;
	}

	LRESULT OnEraseBackground(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		return 1;
	}

	LRESULT OnPaint(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		if(wParam != NULL)
		{
			DoPaint((HDC)wParam);
		}
		else
		{
			//CPaintDC dc(m_hWnd);
			CBufferedPaintDC dc(m_hWnd, 0);
			DoPaint(dc.m_hDC);
		}
		return 0;
	}

	LRESULT OnCommand(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		int wID = LOWORD(wParam);
		switch (wID)
		{
		case IDC_BTN_YES:
			SetMsgBoxResult(IDYES);
			break;
		case IDC_BTN_NO:
			SetMsgBoxResult(IDNO);
			break;
		case IDC_BTN_OK:
			SetMsgBoxResult(IDOK);
			break;
		case IDC_BTN_CANCEL:
			SetMsgBoxResult(IDCANCEL);
			break;
		case IDC_BTN_ABORT:
			SetMsgBoxResult(IDABORT);
			break;
		case IDC_BTN_IGNORE:
			SetMsgBoxResult(IDIGNORE);
			break;
		case IDC_BTN_RETRY:
			SetMsgBoxResult(IDRETRY);
			break;
		}
		GetParent().SendMessage(UM_MINIMIZE, 0, 0);
		return 0;
	}

public:
	CxImage m_icnStop;
	CxImage m_icnWarning;
	CxImage m_icnQuestion;
	CxImage m_icnInfo;
	CxImage m_imgHeader;

	CFontHandle m_fText;
	CFontHandle m_fCaption;
	CFontHandle m_fBtn;

	int m_xCaption;
	int m_yCaption;
	int m_xText;
	int m_yText;
	COLORREF m_clCaptionText;
	COLORREF m_clText;
	COLORREF m_clBtnText;
	COLORREF m_clBtnSelText;
	COLORREF m_clBkg;
	COLORREF m_clLine;

	BOOL m_bDisabledTransp;
	COLORREF m_clNoTranspBackground;
	DWORD m_iTransp;
	BOOL m_bCenterCaption;

	CxImage m_arrDynamicImages[9];

private:
	HWND m_hOwnrWnd;
	UINT m_uType;
	WCHAR m_szText[500];
	WCHAR m_szCaption[500];

	int m_iScaleFactor;



	RECT m_rcText;

	int CalculateTextHeight()
	{
		/*
		RECT rcText;
		SetRect(&rcText, 0, 
			             sy, 
						 rc.right, 
						 rc.bottom - (m_arrImages[0].GetHeight() + BOTTOM_BORDER_HEIGHT * m_iScaleFactor));
		*/
		RECT rcParent; GetParent().GetClientRect(&rcParent);
		int iHeight = 0;
		int iWidth = rcParent.right - rcParent.left - 2 * BORDER_WIDTH * m_iScaleFactor - 2 * LEFT_BORDER_WIDTH * m_iScaleFactor;


//		RECT rc; GetClientRect(&rc);

		SetRect(&m_rcText, 0, 
			               0, 
					       iWidth, 
						   0);

		CDC dc = ::GetDC(m_hWnd);
		CFont oldFont = dc.SelectFont(m_fText);
		dc.DrawText(m_szText, -1, &m_rcText, DT_LEFT | DT_WORDBREAK | DT_END_ELLIPSIS | DT_CALCRECT);
		dc.SelectFont(oldFont);

		iHeight = m_rcText.bottom;

		OffsetRect(&m_rcText, LEFT_BORDER_WIDTH * m_iScaleFactor, m_imgHeader.GetHeight());

		return max( (int)((float)iHeight * 1.5f), MIN_HEIGHT * m_iScaleFactor );
	}

public:
	void ModifyShape()
	{	
		int nUsedRects = 3 * m_iScaleFactor;

		RECT rcParent; GetParent().GetClientRect(&rcParent);
		int iWidth = rcParent.right - rcParent.left - 2 * BORDER_WIDTH * m_iScaleFactor;
		int iHeight = CalculateTextHeight() + m_imgHeader.GetHeight() + m_arrDynamicImages[0].GetHeight() + BOTTOM_BORDER_HEIGHT * m_iScaleFactor;
		//iHeight = max( iHeight, 160 * m_iScaleFactor);

		iHeight = min(iHeight, rcParent.bottom - rcParent.top - 2 * m_iScaleFactor);
		
		// upper region
		RGNDATA *pRgnData;
		RECT *pRect;		
		DWORD dwTemp = sizeof(RGNDATAHEADER) + nUsedRects * sizeof(RECT);
		pRgnData = (RGNDATA*)malloc(dwTemp);	
		pRgnData->rdh.dwSize = sizeof(RGNDATAHEADER);
		pRgnData->rdh.iType = RDH_RECTANGLES;
		pRgnData->rdh.nCount = nUsedRects;
		pRgnData->rdh.nRgnSize = 0;
		pRgnData->rdh.rcBound.left = pRgnData->rdh.rcBound.top=0;
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

		HRGN hrgnTop = ExtCreateRegion(NULL, dwTemp, pRgnData);
		free(pRgnData);
		
		// lower region
		dwTemp = sizeof(RGNDATAHEADER) + nUsedRects * sizeof(RECT);
		pRgnData = (RGNDATA*)malloc(dwTemp);	
		pRgnData->rdh.dwSize = sizeof(RGNDATAHEADER);
		pRgnData->rdh.iType = RDH_RECTANGLES;
		pRgnData->rdh.nCount = nUsedRects;
		pRgnData->rdh.nRgnSize = 0;
		pRgnData->rdh.rcBound.left = pRgnData->rdh.rcBound.top=0;
		pRgnData->rdh.rcBound.right = iWidth;
		pRgnData->rdh.rcBound.bottom = nUsedRects;		

		if (nUsedRects == 3)
		{
			// 1st
			pRect = &(((RECT *)&pRgnData->Buffer)[2]);
			pRect->top = 2;
			pRect->bottom = 3;
			pRect->left = 2;
			pRect->right = iWidth - 2;

			// 2nd
			pRect = &(((RECT *)&pRgnData->Buffer)[1]);
			pRect->top = 1;
			pRect->bottom = 2;
			pRect->left = 1;
			pRect->right = iWidth - 1;

			// 3rd
			pRect = &(((RECT *)&pRgnData->Buffer)[0]);
			pRect->top = 0;
			pRect->bottom = 1;
			pRect->left = 0;
			pRect->right = iWidth;
		}
		else if (nUsedRects == 6)
		{
			// 1st
			pRect = &(((RECT *)&pRgnData->Buffer)[5]);
			pRect->top = 5;
			pRect->bottom = 6;
			pRect->left = 5;
			pRect->right = iWidth - 5;

			// 2nd
			pRect = &(((RECT *)&pRgnData->Buffer)[4]);
			pRect->top = 4;
			pRect->bottom = 5;
			pRect->left = 3;
			pRect->right = iWidth - 3;

			// 3rd
			pRect = &(((RECT *)&pRgnData->Buffer)[3]);
			pRect->top = 3;
			pRect->bottom = 4;
			pRect->left = 2;
			pRect->right = iWidth - 2;

			// 4th
			pRect = &(((RECT *)&pRgnData->Buffer)[2]);
			pRect->top = 2;
			pRect->bottom = 3;
			pRect->left = 1;
			pRect->right = iWidth - 1;

			// 5th
			pRect = &(((RECT *)&pRgnData->Buffer)[1]);
			pRect->top = 1;
			pRect->bottom = 2;
			pRect->left = 1;
			pRect->right = iWidth - 1;

			// 6th
			pRect = &(((RECT *)&pRgnData->Buffer)[0]);
			pRect->top = 0;
			pRect->bottom = 1;
			pRect->left = 0;
			pRect->right = iWidth;

		}

		HRGN hrgnBottom = ExtCreateRegion(NULL, dwTemp, pRgnData);
		::OffsetRgn(hrgnBottom, 0, iHeight - nUsedRects);
		free(pRgnData);

		
		HRGN hrgnMiddle = ::CreateRectRgn(0, 0, iWidth, iHeight - 2 * nUsedRects);
		::OffsetRgn(hrgnMiddle, 0, nUsedRects);

		CombineRgn(hrgnMiddle, hrgnTop, hrgnMiddle, RGN_OR);
		CombineRgn(hrgnMiddle, hrgnBottom, hrgnMiddle, RGN_OR);

		SetWindowRgn(hrgnMiddle, FALSE);

		ResizeClient(iWidth, iHeight, FALSE);
		CenterWindow();
	}

	void ModifyButtons()
	{
		RECT rc; GetClientRect(&rc);
		int btnHeight = m_arrDynamicImages[0].GetHeight();
	
		if (m_btn1.IsWindow())
			m_btn1.DestroyWindow();
		if (m_btn2.IsWindow())
			m_btn2.DestroyWindow();
		if (m_btn3.IsWindow())
			m_btn3.DestroyWindow();

		int type = m_uType & 0x000F;

		if (type == MB_OKCANCEL)
		{
			RECT rcBtn; 
			SetRect(&rcBtn, 0, rc.bottom - btnHeight - BOTTOM_BORDER_HEIGHT * m_iScaleFactor, rc.right / 2, rc.bottom);

			RECT rcBtn1; CopyRect(&rcBtn1, &rcBtn);
			InflateRect(&rcBtn1, -5, 0);

			m_btn1.Create(m_hWnd, FNGRBTN_TYPE_DYNAMIC, TRUE, m_clBkg, m_clBtnText, m_clBtnSelText, LoadResourceString(IDS_OK), m_fBtn, IDC_BTN_OK);
			m_btn1.MoveWindow(&rcBtn1);

			RECT rcBtn2; CopyRect(&rcBtn2, &rcBtn);
			OffsetRect(&rcBtn2, rc.right/2, 0);
			InflateRect(&rcBtn2, -4, 0);

			m_btn2.Create(m_hWnd, FNGRBTN_TYPE_DYNAMIC, TRUE, m_clBkg, m_clBtnText, m_clBtnSelText, LoadResourceString(IDS_CANCEL), m_fBtn, IDC_BTN_CANCEL);
			m_btn2.MoveWindow(&rcBtn2);		
		}

		if (type == MB_YESNO)
		{
			RECT rcBtn; 
			SetRect(&rcBtn, 0, rc.bottom - btnHeight - BOTTOM_BORDER_HEIGHT * m_iScaleFactor, rc.right / 2, rc.bottom);

			RECT rcBtn1; CopyRect(&rcBtn1, &rcBtn);
			InflateRect(&rcBtn1, -5, 0);

			m_btn1.Create(m_hWnd, FNGRBTN_TYPE_DYNAMIC, TRUE, m_clBkg, m_clBtnText, m_clBtnSelText, LoadResourceString(IDS_YES), m_fBtn, IDC_BTN_YES);
			m_btn1.MoveWindow(&rcBtn1);

			RECT rcBtn2; CopyRect(&rcBtn2, &rcBtn);
			OffsetRect(&rcBtn2, rc.right/2, 0);
			InflateRect(&rcBtn2, -4, 0);

			m_btn2.Create(m_hWnd, FNGRBTN_TYPE_DYNAMIC, TRUE, m_clBkg, m_clBtnText, m_clBtnSelText, LoadResourceString(IDS_NO), m_fBtn, IDC_BTN_NO);
			m_btn2.MoveWindow(&rcBtn2);
		}

		if (type == MB_YESNOCANCEL)
		{
			RECT rcBtn; 
			SetRect(&rcBtn, 0, rc.bottom - btnHeight - BOTTOM_BORDER_HEIGHT * m_iScaleFactor, rc.right / 3, rc.bottom);

			RECT rcBtn1; CopyRect(&rcBtn1, &rcBtn);
			InflateRect(&rcBtn1, -5, 0);

			m_btn1.Create(m_hWnd, FNGRBTN_TYPE_DYNAMIC, TRUE, m_clBkg, m_clBtnText, m_clBtnSelText, LoadResourceString(IDS_YES), m_fBtn, IDC_BTN_YES);
			m_btn1.MoveWindow(&rcBtn1);

			RECT rcBtn2; CopyRect(&rcBtn2, &rcBtn);
			OffsetRect(&rcBtn2, rc.right / 3, 0);
			InflateRect(&rcBtn2, -4, 0);

			m_btn2.Create(m_hWnd, FNGRBTN_TYPE_DYNAMIC, TRUE, m_clBkg, m_clBtnText, m_clBtnSelText, LoadResourceString(IDS_NO), m_fBtn, IDC_BTN_NO);
			m_btn2.MoveWindow(&rcBtn2);

			RECT rcBtn3; CopyRect(&rcBtn3, &rcBtn);
			OffsetRect(&rcBtn3, 2 * rc.right / 3, 0);
			InflateRect(&rcBtn3, -4, 0);

			m_btn3.Create(m_hWnd, FNGRBTN_TYPE_DYNAMIC, TRUE, m_clBkg, m_clBtnText, m_clBtnSelText, LoadResourceString(IDS_CANCEL), m_fBtn, IDC_BTN_CANCEL);
			m_btn3.MoveWindow(&rcBtn3);
		}

		if (type == MB_OK)
		{
			RECT rcBtn; 
			SetRect(&rcBtn, 0, rc.bottom - btnHeight - BOTTOM_BORDER_HEIGHT * m_iScaleFactor, rc.right, rc.bottom);
			InflateRect(&rcBtn, -5, 0);

			m_btn1.Create(m_hWnd, FNGRBTN_TYPE_DYNAMIC, TRUE, m_clBkg, m_clBtnText, m_clBtnSelText, LoadResourceString(IDS_OK), m_fBtn, IDC_BTN_OK);
			m_btn1.MoveWindow(&rcBtn);
		}


		if (type == MB_RETRYCANCEL)
		{
			RECT rcBtn; 
			SetRect(&rcBtn, 0, rc.bottom - btnHeight - BOTTOM_BORDER_HEIGHT * m_iScaleFactor, rc.right / 2, rc.bottom);

			RECT rcBtn1; CopyRect(&rcBtn1, &rcBtn);
			InflateRect(&rcBtn1, -5, 0);

			m_btn1.Create(m_hWnd, FNGRBTN_TYPE_DYNAMIC, TRUE, m_clBkg, m_clBtnText, m_clBtnSelText, LoadResourceString(IDS_RETRY), m_fBtn, IDC_BTN_RETRY);
			m_btn1.MoveWindow(&rcBtn1);

			RECT rcBtn2; CopyRect(&rcBtn2, &rcBtn);
			OffsetRect(&rcBtn2, rc.right/2, 0);
			InflateRect(&rcBtn2, -4, 0);

			m_btn2.Create(m_hWnd, FNGRBTN_TYPE_DYNAMIC, TRUE, m_clBkg, m_clBtnText, m_clBtnSelText, LoadResourceString(IDS_CANCEL), m_fBtn, IDC_BTN_CANCEL);
			m_btn2.MoveWindow(&rcBtn2);
		}

		if (type == MB_ABORTRETRYIGNORE)
		{
			RECT rcBtn; 
			SetRect(&rcBtn, 0, rc.bottom - btnHeight - BOTTOM_BORDER_HEIGHT * m_iScaleFactor, rc.right / 3, rc.bottom);

			RECT rcBtn1; CopyRect(&rcBtn1, &rcBtn);
			InflateRect(&rcBtn1, -5, 0);

			m_btn1.Create(m_hWnd, FNGRBTN_TYPE_DYNAMIC, TRUE, m_clBkg, m_clBtnText, m_clBtnSelText, LoadResourceString(IDS_ABORT), m_fBtn, IDC_BTN_ABORT);
			m_btn1.MoveWindow(&rcBtn1);

			RECT rcBtn2; CopyRect(&rcBtn2, &rcBtn);
			OffsetRect(&rcBtn2, rc.right / 3, 0);
			InflateRect(&rcBtn2, -4, 0);

			m_btn2.Create(m_hWnd, FNGRBTN_TYPE_DYNAMIC, TRUE, m_clBkg, m_clBtnText, m_clBtnSelText, LoadResourceString(IDS_RETRY), m_fBtn, IDC_BTN_RETRY);
			m_btn2.MoveWindow(&rcBtn2);

			RECT rcBtn3; CopyRect(&rcBtn3, &rcBtn);
			OffsetRect(&rcBtn3, 2 * rc.right / 3, 0);
			InflateRect(&rcBtn3, -4, 0);

			m_btn3.Create(m_hWnd, FNGRBTN_TYPE_DYNAMIC, TRUE, m_clBkg, m_clBtnText, m_clBtnSelText, LoadResourceString(IDS_IGNORE), m_fBtn, IDC_BTN_IGNORE);
			m_btn3.MoveWindow(&rcBtn3);
		}

		//return TRUE;
	}

};









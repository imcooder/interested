#ifndef __FNGRBTN_H__
#define __FNGRBTN_H__

#pragma once

#ifndef __cplusplus
	#error ATL requires C++ compilation (use a .cpp suffix)
#endif

#ifndef __ATLAPP_H__
	#error fngrscrl.h requires atlapp.h to be included first
#endif

#ifndef __ATLWIN_H__
	#error fngrscrl.h requires atlwin.h to be included first
#endif

#ifndef __CXIMAGE_H
	#error fngrscrl.h requires ximage.h to be included first
#endif


#define FNGRBTN_CLASS		  L"FingerBtn"
//#define NS_FINGERBTN         (WS_CHILD | WS_CLIPSIBLINGS)

#define  FNGRBTN_TYPE_TEXT    0x00
#define  FNGRBTN_TYPE_STATIC  0x01
#define  FNGRBTN_TYPE_DYNAMIC 0x02
#define  FNGRBTN_TYPE_ICON    0x04

typedef enum enumState
{
	NOT_DEFINED = -1,
	NORMAL = 0,
	PUSHED = 3,
	FOCUSED = 6
} STATE;



inline void AddDynamicSet(CxImage arrImages[], STATE state, LPCTSTR szFileNameLeft, LPCTSTR szFileNameCenter, LPCTSTR szFileNameRight)
{
	arrImages[state].Load(szFileNameLeft, CXIMAGE_FORMAT_PNG);
	arrImages[state + 1].Load(szFileNameCenter, CXIMAGE_FORMAT_PNG);
	arrImages[state + 2].Load(szFileNameRight, CXIMAGE_FORMAT_PNG);
}

inline void AddStaticSet(CxImage arrImages[], STATE state, LPCTSTR szFileName)
{
	arrImages[state].Load(szFileName, CXIMAGE_FORMAT_PNG);
}



class CFingerButton : public CWindowImpl<CFingerButton>
{
public:
	DECLARE_WND_CLASS(NULL)

	typedef CWindowImpl<CFingerButton>	Super;
	
	// Constructor/Destructor
	CFingerButton() : 
			m_fNormal(1), 
			m_fPressed(0),
			m_fFocused(0)
	{
	}

	~CFingerButton()
	{
	}

	// Creation
	HWND Create(HWND hParentWnd, DWORD type, BOOL bVisible, COLORREF clBkg, COLORREF clBtnText, COLORREF clBtnSelText,
						LPCTSTR szText, HFONT fBtn, UINT nID)
	{
		SetText(szText);
		m_type = type;
		m_fNormal = 1; 
		m_fPressed = 0;
		m_fFocused = 0;
		m_clBkg = clBkg;
		m_clBtnText = clBtnText;
		m_clBtnSelText = clBtnSelText;
		m_fBtn = fBtn;
		m_icon = NULL;
		return Super::Create(hParentWnd, rcDefault, FNGRBTN_CLASS, (bVisible ? WS_VISIBLE  : 0), 0, nID);
	};

	HWND Create(HWND hParentWnd, DWORD type, BOOL bVisible, COLORREF clBkg, int iconSize, UINT nID)
	{
		m_type = type;
		m_fNormal = 1; 
		m_fPressed = 0;
		m_fFocused = 0;
		m_icon = NULL;
		m_iconSize = iconSize;
		m_clBkg = clBkg;
		return Super::Create(hParentWnd, rcDefault, FNGRBTN_CLASS, (bVisible ? WS_VISIBLE  : 0), 0, nID);
	};

	/*
	void MoveUpDownWindow(int offset)
	{
		
		RECT rc; GetWindowRect(&rc);
		SetWindowPos(NULL, rc.left, rc.top + offset, 0, 0, SWP_NOZORDER | SWP_NOSIZE);

		LOG(L"rc.top = %d offset = %d\n", rc.top, offset);
	}
	*/

	void SetArrayImageSet(CxImage* arrImages)
	{
		m_arrImages = arrImages;
	}

	// Overrideables
	void DoPaint(CDCHandle dc)
	{
		RECT rc; GetClientRect(&rc);


		// paint background
		dc.FillSolidRect(&rc, m_clBkg);


		STATE state = NOT_DEFINED;
		COLORREF clText = 0;
		if (m_fNormal)
		{
			state = NORMAL;
			clText = m_clBtnText;
		}
		if (m_fPressed)
		{
			state = PUSHED;
			clText = m_clBtnSelText;
		}
		if (m_fFocused)
			state = FOCUSED;

		if (m_type & FNGRBTN_TYPE_DYNAMIC)
		{
			int x = m_arrImages[0].GetWidth();
			int y = m_arrImages[0].GetHeight();

			int iWidth =  (rc.right - rc.left) - 2 * x;

			m_arrImages[state].Draw(dc, 0, 0);
			m_arrImages[state + 1].Draw(dc, x, 0, iWidth, y);
			m_arrImages[state + 2].Draw(dc, x + iWidth, 0);
		}

		if (m_type & FNGRBTN_TYPE_STATIC)
		{
			m_arrImages[state].Draw(dc);
		}

		// draw icon
		if ((m_type & FNGRBTN_TYPE_ICON) && (m_icon != NULL))
		{
			int px = (rc.right - m_iconSize) / 2;
			int py = (rc.bottom - m_iconSize) / 2;

			m_icon.DrawIconEx(dc, px, py, m_iconSize, m_iconSize);
		}

		// draw icon 2
		if ((m_type & FNGRBTN_TYPE_ICON) && (m_icon2 != NULL))
		{
			int px = (rc.right - m_iconSize) / 2;
			int py = (rc.bottom - m_iconSize) / 2;

			m_icon2.DrawIconEx(dc, px, py, m_iconSize, m_iconSize);
		}

		if (!(m_type & FNGRBTN_TYPE_ICON))
		{
			CFont oldFont = dc.SelectFont(m_fBtn);
			dc.SetBkMode(TRANSPARENT);
			dc.SetTextColor(clText);
			dc.DrawText(m_text, -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE); 
			dc.SelectFont(oldFont);
		}

	}

	void SetText(LPCTSTR szText)
	{
		if (szText != NULL)
			lstrcpy(m_text, szText);
	}

	void SetIcon (HICON hIcon)
	{
		m_icon = hIcon;

		Invalidate();
		UpdateWindow();
	}

	void SetIcon2 (HICON hIcon)
	{
		m_icon2 = hIcon;

		//Invalidate();
		//UpdateWindow();
	}

// Message map and handlers
	BEGIN_MSG_MAP(CBitmapButton)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackground)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
		MESSAGE_HANDLER(WM_LBUTTONUP, OnLButtonUp)
		MESSAGE_HANDLER(WM_CAPTURECHANGED, OnCaptureChanged)
		MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
	END_MSG_MAP()

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		return 0;
	}

	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		bHandled = FALSE;
		return 1;
	}

	LRESULT OnEraseBackground(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		return 1;   // no background needed
	}

	LRESULT OnPaint(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		if(wParam != NULL)
		{
			DoPaint((HDC)wParam);
		}
		else
		{
			CBufferedPaintDC dc(m_hWnd, 0);
			DoPaint(dc.m_hDC);
		}
		return 0;
	}

	LRESULT OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
	{
		SetCapture();
		if(::GetCapture() == m_hWnd)
		{
			m_fPressed = 1;
			m_fNormal = 0;
			Invalidate();
			UpdateWindow();
		}
		return 0;
	}

	LRESULT OnLButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
	{
		if(::GetCapture() == m_hWnd)
		{
			if(m_fPressed == 1)
				::SendMessage(GetParent(), WM_COMMAND, MAKEWPARAM(GetDlgCtrlID(), BN_CLICKED), (LPARAM)m_hWnd);
			::ReleaseCapture();
		}
		return 0;
	}

	LRESULT OnCaptureChanged(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		if(m_fPressed == 1)
		{
			m_fPressed = 0;
			m_fNormal = 1;
			Invalidate();
			UpdateWindow();
		}
		bHandled = FALSE;
		return 1;
	}

	LRESULT OnMouseMove(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled)
	{
		if(::GetCapture() == m_hWnd)
		{
			POINT ptCursor = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
			ClientToScreen(&ptCursor);
			RECT rect = { 0 };
			GetWindowRect(&rect);
			unsigned int uPressed = ::PtInRect(&rect, ptCursor) ? 1 : 0;
			if(m_fPressed != uPressed)
			{
				m_fPressed = uPressed;
				m_fNormal = 1;
				Invalidate();
				UpdateWindow();
			}
		}
		return 1;
	}

public:
	CxImage* m_arrImages;

protected:
	DWORD m_type;

	// Internal states
	unsigned m_fNormal:1;
	unsigned m_fPressed:1;
	unsigned m_fFocused:1;

	COLORREF m_clBtnText;
	COLORREF m_clBtnSelText;
	COLORREF m_clBkg;

	WCHAR m_text[32];
	CFontHandle m_fBtn;

	CIconHandle m_icon;
	CIconHandle m_icon2;
	int m_iconSize;

};

#endif // __FNGRBTN_H__
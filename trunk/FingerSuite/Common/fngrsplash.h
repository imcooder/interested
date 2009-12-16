#pragma once

#include <atlmisc.h>


class CSplashWindow :
	public CWindowImpl<CSplashWindow, CWindow, CWinTraits<WS_POPUP | WS_VISIBLE, WS_EX_TOOLWINDOW> >
{

public:
	CSplashWindow(HWND hParent = NULL, UINT nIconId = IDR_MAINFRAME, LPCWCHAR lpwszMessage = NULL)
		: m_hParent(hParent)
	{
		// Create the window rect (we will centre the window later)
		int sx = ::GetSystemMetrics(SM_CXSCREEN);
		int sy = ::GetSystemMetrics(SM_CYSCREEN);


		// Create the window

		int ix = 0;
		int iy = 0;
		if (nIconId != NULL)
		{
			m_hIcon = AtlLoadIconImage(nIconId);

			ICONINFO ii;
			BITMAP bmp;
			GetIconInfo(m_hIcon, &ii);
			GetObject(ii.hbmColor, sizeof(BITMAP), &bmp);

			ix = bmp.bmWidth;
			iy = bmp.bmHeight;

		}



		if (lpwszMessage != NULL)
			StringCchCopy(m_szText, MAX_PATH, lpwszMessage);

		CRect rect(0, 0, (int)(sx * 0.8), iy);
		
		if (!Create(NULL, rect))
		{
			ATLTRACE(_T("Failed to create splash window\n"));
			return;
		}
		UpdateWindow();
	}

	void Dismiss()
	{
		PostMessage(WM_CLOSE);
	}

	virtual void OnFinalMessage(HWND /*hWnd*/)
	{
		delete this;
	}

	BEGIN_MSG_MAP(CSplashWindow)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd)
	END_MSG_MAP()

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		CenterWindow(m_hParent);

		return 0;
	}

	LRESULT OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, 
		LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		CPaintDC dc(m_hWnd);
		RECT rc; GetClientRect(&rc);

		dc.FillSolidRect(&rc, RGB(226, 226, 226));

		dc.DrawIcon(0, 0, m_hIcon);

		RECT rcText;
		SetRect(&rcText, (rc.bottom - rc.top), rc.top, rc.right, rc.bottom);
		dc.SetBkMode(TRANSPARENT);
		dc.DrawText(m_szText, -1, &rcText, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS); 

		return 0;
	}

	LRESULT OnEraseBkgnd(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		// No need to paint a background

		return TRUE;
	}

private:
	HWND m_hParent;
	HICON m_hIcon;
	WCHAR m_szText[MAX_PATH];

};
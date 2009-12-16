// Test99View.h : interface of the CTest99View class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __cplusplus
  #error ATL requires C++ compilation (use a .cpp suffix)
#endif

#ifndef __ATLMISC_H__
  #error AboutView.h requires atlmisc.h to be included first
#endif

// Extracted from MFC 7.0 source code by Ma Weida
BOOL AfxExtractSubString(CString& rString, LPCTSTR lpszFullString,
	int iSubString, TCHAR chSep)
{
	if (lpszFullString == NULL)
		return FALSE;

	while (iSubString--)
	{
		lpszFullString = _tcschr(lpszFullString, chSep);
		if (lpszFullString == NULL)
		{
			rString.Empty();        // return empty string as well
			return FALSE;
		}
		lpszFullString++;       // point past the separator
	}
	LPCTSTR lpchEnd = _tcschr(lpszFullString, chSep);
	int nLen = (lpchEnd == NULL) ?
		lstrlen(lpszFullString) : (int)(lpchEnd - lpszFullString);
	ATLASSERT(nLen >= 0);
	memcpy(rString.GetBufferSetLength(nLen), lpszFullString, nLen*sizeof(TCHAR));
	return TRUE;
}

class CAboutView : public CWindowImpl<CAboutView>
{
public:
	DECLARE_WND_CLASS(NULL)

	BOOL PreTranslateMessage(MSG* pMsg)
	{
		pMsg;
		return FALSE;
	}

	void SetCredits(LPCTSTR lpszCredits)
	{
		m_strCredits = lpszCredits;
	}

	void SetBitmap(HBITMAP hBmp, int x, int y)
	{
		m_hBitmap = hBmp;
		m_ptPos.x = x;
		m_ptPos.y = y;
	}

	BEGIN_MSG_MAP(CAboutView)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
	END_MSG_MAP()

// Handler prototypes (uncomment arguments if needed):
//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	LRESULT OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		CPaintDC dc(m_hWnd);
		RECT rc; GetClientRect(&rc);

		//TODO: Add your drawing code here
		CFont cOldFont;

		CFont cFontNormal = dc.GetCurrentFont();
		LOGFONT lf; cFontNormal.GetLogFont(lf);

		lf.lfWeight = FW_BOLD;
		CFont cFontBold; cFontBold.CreateFontIndirect(&lf);

		lf.lfHeight -= 2;
		CFont cFontTitle; cFontTitle.CreateFontIndirect(&lf);

		dc.SetBkMode(TRANSPARENT);

		CString strSub;
		int nCount=0;
		// draw each line, based on specified type
		while(AfxExtractSubString(strSub, m_strCredits, nCount++, '\n'))
		{
			TCHAR nType = 0;

			if (!strSub.IsEmpty())
				nType = strSub.GetAt(0);

			switch(nType)
			{
				case '\t':	// title
					dc.SetTextColor(RGB(16,140,231));
					cOldFont = dc.SelectFont(cFontTitle);
					strSub.TrimLeft('\t');
					dc.DrawText(strSub, strSub.GetLength(), &rc, DT_TOP|DT_CENTER|DT_NOPREFIX | DT_SINGLELINE);
					dc.SelectFont(cOldFont);
					break;
				case '\r':	// bold
					dc.SetTextColor(RGB(0,0,0));
					cOldFont = dc.SelectFont(cFontBold);
					strSub.TrimLeft('\r');
					dc.DrawText(strSub, strSub.GetLength(), &rc, DT_TOP|DT_CENTER|DT_NOPREFIX | DT_SINGLELINE);
					dc.SelectFont(cOldFont);
					break;
				default:	// normal
					dc.SetTextColor(RGB(0,0,0));
					cOldFont = dc.SelectFont(cFontNormal);
					dc.DrawText(strSub, strSub.GetLength(), &rc, DT_TOP|DT_CENTER|DT_NOPREFIX | DT_SINGLELINE);
					dc.SelectFont(cOldFont);
					break;
			}
			// next line
			TEXTMETRIC tm;  
			dc.GetTextMetricsW(&tm);
			rc.top += tm.tmHeight;
		}
		return 0;
	}

private:
	CString m_strCredits;
	HBITMAP m_hBitmap;
	POINT m_ptPos;


};

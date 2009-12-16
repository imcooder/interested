#ifndef __FNGRBUFDC_H__
#define __FNGRBUFDC_H__

#pragma once

#ifndef __cplusplus
	#error ATL requires C++ compilation (use a .cpp suffix)
#endif

#ifndef __ATLAPP_H__
	#error atlgdi.h requires atlapp.h to be included first
#endif

#ifndef __ATLGDI_H__
	#error fngrbufdc.h requires atlgdi.h to be included first
#endif

///////////////////////////////////////////////////////////////////////////////
// Classes in this file:
//
// CBufferedPaintDC

class CBufferedPaintDC : public CDC
{
public:
	HWND m_hWnd;
	PAINTSTRUCT m_ps;

// Data members
	HDC m_hPaintDC;
	RECT m_rcClient;
	CBitmap m_bmp;
	HBITMAP m_hBmpOld;
	int m_offset;

// Constructor/destructor
	CBufferedPaintDC(HWND hWnd, int offset)
	{
		ATLASSERT(::IsWindow(hWnd));
		m_hWnd = hWnd;
		m_offset = offset;
		m_hPaintDC = ::BeginPaint(hWnd, &m_ps);

		// buffer
		::GetClientRect(m_hWnd, &m_rcClient);
		m_hDC = ::CreateCompatibleDC(m_hPaintDC);
		ATLASSERT(m_hDC != NULL);
		m_bmp.CreateCompatibleBitmap(m_hPaintDC, m_rcClient.right - m_rcClient.left, m_rcClient.bottom - m_rcClient.top);
		ATLASSERT(m_bmp.m_hBitmap != NULL);
		m_hBmpOld = SelectBitmap(m_bmp);
	}

	~CBufferedPaintDC()
	{
		ATLASSERT(m_hDC != NULL);
		ATLASSERT(::IsWindow(m_hWnd));

		::BitBlt(m_hPaintDC, m_rcClient.left, m_rcClient.top, m_rcClient.right - m_rcClient.left, m_rcClient.bottom - m_rcClient.top, m_hDC, m_rcClient.left, m_rcClient.top - m_offset, SRCCOPY);
		SelectBitmap(m_hBmpOld);

		::EndPaint(m_hWnd, &m_ps);
		m_hPaintDC = NULL; // detach paint dc
		Detach();

	}
};

#endif
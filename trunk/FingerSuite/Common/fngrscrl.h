#ifndef __FNGRSCRL_H__
#define __FNGRSCRL_H__

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

#ifndef __FNGRBUFDC_H__
	#error fngrscrl.h requires fngrbufdc.h to be included first
#endif

#define IDT_TIMER_ANIMATION    10001
#define TMR_INTERVAL 10

#define IDT_TIMER_AUTOSCROLL   10002
#define TMR_INTERVAL 10


#define UM_SCRL_NOTIFY     WM_USER + 100001

///////////////////////////////////////////////////////////////////////////////
// Classes in this file:
//
// CFingerScrollImpl<T>

template <class T, bool t_bBuffered = false, bool t_bNotifyParentWnd = false>
class CFingerScrollImpl
{
public:

	CFingerScrollImpl()
	{
		Reset();
	}

	~CFingerScrollImpl()
	{
		T* pT = static_cast<T*>(this);
		if (::IsWindow(pT->m_hWnd))
			pT->KillTimer(IDT_TIMER_ANIMATION);
	}
	
// Overrideables
	void DoPaint(CDCHandle /*dc*/)
	{
		// must be implemented in a derived class
		ATLASSERT(FALSE);
	}

	void DoScroll(int /*yOffset*/)
	{
		// can be override
	}	

	void ScrollTo(int offset)
	{
		m_offsetScrollTo = offset;

		if (m_offsetScrollTo < 0)
			m_offsetScrollTo = 0;
		if (m_offsetScrollTo > m_scrollableAreaHeight)
			m_offsetScrollTo = m_scrollableAreaHeight;

		T* pT = static_cast<T*>(this);
		ATLASSERT(::IsWindow(pT->m_hWnd));

		//m_velocity = 1;
		pT->SetTimer(IDT_TIMER_AUTOSCROLL, TMR_INTERVAL);
	}

	void SetFingerScrollRegion(int scrollableAreaHeight, int clientAreaHeight)
	{
		Reset();

		m_scrollableAreaHeight = max(scrollableAreaHeight - clientAreaHeight, 0);
		m_clientAreaHeight = clientAreaHeight;		
	}

	int GetScrollableAreaHeight()
	{
		return m_scrollableAreaHeight;
	}

	void SetMaxVelocity(int value)
	{
		m_maxVelocity = value;
	}

	int GetOffset()
	{
		return m_offset;
	}

	void SetThreshold(int value)
	{
		m_threshold = value;
	}

	int GetThreshold()
	{
		return m_threshold;
	}

	BOOL IsScrolling()
	{
		return ((m_velocity > 0) || m_bMouseIsDown);
	}


	BEGIN_MSG_MAP(CFingerScrollImpl)
		MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
		MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
		MESSAGE_HANDLER(WM_LBUTTONUP, OnLButtonUp)
		MESSAGE_HANDLER(WM_TIMER, OnTimer)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER(WM_CAPTURECHANGED, OnCaptureChanged)
	END_MSG_MAP()

	LRESULT OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		T* pT = static_cast<T*>(this);
		ATLASSERT(::IsWindow(pT->m_hWnd));

        m_bMouseIsDown = TRUE;
		m_tMouseDown = ::GetTickCount();

		pT->KillTimer(IDT_TIMER_ANIMATION);
		pT->KillTimer(IDT_TIMER_AUTOSCROLL);
		
        m_mouseDown.x = LOWORD(lParam);
        m_mouseDown.y = HIWORD(lParam);
        m_mousePrev = m_mouseDown;


		return 0;
	}

	LRESULT OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		T* pT = static_cast<T*>(this);
		ATLASSERT(::IsWindow(pT->m_hWnd));

		int distanceY = m_mousePrev.y - HIWORD(lParam);

		if (abs(distanceY) > m_threshold)
		{
			ModifyOffset(distanceY, FALSE);
		    
			ClipScrollPosition();

			m_mousePrev.x = LOWORD(lParam);
			m_mousePrev.y = HIWORD(lParam);

			UpdateLayout();
		}

		return 0;
	}

	LRESULT OnLButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		T* pT = static_cast<T*>(this);
		ATLASSERT(::IsWindow(pT->m_hWnd));

		int y = HIWORD(lParam);

		// Did the click end on the same item it started on?
		BOOL sameY = (y == m_mouseDown.y);

		if (!(sameY) && (::GetTickCount() - m_tMouseDown < 500) ) 
		{
			m_velocity = m_mouseDown.y - y;
			pT->SetTimer(IDT_TIMER_ANIMATION, TMR_INTERVAL);
		}

		m_mouseDown.y = -1;
		m_bMouseIsDown = FALSE;

		UpdateLayout();

		return 0;
	}


	LRESULT OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		T* pT = static_cast<T*>(this);
		ATLASSERT(::IsWindow(pT->m_hWnd));

		if (wParam == IDT_TIMER_ANIMATION)
		{
            if (m_velocity == 0)
            {
				pT->KillTimer(IDT_TIMER_ANIMATION);
            }

			if (!m_bMouseIsDown && (m_velocity != 0))
            {
				m_velocity = min(m_velocity, m_maxVelocity);
				m_velocity = max(m_velocity, -m_maxVelocity);

				ModifyOffset(m_velocity, FALSE);

                ClipScrollPosition();

                // Slow down
                if (((++m_timerCount) % 10) == 0)
                {
                    if (m_velocity < 0)
                    {
                        //m_velocity ++;
						m_velocity = m_velocity / 2;
                    }
                    else if (m_velocity > 0)
                    {
                        //m_velocity --;
						m_velocity = m_velocity / 2;

                    }
                }
				UpdateLayout();
            }

			//wprintf(L"m_velocity = %d\n", m_velocity);
		}

		if (wParam == IDT_TIMER_AUTOSCROLL)
		{
			int velocity = m_offsetScrollTo - m_offset;

			velocity = min(velocity, m_maxVelocity);
			velocity = max(velocity, -m_maxVelocity);

			ModifyOffset(velocity, FALSE);
			    
			if (m_offset < 0)
			{
				velocity = 0;
				ModifyOffset(0, TRUE);
			}
	        else if (m_offset > m_scrollableAreaHeight)
			{
				velocity = 0;
				ModifyOffset(m_scrollableAreaHeight, TRUE);
			}
        
			if (velocity != 0)
				UpdateLayout();
			else
				pT->KillTimer(IDT_TIMER_AUTOSCROLL);
		}
		return 0;
	}


	LRESULT OnPaint(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		T* pT = static_cast<T*>(this);
		ATLASSERT(::IsWindow(pT->m_hWnd));
		if(wParam != NULL)
		{
			CDCHandle dc = (HDC)wParam;
			POINT ptViewportOrg = { 0, 0 };
			dc.SetViewportOrg(0, -m_offset, &ptViewportOrg);
			pT->DoPaint(dc);
			dc.SetViewportOrg(ptViewportOrg);
		}
		else
		{
			if (t_bBuffered)
			{
				CBufferedPaintDC dc(pT->m_hWnd, -m_offset);
				dc.SetViewportOrg(0, -m_offset);
				pT->DoPaint(dc.m_hDC);
			}
			else
			{
				CPaintDC dc(pT->m_hWnd);
				dc.SetViewportOrg(0, -m_offset);
				pT->DoPaint(dc.m_hDC);
			}
		}
		return 0;
	}

	LRESULT OnCaptureChanged(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		m_bMouseIsDown = FALSE;
		return 0;
	}

private:

	void Reset()
	{
		//m_maxVelocity = 15;
		m_velocity = 0;
		ModifyOffset(0, TRUE);
		m_timerCount = 0;
        m_mouseDown.x = -1; m_mouseDown.y = -1;
        m_mousePrev.x = -1; m_mousePrev.y = -1;
		m_t1 = ::GetTickCount();
		//m_bMiddlePosNotified = FALSE;
	}

	/*
    void ClipVelocity()
    {
        m_velocity = min(m_velocity, m_maxVelocity);
        m_velocity = max(m_velocity, -m_maxVelocity);
    }
	*/

    void ClipScrollPosition()
    {
        if (m_offset < 0)
        {
            ModifyOffset(0, TRUE);
            m_velocity = 0;
        }
        else if (m_offset > m_scrollableAreaHeight)
        {
            ModifyOffset(m_scrollableAreaHeight, TRUE);
			m_velocity = 0;
        }
    }

	void UpdateLayout()
	{
		T* pT = static_cast<T*>(this);
		ATLASSERT(::IsWindow(pT->m_hWnd));

		pT->InvalidateRect(NULL, TRUE);

		DWORD t2 = ::GetTickCount();
		if (t2 - m_t1 > 50) // ms elapsed
		{
			m_t1 = ::GetTickCount();
			pT->UpdateWindow();
		}

	}

	void ModifyOffset(int value, BOOL bAbsolute)
	{
		T* pT = static_cast<T*>(this);

		if (bAbsolute)
			m_offset = value;
		else
		{
			m_offset += value;
			//pT->DoScroll(value); TODO
		}

		if ((t_bNotifyParentWnd) && (::IsWindow(pT->m_hWnd)))
			::SendMessage(pT->GetParent(), UM_SCRL_NOTIFY, (WPARAM)m_offset, 0); 
	}

protected:
	// nuovi
    POINT m_mouseDown;
    POINT m_mousePrev;
	int m_scrollableAreaHeight;
	int m_clientAreaHeight;
	BOOL m_bMouseIsDown;
	
	//BOOL m_bMiddlePosNotified;
	int m_maxVelocity;
	int m_velocity;
	int m_offset;
	int m_timerCount;
	DWORD m_t1;
	int m_offsetScrollTo;
	DWORD m_tMouseDown;
	int m_threshold;
	//HWND m_hWndNotify;
};




/*
//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
template <class T>
class CFingerScrollItemImpl
{
public:

	CFingerScrollItemImpl()
	{
		m_top = 0;
	}

	~CFingerScrollItemImpl() {}

	BEGIN_MSG_MAP(CFingerScrollItemImpl)
		MESSAGE_HANDLER(WM_WINDOWPOSCHANGED, OnWindowPosChanged)
		MESSAGE_HANDLER(WM_LBUTTONDOWN, OnForwardMessage)
		MESSAGE_HANDLER(WM_MOUSEMOVE, OnForwardMessage)
		MESSAGE_HANDLER(WM_LBUTTONUP, OnForwardMessage)
	END_MSG_MAP()

	LRESULT OnWindowPosChanged(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		LPWINDOWPOS lpwp = (LPWINDOWPOS) lParam;
		m_top = lpwp->y;
		return 0;
	}

	LRESULT OnForwardMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		T* pT = static_cast<T*>(this);
		ATLASSERT(::IsWindow(pT->m_hWnd));
		int y = m_top + HIWORD(lParam);
		pT->GetParent().SendMessage(uMsg, wParam, (LPARAM)MAKELONG(0, y)); // no matter on x position

		return 0;
	}
protected:
	int m_top;

};
*/
#endif // __FNGRSCRL_H__
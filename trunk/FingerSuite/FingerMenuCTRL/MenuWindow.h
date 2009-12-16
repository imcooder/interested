// WTLApp1View.h : interface of the CWTLApp1View class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once




#ifndef __FNGRSCRL_H__
	#error menu.h requires fngrscrl.h to be included first
#endif

#define IDT_TIMER_MENU_ANIMATION  10110
#define TMR_MENU_ANIMATION        10

#define BORDER_WIDTH           10
#define UM_MINIMIZE               WM_USER + 2
#define UM_SETNEWMENU             WM_USER + 4
#define UM_GETDESTWND             WM_USER + 5



class CMenuWindow : public CWindowImpl<CMenuWindow>, public CFingerScrollImpl<CMenuWindow, true, true>
{
public:
	DECLARE_WND_CLASS(NULL)

	BOOL PreTranslateMessage(MSG* pMsg)
	{
		pMsg;
		return FALSE;
	}

	CMenuWindow ()
	{
		m_idxDPadSelected = -1;
	}

	void DoPaint(CDCHandle dc)
	{
		// must be implemented in a derived class
		RECT rc; GetClientRect(&rc);
		SIZE imgSize;
		imgSize.cx = m_imgSubmenuImage.GetWidth();
		imgSize.cy = m_imgSubmenuImage.GetHeight();

		// font
		CFont oldFont = dc.SelectFont(m_fText);

		for (int i = 0; i < m_menuItems.GetSize(); i++)	
		{
			MENUINFO mi = (MENUINFO)m_menuItems[i];

			COLORREF clText;
			COLORREF clBkg = m_clBkg;
			if ((i == m_idxSelected) || (i == m_idxDPadSelected))
			{
				clText = m_clSelText;
			}
			else
			{
				clText = m_clText;
			}

			if (mi.mii.fState & MF_GRAYED)
			{
				clText = m_clDisabledText;
			}

			
			RECT rcItem = {0, i * m_itemHeight, rc.right, (i + 1) * m_itemHeight };

			// draw rectangle without borders
			CBrush br; br.CreateSolidBrush(clBkg);
			CBrush brOld = dc.SelectBrush(br);
			CPen pen; pen.CreatePen(PS_NULL, 1, RGB(255,255,255));
			CPen penOld = dc.SelectPen(pen);
			dc.Rectangle(&rcItem);
			dc.SelectPen(penOld);
			dc.SelectBrush(brOld);


			if ((i == m_idxSelected) && !(mi.mii.fState & MF_GRAYED))
			{
				m_imgSelectionImage.Draw(dc, rcItem);
			}

			// draw dpad selection
			if ( i == m_idxDPadSelected)
			{
				m_imgDPadCursorImage.Draw(dc, rcItem);
			}


			// draw bottom line
			if (m_menuItems[i].bFollowSeparator)
			{
				// solid line 1px
				CPen pen2; pen2.CreatePen(PS_SOLID, 1, m_clLine);
				CPen penOld2 = dc.SelectPen(pen2);
				dc.MoveTo(rcItem.left, rcItem.bottom - 1);
				dc.LineTo(rcItem.right, rcItem.bottom - 1);
				dc.SelectPen(penOld2);
			}
			else
			{
				// dotted line 1px
				CPen pen2; pen2.CreatePen(PS_DASH, 1, m_clLine);
				CPen penOld2 = dc.SelectPen(pen2);
				dc.MoveTo(rcItem.left, rcItem.bottom - 1);
				dc.LineTo(rcItem.right, rcItem.bottom - 1);
				dc.SelectPen(penOld2);				
			}

			// draw text
			dc.SetBkMode(TRANSPARENT);
			dc.SetTextColor(clText);
			dc.DrawText(m_menuItems[i].text, -1, &rcItem, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS); 

			if (mi.mii.fType & MFT_RADIOCHECK)
			{
				if (mi.mii.fState & MFS_CHECKED)
				{
					RECT rcImg = {rcItem.right - imgSize.cx, rcItem.top, rcItem.right, rcItem.top + imgSize.cy};
					m_imgRadioCheckedImage.Draw(dc, rcImg);
				}
				else
				{
					RECT rcImg = {rcItem.right - imgSize.cx, rcItem.top, rcItem.right, rcItem.top + imgSize.cy};
					m_imgRadioUncheckedImage.Draw(dc, rcImg);
				}
			}
			else
			{
				// draw check mark
				if (mi.mii.fState & MFS_CHECKED)
				{
					RECT rcImg = {rcItem.right - imgSize.cx, rcItem.top, rcItem.right, rcItem.top + imgSize.cy};
					m_imgCheckedImage.Draw(dc, rcImg);
				}
			}

			// draw sub menu icon
			if (mi.mii.hSubMenu != NULL)
			{
				RECT rcImg = {rcItem.right - imgSize.cx, rcItem.top, rcItem.right, rcItem.top + imgSize.cy};
				m_imgSubmenuImage.Draw(dc, rcImg);
			}

			if (mi.mii.fType & MFT_OWNERDRAW)
			{
				CDC dcTemp; dcTemp.CreateCompatibleDC(dc);

				DRAWITEMSTRUCT dis;
				ZeroMemory(&dis, sizeof(DRAWITEMSTRUCT));

				dis.CtlType = ODT_MENU;
				dis.itemID = mi.mii.wID;
				dis.itemAction = ODA_DRAWENTIRE;
				//dis.itemState = ODS_SELECTED; // TODO
				dis.hwndItem = (HWND)mi.hOwnerMenu;
				dis.hDC = dc;
				CopyRect(&dis.rcItem, &rcItem);
				InflateRect(&dis.rcItem, 0, -8 * m_scaleFactor);
				dis.itemData = mi.mii.dwItemData;

				//LOG(L"Avvio draw....");

				LRESULT lResult = SendMessageRemote(mi.hDestWnd, WM_DRAWITEM, 0, (LPARAM)&dis, sizeof(DRAWITEMSTRUCT), 50);

				if (lResult == 0)
				{
					GetParent().GetParent().PostMessage(WM_COMMAND, ID_MENU_SHOWORIGINAL, 0);
					return;
				}

				//LOG(L"......completato\n");
			}

		}
		dc.SelectFont(oldFont);


		// draw scroll bar
		if (!m_bAnimating)
		{
			int totalHeight = m_menuItems.GetSize() * m_itemHeight;
			if (rc.bottom < totalHeight)
			{
				int height = (int)(((float)rc.bottom / (float)totalHeight) * rc.bottom);
				int top = (int)(((float)GetOffset() / (float)totalHeight) * rc.bottom);

				RECT rcScrollbar;
				rcScrollbar.right = rc.right;
				rcScrollbar.left = rcScrollbar.right - 3 * m_scaleFactor;
				rcScrollbar.top = top + GetOffset();
				rcScrollbar.bottom = rcScrollbar.top + height;
				dc.FillSolidRect(&rcScrollbar, m_clScrollbar);
			}
		}
	}

	typedef CWindowImpl<CMenuWindow>  winbaseClass;
	typedef CFingerScrollImpl<CMenuWindow, true, true> scrollbaseClass;

	BEGIN_MSG_MAP(CMenuWindow)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackground)
		MESSAGE_HANDLER(WM_TIMER, OnTimer)
		MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
		MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
		MESSAGE_HANDLER(WM_LBUTTONUP, OnLButtonUp)
		CHAIN_MSG_MAP(scrollbaseClass)
	END_MSG_MAP()

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		return 0;
	}

	LRESULT OnSize(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
	{
		if(wParam != SIZE_MINIMIZED)
		{
			RECT rc; GetClientRect(&rc);
			SetFingerScrollRegion(m_menuItems.GetSize() * m_itemHeight, rc.bottom);
		}

		bHandled = FALSE;
		return 0;
	}

	LRESULT OnEraseBackground(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		return 1;
	}

	LRESULT OnLButtonDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled)
	{
		m_xPos = LOWORD(lParam);
		m_yPos = HIWORD(lParam);
		m_bMouseIsDown = TRUE;

		m_idxSelected = (GetOffset() + m_yPos) / m_itemHeight;
		m_idxDPadSelected = -1;

		if ((m_idxSelected >= 0) && (m_idxSelected < m_menuItems.GetSize()))
		{
			InvalidateRect(NULL, FALSE);
			UpdateWindow();
		}
		else
			m_idxSelected = -1;

		bHandled = FALSE; // very important
		return 0;
	}

	LRESULT OnMouseMove(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled)
	{
		int y = HIWORD(lParam);
		if  (abs(y - m_yPos) >  GetThreshold())
		{
			if (m_idxSelected != -1)
			{
				m_idxSelected = -1;
				InvalidateRect(NULL, FALSE);
				UpdateWindow();
			}
		}

		m_yPos = HIWORD(lParam);

 		bHandled = FALSE; // very important
		return 0;
	}

	LRESULT OnLButtonUp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled)
	{
		/*
		if (m_bMouseIsDown)
		{
			// back gesture recognition
			int xPosNew =  LOWORD(lParam);
			
			if ((m_xPos - xPosNew) > 100 * m_scaleFactor)
			{
				// back gesture recognized 
				if (Back())
				{
					bHandled = FALSE; // very important
					return 0;
				}
			}
		}
		*/
		
		m_bMouseIsDown = FALSE;


		// normal mode
		if (m_idxSelected != -1)
		{
			MENUINFO mi = (MENUINFO)m_menuItems[m_idxSelected];
			if (!(mi.mii.fState & MF_GRAYED))
			{
				if (mi.mii.hSubMenu != NULL)
				{
					// manage submenu
					LPMENU pMu = NULL;
					for (int i = 0; i < m_menus.GetSize(); i++)
					{
						if (m_menus[i].hMenu == mi.mii.hSubMenu)
						{
							pMu = &m_menus[i];
							break;
						}
					}

					if (pMu == NULL)
					{
						MENU m;
						m.hMenu = mi.mii.hSubMenu;
						m.bInitMenuSent = FALSE;
						m.hPrevMenu = mi.hOwnerMenu;
						m_menus.Add(m);
					}

					GetParent().GetParent().SendMessage(UM_SETNEWMENU, (WPARAM)mi.mii.hSubMenu);
				}
				else 
				{
					GetParent().GetParent().SendMessage(UM_MINIMIZE, (WPARAM)mi.mii.wID, 0);
				}
			}
		}

		bHandled = FALSE; // very important
		return 0;
	}



public:
	COLORREF m_clBkg;
	COLORREF m_clText;
	COLORREF m_clSelText;
	COLORREF m_clDisabledText;
	COLORREF m_clLine;
	COLORREF m_clScrollbar;

	CFontHandle m_fText;
	
	int m_itemHeight;
	int m_scaleFactor;
	BOOL m_bAnimating;

	CxImage m_imgSubmenuImage;
	CxImage m_imgCheckedImage;
	CxImage m_imgRadioCheckedImage;
	CxImage m_imgRadioUncheckedImage;
	CxImage m_imgSelectionImage;
	CxImage m_imgDPadCursorImage;

	int m_idxDPadSelected;
	int m_xPos;
	int m_yPos;
	int m_idxSelected;


protected:


	HRGN m_hRgn;
	BOOL m_bMouseIsDown;
};


class CWrapMenuWindow : public CWindowImpl<CWrapMenuWindow>
{
public:

	CMenuWindow m_menu;

	DECLARE_WND_CLASS(NULL)

	CWrapMenuWindow ()
	{
		m_nMaxMenuItemCount = 1000;
		m_bMouseIsDown = FALSE;
	}

	BOOL PreTranslateMessage(MSG* pMsg)
	{
		pMsg;
		return FALSE;
	}

	void SetMaxMenuItemCount(int value)
	{
		m_nMaxMenuItemCount = value;
	}

	void SetScaleFactor(int value)
	{
		m_scaleFactor = value;
		m_menu.m_scaleFactor = value;

		m_menu.SetThreshold(5 * m_scaleFactor);
	}

	int GetScaleFactor()
	{
		return m_scaleFactor;
	}

	void SetItemHeight(int value)
	{
		m_itemHeight = value; //* m_scaleFactor;
		m_menu.m_itemHeight = value; // * m_scaleFactor;
	}

	void SetButtonHeight(int value)
	{
		m_buttonHeight = value;
	}

    int GetItemHeight()
	{
		return m_itemHeight;
	}
	
	/*
	void SetUsedRect(int value)
	{
		m_nUsedRects = value * m_scaleFactor;
	}
	*/

	BOOL Show()
	{
		ModifyShape();
		StartAnimation();

		return TRUE;
	}



	BEGIN_MSG_MAP(CWrapMenuWindow)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackground)
		MESSAGE_HANDLER(WM_TIMER, OnTimer)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		//MESSAGE_HANDLER(UM_STARTANIM, OnStartAnim)
		MESSAGE_HANDLER(UM_SCRL_NOTIFY, OnScrlNotify)
		MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
		MESSAGE_HANDLER(WM_LBUTTONUP, OnLButtonUp)
		MESSAGE_HANDLER(WM_KEYDOWN, OnKeyDown)
	END_MSG_MAP()

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		ModifyTopShape();
		// menu
		m_menu.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE |WS_CLIPSIBLINGS | WS_CLIPCHILDREN);  
		return 0;
	}

	LRESULT OnSize(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
	{
		RECT rc; GetClientRect(&rc);
		if ((m_menuItems.GetSize() > m_nMaxMenuItemCount) && (!m_bNoButtons))
		{
	    	rc.top += m_buttonHeight;
			rc.bottom -= m_buttonHeight;
			m_bButtonsVisible = TRUE;
		}
		m_menu.MoveWindow(&rc, TRUE);
		return 0;
	}

	LRESULT OnEraseBackground(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		return 1;
	}

	LRESULT OnPaint(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		RECT rc; GetClientRect(&rc);
		CBufferedPaintDC dc(m_hWnd, 0);

		if (m_bButtonsVisible)
		{
			RECT rcItem; CopyRect(&rcItem, &rc);
			rcItem.bottom = m_buttonHeight;
			RECT rcArrow; CopyRect(&rcArrow, &rcItem);
			int w = m_bmpBtnArrowUp.GetWidth();
			int h = m_bmpBtnArrowUp.GetHeight();
			int dx = (w - rcItem.right)/2;
			int dy = (h - rcItem.bottom)/2;
			InflateRect(&rcArrow, dx, dy);
			rcArrow.right = rcArrow.left + w;
			rcArrow.bottom = rcArrow.top + h;

			if (m_bBtnUpDisabled)
			{
				m_bmpBtnUpDisabled.Draw(dc, rcItem);
			}
			else if (m_bBtnUpPressed)
			{
				m_bmpBtnUpPressed.Draw(dc, rcItem);
				// draw arrow
				m_bmpBtnArrowUp.Draw(dc, rcArrow);
			}
			else
			{
				m_bmpBtnUp.Draw(dc, rcItem);
				// draw arrow
				m_bmpBtnArrowUp.Draw(dc, rcArrow);
			}

			CopyRect(&rcItem, &rc);
			rcItem.top = rcItem.bottom - m_buttonHeight;
			CopyRect(&rcArrow, &rcItem);
			w = m_bmpBtnArrowDown.GetWidth();
			h = m_bmpBtnArrowDown.GetHeight();
			dx = (w - rcItem.right)/2;
			dy = (h - (rcItem.bottom - rcItem.top))/2;
			InflateRect(&rcArrow, dx, dy);
			rcArrow.right = rcArrow.left + w;
			rcArrow.bottom = rcArrow.top + h;
			
			if (m_bBtnDownDisabled)
			{
				m_bmpBtnDownDisabled.Draw(dc, rcItem);
			}
			else if (m_bBtnDownPressed)
			{
				m_bmpBtnDownPressed.Draw(dc, rcItem);
				// draw arrow
				m_bmpBtnArrowDown.Draw(dc, rcArrow);
			}
			else
			{
				m_bmpBtnDown.Draw(dc, rcItem);
				// draw arrow
				m_bmpBtnArrowDown.Draw(dc, rcArrow);
			}
		}
		return 0;
	}

	LRESULT OnTimer(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
	{
		if (wParam == IDT_TIMER_MENU_ANIMATION)
		{
			m_iStep = m_iStep / 2;
			//int iVisibleItems = min(m_nMaxMenuItemCount, m_menu.GetMenuItemCount());
			//MoveWindow(&m_rect, TRUE);
			RECT animRect; CopyRect(&animRect, &m_animRect);
			animRect.top = animRect.top + m_iStep;
			MoveWindow(&animRect, TRUE);

			if (m_iStep == 0)
			{
				KillTimer(IDT_TIMER_MENU_ANIMATION);
				m_bAnimating = FALSE;
				m_menu.m_bAnimating = FALSE;
				m_iStep = 0;
			}

			bHandled = TRUE;
			return 0;
		}

		bHandled = FALSE;
		return 0;
	}

	
	//LRESULT OnStartAnim(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	//{
	//	ModifyShape();
	//	StartAnimation();
	//	return 0;
	//}
	
	LRESULT OnScrlNotify(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		if (!m_bAnimating)
		{
			BOOL bUpdate = FALSE;
			int offset = (int)wParam;
			if (offset == 0)
			{
				bUpdate = (m_bBtnUpDisabled != TRUE);
				m_bBtnUpDisabled = TRUE;
			}

			if (offset == m_menu.GetScrollableAreaHeight())
			{
				bUpdate = (m_bBtnDownDisabled != TRUE);
				m_bBtnDownDisabled = TRUE;
			}

			if ((offset > 0) && (offset < m_menu.GetScrollableAreaHeight()))
			{
				bUpdate = (m_bBtnUpDisabled != FALSE) || (m_bBtnDownDisabled != FALSE);
				m_bBtnUpDisabled   = FALSE;
				m_bBtnDownDisabled = FALSE;
			}

			if (bUpdate)
			{
				InvalidateRect(NULL, TRUE);
				UpdateWindow();
			}
		}		
		return 0;
	}


	LRESULT OnLButtonDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
	{
		RECT rc; GetClientRect(&rc);
		UINT yPos = HIWORD(lParam);

		m_menu.m_idxDPadSelected = -1;

		m_bMouseIsDown = TRUE;

		if ((yPos < m_itemHeight) && (!(m_bBtnUpDisabled)))
		{
			m_bBtnUpPressed = TRUE;
			
			RECT rcBtn; CopyRect(&rcBtn, &rc);
			rcBtn.bottom = m_itemHeight;
			InvalidateRect(&rcBtn, FALSE);
			UpdateWindow();
		}

		if ((yPos > (rc.bottom - m_itemHeight)) && (!(m_bBtnDownDisabled)))
		{
			m_bBtnDownPressed = TRUE;
			
			RECT rcBtn; CopyRect(&rcBtn, &rc);
			rcBtn.top = rc.bottom - m_itemHeight;
			InvalidateRect(&rcBtn, FALSE);
			UpdateWindow();
		}
		
		return 0;
	}


	LRESULT OnLButtonUp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
	{
		RECT rc; GetClientRect(&rc);
		UINT yPos = HIWORD(lParam);

		if (m_bMouseIsDown)
		{
			if ((yPos < m_itemHeight) && (!(m_bBtnUpDisabled)))
			{
				m_bBtnUpPressed = FALSE;
			
				int d = (m_menu.GetOffset() - 3 * m_itemHeight) - ( (m_menu.GetOffset() - 3 * m_itemHeight) % m_itemHeight );
				m_menu.ScrollTo(d);

				RECT rcBtn; CopyRect(&rcBtn, &rc);
				rcBtn.bottom = m_itemHeight;
				InvalidateRect(&rcBtn, FALSE);
				UpdateWindow();
			}

			if ((yPos > (rc.bottom - m_itemHeight)) && (!(m_bBtnDownDisabled)))
			{
				m_bBtnDownPressed = FALSE;

				int d = (m_menu.GetOffset() + 3 * m_itemHeight) - ( (m_menu.GetOffset() + 3 * m_itemHeight) % m_itemHeight );
				m_menu.ScrollTo(d);

				RECT rcBtn; CopyRect(&rcBtn, &rc);
				rcBtn.top = rc.bottom - m_itemHeight;
				InvalidateRect(&rcBtn, FALSE);
				UpdateWindow();
			}
		}

		m_bMouseIsDown = FALSE;
		return 0;
	}

	LRESULT OnKeyDown(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		static int nKeyRepeatCount = 0;
		int nVirtKey = (int)wParam; 
		BOOL bUpdate = FALSE; 

		BOOL bRepeating = (lParam & (1 << 30)) != 0;

		if (bRepeating)
		{
			nKeyRepeatCount++;
			if (nKeyRepeatCount % 5 != 0)
				return 0;
		}
		else
		{
		  nKeyRepeatCount = 0;
		}

		switch (nVirtKey)
		{
		case VK_LEFT:
			GetParent().SendMessage(WM_COMMAND, (WPARAM)ID_BACK);
			break;

		case VK_RIGHT:
		case VK_RETURN:
			if ((m_menu.m_idxDPadSelected >= 0) && (m_menu.m_idxDPadSelected < m_menuItems.GetSize()))
			{
				m_menu.m_idxSelected = m_menu.m_idxDPadSelected;
				m_menu.SendMessage(WM_LBUTTONUP, 0, MAKELPARAM(m_menu.m_xPos, m_menu.m_yPos) );   //(LPARAM)m_menu.m_xPos);
			}
			else
			{
				// as requested select first item
				m_menu.m_idxDPadSelected = 0;

				m_menu.m_idxSelected = m_menu.m_idxDPadSelected;
				m_menu.SendMessage(WM_LBUTTONUP, 0, MAKELPARAM(0, m_menu.m_yPos) );
			}
			break;

		case VK_UP: 
			if ((m_menu.m_idxDPadSelected >= 0) && (m_menu.m_idxDPadSelected < m_menuItems.GetSize()))
			{
				bUpdate = TRUE;
				m_menu.m_idxDPadSelected --;
				if (m_menu.m_idxDPadSelected < (m_menuItems.GetSize() - 3))
				{
					int d = (m_menu.GetOffset() - m_itemHeight) - ( (m_menu.GetOffset() - m_itemHeight) % m_itemHeight );
					m_menu.ScrollTo(d);
				}
			}
			else
			{
				// mi metto sull'ultimo visibile
				bUpdate = TRUE;
				m_menu.ScrollTo((m_menuItems.GetSize() - 1) * m_itemHeight);
				m_menu.m_idxDPadSelected = m_menuItems.GetSize() - 1;
			}
			break;

		case VK_DOWN: 
			if ((m_menu.m_idxDPadSelected >= 0) && (m_menu.m_idxDPadSelected < m_menuItems.GetSize()))
			{
				bUpdate = TRUE;
				m_menu.m_idxDPadSelected ++;
				if (m_menu.m_idxDPadSelected > 2)
				{
					int d = (m_menu.GetOffset() + m_itemHeight) - ( (m_menu.GetOffset() + m_itemHeight) % m_itemHeight );
					m_menu.ScrollTo(d);
				}
			}
			else
			{
				// mi metto sul primo visibile
				bUpdate = TRUE;
				m_menu.ScrollTo(0);
				m_menu.m_idxDPadSelected = 0;
			}
			break;

		}

		if (bUpdate)
		{
			m_menu.m_idxDPadSelected = min(m_menu.m_idxDPadSelected, m_menuItems.GetSize() - 1);
			m_menu.m_idxDPadSelected = max(m_menu.m_idxDPadSelected, 0);

			m_menu.Invalidate(FALSE);
			m_menu.UpdateWindow();
		}

		return 0;
	}

	//HBITMAP m_bmpBtnUp;
	//HBITMAP m_bmpBtnUpPressed;
	//HBITMAP m_bmpBtnUpDisabled;
	//HBITMAP m_bmpBtnDown;
	//HBITMAP m_bmpBtnDownPressed;
	//HBITMAP m_bmpBtnDownDisabled;

	CxImage m_bmpBtnUp;
	CxImage m_bmpBtnUpPressed;
	CxImage m_bmpBtnUpDisabled;
	CxImage m_bmpBtnDown;
	CxImage m_bmpBtnDownPressed;
	CxImage m_bmpBtnDownDisabled;
	CxImage m_bmpBtnArrowUp;
	CxImage m_bmpBtnArrowDown;

	DWORD m_iMaxVisibleItemCount;
	BOOL m_bNoButtons;
	BOOL m_bEnableAnimation;


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
		pRgnData->rdh.rcBound.left = pRgnData->rdh.rcBound.top=0;
		pRgnData->rdh.rcBound.right = iWidth;
		pRgnData->rdh.rcBound.bottom = nUsedRects;		

		/*
		for (int i = 0; i < nUsedRects; i++)
		{
			pRect=&(((RECT *)&pRgnData->Buffer)[i]);
			pRect->top = i;
			pRect->bottom = i + 1;
			pRect->left = nUsedRects - i;
			pRect->right = iWidth - (nUsedRects - i);
		}
		*/
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

public:
	//int m_nUsedRects; //6 for VGA
	int m_itemHeight; 
	int m_buttonHeight;
	RECT m_animRect; 
	BOOL m_bAnimating;
	int m_iStep;
	BOOL m_bButtonsVisible;
	int m_nMaxMenuItemCount; 
	BOOL m_bBtnUpDisabled;
	BOOL m_bBtnUpPressed;
	BOOL m_bBtnDownDisabled;
	BOOL m_bBtnDownPressed;
	int m_scaleFactor;
	int m_height;
	BOOL m_bMouseIsDown;
	HRGN m_hrgnTop;
	int  m_heightHrgnTop;


	void ModifyShape()
	{
		m_height = 0;
		BOOL bButtonsVisible = FALSE;
		if ((m_menuItems.GetSize() > m_nMaxMenuItemCount) && (!m_bNoButtons))
		{
			bButtonsVisible = TRUE;
		}

		RECT rcParent; GetParent().GetClientRect(&rcParent);
		int iWidth = rcParent.right - 2 * BORDER_WIDTH * m_scaleFactor;

		int nMaxMenuItemCount = (rcParent.bottom - rcParent.top) / m_itemHeight;
		m_nMaxMenuItemCount = min( m_iMaxVisibleItemCount, nMaxMenuItemCount );

		int nMenuItemCount = m_menuItems.GetSize();
		int nItems = (nMenuItemCount == 0) ? 2 : nMenuItemCount;
		int iVisibleItems = min(m_nMaxMenuItemCount, nItems);
		
		HRGN hrgnBottom = NULL;
		if (bButtonsVisible)
		{
			hrgnBottom = ::CreateRectRgn(0, 0, iWidth, m_buttonHeight - m_heightHrgnTop);
			m_height += m_buttonHeight;
		}
		else
		{
			hrgnBottom = ::CreateRectRgn(0, 0, iWidth, m_itemHeight - m_heightHrgnTop);
			m_height += m_itemHeight;
		}
		::OffsetRgn(hrgnBottom, 0, m_heightHrgnTop);

		CombineRgn(hrgnBottom, m_hrgnTop, hrgnBottom, RGN_OR);

		HRGN hrgnFinal = NULL;
		if (bButtonsVisible)
		{
			hrgnFinal = CreateRectRgn(0, m_buttonHeight, iWidth, (iVisibleItems - 1) * m_itemHeight + m_buttonHeight);
		}
		else
		{
			hrgnFinal = CreateRectRgn(0, m_itemHeight, iWidth, iVisibleItems * m_itemHeight);
		}
		m_height += (iVisibleItems - 1) * m_itemHeight;

		int res = CombineRgn(hrgnFinal, hrgnBottom, hrgnFinal, RGN_OR);
		if (res > 0)
		{
			SetWindowRgn(hrgnFinal, FALSE);
		}
	}

	void StartAnimation()
	{
		// window position
		m_bAnimating = (m_bEnableAnimation) ? TRUE : FALSE;
		m_menu.m_bAnimating = (m_bEnableAnimation) ? TRUE : FALSE;
		m_iStep = (m_bEnableAnimation) ? m_itemHeight / 3 : 0;

		RECT rcParent; GetParent().GetClientRect(&rcParent);

		m_animRect.left = BORDER_WIDTH * m_scaleFactor;
		m_animRect.right = rcParent.right - BORDER_WIDTH * m_scaleFactor;
		m_animRect.top = rcParent.bottom - m_height;
		m_animRect.bottom = rcParent.bottom;


		RECT animRect; CopyRect(&animRect, &m_animRect);
		animRect.top = animRect.top + m_iStep;
		MoveWindow(&animRect, TRUE);

		m_bBtnDownDisabled = FALSE;
		m_bBtnUpDisabled = TRUE;
		m_bBtnDownPressed = FALSE;
		m_bBtnUpPressed = FALSE;

		if (m_bEnableAnimation)
			SetTimer(IDT_TIMER_MENU_ANIMATION, TMR_MENU_ANIMATION);
	}



};


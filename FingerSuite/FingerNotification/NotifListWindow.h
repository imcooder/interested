// WTLApp1View.h : interface of the CWTLApp1View class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once



#include <Connmgr_status.h>
//#include "snapi.h"

#ifndef __FNGRSCRL_H__
	#error menu.h requires fngrscrl.h to be included first
#endif

#define UM_SELECTNOTIF WM_USER + 98
#define UM_STATENOTIF  WM_USER + 99
#define UM_MINIMIZE    WM_USER + 100
#define UM_ACTIVATEMINIMIZETIMER WM_USER + 101
#define UM_STOPMINIMIZETIMER WM_USER + 102

#define IDT_AUTOMINIMIZE   100001

// Which values to refresh
//#define TB_BLUETOOTH_MASK       0x10
//#define TB_TIME_MASK            0x04
//#define TB_CONNECTIONS_MASK     0x40
#define TB_BATTERY_MASK              0x08
#define TB_VOLUME_MASK               0x20
#define TB_SIGNAL_MASK               0x01
#define TB_OPERATOR_MASK             0x02
#define TB_PHONE_STATUS_MASK         0x80
#define TB_CELLSYSTEMCONNECTED_MASK  0x0100
#define TB_PHONEACTIVECALLCOUNT_MASK 0x0200

#define SN_RINGMODE_ROOT        HKEY_CURRENT_USER
#define SN_RINGMODE_PATH        TEXT("ControlPanel\\Notifications\\ShellOverrides")
#define SN_RINGMODE_VALUE       TEXT("Mode")
#define SN_RINGMODE_SOUND       0
#define SN_RINGMODE_VIBRATE     1
#define SN_RINGMODE_MUTE        2

#define BTN_ICON_SIZE  16

#define IDC_BTN_VOLUME          10001
#define IDC_BTN_BATTERY         10002
#define IDC_BTN_CELLULAR        10003


class CListWindow : public CWindowImpl<CListWindow>, public CFingerScrollImpl<CListWindow, true, false>
{
public:
	DECLARE_WND_CLASS(NULL)

	BOOL PreTranslateMessage(MSG* pMsg)
	{
		pMsg;
		return FALSE;
	}

	CListWindow()
	{
	}

	void DoPaint(CDCHandle dc)
	{
		int cx = 32 * m_scaleFactor;
		int cy = 32 * m_scaleFactor;

		RECT rc; GetClientRect(&rc);
		// background
		dc.FillSolidRect(&rc, m_clBkg);

		CFont oldFont = dc.SelectFont(m_fText);

		if (g_pNotifications.GetSize() == 0)
		{
			RECT rcText; CopyRect(&rcText, &rc);

			// draw text 
			rcText.top = rcText.top + 5 * m_scaleFactor;
			dc.SetBkMode(TRANSPARENT);
			dc.SetTextColor(m_clText);
			dc.DrawText( LoadResourceString(IDS_NO_NEW_NOTIFICATIONS), -1, &rcText, DT_CENTER | DT_TOP | DT_SINGLELINE | DT_END_ELLIPSIS); 
		}

		for (int i = 0; i < g_pNotifications.GetSize(); i++)
		{
			RECT rcItem = {0, i * m_itemHeight, rc.right, (i + 1) * m_itemHeight};
			LPNOTIFICATIONINFO pInfo = g_pNotifications[i];
			
			// background
			//dc.FillSolidRect(&rcItem, m_clBkg);

			if (i == m_idxSelected)
			{
				m_imgSelection.Draw(dc, rcItem);
			}

			// draw icon
			RECT rcImg = {0, rcItem.top + (m_itemHeight - cy)/2, cx, rcItem.bottom - (m_itemHeight - cy)/2};
			OffsetRect(&rcImg, 2 * m_scaleFactor, 0);
			//m_imgIconBkg.Draw(dc, rcImg);
			
			InflateRect(&rcImg, -2 * m_scaleFactor, -2 * m_scaleFactor);
			CIconHandle icon = pInfo->nd.hicon;
			icon.DrawIconEx(dc, rcImg.left, rcImg.top, (rcImg.right - rcImg.left), (rcImg.bottom - rcImg.top));

			// draw text 
			RECT rcText; CopyRect(&rcText, &rcItem);
			rcText.left = cx + 16 * m_scaleFactor;
			dc.SetBkMode(TRANSPARENT);
			dc.SetTextColor(m_clText);
			dc.DrawText(pInfo->nd.pszTitle, -1, &rcText, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS); 


			// dotted line 1px
			CPen pen2; pen2.CreatePen(PS_DASH, 1, m_clLine);
			CPen penOld2 = dc.SelectPen(pen2);
			dc.MoveTo(rcItem.left, rcItem.bottom - 1);
			dc.LineTo(rcItem.right, rcItem.bottom - 1);
			dc.SelectPen(penOld2);	
		}

		dc.SelectFont(oldFont);
	}

	void DoScroll(int yOffset)
	{
		//LOG(L"offset = %d\n", -yOffset);
		//ScrollWindowEx(0, -yOffset, SW_SCROLLCHILDREN | SW_INVALIDATE); 
		//m_btnBattery.MoveUpDownWindow(yOffset);
	}	

	void AddIcon(CString key, CString path)
	{
		CxImage img;
		img.Load(path, CXIMAGE_FORMAT_PNG);

		m_icons.Add(key, img);
	}

	typedef CFingerScrollImpl<CListWindow, true, false> scrollbaseClass;

	BEGIN_MSG_MAP(CListWindow)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		//MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackground)
		MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
		MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
		MESSAGE_HANDLER(WM_LBUTTONUP, OnLButtonUp)
		CHAIN_MSG_MAP(scrollbaseClass)
	END_MSG_MAP()

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		m_itemHeight = m_imgSelection.GetHeight();		
		return 0;
	}

	LRESULT OnSize(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
	{
		if(wParam != SIZE_MINIMIZED)
		{
			RECT rc; GetClientRect(&rc);
			SetFingerScrollRegion(g_pNotifications.GetSize() * m_itemHeight, rc.bottom);
		}
		return 0;
	}

	LRESULT OnEraseBackground(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		return 1;
	}

	LRESULT OnLButtonDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled)
	{
		// gesture recognize
		SHRGINFO    shrg;
		shrg.cbSize = sizeof(shrg);
		shrg.hwndClient = GetParent().GetParent();
		shrg.ptDown.x = LOWORD(lParam);
		shrg.ptDown.y = GetOffset() + HIWORD(lParam);
		shrg.dwFlags = SHRG_RETURNCMD;
		if (SHRecognizeGesture(&shrg) == GN_CONTEXTMENU) 
		{
			GetParent().SendMessage(UM_STOPMINIMIZETIMER);
			
			HMENU hMenu = ::AtlLoadMenu(IDR_MAINFRAME);
			HMENU hSubMenu = GetSubMenu(hMenu, 0);
			BOOL bRes = TrackPopupMenuEx(hSubMenu, TPM_LEFTALIGN, LOWORD(lParam), GetOffset() + HIWORD(lParam), GetParent().GetParent(), NULL);

			GetParent().SendMessage(UM_ACTIVATEMINIMIZETIMER);

			return 0;
			//GetParent().GetParent().PostMessage(UM_MINIMIZE);
		}


		m_xPos = LOWORD(lParam);
		m_yPos = HIWORD(lParam);
		m_bMouseIsDown = TRUE;

		m_idxSelected = (GetOffset() + m_yPos) / m_itemHeight;
		m_idxDPadSelected = -1;

		if ((m_idxSelected >= 0) && (m_idxSelected < g_pNotifications.GetSize()))
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

		GetParent().PostMessage(UM_ACTIVATEMINIMIZETIMER); 

 		bHandled = FALSE; // very important
		return 0;
	}

	LRESULT OnLButtonUp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled)
	{
		m_bMouseIsDown = FALSE;

		// normal mode
		if (m_idxSelected != -1)
		{
			GetParent().GetParent().SendMessage(UM_SELECTNOTIF, (WPARAM)m_idxSelected, 0);
		}

		bHandled = FALSE; // very important
		return 0;
	}

private:
	int m_xPos;
	int m_yPos;
	int m_idxSelected;
	int m_idxDPadSelected;
	BOOL m_bMouseIsDown;

	CSimpleMap<CString, CxImage> m_icons;

public:
	int m_itemHeight;
	CxImage m_imgSelection;
	CFontHandle m_fText;
	int m_scaleFactor;
	COLORREF m_clText;
	COLORREF m_clLine;
	COLORREF m_clBkg;

	
};


class CNotifListWindow : public CWindowImpl<CNotifListWindow>, public COffscreenDraw<CNotifListWindow>
{
public:
	DECLARE_WND_CLASS(NULL)

	CListWindow m_notiflist;

	BOOL PreTranslateMessage(MSG* pMsg)
	{
		pMsg;
		return FALSE;
	}

	CNotifListWindow()
	{
	}

	void Show()
	{
		//m_notiflist.m_idxSelected = -1;
		RECT rc; GetParent().GetClientRect(&rc);
		MoveWindow(&rc, TRUE);

		ActivateMinimizeTimer();
	}



	void DoPaint(CDCHandle dc)
	{
		RECT rc; GetClientRect(&rc);
		

		SIZE imgSize;
		imgSize.cx = m_imgIconBkg.GetWidth();
		imgSize.cy = m_imgIconBkg.GetHeight();

		RECT rcHHTaskTar; ::GetClientRect(::FindWindow(L"HHTaskBar", NULL), &rcHHTaskTar);
		int hTaskBar = rcHHTaskTar.bottom - rcHHTaskTar.top;

		// HEADER
		RECT rcHeader;
		int h = m_arrStaticImages[0].GetHeight();
		int w = m_arrStaticImages[0].GetWidth();

		CPen pen; pen.CreatePen(PS_SOLID, 1, m_clLine);	
		CPen oldPen = dc.SelectPen(pen);
		dc.SetBkMode(TRANSPARENT);
		dc.SetTextColor(m_clText);

		// title
		CFont oldFont = dc.SelectFont(m_fTitleText);
		dc.DrawText( LoadResourceString(IDR_MAINFRAME), -1, &rcHHTaskTar, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS); 
		dc.SelectFont(oldFont);

		
		// volume
		SetRect(&rcHeader, 0, hTaskBar, rc.right, hTaskBar + h);
		dc.FillSolidRect(&rcHeader, m_clBkgHeader);
		dc.MoveTo(rcHeader.left, rcHeader.bottom );
		dc.LineTo(rcHeader.right, rcHeader.bottom );
		rcHeader.left += 10 * m_scaleFactor;
		oldFont = dc.SelectFont(m_fHeaderText);
		dc.DrawText(LoadResourceString(IDS_VOLUME), -1, &rcHeader, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS); 
		
		// battery
		SetRect(&rcHeader, 0, hTaskBar + h + 1, rc.right, hTaskBar + h + 1 + h);
		dc.FillSolidRect(&rcHeader, m_clBkgHeader);
		dc.MoveTo(rcHeader.left, rcHeader.bottom );
		dc.LineTo(rcHeader.right, rcHeader.bottom );
		//InflateRect(&rcHeader, -10 * m_scaleFactor, 0);
		rcHeader.left += 10 * m_scaleFactor;
		dc.DrawText(LoadResourceString(IDS_BATTERY), -1, &rcHeader, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS); 
		rcHeader.right -= (w + 5 * m_scaleFactor);
		WCHAR text[50];
		wsprintf(text, L"%d%%", m_nBatteryPercent);
		if (m_nBatteryPercent != BATTERY_PERCENTAGE_UNKNOWN)
			dc.DrawText(text, -1, &rcHeader, DT_RIGHT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS); 

		// cellular
		SetRect(&rcHeader, 0, hTaskBar + h + 1 + h + 1 + 1, rc.right, hTaskBar + h + 1 + h + 1 + h);
		dc.FillSolidRect(&rcHeader, m_clBkgHeader);
		dc.MoveTo(rcHeader.left, rcHeader.bottom );
		dc.LineTo(rcHeader.right, rcHeader.bottom );
		rcHeader.left += 10 * m_scaleFactor;
		dc.DrawText(m_szCarrier, -1, &rcHeader, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS); 



		dc.SelectPen(oldPen);	
		dc.SelectFont(oldFont);		


	}

	typedef CWindowImpl<CNotifListWindow>  winbaseClass;
	typedef COffscreenDraw<CNotifListWindow> offscreenbaseClass;

	BEGIN_MSG_MAP(CNotifListWindow)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_TIMER, OnTimer)
		MESSAGE_HANDLER(UM_STATENOTIF, OnStateNotif)
		MESSAGE_HANDLER(UM_ACTIVATEMINIMIZETIMER, OnActivateMinimizeTimer)
		MESSAGE_HANDLER(UM_STOPMINIMIZETIMER, OnStopMinimizeTimer)
		CHAIN_MSG_MAP(offscreenbaseClass)
	END_MSG_MAP()

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		m_notiflist.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);

		m_btnVolume.Create(m_hWnd, FNGRBTN_TYPE_STATIC | FNGRBTN_TYPE_ICON, TRUE, m_clBkgHeader, BTN_ICON_SIZE * m_scaleFactor, IDC_BTN_VOLUME);
		m_btnBattery.Create(m_hWnd, FNGRBTN_TYPE_STATIC | FNGRBTN_TYPE_ICON, TRUE, m_clBkgHeader, BTN_ICON_SIZE * m_scaleFactor, IDC_BTN_BATTERY);
		m_btnCellular.Create(m_hWnd, FNGRBTN_TYPE_STATIC | FNGRBTN_TYPE_ICON, TRUE, m_clBkgHeader, BTN_ICON_SIZE * m_scaleFactor, IDC_BTN_CELLULAR);

		m_btnVolume.SetArrayImageSet(m_arrStaticImages);
		m_btnBattery.SetArrayImageSet(m_arrStaticImages);
		m_btnCellular.SetArrayImageSet(m_arrStaticImages);

		// notifications
		// Speaker / Volume
		HRESULT hr = ::RegistryNotifyWindow( SN_RINGMODE_ROOT,
										   SN_RINGMODE_PATH,	
	                                       SN_RINGMODE_VALUE,
                                           m_hWnd, UM_STATENOTIF, TB_VOLUME_MASK,
		                                   NULL, &m_hVolume);

		// Battery
		hr = ::RegistryNotifyWindow(
					SN_POWERBATTERYSTRENGTH_ROOT, 
					SN_POWERBATTERYSTRENGTH_PATH, 
					SN_POWERBATTERYSTRENGTH_VALUE,
					m_hWnd, UM_STATENOTIF, TB_BATTERY_MASK,
					NULL, &m_hBattery);

		// phone status
		hr = ::RegistryNotifyWindow(
					SN_PHONERADIOOFF_ROOT, 
					SN_PHONERADIOOFF_PATH, 
					SN_PHONERADIOOFF_VALUE,
					m_hWnd, UM_STATENOTIF, TB_PHONE_STATUS_MASK,
					NULL, &m_hPhoneStatus);

		// signal strengh
		hr = ::RegistryNotifyWindow(
					SN_PHONESIGNALSTRENGTH_ROOT,
					SN_PHONESIGNALSTRENGTH_PATH,
					SN_PHONESIGNALSTRENGTH_VALUE, 
					m_hWnd, UM_STATENOTIF, TB_SIGNAL_MASK,
					NULL, &m_hSignal);

		// operator name
		hr = ::RegistryNotifyWindow(
					SN_PHONEOPERATORNAME_ROOT, 
					SN_PHONEOPERATORNAME_PATH, 
					SN_PHONEOPERATORNAME_VALUE,  
					m_hWnd, UM_STATENOTIF, TB_OPERATOR_MASK,
					NULL, &m_hOperator);

		// cell system
		hr = ::RegistryNotifyWindow(
					SN_CELLSYSTEMCONNECTED_ROOT, 
					SN_CELLSYSTEMCONNECTED_PATH, 
					SN_CELLSYSTEMCONNECTED_VALUE,  
					m_hWnd, UM_STATENOTIF, TB_CELLSYSTEMCONNECTED_MASK,
					NULL, &m_hCellSystem);


		// active phone call
		hr = ::RegistryNotifyWindow(
					SN_PHONEACTIVECALLCOUNT_ROOT, 
					SN_PHONEACTIVECALLCOUNT_PATH, 
					SN_PHONEACTIVECALLCOUNT_VALUE,  
					m_hWnd, UM_STATENOTIF, TB_PHONEACTIVECALLCOUNT_MASK,
					NULL, &m_hActivePhoneCall);


		BOOL dummy;
		OnStateNotif(0, 0, 0xFFFF, dummy);

		return 0;
	}

	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		if (m_hVolume != NULL)
			RegistryCloseNotification(m_hVolume);

		if (m_hBattery != NULL)
			RegistryCloseNotification(m_hBattery);

		return 0;
	}

	LRESULT OnStateNotif(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
	{
		DWORD dwWhich = (UINT)lParam;
		
		if (
                (dwWhich & TB_SIGNAL_MASK) 
             || (dwWhich & TB_OPERATOR_MASK) 
			 || (dwWhich & TB_PHONE_STATUS_MASK) 
             || (dwWhich & TB_CELLSYSTEMCONNECTED_MASK) 
             || (dwWhich & TB_PHONEACTIVECALLCOUNT_MASK) 
		   )	
			dwWhich |= TB_SIGNAL_MASK | TB_OPERATOR_MASK | TB_PHONE_STATUS_MASK | TB_CELLSYSTEMCONNECTED_MASK | TB_PHONEACTIVECALLCOUNT_MASK;


		RefreshStatus(dwWhich);


		if (m_bSpeakerOn)
			m_btnVolume.SetIcon( AtlLoadIcon(IDI_VOLUME_ON) );
		else if (m_bVibrate)
			m_btnVolume.SetIcon( AtlLoadIcon(IDI_VOLUME_VIBRATE) );
		else
			m_btnVolume.SetIcon( AtlLoadIcon(IDI_VOLUME_OFF) );		

		if (m_bCharging)
			m_btnBattery.SetIcon( AtlLoadIcon(IDI_BATTERY_CHARGING) );
		else if (m_bNoBattery)
			m_btnBattery.SetIcon( AtlLoadIcon(IDI_BATTERY_ABSENT) );
		else if (m_bBatteryCritical)
			m_btnBattery.SetIcon( AtlLoadIcon(IDI_BATTERY_CRITICAL) );
		else
		{
			if (m_nBatteryPercent != BATTERY_PERCENTAGE_UNKNOWN)
			{
				if ((m_nBatteryPercent >= 0) && (m_nBatteryPercent <= 20))
					m_btnBattery.SetIcon( AtlLoadIcon(IDI_BATTERY_1) );
				if ((m_nBatteryPercent >= 21) && (m_nBatteryPercent <= 40))
					m_btnBattery.SetIcon( AtlLoadIcon(IDI_BATTERY_2) );
				if ((m_nBatteryPercent >= 41) && (m_nBatteryPercent <= 60))
					m_btnBattery.SetIcon( AtlLoadIcon(IDI_BATTERY_3) );
				if ((m_nBatteryPercent >= 61) && (m_nBatteryPercent <= 80))
					m_btnBattery.SetIcon( AtlLoadIcon(IDI_BATTERY_4) );
				if ((m_nBatteryPercent >= 81) && (m_nBatteryPercent <= 100))
					m_btnBattery.SetIcon( AtlLoadIcon(IDI_BATTERY_5) );
			}
		}

		// phone
		if (m_bPhoneOff)
		{
			wsprintf(m_szCarrier, L"No service");
			m_btnCellular.SetIcon( AtlLoadIcon(IDI_PHONE) );
			m_btnCellular.SetIcon2( AtlLoadIcon(IDI_OFF) );
		}
		else
		{
			if (m_bActiveDataCall)
			{
				if (m_dwConnectionType & SN_CELLSYSTEMCONNECTED_GPRS_BITMASK)
					m_btnCellular.SetIcon( AtlLoadIcon(IDI_CONN_G) );

				if (m_dwConnectionType & SN_CELLSYSTEMCONNECTED_EDGE_BITMASK)
					m_btnCellular.SetIcon( AtlLoadIcon(IDI_CONN_E) );

				if (m_dwConnectionType & SN_CELLSYSTEMCONNECTED_UMTS_BITMASK)
					m_btnCellular.SetIcon( AtlLoadIcon(IDI_CONN_3G) );

				if (m_dwConnectionType & SN_CELLSYSTEMCONNECTED_HSDPA_BITMASK)
					m_btnCellular.SetIcon( AtlLoadIcon(IDI_CONN_H) );

			}
			else if (m_bActiveCall)
				m_btnCellular.SetIcon( AtlLoadIcon(IDI_INCALL) );
			else
				m_btnCellular.SetIcon( AtlLoadIcon(IDI_PHONE) );
		}

		if (m_bNoService)
		{
			if (!(m_bPhoneOff))
			{
				wsprintf(m_szCarrier, L"No service");
				m_btnCellular.SetIcon2( AtlLoadIcon(IDI_NO_SERVICE) );
			}
		}
		else if (m_bSearchingForService)
		{
			wsprintf(m_szCarrier, L"Searching");
			m_btnCellular.SetIcon2( AtlLoadIcon(IDI_SEARCHING) );
		}
		else
		{
			m_nBars -= 1;

			if (m_nBars == 0)
				m_btnCellular.SetIcon2( NULL );
			if (m_nBars == 1)
				m_btnCellular.SetIcon2( AtlLoadIcon(IDI_SIGNAL_1) );
			if (m_nBars == 2)
				m_btnCellular.SetIcon2( AtlLoadIcon(IDI_SIGNAL_2) );
			if (m_nBars == 3)
				m_btnCellular.SetIcon2( AtlLoadIcon(IDI_SIGNAL_3) );
			if (m_nBars == 4)
				m_btnCellular.SetIcon2( AtlLoadIcon(IDI_SIGNAL_4) );
		}




		return 0;
	}


	LRESULT OnSize(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
	{
		if(wParam != SIZE_MINIMIZED)
		{
			RECT rc; GetClientRect(&rc);

			// move buttons
			RECT rcHHTaskTar; ::GetClientRect(::FindWindow(L"HHTaskBar", NULL), &rcHHTaskTar);
			int hTaskBar = rcHHTaskTar.bottom - rcHHTaskTar.top;


			int h = m_arrStaticImages[0].GetHeight();
			int w = m_arrStaticImages[0].GetWidth();
			
			RECT rcBtn;
			SetRect(&rcBtn, rc.right - w, hTaskBar, rc.right, hTaskBar + h);
			m_btnVolume.MoveWindow(&rcBtn);

			SetRect(&rcBtn, rc.right - w, hTaskBar + h + 1, rc.right, hTaskBar + h + 1 + h);
			m_btnBattery.MoveWindow(&rcBtn);

			SetRect(&rcBtn, rc.right - w, hTaskBar + h + 1 + h + 1 + 1, rc.right, hTaskBar + h + 1 + h + 1 + h);
			m_btnCellular.MoveWindow(&rcBtn);


			int headOffset = hTaskBar + 3 * (h + 1);
			RECT rcNotifyList = {0, headOffset, rc.right, rc.bottom};
			m_notiflist.MoveWindow(&rcNotifyList);


		}

		bHandled = FALSE;
		return 0;
	}


	LRESULT OnTimer(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		if ((wParam == IDT_AUTOMINIMIZE) && (IsWindowVisible()))
		{
			StopMinimizeTimer();
			GetParent().PostMessage(UM_MINIMIZE);
		}

		return 0;
	}

	LRESULT OnActivateMinimizeTimer(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		ActivateMinimizeTimer();
		return 0;
	}

	LRESULT OnStopMinimizeTimer(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		StopMinimizeTimer();
		return 0;
	}

public: 

	CxImage m_imgIconBkg;
	CFontHandle m_fHeaderText;
	CFontHandle m_fTitleText;
	COLORREF m_clText;
	COLORREF m_clLine;

	COLORREF m_clBkgHeader;
	int m_scaleFactor;
	DWORD m_tmrMinimize;

	CxImage m_arrStaticImages[9];

	void SetScaleFactor(int value)
	{
		m_scaleFactor = value;
		m_notiflist.m_scaleFactor = value;
	}

	void ActivateMinimizeTimer()
	{
		KillTimer(IDT_AUTOMINIMIZE);
		SetTimer(IDT_AUTOMINIMIZE, m_tmrMinimize);
	}

	void StopMinimizeTimer()
	{
		KillTimer(IDT_AUTOMINIMIZE);
	}


private:
	// notifications
	HREGNOTIFY m_hVolume;
	HREGNOTIFY m_hBattery;
	HREGNOTIFY m_hPhoneStatus;
	HREGNOTIFY m_hSignal;
	HREGNOTIFY m_hOperator;
	HREGNOTIFY m_hCellSystem;
	HREGNOTIFY m_hActivePhoneCall;

	BYTE m_nBatteryPercent;
	BOOL m_bSpeakerOn;
	BOOL m_bVibrate;
	BOOL m_bCharging;
	BOOL m_bNoBattery;
	BOOL m_bBatteryCritical;
	BOOL m_bPhoneOff;
	BOOL m_bActiveDataCall;
	BOOL m_bActiveCall;
	BOOL m_bNoService;
	BOOL m_bSearchingForService;
	WCHAR m_szCarrier[50];
	DWORD m_dwConnectionType;
	int m_nBars; //0-5


	CFingerButton m_btnVolume;
	CFingerButton m_btnBattery;
	CFingerButton m_btnCellular;

	void RefreshStatus(DWORD dwWhich)
	{
		DWORD dw;
		HRESULT hr;


		// Speaker / volume
		if (dwWhich & TB_VOLUME_MASK) {
			hr = RegistryGetDWORD(
									SN_RINGMODE_ROOT,
									SN_RINGMODE_PATH,	
									SN_RINGMODE_VALUE,
										&dw);

			if (SUCCEEDED(hr)) {
				m_bSpeakerOn = dw == SN_RINGMODE_SOUND;
				m_bVibrate = dw == SN_RINGMODE_VIBRATE;
			}


		}

	    // Battery level & charging state
		if (dwWhich & TB_BATTERY_MASK) {
			SYSTEM_POWER_STATUS_EX2 sps = {0};
			DWORD result = ::GetSystemPowerStatusEx2(&sps,
				sizeof(SYSTEM_POWER_STATUS_EX2), false);

			if (result > 0) 
			{
				m_bCharging = (sps.BatteryFlag & BATTERY_FLAG_CHARGING) > 0;
				m_bNoBattery = (sps.BatteryFlag & BATTERY_FLAG_NO_BATTERY) > 0;
				m_bBatteryCritical = (sps.BatteryFlag & BATTERY_FLAG_CRITICAL) > 0;
				m_nBatteryPercent = sps.BatteryLifePercent;
			}


		}

		// Operator name
		if (dwWhich & TB_OPERATOR_MASK) {
			hr = RegistryGetString(
						SN_PHONEOPERATORNAME_ROOT, 
						SN_PHONEOPERATORNAME_PATH, 
						SN_PHONEOPERATORNAME_VALUE, 
						m_szCarrier, sizeof(m_szCarrier));
		}

		// Signal Strength
		if (dwWhich & TB_SIGNAL_MASK) {
			hr = RegistryGetDWORD(
						SN_PHONESIGNALSTRENGTH_ROOT, 
						SN_PHONESIGNALSTRENGTH_PATH, 
						SN_PHONESIGNALSTRENGTH_VALUE,
						&dw);
			if (SUCCEEDED(hr))
				m_nBars = (int)((double)dw / 20.0);
		}

		// connection type
		if (dwWhich & TB_CELLSYSTEMCONNECTED_MASK) {
			hr = RegistryGetDWORD(
						SN_CELLSYSTEMCONNECTED_ROOT, 
						SN_CELLSYSTEMCONNECTED_PATH, 
						SN_CELLSYSTEMCONNECTED_VALUE,
						&dw);
			if (SUCCEEDED(hr))
				m_dwConnectionType = dw;
		}
		
		// phone status
		if (dwWhich & TB_PHONE_STATUS_MASK) {
			hr = RegistryGetDWORD(
						SN_PHONERADIOOFF_ROOT, 
						SN_PHONERADIOOFF_PATH, 
						SN_PHONERADIOOFF_VALUE,
						&dw);
			if (SUCCEEDED(hr))
			{
				m_bPhoneOff = (dw & SN_PHONERADIOOFF_BITMASK) > 0;
				m_bActiveDataCall = (dw & SN_PHONEACTIVEDATACALL_BITMASK) > 0;
				m_bSearchingForService = (dw & SN_PHONESEARCHINGFORSERVICE_BITMASK) > 0;
				m_bNoService = (dw & SN_PHONENOSERVICE_BITMASK) > 0;
			}
		}

		// active call
		if (dwWhich & TB_PHONEACTIVECALLCOUNT_MASK) {
			hr = RegistryGetDWORD(
						SN_PHONEACTIVECALLCOUNT_ROOT, 
						SN_PHONEACTIVECALLCOUNT_PATH, 
						SN_PHONEACTIVECALLCOUNT_VALUE,
						&dw);
			if (SUCCEEDED(hr))
				m_bActiveCall = dw > 0;
			else
				m_bActiveCall = FALSE;
		}

		CONNMGR_CONNECTION_DETAILED_STATUS  *pConnMgrDet = NULL;
		HRESULT hResult; 
		DWORD dwBufferSize=0; //Code Snippet
		hResult = ConnMgrQueryDetailedStatus(pConnMgrDet, &dwBufferSize);

		if (hResult == (HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER)))
		{
			pConnMgrDet = (CONNMGR_CONNECTION_DETAILED_STATUS*)malloc(dwBufferSize);
			hResult = ConnMgrQueryDetailedStatus(pConnMgrDet, &dwBufferSize);
		}

		CONNMGR_CONNECTION_DETAILED_STATUS  *pCurrent = pConnMgrDet;

		if (hResult == S_OK)
		{
			while (pCurrent != NULL) 
			{
				//Do what you want eg. if you want to check cellular network
				if (pCurrent->dwType == CM_CONNTYPE_CELLULAR) 
				{
					//Do some thing 
				}

				pCurrent = pCurrent->pNext;
			}
		}

		free(pConnMgrDet);
	}


};
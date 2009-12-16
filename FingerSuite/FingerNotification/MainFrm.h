// MainFrm.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "..\Common\AboutView.h"

typedef enum {QVGA = 320, WQVGA = 400, VGA = 640, WVGA = 800} RESOLUTION;

static UINT UWM_INTERCEPT_NOTIF = ::RegisterWindowMessage(UWM_INTERCEPT_NOTIF_MSG);
static UINT UWM_NOTIF_POPUP = ::RegisterWindowMessage(UWM_NOTIF_POPUP_MSG);





typedef struct tagNMSHN_LINK
{
	NMSHN inner;
	WCHAR szLink[1024];
} NMSHN_LINK, *LPNMSHN_LINK;





class CMainFrame : public CFrameWindowImpl<CMainFrame>, public CUpdateUI<CMainFrame>,
 	public CFullScreenFrame<CMainFrame, false>, public CMessageFilter, public CIdleHandler
{
public:
	DECLARE_FRAME_WND_CLASS(NULL, IDR_MAINFRAME)

	CHTMLNotifWindow m_notif;
	CNotifListWindow m_list;
	CAboutView       m_aboutView;

	virtual BOOL PreTranslateMessage(MSG* pMsg)
	{
		if(CFrameWindowImpl<CMainFrame>::PreTranslateMessage(pMsg))
			return TRUE;

		return m_list.PreTranslateMessage(pMsg);
	}

	virtual BOOL OnIdle()
	{
		return FALSE;
	}


	BEGIN_UPDATE_UI_MAP(CMainFrame)
	END_UPDATE_UI_MAP()

	BEGIN_MSG_MAP(CMainFrame)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_ACTIVATE, OnActivate)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd)
		MESSAGE_HANDLER(UWM_INTERCEPT_NOTIF, OnInterceptNotif)
		MESSAGE_HANDLER(UWM_NOTIF_POPUP, OnNotifPopup)
		MESSAGE_HANDLER(WM_NOTIFY, OnNotify)
		MESSAGE_HANDLER(UM_SELECTNOTIF, OnSelectNotif)
		MESSAGE_HANDLER(UM_MINIMIZE, OnMinimize)
		COMMAND_ID_HANDLER(ID_MENU_EXIT, OnMenuExit)
		COMMAND_ID_HANDLER(ID_MENU_ABOUT, OnMenuAbout)
		COMMAND_ID_HANDLER(ID_MENU_SHOWORIGINAL, OnMenuShowOriginal)
		MESSAGE_HANDLER(WM_COMMAND, OnCommand)
		CHAIN_MSG_MAP(CUpdateUI<CMainFrame>)
		CHAIN_MSG_MAP(CFrameWindowImpl<CMainFrame>)
	END_MSG_MAP()

// Handler prototypes (uncomment arguments if needed):
//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		CreateSimpleCEMenuBar();

		LoadConfiguration();

		m_hWndClient = m_list.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);

		m_notif.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);

		CString strCredits = "\n\n"
			    "\tFingerNotification v1.00\n\n"
				"\rDeveloped by:\n"
				"Francesco Carlucci\n"
				"<francarl@gmail.com>\n"
				"\n\n"
				"http://forum.xda-developers.com/\n"
				"                showthread.php?t=459125\n";

		m_aboutView.SetCredits(strCredits);
		m_aboutView.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE |WS_CLIPSIBLINGS | WS_CLIPCHILDREN);  //


		// register object for message filtering and idle updates
		CMessageLoop* pLoop = _Module.GetMessageLoop();
		ATLASSERT(pLoop != NULL);
		pLoop->AddMessageFilter(this);
		pLoop->AddIdleHandler(this);

		SetHWndServerNotif(m_hWnd);

		InstallHookOnShell();

		Sleep(1000);

		//ParseStandardNotification();
		ModifyStyle(0, WS_NONAVDONEBUTTON, SWP_NOSIZE);

		InstallHookOnHHTaskBar();

		LOG(L"FingerNotification started.\n");

		return 0;
	}

	LRESULT OnSize(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
	{
		if(wParam != SIZE_MINIMIZED)
		{
		}
			
		return 0;
	}

	LRESULT OnActivate(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		int fActive = LOWORD(wParam); 
		if ((fActive == WA_ACTIVE) || (fActive == WA_CLICKACTIVE))
		{
		}
		else if (fActive == WA_INACTIVE)
		{
			if (IsWindowVisible())
			{
				Minimize();
			}
		}
		return 0;
	}

	LRESULT OnEraseBkgnd(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		CDCHandle dc( (HDC) wParam );
		RECT rc; GetClientRect(&rc);

		if (m_bDisabledTransp)
		{
			dc.FillSolidRect(&rc, m_clNoTranspBackground);
		}
		else
		{
			if (!(m_imgBkg.IsValid()))
				CaptureScreen(dc);

			m_imgBkg.Draw(dc, rc);
		}
		return 1;
	}



	LRESULT OnInterceptNotif(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
	{
		int idx;
		LPNOTIFICATIONINFO pInfo = (LPNOTIFICATIONINFO)malloc(sizeof(NOTIFICATIONINFO));
		memcpy(pInfo, (LPNOTIFICATIONINFO)lParam, sizeof(NOTIFICATIONINFO));

		// adjust pointers
		pInfo->nd.pszTitle = pInfo->szTitle;
		pInfo->nd.pszHTML = pInfo->szHTML;

		// menu
		if (pInfo->nd.grfFlags & SHNF_HASMENU)
		{
			pInfo->nd.skm.prgskc = pInfo->rgskc;
		}
		else
		{
			pInfo->nd.rgskn[0].pszTitle = pInfo->sk1Title;
			pInfo->nd.rgskn[1].pszTitle = pInfo->sk2Title;
		}

		DUMP_SHN(&pInfo->nd);


		switch (wParam)
		{
		case NOTIF_ADD:

			LOG(L"\nNOTIF_ADD\n");

			g_pNotifications.Add(pInfo);

			if (pInfo->isManaged)
			{
				// tempor disabled
				//ShowNotification(g_pNotifications.GetSize() - 1); // show last notification added
			}
			
			break;
		case NOTIF_UPD:

			LOG(L"\nNOTIF_UPD\n");

			idx = -1;
			for (int i = 0; i < g_pNotifications.GetSize(); i++)
			{
				LPNOTIFICATIONINFO pFind = g_pNotifications[i];
				if (
					//(pFind->nd.dwID == pData->nd.dwID) &&
					(IsEqualCLSID(pFind->nd.clsid, pInfo->nd.clsid)))
				{
					idx = i;
					break;
				}
			}

			if (idx > -1)
			{
				free(g_pNotifications[idx]);
				g_pNotifications.RemoveAt(idx);
				g_pNotifications.Add(pInfo);

				// tempor disabled
				if (pInfo->isManaged)
					ShowNotification(g_pNotifications.GetSize() - 1); // show last notification added
			}

			break;
		case NOTIF_DEL:

			LOG(L"\nNOTIF_DEL\n");

			idx = -1;
			for (int i = 0; i < g_pNotifications.GetSize(); i++)
			{
				LPNOTIFICATIONINFO pFind = g_pNotifications[i];
				if (
					//(pFind->nd.dwID == pData->nd.dwID) &&
					(IsEqualCLSID(pFind->nd.clsid, pInfo->nd.clsid)))
				{
					idx = i;
					break;
				}
			}

			if (idx > -1)
			{
				free(g_pNotifications[idx]);
				g_pNotifications.RemoveAt(idx);
			}


			break;
		} // end switch

		return 0;
	}

	LRESULT OnNotifPopup(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		ShowNotificationList();
		return 0;
	}

	LRESULT OnCommand(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		WORD wID = LOWORD(wParam); 

		// ricerca 
		LPNOTIFICATIONINFO pInfo = g_pNotifications[m_idx];
		BOOL bSendCommand = TRUE;
		DWORD dwSoftkeyType = 0;
		HWND hwndSink = pInfo->nd.hwndSink;

		if (pInfo->nd.grfFlags & SHNF_HASMENU)
		{
			for (UINT i = 0; i < pInfo->nd.skm.cskc; i++)
			{
				if (wID == pInfo->nd.skm.prgskc[i].wpCmd)
				{
					dwSoftkeyType = pInfo->nd.skm.prgskc[i].grfFlags;
					break; // exit for
				}
			}
		}
		else
		{
			if (pInfo->nd.rgskn[0].skc.wpCmd == wID)
				dwSoftkeyType = pInfo->nd.rgskn[0].skc.grfFlags;

			if (pInfo->nd.rgskn[1].skc.wpCmd == wID)
				dwSoftkeyType = pInfo->nd.rgskn[1].skc.grfFlags;
		}


		if (dwSoftkeyType == NOTIF_SOFTKEY_FLAGS_DISMISS)
		{
			SendNotifyDismiss(pInfo, FALSE);
			SHNotificationRemove(&pInfo->nd.clsid, pInfo->nd.dwID);
			Minimize();
		}
		if (dwSoftkeyType & NOTIF_SOFTKEY_FLAGS_HIDE)
		{
			Minimize();
		}
		if (dwSoftkeyType & NOTIF_SOFTKEY_FLAGS_STAYOPEN)
		{
		}
		if (dwSoftkeyType & NOTIF_SOFTKEY_FLAGS_SUBMIT_FORM)
		{
			bSendCommand = FALSE;
			m_notif.SubmitForm();
			Minimize();
		}

		if (bSendCommand)
			::PostMessage(hwndSink, WM_COMMAND, (WPARAM)wID, (LPARAM)0);


		return 0;
	}

	LRESULT OnNotify(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
	{
		LOG(L"\n");

		LPNOTIFICATIONINFO pInfo = g_pNotifications[m_idx];

		DWORD dwCmd = 0;

		int idCtrl = (int) wParam; 

		NM_HTMLVIEW * pnmHTML = (NM_HTMLVIEW *) lParam;
		LPNMHDR pnmh = (LPNMHDR) &(pnmHTML->hdr);

		switch (pnmh->code)
		{
		case NM_INLINE_IMAGE:
			LOG(L"NM_INLINE_IMAGE\n\n");
		   	if (lstrcmpi(pnmHTML->szTarget, L"fngrnotif_btn_bkg.gif") == 0)
			{
				INLINEIMAGEINFO imageInfo;
				imageInfo.dwCookie    = pnmHTML->dwCookie;
				imageInfo.bOwnBitmap  = FALSE;
				imageInfo.hbm         = m_bmpHtmlButton;
				imageInfo.iOrigWidth  = 1;
				imageInfo.iOrigHeight = 70;

				m_notif.m_html.SetImage(&imageInfo);
			}

			break;
		case NM_HOTSPOT:
			LOG(L"NM_HOTSPOT\n\n");

			SendNotifyDismiss(pInfo, FALSE);
			Minimize();

			// command in format of cmd:XXX 
			// sent to hwndSink
			
			swscanf(pnmHTML->szTarget, L"cmd:%d", &dwCmd);

			if (dwCmd != 0)
			{
				::PostMessage(pInfo->nd.hwndSink, WM_COMMAND, (WPARAM)dwCmd, (LPARAM)0);
			}
			else
			{
				// send SHNM_LINKSEL
				SendNotifyLink(pInfo, pnmHTML->szTarget);
			}

			break;
		
		case NM_BEFORENAVIGATE:
			LOG(L"NM_BEFORENAVIGATE\n");
			LOG(L"\n");
			break;
		case NM_DOCUMENTCOMPLETE:
			LOG(L"NM_DOCUMENTCOMPLETE\n");
			LOG(L"\n");
			m_notif.UpdateWindow();
			break;
		
		default:
			LOG(L"NOT MANAGED: %X\n", pnmh->code);
			LOG(L"\n");
		}

		LOG(L"pnmHTML->szTarget: %s\n", pnmHTML->szTarget);
		LOG(L"pnmHTML->szData: %s\n", pnmHTML->szData);
		LOG(L"pnmHTML->szExInfo: %s\n", pnmHTML->szExInfo);

		return 0;
	}

	LRESULT OnSelectNotif(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		int idx = (int)wParam;
		LPNOTIFICATIONINFO pInfo = g_pNotifications[idx];
		if (pInfo->nd.npPriority != SHNP_ICONIC)
			ShowNotification(idx);
		return 0;
	}

	LRESULT OnMinimize(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		Minimize();
		return 0;
	}

	LRESULT OnMenuExit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& bHandled)
	{
		Minimize();

		PostMessage(WM_CLOSE);

		bHandled = TRUE;
		return 0;
	}


	LRESULT OnMenuAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& bHandled)
	{
		SwitchToAboutView();
		SetForegroundWindow((HWND)((ULONG) m_hWnd | 0x00000001));
		ShowWindow(SW_SHOW);	
		bHandled = TRUE;
		return 0;
	}		

	LRESULT OnMenuShowOriginal(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& bHandled)
	{
		Minimize();
		// TODO
		
		bHandled = TRUE;
		return 0;
	}

public:


	static HRESULT ActivatePreviousInstance(HINSTANCE hInstance)
	{
		const TCHAR* pszMutex = L"FINGERNOTIFMUTEX";
		const TCHAR* pszClass = L"FINGER_NOTIFICATION";

		const DWORD dRetryInterval = 100; 

		const int iMaxRetries = 25;

		for(int i = 0; i < iMaxRetries; ++i)
		{
			HANDLE hMutex = CreateMutex(NULL, FALSE, pszMutex);

			DWORD dw = GetLastError();

			if(hMutex == NULL)
			{
				HRESULT hr = (dw == ERROR_INVALID_HANDLE) ? E_INVALIDARG : E_FAIL;
				return hr;
			}

			if(dw == ERROR_ALREADY_EXISTS)
			{
				CloseHandle(hMutex);

				HWND hwnd = FindWindow(pszClass, NULL);

				if(hwnd == NULL)
				{
					Sleep(dRetryInterval);
					continue;
				}
				else
				{
					if(SetForegroundWindow(reinterpret_cast<HWND>(reinterpret_cast<ULONG>(hwnd) | 0x1)) != 0)
					{
						return S_FALSE;
					}
				}
			}
			else
			{
				return S_OK;
			}
		}

		return S_OK;
	}

	void Show()
	{
		SetForegroundWindow((HWND)((ULONG) m_hWnd | 0x00000001));
		ShowWindow(SW_SHOW);
	}

	void Minimize()
	{
		ShowWindow(SW_HIDE);
	}

private:

	int m_idx;
	CxImage m_imgBkg;
	DWORD m_iTransp;
	BOOL m_bDisabledTransp;
	COLORREF m_clNoTranspBackground;
	HBITMAP m_bmpHtmlButton;

	void SendNotifyDismiss(LPNOTIFICATIONINFO pData, BOOL bTimeout)
	{
		NMSHN nm;
		ZeroMemory(&nm, sizeof(NMSHN));
		nm.hdr.code = SHNN_DISMISS;
		nm.hdr.idFrom = pData->nd.dwID;
		nm.lParam = pData->nd.lParam;
		nm.fTimeout = bTimeout;
		
		SendMessageRemote(pData->nd.hwndSink, WM_NOTIFY, 0, (LPARAM)&nm, sizeof(NMSHN));
	}

	void SendNotifyShow(LPNOTIFICATIONINFO pData)
	{
		NMSHN nm;
		ZeroMemory(&nm, sizeof(NMSHN));
		nm.hdr.code = SHNN_SHOW;
		nm.hdr.idFrom = pData->nd.dwID;
		nm.lParam = pData->nd.lParam;
		nm.pt.x = 1;
		nm.pt.y = 1;
		
		SendMessageRemote(pData->nd.hwndSink, WM_NOTIFY, 0, (LPARAM)&nm, sizeof(NMSHN));
	}

	void SendNotifyLink(LPNOTIFICATIONINFO pData, LPCTSTR pszLink)
	{
		HANDLE hFile = NULL;
		LPVOID pView = PrepareRemoteMessage(&hFile, sizeof(NMSHN_LINK));

		if (pView != NULL)
		{
			LPNMSHN_LINK lpnm = (LPNMSHN_LINK)pView;
			ZeroMemory(lpnm, sizeof(NMSHN_LINK));
			lpnm->inner.hdr.code = SHNN_LINKSEL;
			lpnm->inner.hdr.idFrom = pData->nd.dwID;
			lpnm->inner.lParam = pData->nd.lParam;
			lpnm->inner.dwReturn = 1;
			wsprintf(lpnm->szLink, L"%s", pszLink);
			lpnm->inner.pszLink = lpnm->szLink;
			
			SendMessage(pData->nd.hwndSink, WM_NOTIFY, 0, (LPARAM)lpnm);
			
			CloseRemoteMessage(hFile, pView);
		}
	}

	void LoadConfiguration()
	{
		WCHAR keyName[] = L"Software\\FingerNotification";

		// load transparency level

		m_iTransp = 128;
		if (!(RegReadDWORD(HKEY_LOCAL_MACHINE, keyName, L"TransparencyLevel", m_iTransp)))
			wprintf(L"Unable to read TransparencyLevel from registry: %s\n", ErrorString(GetLastError()));
		m_iTransp = m_iTransp;

		m_bDisabledTransp = FALSE;
		if (!(RegReadBOOL(HKEY_LOCAL_MACHINE, keyName, L"DisableTransparency", m_bDisabledTransp)))
			wprintf(L"Unable to read DisableTransparency from registry: %s\n", ErrorString(GetLastError()));

		m_clNoTranspBackground = RGB(0,0,0);
		RegReadColor(HKEY_LOCAL_MACHINE, keyName, L"NoTranspBkgColor", m_clNoTranspBackground);

		m_notif.m_bEnableAnimation = TRUE;
		if (!(RegReadBOOL(HKEY_LOCAL_MACHINE, keyName, L"EnableAnimation", m_notif.m_bEnableAnimation)))
			LOG(L"Unable to read EnableAnimation from registry: %s\n", ErrorString(GetLastError()));
		
		if ( RegistryGetDWORD ( HKEY_LOCAL_MACHINE, keyName, L"AutoMinimizeInterval", &m_list.m_tmrMinimize) != S_OK )
			m_list.m_tmrMinimize = 3000;


		WCHAR szSkin[60] = L"default";
		RegReadString(HKEY_LOCAL_MACHINE, keyName, L"Skin", szSkin);
		WCHAR szAppPath[MAX_PATH] = L"";
		CString strAppDirectory;
		GetModuleFileName(NULL, szAppPath, MAX_PATH);
		strAppDirectory = szAppPath;
		strAppDirectory = strAppDirectory.Left(strAppDirectory.ReverseFind('\\'));
		CString skinBasePath; skinBasePath.Format(L"%s\\skins\\%s", strAppDirectory, szSkin);
		
		// detect resolution
		RESOLUTION resolution = QVGA;
		DEVMODE dm;
		::ZeroMemory(&dm, sizeof(DEVMODE));
		dm.dmSize = sizeof(DEVMODE);
		if (EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dm))
		{
			resolution = (RESOLUTION)dm.dmPelsHeight;
		}

		// load icon images
		switch (resolution)
		{
		case QVGA:
		case WQVGA:
			m_notif.SetScaleFactor(1);
			m_list.SetScaleFactor(1);
			break;
		case VGA:
		case WVGA:
			m_notif.SetScaleFactor(2);			
			m_list.SetScaleFactor(2);			
			break;
		}		

		if ( (resolution == VGA) || (resolution == WVGA))
		{
			m_notif.m_imgHeader.Load(skinBasePath + L"\\notif_header_vga.png", CXIMAGE_FORMAT_PNG);
			m_notif.m_imgCalendar.Load(skinBasePath + L"\\icon_calendar_vga.png", CXIMAGE_FORMAT_PNG);
			m_list.m_notiflist.m_imgSelection.Load(skinBasePath + L"\\list_selection_vga.png", CXIMAGE_FORMAT_PNG);
			//m_list.m_imgIconBkg.Load(skinBasePath + L"\\icon_bkg_vga.png", CXIMAGE_FORMAT_PNG);

			// normal
			AddStaticSet(m_list.m_arrStaticImages, NORMAL, skinBasePath + L"\\notif_button_normal_vga.png");

			// pressed
			AddStaticSet(m_list.m_arrStaticImages, PUSHED, skinBasePath + L"\\notif_button_pressed_vga.png");

			// icons
			m_list.m_notiflist.AddIcon(L"missedcall", skinBasePath + L"\\icons\\notifications_icon_MissedCall_vga.png");
			m_list.m_notiflist.AddIcon(L"reminder", skinBasePath + L"\\icons\\notifications_icon_Reminder_vga.png");
			m_list.m_notiflist.AddIcon(L"message", skinBasePath + L"\\icons\\notifications_icon_Messages_vga.png");
			m_list.m_notiflist.AddIcon(L"warning", skinBasePath + L"\\icons\\notifications_icon_Warning_vga.png");
			m_list.m_notiflist.AddIcon(L"email", skinBasePath + L"\\icons\\notifications_icon_Email_vga.png");
			m_list.m_notiflist.AddIcon(L"wifi", skinBasePath + L"\\icons\\notifications_icon_Wifi_vga.png");
			m_list.m_notiflist.AddIcon(L"bluetooth", skinBasePath + L"\\icons\\notifications_icon_Bluetooth_vga.png");

		}
		else
		{
			m_notif.m_imgHeader.Load(skinBasePath + L"\\notif_header_qvga.png", CXIMAGE_FORMAT_PNG);
			m_notif.m_imgCalendar.Load(skinBasePath + L"\\icon_calendar_qvga.png", CXIMAGE_FORMAT_PNG);
			m_list.m_notiflist.m_imgSelection.Load(skinBasePath + L"\\list_selection_qvga.png", CXIMAGE_FORMAT_PNG);
			//m_list.m_imgIconBkg.Load(skinBasePath + L"\\icon_bkg_qvga.png", CXIMAGE_FORMAT_PNG);

			// normal
			AddStaticSet(m_list.m_arrStaticImages, NORMAL, skinBasePath + L"\\notif_button_normal_qvga.png");

			// pressed
			AddStaticSet(m_list.m_arrStaticImages, PUSHED, skinBasePath + L"\\notif_button_pressed_qvga.png");

			// icons
			m_list.m_notiflist.AddIcon(L"missedcall", skinBasePath + L"\\icons\\notifications_icon_MissedCall_qvga.png");
			m_list.m_notiflist.AddIcon(L"reminder", skinBasePath + L"\\icons\\notifications_icon_Reminder_qvga.png");
			m_list.m_notiflist.AddIcon(L"message", skinBasePath + L"\\icons\\notifications_icon_Messages_qvga.png");
			m_list.m_notiflist.AddIcon(L"warning", skinBasePath + L"\\icons\\notifications_icon_Warning_qvga.png");
			m_list.m_notiflist.AddIcon(L"email", skinBasePath + L"\\icons\\notifications_icon_Email_qvga.png");
			m_list.m_notiflist.AddIcon(L"wifi", skinBasePath + L"\\icons\\notifications_icon_Wifi_qvga.png");
			m_list.m_notiflist.AddIcon(L"bluetooth", skinBasePath + L"\\icons\\notifications_icon_Bluetooth_qvga.png");

		}

		m_bmpHtmlButton = SHLoadImageFile(skinBasePath + L"\\fngrnotif_btn_bkg.gif");
		
		// load skin settings
		CSimpleIniW ini(TRUE, TRUE, TRUE);
		SI_Error rc = ini.LoadFile(skinBasePath + L"\\settings_fingernotif.ini");

		// load background and selected color
		COLORREF clValue = 0;
		m_notif.m_clCaptionText     = (StringRGBToColor(ini.GetValue(L"colors", L"CaptionTextColor"), clValue)) ? clValue : RGB(255,255,255);
		m_list.m_clText             = (StringRGBToColor(ini.GetValue(L"colors", L"TextColor"), clValue)) ? clValue : RGB(255,255,255);
		m_list.m_notiflist.m_clText = m_list.m_clText;
		m_list.m_clLine             = (StringRGBToColor(ini.GetValue(L"colors", L"LineColor"), clValue)) ? clValue : RGB(97,97,97);
		m_list.m_notiflist.m_clLine = m_list.m_clLine;
		m_list.m_notiflist.m_clBkg  = (StringRGBToColor(ini.GetValue(L"colors", L"ListBkgColor"), clValue)) ? clValue : RGB(60,60,60);
		m_list.m_clBkgHeader        = (StringRGBToColor(ini.GetValue(L"colors", L"ListBkgHeaderColor"), clValue)) ? clValue : RGB(40,40,40);

		LOGFONT lf;
		if (StringToLogFont(ini.GetValue(L"fonts", L"NotifCaptionFont"), lf))
		{
			m_notif.m_fCaption.CreateFontIndirect(&lf);
		}
		if (StringToLogFont(ini.GetValue(L"fonts", L"ListHeaderTextFont"), lf))
		{
			m_list.m_fHeaderText.CreateFontIndirect(&lf);
		}
		if (StringToLogFont(ini.GetValue(L"fonts", L"ListTextFont"), lf))
		{
			m_list.m_notiflist.m_fText.CreateFontIndirect(&lf);
		}
		if (StringToLogFont(ini.GetValue(L"fonts", L"ListTitleFont"), lf))
		{
			m_list.m_fTitleText.CreateFontIndirect(&lf);
		}
	}



	void ShowNotification(int idx)
	{
		m_idx = idx;  

		LPNOTIFICATIONINFO pInfo = g_pNotifications[m_idx];

		if (pInfo->nd.npPriority != SHNP_ICONIC)
		{
			if (   (!(pInfo->grfFlagsOriginal & SHNF_STRAIGHTTOTRAY)) 
				|| (!(pInfo->grfFlagsOriginal & SHNF_SILENT)) 
				|| (pInfo->grfFlagsOriginal & SHNF_FORCEMESSAGE)
				|| (pInfo->grfFlagsOriginal & SHNF_CRITICAL)
				 )
			{
				CreateMenuBar(pInfo);
				SendNotifyShow(pInfo);
				m_list.StopMinimizeTimer();
				m_notif.SetNotification(pInfo);
				SetFullScreen(false);
				Show();
				SwitchToNotifView();
				m_notif.Show();
			}
		}
	}

	void ShowNotificationList()
	{
		SwitchToListView();
		
		SetFullScreen(true);
		
		Show();
		m_list.Show();
	}

	void CreateMenuBar(LPNOTIFICATIONINFO pData)
	{
		if (::IsWindow(m_hWndCECommandBar))
		{
			::DestroyWindow(m_hWndCECommandBar);
		}

		if (pData->nd.grfFlags  & SHNF_HASMENU)
		{
			SHMENUBARINFO mbi;
			ZeroMemory(&mbi, sizeof(SHMENUBARINFO));
			mbi.cbSize     = sizeof(SHMENUBARINFO);
			mbi.hwndParent = m_hWnd;
			mbi.nToolBarId = (UINT)(pData->nd.skm.hMenu);
			mbi.hInstRes   = ModuleHelper::GetModuleInstance();
			mbi.dwFlags    = SHCMBF_HMENU;
			if(SHCreateMenuBar(&mbi))
			{
				m_hWndCECommandBar = mbi.hwndMB;
				SizeToMenuBar();
			}
			else
			{
				LOG(L"Error: %s\n", ErrorString(GetLastError()));
			}
		}
		else
		{
			// menu from soft key
			CMenuHandle m; m.CreatePopupMenu();
			if (pData->nd.rgskn[0].skc.wpCmd != 0)
				m.AppendMenu(MF_STRING, pData->nd.rgskn[0].skc.wpCmd, pData->nd.rgskn[0].pszTitle);
			if (pData->nd.rgskn[1].skc.wpCmd != 0)
				m.AppendMenu(MF_STRING, pData->nd.rgskn[1].skc.wpCmd, pData->nd.rgskn[1].pszTitle);
			
			SHMENUBARINFO mbi;
			ZeroMemory(&mbi, sizeof(SHMENUBARINFO));
			mbi.cbSize     = sizeof(SHMENUBARINFO);
			mbi.hwndParent = m_hWnd;
			mbi.nToolBarId = (UINT)m.m_hMenu;
			mbi.hInstRes   = ModuleHelper::GetModuleInstance();
			mbi.dwFlags    = SHCMBF_HMENU;
			if(SHCreateMenuBar(&mbi))
			{
				m_hWndCECommandBar = mbi.hwndMB;
				SizeToMenuBar();
			}
			else
			{
				LOG(L"Error: %s\n", ErrorString(GetLastError()));
			}
		}
	}


	void ParseStandardNotification()
	{
		CSimpleArray<CString> clsids;

		SHNOTIFICATIONDATA shnd;
		CLSID clsid;
		LRESULT result;
		DWORD dwID = 0;

		clsids.Add("{A877D661-239C-47a7-9304-0D347F580408}");
		clsids.Add("{A877D660-239C-47a7-9304-0D347F580408}");
		clsids.Add("{A877D65A-239C-47a7-9304-0D347F580408}");
		clsids.Add("{A877D65D-239C-47a7-9304-0D347F580408}");
		clsids.Add("{A877D659-239C-47a7-9304-0D347F580408}");
		clsids.Add("{A877D658-239C-47a7-9304-0D347F580408}");
		clsids.Add("{15F11F90-8A5F-454c-89FC-BA9B7AAB0CAD}");
		clsids.Add("{B67B425B-365E-42c0-95F7-B75503D775B8}");
		clsids.Add("{DDBD3B44-80B0-4b24-9DC4-839FEA6E559E}");
		clsids.Add("{8ddf46e8-56ed-4750-9e58-afc6ce486d03}");
		clsids.Add("{8ddf46e7-56ed-4750-9e58-afc6ce486d03}");
		clsids.Add("{0D3132C4-1298-469c-B2B8-F28CE2D649D0}");
		clsids.Add("{A877D663-239C-47a7-9304-0D347F580408}");
		clsids.Add("{E0F2B9DD-EDC6-45d4-B440-2C5B5A04A3E3}");
		clsids.Add("{A877D65B-239C-47a7-9304-0D347F580408}");
		clsids.Add("{F55615D6-D29E-4db8-8C75-98125D1A7253}");

		LOG(L"BEGIN parse\n");

		for (int i = 0; i < clsids.GetSize(); i++)
		{
			LOG(L"\nFinding %s\n", clsids[i]);

			LPOLESTR c = (LPOLESTR)((LPCTSTR)clsids[i]);
			if (0 == CLSIDFromString(c, &clsid)) 
			{
				dwID = 0;
				memset(&shnd, 0, sizeof(shnd));
				shnd.cbStruct = sizeof(SHNOTIFICATIONDATA);
				do {
					LOG(L".");

					result = SHNotificationGetData(&clsid, dwID, &shnd);
					if (ERROR_SUCCESS == result) {

						LOG(L"\nNotification found\n");
						DUMP_SHN(&shnd);
						LOG(L"\n\n");

						if (shnd.pszHTML) free((void *) shnd.pszHTML);
						shnd.pszHTML = NULL;
						if (shnd.pszTitle) free((void *) shnd.pszTitle);
						shnd.pszTitle = NULL;
					} else dwID++;
				} while ((ERROR_SUCCESS != result) && (dwID < 70000));
			}
		}

		LOG(L"END parse\n");
	}


	void SwitchToNotifView()
	{
		m_hWndClient = m_notif;

		m_list.ShowWindow(SW_HIDE);
		m_list.SetWindowLongPtr(GWL_ID, 0);

		m_notif.ShowWindow(SW_SHOW);
		m_notif.SetWindowLongPtr(GWL_ID, ATL_IDW_PANE_FIRST);
		m_notif.SetFocus();
	}

	void SwitchToListView()
	{
		m_hWndClient = m_list;

		m_notif.ShowWindow(SW_HIDE);
		m_notif.SetWindowLongPtr(GWL_ID, 0);

		m_list.ShowWindow(SW_SHOW);
		m_list.SetWindowLongPtr(GWL_ID, ATL_IDW_PANE_FIRST);
		m_list.SetFocus();
		m_list.Invalidate();
		m_list.UpdateWindow();

	}

	void SwitchToAboutView()
	{
		SetFullScreen(FALSE);

		m_hWndClient = m_aboutView;

		m_list.ShowWindow(SW_HIDE);
		m_list.SetWindowLongPtr(GWL_ID, 0);

		m_notif.ShowWindow(SW_HIDE);
		m_notif.SetWindowLongPtr(GWL_ID, 0);

		m_aboutView.ShowWindow(SW_SHOW);
		m_aboutView.SetWindowLongPtr(GWL_ID, ATL_IDW_PANE_FIRST);
		m_aboutView.SetFocus();
		UpdateLayout();
	}

	void CaptureScreen(CDCHandle dc)
	{
		RECT rc; GetClientRect(&rc);
		CDC dcTemp; dcTemp.CreateCompatibleDC(dc);
		CBitmap bmpBkg; bmpBkg.CreateCompatibleBitmap(dc, rc.right, rc.bottom);
		CBitmap bmpOld = dcTemp.SelectBitmap(bmpBkg);
		dcTemp.FillSolidRect(&rc, RGB(0,0,0));
		dcTemp.SelectBitmap(bmpOld);
		m_imgBkg.CreateFromHBITMAP(bmpBkg);
		m_imgBkg.AlphaCreate();
		m_imgBkg.AlphaSet((BYTE)m_iTransp);
	}

};

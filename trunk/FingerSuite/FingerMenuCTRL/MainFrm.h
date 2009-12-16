// MainFrm.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

//DEFINE_GUID(CLSID_ImagingFactory, 0x327abda8,0x072b,0x11d3,0x9d,0x7b,0x00,0x00,0xf8,0x1e,0xf3,0x2e);

#include "..\Common\AboutView.h"
#include "..\FingerSuiteCPL\Commons.h"
#include "..\Common\fngrsplash.h"

typedef enum {SQVGA = 240, QVGA = 320, WQVGA = 400, VGA = 640, WVGA = 800} RESOLUTION;

static UINT UWM_INTERCEPT_MENU = ::RegisterWindowMessage(UWM_INTERCEPT_MENU_MSG);

static UINT UWM_UPDATECONFIGURATION = ::RegisterWindowMessage(UWM_UPDATECONFIGURATION_MSG);


class CMainFrame : public CFrameWindowImpl<CMainFrame>, public CUpdateUI<CMainFrame>,
		public CMessageFilter, public CIdleHandler
{
public:
	DECLARE_FRAME_WND_CLASS(L"FINGER_MENU", IDR_MAINFRAME)

	CWrapMenuWindow  m_menuView;
	CAboutView       m_aboutView;


	virtual BOOL PreTranslateMessage(MSG* pMsg)
	{
		if(CFrameWindowImpl<CMainFrame>::PreTranslateMessage(pMsg))
			return TRUE;

		if (pMsg->message == WM_LBUTTONDOWN)
		{
			m_xPos = LOWORD(pMsg->lParam);
			m_bMouseIsDown = TRUE;
		}

		if (pMsg->message == WM_LBUTTONUP)
		{
			// back gesture recognition
			if (m_bMouseIsDown)
			{
				m_bMouseIsDown = FALSE;
				int xPosNew =  LOWORD(pMsg->lParam);
				
				if ((m_xPos - xPosNew) > 100 * m_menuView.GetScaleFactor())
				{
					// back gesture recognized 
					Back();
					return TRUE;
				}
			}
		}

		return m_menuView.PreTranslateMessage(pMsg);
	}

	virtual BOOL OnIdle()
	{
		return FALSE;
	}
	
	BEGIN_UPDATE_UI_MAP(CMainFrame)
	END_UPDATE_UI_MAP()

	BEGIN_MSG_MAP(CMainFrame)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd)
		MESSAGE_HANDLER(WM_ACTIVATE, OnActivate)
		MESSAGE_HANDLER(WM_SETTINGCHANGE , OnSettingsChange)
		MESSAGE_HANDLER(UM_MINIMIZE, OnMinimize)
		MESSAGE_HANDLER(UWM_INTERCEPT_MENU, OnInterceptMenu)
		MESSAGE_HANDLER(UWM_UPDATECONFIGURATION, OnUpdateConfiguration)
		MESSAGE_HANDLER(UM_SETNEWMENU, OnSetNewMenu)
		MESSAGE_HANDLER(UM_GETDESTWND, OnGetDestWnd)
		COMMAND_ID_HANDLER(ID_MENU_CANCEL, OnCancel)
		COMMAND_ID_HANDLER(ID_ACTION, OnAction)
		COMMAND_ID_HANDLER(ID_MENU_EXIT, OnMenuExit)
		COMMAND_ID_HANDLER(ID_MENU_ADDTOEXCLUSIONLIST, OnMenuAddToExclusionList)
		COMMAND_ID_HANDLER(ID_MENU_ADDTOWNDEXCLUSIONLIST, OnMenuAddToWndExclusionList)
		COMMAND_ID_HANDLER(ID_MENU_SHOWORIGINAL, OnMenuShowOriginal)
		COMMAND_ID_HANDLER(ID_MENU_ABOUT, OnMenuAbout)
		COMMAND_ID_HANDLER(ID_BACK, OnBack)
		CHAIN_MSG_MAP(CUpdateUI<CMainFrame>)
		CHAIN_MSG_MAP(CFrameWindowImpl<CMainFrame>)
	END_MSG_MAP()


	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		RestoreMenuBar();

		CSplashWindow* pSplash = new CSplashWindow(m_hWnd, IDR_MAINFRAME, L"FingerMenu");


		LoadConfiguration();

		// menu
		m_hWndClient = m_menuView.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE |WS_CLIPSIBLINGS | WS_CLIPCHILDREN);  //

		// about
		CString strCredits = "\n\n"
			    "\tFingerMenu v1.12\n\n"
				"\rdeveloped by:\n"
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

		CreateConfigMenu();

		if (m_bEnableSIP)
			InstallHookOnSIP();

		SetHWndServer(m_hWnd);

		InstallHook();

		SignalWaitEvent(EVT_FNGRMENU);

		ModifyStyle(0, WS_NONAVDONEBUTTON, SWP_NOSIZE);

		pSplash->Dismiss();

		return 0;  
	}

	LRESULT OnSize(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
	{
		if(wParam != SIZE_MINIMIZED)
		{

		}
			
		return 0;
	}


	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		//RemoveHook(); TODO 

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
				Minimize(FALSE);
			}
		}
		return 0;
	}

	LRESULT OnSettingsChange(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		if (wParam == SETTINGCHANGE_RESET)
		{
			m_menuView.ModifyTopShape();

			m_menuView.Show();
		}
		return 0;
	}

	LRESULT OnGetDestWnd(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		return (LRESULT)m_hDestWnd;
	}

	LRESULT OnSetNewMenu(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		// always called for sub menu
		CreateBackMenuBar();
		SetNewMenu((HMENU)wParam);
		return 0;
	}



	LRESULT OnMinimize(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		Minimize((int)wParam);
		return 0;
	}

	LRESULT OnCancel(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		Minimize(FALSE);

		return 0;
	}

	/*
	LRESULT OnInterceptMenu1(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		LOG(L"Intercepting TrackMenuPopupEx (1) ...hMenu=%08x hWnd=%08X\n", (HMENU)wParam, (HWND)lParam);

		if (IsDeviceLocked())
			return 1;

		HWND hDestWnd = (HWND)lParam;
		HWND hWndSIP = FindWindow(NULL, L"MS_SIPBUTTON");
		if (hDestWnd == hWndSIP) 
		{
			if (m_bEnableSIP)
				m_menuView.SetNewMenu((HMENU)wParam, NULL, NULL, FALSE);	
			else
				return 1;
		}
		else
		{
			if (IsExcludedApp(hDestWnd))
				return 1;
			
			HWND hDestCtrl = ManageDOTNETApp(hDestWnd);

			if (!(m_menuView.SetNewMenu((HMENU)wParam, hDestWnd, hDestCtrl, TRUE)))
				return 1;
		}
		
		//Sleep(100);

		RestoreMenuBar();
		
		SwitchToMenuView();

		SetForegroundWindow((HWND)((ULONG) m_hWnd | 0x00000001));
		ShowWindow(SW_SHOW);		
		
		bHandled = TRUE;
		return 0;
	}
	*/


	LRESULT OnInterceptMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled)
	{
		//LOG(L"Intercepting TrackMenuPopupEx (2)...hMenu=%08x hWnd=%08X\n", (HMENU)wParam, (HWND)lParam);

		if (IsDeviceLocked())
			return 1;

		LPTRACKPOPUPMENUINFO lpInfo = (LPTRACKPOPUPMENUINFO)lParam;

		m_hPrevDestWnd = m_hDestWnd;

		m_hDestWnd = lpInfo->hWnd;
		m_uFlags = lpInfo->uFlags;


		HWND hWndSIP = FindWindow(NULL, L"MS_SIPBUTTON");
		if (m_hDestWnd == hWndSIP) 
		{
			if (!(m_bEnableSIP))
				return 1;
		}
		else
		{
			if (IsExcludedApp(m_hDestWnd))
				return 1;

			if (IsExcludedWnd(m_hDestWnd))
				return 1;

		}

		m_menus.RemoveAll();
		

		if (!(SetNewMenu(lpInfo->hMenu)))
			return 1;

		RestoreMenuBar();
		
		SwitchToMenuView();

		SetForegroundWindow((HWND)((ULONG) m_hWnd | 0x00000001));
		ShowWindow(SW_SHOW);		
		
		bHandled = TRUE;
		return 0;
	}

	LRESULT OnAction(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		m_hPrevDestWnd = m_hDestWnd;
		m_hDestWnd = m_hWnd;

		SetNewMenu(m_menuConfig);

		SwitchToMenuView();

		SetForegroundWindow((HWND)((ULONG) m_hWnd | 0x00000001));
		ShowWindow(SW_SHOW);		
		return 0;
	}

	LRESULT OnBack(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		Back();
		return 0;
	}

	LRESULT OnMenuExit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		Minimize(FALSE);
		PostMessage(WM_CLOSE);
		return 0;
	}

	LRESULT OnMenuAddToExclusionList(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		DWORD dwProcessId; GetWindowThreadProcessId(m_hPrevDestWnd, &dwProcessId);
		HMODULE hProcess = (HMODULE)dwProcessId;
		WCHAR szName[MAX_PATH];
		if (GetModuleFileName(hProcess, szName, MAX_PATH))
		{
			CString szApp = (LPTSTR)szName;
			szApp = szApp.Right(szApp.GetLength() - szApp.ReverseFind('\\') - 1);

			if (m_excludedApps.Find(szApp) == -1)
				m_excludedApps.Add(szApp);

			SaveExclusionList(m_excludedApps, L"Software\\FingerMenu");
		}

		Minimize(-1);

		return 0;
	}

	LRESULT OnMenuAddToWndExclusionList(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		WCHAR szTitle[MAX_PATH];
		::GetWindowText(m_hPrevDestWnd, szTitle, MAX_PATH); 

		WCHAR szClassName[MAX_PATH];
		::GetClassName(m_hPrevDestWnd, szClassName, MAX_PATH);

		if (lstrcmp(szClassName, L"MS_SOFTKEY_CE_1.0") == 0)
		{
			Minimize(FALSE);
			return 0;
		}

		WCHAR szName[MAX_PATH];
		wsprintf(szName, L"%s - %s", szTitle, szClassName);
		
		CString szWnd = (LPTSTR)szName;
		if (m_excludedWnds.Find(szWnd) == -1)
			m_excludedWnds.Add(szWnd);

		SaveWndExclusionList(m_excludedWnds, L"Software\\FingerMenu");

		Minimize(-1);

		return 0;
	}

	LRESULT OnMenuShowOriginal(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		Minimize(-1);
		
		return 0;
	}

	LRESULT OnMenuAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		SwitchToAboutView();
		SetForegroundWindow((HWND)((ULONG) m_hWnd | 0x00000001));
		ShowWindow(SW_SHOW);	
		return 0;
	}		

	LRESULT OnUpdateConfiguration(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		LoadConfiguration();

		Minimize(FALSE);
		return 0;
	}

private:

	BOOL RestoreMenuBar()
	{
		SHMENUBARINFO mbi = { 0 };
		mbi.cbSize = sizeof(mbi);
		mbi.hwndParent = m_hWnd;
		mbi.dwFlags = SHCMBF_HIDESIPBUTTON;
		mbi.nToolBarId = FM_IDW_MENU_BAR;
		mbi.hInstRes  = ModuleHelper::GetResourceInstance();
		mbi.hwndMB = NULL;   // This gets set by SHCreateMenuBar

		BOOL bRet = ::SHCreateMenuBar(&mbi);
		if (bRet != FALSE)
		{
			m_hWndCECommandBar = mbi.hwndMB;
			SizeToMenuBar();
		}

		return bRet;
	}

	BOOL CreateBackMenuBar()
	{
		SHMENUBARINFO mbi = { 0 };
		mbi.cbSize = sizeof(mbi);
		mbi.hwndParent = m_hWnd;
		mbi.dwFlags = SHCMBF_HIDESIPBUTTON;
		mbi.nToolBarId = FM_IDW_BACK_MENU_BAR;
		mbi.hInstRes  = ModuleHelper::GetResourceInstance();
		mbi.hwndMB = NULL;   // This gets set by SHCreateMenuBar

		BOOL bRet = ::SHCreateMenuBar(&mbi);
		if( bRet != FALSE)
		{
			m_hWndCECommandBar = mbi.hwndMB;
			SizeToMenuBar();
		}

		return TRUE;
	}		



	void Minimize(int iResult)
	{
		// check if service menu
		if ((m_hWnd == m_hDestWnd) && (iResult > 0))
		{
			// service menu
			PostMessage(WM_COMMAND, (WPARAM)iResult);
			return;
		}
					
		ShowWindow(SW_HIDE);

		SetTrackPopupMenuExResult(iResult);

		SignalWaitEvent(EVT_FNGRMENU);

		if ((!(m_uFlags & TPM_RETURNCMD)) && (iResult > 0) && (m_hWnd != m_hDestWnd)) 
			::PostMessage(m_hDestWnd, WM_COMMAND, (WPARAM)iResult, (LPARAM)0);
	}

	
	HWND ManageDOTNETApp(HWND& hDestWnd)
	{
		HWND hDestCtrl = NULL;
		WCHAR szClass[50];
		::GetClassName(hDestWnd, szClass, 50);
		if (lstrcmpi(szClass, L"MS_SOFTKEY_CE_1.0") == 0)
		{
			DWORD dwProcessId; GetWindowThreadProcessId(hDestWnd, &dwProcessId);

			HWND hDotNETWnd = ::FindWindow(L"#NETCF_AGL_BASE_", NULL);
			if (hDotNETWnd != NULL)
			{
				DWORD dwProcess2Id; GetWindowThreadProcessId(hDotNETWnd, &dwProcess2Id);
				if (dwProcess2Id == dwProcessId)
				{
					hDestCtrl = hDestWnd;
					hDestWnd = hDotNETWnd;
				}
			}
		}
		return hDestCtrl;
	}
	

	BOOL IsExcludedApp(HWND hWnd)
	{
		DWORD dwProcessId; GetWindowThreadProcessId(hWnd, &dwProcessId);
		HMODULE hProcess = (HMODULE)dwProcessId;
		// execute GetCommandLine() on remote process
		WCHAR szName[MAX_PATH];
		if (GetModuleFileName(hProcess, szName, MAX_PATH))
		{
			CString szApp = (LPTSTR)szName;
			szApp = szApp.Right(szApp.GetLength() - szApp.ReverseFind('\\') - 1);
			for (int i = 0; i < m_excludedApps.GetSize(); i ++)
			{
				if (m_excludedApps[i].CompareNoCase( szApp ) == 0)
					return TRUE;
			}
		}

		return FALSE;
	}

	BOOL IsExcludedWnd(HWND hWnd)
	{
		WCHAR szTitle[MAX_PATH];
		::GetWindowText(hWnd, szTitle, MAX_PATH); 

		WCHAR szClassName[MAX_PATH];
		::GetClassName(hWnd, szClassName, MAX_PATH);

		WCHAR szName[MAX_PATH];
		wsprintf(szName, L"%s - %s", szTitle, szClassName);


		CString szWnd = (LPTSTR)szName;
		for (int i = 0; i < m_excludedWnds.GetSize(); i ++)
		{
			if (m_excludedWnds[i].CompareNoCase( szWnd ) == 0)
				return TRUE;
		}

		return FALSE;
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

	void LoadConfiguration()
	{
		WCHAR keyName[] = L"Software\\FingerMenu";
		// load transparency level
		m_iTransp = 128;
		if (!(RegReadDWORD(HKEY_LOCAL_MACHINE, keyName, L"TransparencyLevel", m_iTransp)))
			LOG(L"Unable to read TransparencyLevel from registry: %s\n", ErrorString(GetLastError()));

		m_bEnableSIP = TRUE;
		if (!(RegReadBOOL(HKEY_LOCAL_MACHINE, keyName, L"EnableSIP", m_bEnableSIP)))
			LOG(L"Unable to read EnableSIP from registry: %s\n", ErrorString(GetLastError()));

		m_menuView.m_iMaxVisibleItemCount = 0;
		if (!(RegReadDWORD(HKEY_LOCAL_MACHINE, keyName, L"MaxMenuItemCount", m_menuView.m_iMaxVisibleItemCount)))
			LOG(L"Unable to read MaxMenuItemCount from registry: %s\n", ErrorString(GetLastError()));
		if (m_menuView.m_iMaxVisibleItemCount == 0)
			m_menuView.m_iMaxVisibleItemCount = 1000;

		DWORD iMaxScrollVelocity = 15;
		if (!(RegReadDWORD(HKEY_LOCAL_MACHINE, keyName, L"MaxScrollVelocity", iMaxScrollVelocity)))
			LOG(L"Unable to read MaxScrollVelocity from registry: %s\n", ErrorString(GetLastError()));
		m_menuView.m_menu.SetMaxVelocity(iMaxScrollVelocity);


		m_bDisabledTransp = FALSE;
		if (!(RegReadBOOL(HKEY_LOCAL_MACHINE, keyName, L"DisableTransparency", m_bDisabledTransp)))
			LOG(L"Unable to read DisableTransparency from registry: %s\n", ErrorString(GetLastError()));

		m_clNoTranspBackground = RGB(0,0,0);
		RegReadColor(HKEY_LOCAL_MACHINE, keyName, L"NoTranspBkgColor", m_clNoTranspBackground);

		//m_bEnableDD = FALSE;
		//if (!(RegReadBOOL(HKEY_LOCAL_MACHINE, keyName, L"EnableDirectDraw", m_bEnableDD)))
		//	LOG(L"Unable to read EnableDirectDraw from registry: %s\n", ErrorString(GetLastError()));

		m_menuView.m_bNoButtons = FALSE;
		if (!(RegReadBOOL(HKEY_LOCAL_MACHINE, keyName, L"NeverShowButtons", m_menuView.m_bNoButtons)))
			LOG(L"Unable to read NeverShowButtons from registry: %s\n", ErrorString(GetLastError()));

		m_menuView.m_bEnableAnimation = TRUE;
		if (!(RegReadBOOL(HKEY_LOCAL_MACHINE, keyName, L"EnableAnimation", m_menuView.m_bEnableAnimation)))
			LOG(L"Unable to read EnableAnimation from registry: %s\n", ErrorString(GetLastError()));


		// load icon images
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

		switch (resolution)
		{
		case SQVGA:
		case QVGA:
		case WQVGA:
			m_menuView.SetScaleFactor(1);
			break;
		case VGA:
		case WVGA:
			m_menuView.SetScaleFactor(2);
			break;
		}		

		if ( (resolution == VGA) || (resolution == WVGA))
		{
			m_menuView.m_menu.m_imgSubmenuImage.Load(skinBasePath + L"\\submenu_vga.png", CXIMAGE_FORMAT_PNG);
			m_menuView.m_menu.m_imgCheckedImage.Load(skinBasePath + L"\\checked_vga.png", CXIMAGE_FORMAT_PNG);
			m_menuView.m_menu.m_imgRadioCheckedImage.Load(skinBasePath + L"\\radio_checked_vga.png", CXIMAGE_FORMAT_PNG);
			m_menuView.m_menu.m_imgRadioUncheckedImage.Load(skinBasePath + L"\\radio_unchecked_vga.png", CXIMAGE_FORMAT_PNG);
			m_menuView.m_menu.m_imgSelectionImage.Load(skinBasePath + L"\\selection_vga.png", CXIMAGE_FORMAT_PNG);
			m_menuView.m_menu.m_imgDPadCursorImage.Load(skinBasePath + L"\\dpad_cursor_vga.png", CXIMAGE_FORMAT_PNG);
			m_menuView.m_bmpBtnUp.Load(skinBasePath + L"\\btn_up_vga.png", CXIMAGE_FORMAT_PNG);
			m_menuView.m_bmpBtnUpPressed.Load(skinBasePath + L"\\btn_up_press_vga.png", CXIMAGE_FORMAT_PNG);
			m_menuView.m_bmpBtnUpDisabled.Load(skinBasePath + L"\\btn_up_disabled_vga.png", CXIMAGE_FORMAT_PNG);
			m_menuView.m_bmpBtnDown.Load(skinBasePath + L"\\btn_down_vga.png", CXIMAGE_FORMAT_PNG);
			m_menuView.m_bmpBtnDownPressed.Load(skinBasePath + L"\\btn_down_press_vga.png", CXIMAGE_FORMAT_PNG);
			m_menuView.m_bmpBtnDownDisabled.Load(skinBasePath + L"\\btn_down_disabled_vga.png", CXIMAGE_FORMAT_PNG);
			m_menuView.m_bmpBtnArrowDown.Load(skinBasePath + L"\\btn_arrow_down_vga.png", CXIMAGE_FORMAT_PNG);
			m_menuView.m_bmpBtnArrowUp.Load(skinBasePath + L"\\btn_arrow_up_vga.png", CXIMAGE_FORMAT_PNG);
		}
		else
		{
			m_menuView.m_menu.m_imgSubmenuImage.Load(skinBasePath + L"\\submenu_qvga.png", CXIMAGE_FORMAT_PNG);
			m_menuView.m_menu.m_imgCheckedImage.Load(skinBasePath + L"\\checked_qvga.png", CXIMAGE_FORMAT_PNG);
			m_menuView.m_menu.m_imgRadioCheckedImage.Load(skinBasePath + L"\\radio_checked_qvga.png", CXIMAGE_FORMAT_PNG);
			m_menuView.m_menu.m_imgRadioUncheckedImage.Load(skinBasePath + L"\\radio_unchecked_qvga.png", CXIMAGE_FORMAT_PNG);
			m_menuView.m_menu.m_imgSelectionImage.Load(skinBasePath + L"\\selection_qvga.png", CXIMAGE_FORMAT_PNG);
			m_menuView.m_menu.m_imgDPadCursorImage.Load(skinBasePath + L"\\dpad_cursor_qvga.png", CXIMAGE_FORMAT_PNG);
			m_menuView.m_bmpBtnUp.Load(skinBasePath + L"\\btn_up_qvga.png", CXIMAGE_FORMAT_PNG);
			m_menuView.m_bmpBtnUpPressed.Load(skinBasePath + L"\\btn_up_press_qvga.png", CXIMAGE_FORMAT_PNG);
			m_menuView.m_bmpBtnUpDisabled.Load(skinBasePath + L"\\btn_up_disabled_qvga.png", CXIMAGE_FORMAT_PNG);
			m_menuView.m_bmpBtnDown.Load(skinBasePath + L"\\btn_down_qvga.png", CXIMAGE_FORMAT_PNG);
			m_menuView.m_bmpBtnDownPressed.Load(skinBasePath + L"\\btn_down_press_qvga.png", CXIMAGE_FORMAT_PNG);
			m_menuView.m_bmpBtnDownDisabled.Load(skinBasePath + L"\\btn_down_disabled_qvga.png", CXIMAGE_FORMAT_PNG);
			m_menuView.m_bmpBtnArrowDown.Load(skinBasePath + L"\\btn_arrow_down_qvga.png", CXIMAGE_FORMAT_PNG);
			m_menuView.m_bmpBtnArrowUp.Load(skinBasePath + L"\\btn_arrow_up_qvga.png", CXIMAGE_FORMAT_PNG);
		}
	                
		m_menuView.SetItemHeight(m_menuView.m_menu.m_imgSelectionImage.GetHeight());
		m_menuView.SetButtonHeight(m_menuView.m_bmpBtnUp.GetHeight());


		// load skin settings
		CSimpleIniW ini(TRUE, TRUE, TRUE);
		SI_Error rc = ini.LoadFile(skinBasePath + L"\\settings_fingermenu.ini");

		// load background and selected color
	
		COLORREF clValue = 0;
		m_menuView.m_menu.m_clBkg          = (StringRGBToColor(ini.GetValue(L"colors", L"BkgColor"), clValue)) ? clValue : RGB(255,255,255);
		m_menuView.m_menu.m_clText         = (StringRGBToColor(ini.GetValue(L"colors", L"TextColor"), clValue)) ? clValue : RGB(0,0,0);
		m_menuView.m_menu.m_clSelText      = (StringRGBToColor(ini.GetValue(L"colors", L"SelTextColor"), clValue)) ? clValue : RGB(255,255,255);
		m_menuView.m_menu.m_clDisabledText = (StringRGBToColor(ini.GetValue(L"colors", L"DisabledTextColor"), clValue)) ? clValue : RGB(160,160,160);
		m_menuView.m_menu.m_clLine         = (StringRGBToColor(ini.GetValue(L"colors", L"LineColor"), clValue)) ? clValue : RGB(160,160,160);
		m_menuView.m_menu.m_clScrollbar    = (StringRGBToColor(ini.GetValue(L"colors", L"ScrollbarColor"), clValue)) ? clValue : RGB(160,160,160);

		// font
		if (!(m_menuView.m_menu.m_fText.IsNull())) m_menuView.m_menu.m_fText.DeleteObject();

		LOGFONT lf;
		if (StringToLogFont(ini.GetValue(L"fonts", L"TextFont"), lf))
		{
			m_menuView.m_menu.m_fText.CreateFontIndirect(&lf);
		}

		// list of excluded apps
		LoadExclusionList(m_excludedApps, L"Software\\FingerMenu");

		LoadWndExclusionList(m_excludedWnds, L"Software\\FingerMenu");
	}

	int Back()
	{
		if ((m_menus.GetSize() - 2) >= 0) 
		{
			int idx = -1;
			HMENU hCurrMenu = m_menuItems[0].hOwnerMenu;
			for (int i = 0; i < m_menus.GetSize(); i++)
			{
				if (m_menus[i].hMenu == hCurrMenu)
				{
					idx = i;
					break;
				}
			}

			if (idx >= 0)
			{
				if (m_menus[idx].hPrevMenu != NULL)
				{
					SetNewMenu(m_menus[idx].hPrevMenu);
					if (idx == 1)
						RestoreMenuBar();
					return idx;
				}
				else
				{
					PostMessage(WM_COMMAND, ID_MENU_CANCEL, 0);
					return 0;
				}
			}
		}
		else
		{
			PostMessage(WM_COMMAND, ID_MENU_CANCEL, 0);
			return 0;
		}
		
		m_menuView.m_menu.m_idxSelected = -1;
		return 0;
	}

	BOOL SetNewMenu(HMENU hMenu)
	{
		// menu
		LPMENU pMu = NULL;
		for (int i = 0; i < m_menus.GetSize(); i++)
		{
			if (m_menus[i].hMenu == hMenu)
			{
				pMu = &m_menus[i];
				break;
			}
		}
		if (pMu == NULL)
		{
			MENU m;
			m.hMenu = hMenu;
			m.bInitMenuSent = FALSE;
			m.hPrevMenu = NULL;
			m_menus.Add(m);

			pMu = &m_menus[m_menus.GetSize() - 1];
		}

		if ((hMenu == NULL) || (hMenu == 0))
		{
			m_menuItems.RemoveAll();
			return FALSE;
		}

		// reset 
		m_menuView.m_menu.m_idxSelected = -1;
		m_menuView.m_menu.m_idxDPadSelected = -1;

		m_menuItems.RemoveAll();


		// let other app initialize menu
		if (!pMu->bInitMenuSent)
		{
			pMu->bInitMenuSent = TRUE;
			DWORD dwResult;
			SendMessageTimeout(m_hDestWnd, WM_INITMENUPOPUP, (WPARAM)hMenu, 0, SMTO_NORMAL, 500, &dwResult);
			SendMessageTimeout(m_hDestWnd, WM_ENTERMENULOOP, (WPARAM)1, 0, SMTO_NORMAL, 500, &dwResult);
		}

		CMenuHandle menuPopup(hMenu);
		int i = 0;
		int nBlankCount = 0;
		while (1)
		{
			MENUINFO mi;
			ZeroMemory(&mi, sizeof(MENUINFO));
			mi.text[0] = ' ';
			mi.text[1] = ' ';
			mi.hOwnerMenu = hMenu;
			mi.mii.cbSize = sizeof(MENUITEMINFO);
			mi.mii.fMask = MIIM_CHECKMARKS | MIIM_DATA | MIIM_ID | MIIM_STATE | MIIM_SUBMENU | MIIM_TYPE | MIIM_FULLSTR;
			mi.mii.dwTypeData = (LPTSTR)&mi.text[2];
			mi.mii.cch = MENU_ITEM_TEXT_SIZE; //sizeof(mi.text);

			if (menuPopup.GetMenuItemInfo(i, TRUE, &mi.mii))
			{
				//DUMP_MENUITEMINFO(&mi.mii);

				if (mi.mii.fType & MFT_SEPARATOR)
				{
					if ((m_menuItems.GetSize() - 1) >= 0)
						m_menuItems[m_menuItems.GetSize() - 1].bFollowSeparator = TRUE;
				}
				else
				{
					// modifiche al campo text
					int j = 0;
					WCHAR c;
					while ((c = mi.text[j]) != L'\0')
					{
						if (mi.text[j] == L'\t')
							mi.text[j] = L' ';
						j++;
					}

					mi.hDestWnd = m_hDestWnd;
					mi.fPosition = i;

					if (mi.mii.dwTypeData == NULL)
						nBlankCount ++;

					if (mi.mii.fType & MFT_OWNERDRAW)
					{
						MEASUREITEMSTRUCT mis;
						ZeroMemory(&mis, sizeof(MEASUREITEMSTRUCT));
						mis.CtlType = ODT_MENU;
						mis.itemData = i; //mi.mii.dwItemData;

						// TODO
						//SendMessageRemote(mi.hDestWnd, WM_MEASUREITEM, 0, (LPARAM)&mis, sizeof(MEASUREITEMSTRUCT));
						/*
						RECT rc; GetClientRect(&rc);
						mis.itemWidth = rc.right - rc.left;
						mis.itemHeight = m_itemHeight; 

						LOG(L"SONO QUI\n");

						DRAWITEMSTRUCT dis;
						ZeroMemory(&dis, sizeof(DRAWITEMSTRUCT));

						CDC dc = ::GetDC(NULL);
						CDC dcTemp; dcTemp.CreateCompatibleDC(dc);
						CBitmap bmp; bmp.CreateCompatibleBitmap(dcTemp, mis.itemWidth, mis.itemHeight);
						CBitmap bmpOld = dcTemp.SelectBitmap(bmp);

						dcTemp.MoveTo(0,0);
						dcTemp.LineTo(mis.itemWidth, mis.itemHeight);

						dis.CtlType = ODT_MENU;
						dis.itemID = mi.mii.wID;
						dis.itemAction = ODA_DRAWENTIRE;
						//dis.itemState = ODS_SELECTED; // TODO
						dis.hwndItem = (HWND)mi.hOwnerMenu;
						dis.hDC = dcTemp;
						SetRect(&dis.rcItem, 0, 0, mis.itemWidth, mis.itemHeight);
						dis.itemData = mi.mii.dwItemData;

						SendMessageRemote(mi.hDestWnd, WM_DRAWITEM, 0, (LPARAM)&dis, sizeof(DRAWITEMSTRUCT));

						dcTemp.SelectBitmap(bmpOld);

						mi.imgOwnerDraw.CreateFromHBITMAP(bmp);
						::ReleaseDC(NULL, dc);

						CString fName; fName.Format(L"\\test%d.png", i);
						mi.imgOwnerDraw.Save(fName, CXIMAGE_FORMAT_PNG);

						LOG(L"SONO QUI 2\n");
						*/
					}

					// save 
					m_menuItems.Add(mi);
				}
			}
			else
			{
				//LOG(L"Error in SetMenu: %s\n", ErrorString(GetLastError()));
				break;
			}
			i ++; 
		}


		/*
		if ((nBlankCount != 0) &&
		    (nBlankCount >= (m_menuItems.GetSize() - 1))
			)
			return FALSE;
		*/

		// gestione radio button
		for (int i = 0; i < m_menuItems.GetSize(); i++)
		{
			if (m_menuItems[i].mii.fType & MFT_RADIOCHECK)
			{
				// back
				for (int k = i; k >= 0; k--)
				{
					if (!(m_menuItems[k].bFollowSeparator))
						m_menuItems[k].mii.fType |= MFT_RADIOCHECK;
					else
						break;
				}
				// forward
				for (int k = i; k < m_menuItems.GetSize(); k++)
				{
					if (!(m_menuItems[k].bFollowSeparator))
						m_menuItems[k].mii.fType |= MFT_RADIOCHECK;
					else
						break;
				}
				break;
			}
		}

		m_menuView.Show();

		return TRUE;
	}


public:
	static HRESULT ActivatePreviousInstance(HINSTANCE hInstance)
	{
		const TCHAR* pszMutex = L"FINGERMENUMUTEX";
		const TCHAR* pszClass = L"FINGER_MENU";

		const DWORD dRetryInterval = 100; 

		const int iMaxRetries = 25;

		for(int i = 0; i < iMaxRetries; ++i)
		{
			// Don't need ownership of the mutex
			HANDLE hMutex = CreateMutex(NULL, FALSE, pszMutex);

			DWORD dw = GetLastError();

			if(hMutex == NULL)
			{
				// ERROR_INVALID_HANDLE - A non-mutex object with this name already exists.
				HRESULT hr = (dw == ERROR_INVALID_HANDLE) ? E_INVALIDARG : E_FAIL;
				return hr;
			}

			// If the mutex already exists, then there should be another instance running
			if(dw == ERROR_ALREADY_EXISTS)
			{
				// Just needed the error result, in this case, so close the handle.
				CloseHandle(hMutex);

				// Try to find the other instance, don't need to close HWND's.
				// Don't check title in case it is changed by app after init.
				HWND hwnd = FindWindow(pszClass, NULL);

				if(hwnd == NULL)
				{
					// It's possible that the other istance is in the process of starting up or shutting down.
					// So wait a bit and try again.
					Sleep(dRetryInterval);
					continue;
				}
				else
				{
					// Set the previous instance as the foreground window

					// The "| 0x1" in the code below activates the correct owned window 
					// of the previous instance's main window according to the SmartPhone 2003
					// wizard generated code.
					if(SetForegroundWindow(reinterpret_cast<HWND>(reinterpret_cast<ULONG>(hwnd) | 0x1)) != 0)
					{
						// S_FALSE indicates that another instance was activated, so this instance should terminate.
						return S_FALSE;
					}
				}
			}
			else
			{
				// This is the first istance, so return S_OK.
				// Don't close the mutext handle here.
				// Do it on app shutdown instead.
				return S_OK;
			}
		}

		// The mutex was created by another instance, but it's window couldn't be brought
		// to the foreground, so ssume  it's not a invalid instance (not this app, hung, etc.)
		// and let this one start.
		return S_OK;
	}

	void CreateConfigMenu()
	{
		m_menuConfig.CreatePopupMenu();
		m_menuConfig.AppendMenuW(MF_STRING, ID_MENU_ADDTOEXCLUSIONLIST, LoadResourceString(IDS_MENU_ADDTOEXCLUSIONLIST)); // L"Add to Exclusion list");
		m_menuConfig.AppendMenuW(MF_STRING, ID_MENU_ADDTOWNDEXCLUSIONLIST, LoadResourceString(IDS_MENU_ADDTOWNDEXCLUSIONLIST)); // L"Add to Exclusion list");
		m_menuConfig.AppendMenuW(MF_STRING, ID_MENU_SHOWORIGINAL, LoadResourceString(IDS_MENU_SHOWORIGINAL));
		m_menuConfig.AppendMenuW(MF_STRING, ID_MENU_ABOUT, LoadResourceString(IDS_MENU_ABOUT));
		m_menuConfig.AppendMenuW(MF_STRING, ID_MENU_EXIT, LoadResourceString(IDS_MENU_EXIT));
	}

	void SwitchToMenuView()
	{
		m_hWndClient = m_menuView;

		m_aboutView.ShowWindow(SW_HIDE);
		m_aboutView.SetWindowLongPtr(GWL_ID, 0);

		m_menuView.ShowWindow(SW_SHOW);
		m_menuView.SetWindowLongPtr(GWL_ID, ATL_IDW_PANE_FIRST);
		m_menuView.SetFocus();

	}

	void SwitchToAboutView()
	{
		m_hWndClient = m_aboutView;

		m_menuView.ShowWindow(SW_HIDE);
		m_menuView.SetWindowLongPtr(GWL_ID, 0);

		m_aboutView.ShowWindow(SW_SHOW);
		m_aboutView.SetWindowLongPtr(GWL_ID, ATL_IDW_PANE_FIRST);
		m_aboutView.SetFocus();
		UpdateLayout();
	}





	void ManageSIP()
	{

	}

protected:
	CxImage m_imgBkg;

	BOOL m_bDisabledTransp;
	COLORREF m_clNoTranspBackground;
	DWORD m_iTransp;
	BOOL m_bEnableSIP;
	BOOL m_bEnableDD;
	CMenu m_menuConfig;
	CSimpleArray<CString, CStringEqualHelper<CString>> m_excludedApps;
	CSimpleArray<CString, CStringEqualHelper<CString>> m_excludedWnds;
	DWORD m_iDisableInterval;
	
	// back gesture recognition
	int m_xPos;
	int m_backGestureThreshold;
	BOOL m_bMouseIsDown;


	HWND m_hDestWnd;
	HWND m_hPrevDestWnd;
	HWND m_hDestCtrl;
	BOOL m_bSendCmd;
	UINT m_uFlags;

};




// MainFrm.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "sharedmem.h"
#include "InterceptEngine.h"
#include "..\Common\Utils.h"
#include "..\Common\log\logger.h"
#include "..\Common\AboutView.h"
#include "..\Common\fngrbtn.h"
#include "..\FingerSuiteCPL\Commons.h"
#include "MsgBoxWindow.h"
#include "..\Common\fngrsplash.h"

#define IDT_TMR_DISABLED 10012
#define MASK_EVENTMUTE   0x0004

typedef enum {QVGA = 320, WQVGA = 400, VGA = 640, WVGA = 800} RESOLUTION;

static UINT UWM_INTERCEPT_MSGBOX = ::RegisterWindowMessage(UWM_INTERCEPT_MSGBOX_MSG);

static UINT UWM_UPDATECONFIGURATION = ::RegisterWindowMessage(UWM_UPDATECONFIGURATION_MSG);



class CMainFrame : public CFrameWindowImpl<CMainFrame>, public CUpdateUI<CMainFrame>,
		public CMessageFilter, public CIdleHandler
{
public:
	DECLARE_FRAME_WND_CLASS(L"FINGER_MSGBOX", IDR_MAINFRAME)

	CMsgBoxWindow    m_msgBoxView;
	CAboutView       m_aboutView;


	virtual BOOL PreTranslateMessage(MSG* pMsg)
	{
		if(CFrameWindowImpl<CMainFrame>::PreTranslateMessage(pMsg))
			return TRUE;

		return m_msgBoxView.PreTranslateMessage(pMsg);
	}

	virtual BOOL OnIdle()
	{
		return FALSE;
	}

	void DoPaint(CDCHandle dc)
	{
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
	}

	BEGIN_UPDATE_UI_MAP(CMainFrame)
	END_UPDATE_UI_MAP()

	BEGIN_MSG_MAP(CMainFrame)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER(WM_ACTIVATE, OnActivate)
		MESSAGE_HANDLER(WM_WININICHANGE, OnWininichange)
		MESSAGE_HANDLER(UM_MINIMIZE, OnMinimize)
		MESSAGE_HANDLER(UWM_INTERCEPT_MSGBOX, OnInterceptMsgBox)
		MESSAGE_HANDLER(UWM_UPDATECONFIGURATION, OnUpdateConfiguration)
		COMMAND_ID_HANDLER(ID_MENU_CANCEL, OnCancel)
		COMMAND_ID_HANDLER(ID_MENU_EXIT, OnMenuExit)
		COMMAND_ID_HANDLER(ID_MENU_ABOUT, OnMenuAbout)
		COMMAND_ID_HANDLER(ID_MENU_SHOWORIGINAL, OnMenuShowOriginal)
		COMMAND_ID_HANDLER(ID_MENU_ADDTOEXCLUSIONLIST, OnMenuAddToExclusionList)
		COMMAND_ID_HANDLER(ID_MENU_ADDTOWNDEXCLUSIONLIST, OnMenuAddToWndExclusionList)
		CHAIN_MSG_MAP(CUpdateUI<CMainFrame>)
		CHAIN_MSG_MAP(CFrameWindowImpl<CMainFrame>)
	END_MSG_MAP()


	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		CreateSimpleCEMenuBar(FM_IDW_MENU_BAR, SHCMBF_HIDESIPBUTTON);

		CSplashWindow* pSplash = new CSplashWindow(m_hWnd, IDR_MAINFRAME, L"FingerMsgbox");

		LoadConfiguration();

		// msgbox
		m_hWndClient = m_msgBoxView.Create(m_hWnd);

		// about
		CString strCredits = "\n\n"
			    "\tFingerMsgBox v1.01\n\n"
				"\rProgrammed by:\n"
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

		SetHWndServerMsgBox(m_hWnd);

		InstallHook();

		SignalWaitEvent(EVT_FNGRMSGBOX);

		ModifyStyle(0, WS_NONAVDONEBUTTON, SWP_NOSIZE);

		m_bDisabled = FALSE;

		pSplash->Dismiss();

		return 0;  
	}

	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		return 0;
	}

	LRESULT OnSize(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
	{
		if(wParam != SIZE_MINIMIZED)
		{

		}
			
		return 0;
	}


	LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		RemoveHook();

		if (!(m_fText.IsNull()))
		{
			m_fText.DeleteObject();
			m_fText = NULL;
		}

		if (!(m_fCaption.IsNull()))
		{
			m_fCaption.DeleteObject();
			m_fCaption = NULL;
		}

		if (!(m_fBtn.IsNull()))
		{
			m_fBtn.DeleteObject();
			m_fBtn = NULL;
		}

		bHandled = FALSE;
		return 0;
	}

	LRESULT OnEraseBkgnd(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
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
			CPaintDC dc(m_hWnd);
			DoPaint(dc.m_hDC);
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
				SetMsgBoxResult(IDCANCEL);
				Minimize();
			}
		}
		return 0;
	}

	LRESULT OnWininichange(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		if (wParam == SETTINGCHANGE_RESET)
		{
			//SetMsgBoxResult(IDCANCEL);
			//Minimize();

			m_msgBoxView.ModifyShape();
			m_msgBoxView.ModifyButtons();
		}
		return 0;
	}

	LRESULT OnMinimize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		Minimize();
		return 0;
	}

	LRESULT OnCancel(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		SetMsgBoxResult(IDCANCEL);
		Minimize();

		return 0;
	}

	LRESULT OnInterceptMsgBox(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled)
	{
		LPMSGBOXINFO lpInfo = (LPMSGBOXINFO)lParam;

		HWND hDestWnd = lpInfo->hWnd;
		UINT uType = lpInfo->uType;

		LOG(L"Intercepting MessageBoxW...uType=%08x hWnd=%08X\n", uType, hDestWnd);

		if (m_bDisabled)
			return 1;

		if (IsDeviceLocked())
			return 1;

		m_hForeWnd = ::GetForegroundWindow();

		if (IsExcludedApp(hDestWnd))
			return 1;

		if (IsExcludedWnd(hDestWnd))
			return 1;

		if (!(m_msgBoxView.SetMsgBox(hDestWnd, uType, lpInfo->szCaption, lpInfo->szText)))
			return 1;
		
		Sleep(100);

		SwitchToMsgBoxView();

		SetForegroundWindow((HWND)((ULONG) m_hWnd | 0x00000001));
		ShowWindow(SW_SHOW);		
		
		PlayMessageBoxSound(uType);

		bHandled = TRUE;
		return 0;
	}

	LRESULT OnMenuExit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		SetMsgBoxResult(IDCANCEL);
		Minimize();

		PostMessage(WM_CLOSE);
		return 0;
	}


	LRESULT OnMenuAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		SwitchToAboutView();
		SetForegroundWindow((HWND)((ULONG) m_hWnd | 0x00000001));
		ShowWindow(SW_SHOW);	
		return 0;
	}		

	LRESULT OnMenuShowOriginal(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		SetMsgBoxResult(-1);
		Minimize();
		
		return 0;
	}

	LRESULT OnMenuAddToExclusionList(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		HWND hDestWnd = m_msgBoxView.GetOwnerWnd();
		DWORD dwProcessId; GetWindowThreadProcessId(hDestWnd, &dwProcessId);
		HMODULE hProcess = (HMODULE)dwProcessId;
		WCHAR szName[MAX_PATH];
		if (GetModuleFileName(hProcess, szName, MAX_PATH))
		{
			CString szApp = (LPTSTR)szName;
			szApp = szApp.Right(szApp.GetLength() - szApp.ReverseFind('\\') - 1);
			
			if (m_excludedApps.Find(szApp) == -1)
				m_excludedApps.Add(szApp);
			
			SaveExclusionList(m_excludedApps, L"Software\\FingerMsgbox");
		}

		SetMsgBoxResult(-1);
		Minimize();

		return 0;
	}

	LRESULT OnMenuAddToWndExclusionList(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		HWND hDestWnd = m_msgBoxView.GetOwnerWnd();

		WCHAR szTitle[MAX_PATH];
		::GetWindowText(hDestWnd, szTitle, MAX_PATH); 

		WCHAR szClassName[MAX_PATH];
		::GetClassName(hDestWnd, szClassName, MAX_PATH);

		WCHAR szName[MAX_PATH];
		wsprintf(szName, L"%s - %s", szTitle, szClassName);
		
		CString szWnd = (LPTSTR)szName;
		if (m_excludedWnds.Find(szWnd) == -1)
			m_excludedWnds.Add(szWnd);

		SaveWndExclusionList(m_excludedWnds, L"Software\\FingerMsgbox");

		SetMsgBoxResult(-1);
		Minimize();

		return 0;
	}

	LRESULT OnUpdateConfiguration(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		LoadConfiguration();

		SetMsgBoxResult(FALSE);
		Minimize();
		return 0;
	}


private:

	void Minimize()
	{
		ShowWindow(SW_HIDE);

		SignalWaitEvent(EVT_FNGRMSGBOX);
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
		WCHAR keyName[] = L"Software\\FingerMsgBox";

		// load transparency level
		m_iTransp = 128;
		if (!(RegReadDWORD(HKEY_LOCAL_MACHINE, keyName, L"TransparencyLevel", m_iTransp)))
			wprintf(L"Unable to read TransparencyLevel from registry: %s\n", ErrorString(GetLastError()));
		m_msgBoxView.m_iTransp = m_iTransp;

		m_bDisabledTransp = FALSE;
		if (!(RegReadBOOL(HKEY_LOCAL_MACHINE, keyName, L"DisableTransparency", m_bDisabledTransp)))
			wprintf(L"Unable to read DisableTransparency from registry: %s\n", ErrorString(GetLastError()));
		m_msgBoxView.m_bDisabledTransp = m_bDisabledTransp;

		m_clNoTranspBackground = RGB(0,0,0);
		RegReadColor(HKEY_LOCAL_MACHINE, keyName, L"NoTranspBkgColor", m_clNoTranspBackground);
		m_msgBoxView.m_clNoTranspBackground = m_clNoTranspBackground;

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
			m_msgBoxView.SetScaleFactor(1);
			break;
		case VGA:
		case WVGA:
			m_msgBoxView.SetScaleFactor(2);			
			break;
		}		

		if ( (resolution == VGA) || (resolution == WVGA))
		{
			m_msgBoxView.m_imgHeader.Load(skinBasePath + L"\\msgbox_header_vga.png", CXIMAGE_FORMAT_PNG);
			m_msgBoxView.m_icnInfo.Load(skinBasePath + L"\\msgbox_iconinfo_vga.png", CXIMAGE_FORMAT_PNG);
			m_msgBoxView.m_icnQuestion.Load(skinBasePath + L"\\msgbox_iconquestion_vga.png", CXIMAGE_FORMAT_PNG);
			m_msgBoxView.m_icnStop.Load(skinBasePath + L"\\msgbox_iconstop_vga.png", CXIMAGE_FORMAT_PNG);
			m_msgBoxView.m_icnWarning.Load(skinBasePath + L"\\msgbox_iconwarning_vga.png", CXIMAGE_FORMAT_PNG);

			// normal
			AddDynamicSet(m_msgBoxView.m_arrDynamicImages, NORMAL, skinBasePath + L"\\msgbox_button_normal_left_vga.png", 
				           skinBasePath + L"\\msgbox_button_normal_center_vga.png",
						   skinBasePath + L"\\msgbox_button_normal_right_vga.png");

			// pressed
			AddDynamicSet(m_msgBoxView.m_arrDynamicImages, PUSHED, skinBasePath + L"\\msgbox_button_pressed_left_vga.png", 
				           skinBasePath + L"\\msgbox_button_pressed_center_vga.png",
						   skinBasePath + L"\\msgbox_button_pressed_right_vga.png");

		}
		else
		{
			m_msgBoxView.m_imgHeader.Load(skinBasePath + L"\\msgbox_header_qvga.png", CXIMAGE_FORMAT_PNG);
			m_msgBoxView.m_icnInfo.Load(skinBasePath + L"\\msgbox_iconinfo_qvga.png", CXIMAGE_FORMAT_PNG);
			m_msgBoxView.m_icnQuestion.Load(skinBasePath + L"\\msgbox_iconquestion_qvga.png", CXIMAGE_FORMAT_PNG);
			m_msgBoxView.m_icnStop.Load(skinBasePath + L"\\msgbox_iconstop_qvga.png", CXIMAGE_FORMAT_PNG);
			m_msgBoxView.m_icnWarning.Load(skinBasePath + L"\\msgbox_iconwarning_qvga.png", CXIMAGE_FORMAT_PNG);

			// normal
			AddDynamicSet(m_msgBoxView.m_arrDynamicImages, NORMAL, skinBasePath + L"\\msgbox_button_normal_left_qvga.png", 
				           skinBasePath + L"\\msgbox_button_normal_center_qvga.png",
						   skinBasePath + L"\\msgbox_button_normal_right_qvga.png");

			// pressed
			AddDynamicSet(m_msgBoxView.m_arrDynamicImages, PUSHED, skinBasePath + L"\\msgbox_button_pressed_left_qvga.png", 
				           skinBasePath + L"\\msgbox_button_pressed_center_qvga.png",
						   skinBasePath + L"\\msgbox_button_pressed_right_qvga.png");
		}

		// load skin settings
		CSimpleIniW ini(TRUE, TRUE, TRUE);
		SI_Error rc = ini.LoadFile(skinBasePath + L"\\settings_fingermsgbox.ini");

		// load background and selected color
	
		COLORREF clValue = 0;
		m_msgBoxView.m_clBkg         = (StringRGBToColor(ini.GetValue(L"colors", L"BkgColor"), clValue)) ? clValue : RGB(255,255,255);
		m_msgBoxView.m_clCaptionText = (StringRGBToColor(ini.GetValue(L"colors", L"CaptionTextColor"), clValue)) ? clValue : RGB(255,255,255);
		m_msgBoxView.m_clText        = (StringRGBToColor(ini.GetValue(L"colors", L"TextColor"), clValue)) ? clValue : RGB(0,0,0);
		m_msgBoxView.m_clBtnText     = (StringRGBToColor(ini.GetValue(L"colors", L"ButtonTextColor"), clValue)) ? clValue : RGB(0,0,0);
		m_msgBoxView.m_clBtnSelText  = (StringRGBToColor(ini.GetValue(L"colors", L"SelButtonTextColor"), clValue)) ? clValue : RGB(255,255,255);
		m_msgBoxView.m_clLine        = (StringRGBToColor(ini.GetValue(L"colors", L"LineColor"), clValue)) ? clValue : RGB(160,160,160);


		if (!(m_fText.IsNull())) m_fText.DeleteObject();
		if (!(m_fCaption.IsNull())) m_fCaption.DeleteObject();
		if (!(m_fBtn.IsNull())) m_fBtn.DeleteObject();

		LOGFONT lf;
		if (StringToLogFont(ini.GetValue(L"fonts", L"TextFont"), lf))
		{
			m_fText.CreateFontIndirect(&lf);
		}
		if (StringToLogFont(ini.GetValue(L"fonts", L"CaptionFont"), lf))
		{
			m_fCaption.CreateFontIndirect(&lf);
		}
		if (StringToLogFont(ini.GetValue(L"fonts", L"ButtonFont"), lf))
		{
			m_fBtn.CreateFontIndirect(&lf);
		}

		m_msgBoxView.m_fText = m_fText;
		m_msgBoxView.m_fCaption = m_fCaption;
		m_msgBoxView.m_fBtn = m_fBtn;

		//sounds
		m_szDefaultSound   = ini.GetValue(L"sounds", L"DefaultSound"  );
		m_szErrorSound     = ini.GetValue(L"sounds", L"ErrorSound"    );
		m_szWarningSound   = ini.GetValue(L"sounds", L"WarningSound"  );
		m_szInfoSound      = ini.GetValue(L"sounds", L"InfoSound"     );
		m_szQuestionSound  = ini.GetValue(L"sounds", L"QuestionSound" );


		//general options
		LPCWSTR lpwszCenterCaption = ini.GetValue(L"general", L"CenterCaption");
		if (lstrcmpi(lpwszCenterCaption, L"true") == 0)
			m_msgBoxView.m_bCenterCaption = TRUE;
		else
			m_msgBoxView.m_bCenterCaption = FALSE;

		// list of excluded apps
		LoadExclusionList(m_excludedApps, L"Software\\FingerMsgbox");

		LoadWndExclusionList(m_excludedWnds, L"Software\\FingerMsgbox");
	}

public:
	static HRESULT ActivatePreviousInstance(HINSTANCE hInstance)
	{
		const TCHAR* pszMutex = L"FINGERMSGBOXMUTEX";
		const TCHAR* pszClass = L"FINGER_MSGBOX";

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

	void SwitchToMsgBoxView()
	{
		m_hWndClient = m_msgBoxView;

		m_aboutView.ShowWindow(SW_HIDE);
		m_aboutView.SetWindowLongPtr(GWL_ID, 0);

		m_msgBoxView.ShowWindow(SW_SHOW);
		m_msgBoxView.SetWindowLongPtr(GWL_ID, ATL_IDW_PANE_FIRST);
		m_msgBoxView.SetFocus();
	}

	void SwitchToAboutView()
	{
		m_hWndClient = m_aboutView;

		m_msgBoxView.ShowWindow(SW_HIDE);
		m_msgBoxView.SetWindowLongPtr(GWL_ID, 0);

		m_aboutView.ShowWindow(SW_SHOW);
		m_aboutView.SetWindowLongPtr(GWL_ID, ATL_IDW_PANE_FIRST);
		m_aboutView.SetFocus();
		UpdateLayout();
	}

	void PlayMessageBoxSound(int uType)
	{
		if (!(IsEvtSoundActive()))
			return;

		CString fileName;
		if ((uType & MB_ICONEXCLAMATION) || (uType & MB_ICONWARNING))
		{
			fileName = m_szWarningSound;
			goto soundend;
		}

		if ((uType & MB_ICONINFORMATION) || (uType & MB_ICONASTERISK))
		{
			fileName = m_szInfoSound;
			goto soundend;
		}

		if (uType & MB_ICONQUESTION)
		{
			fileName = m_szQuestionSound;
			goto soundend;
		}

		if ((uType & MB_ICONSTOP) || (uType & MB_ICONERROR) || (uType & MB_ICONHAND))
		{
			fileName = m_szErrorSound;
			goto soundend;
		}
soundend:
		if (fileName.IsEmpty())
			fileName = m_szDefaultSound;
		PlaySound(fileName, NULL, SND_ASYNC | SND_FILENAME | SND_NODEFAULT);
	}


protected:
	HWND m_hForeWnd;
	CxImage m_imgBkg;

	BOOL m_bDisabledTransp;
	COLORREF m_clNoTranspBackground;
	DWORD m_iTransp;
	CSimpleArray<CString, CStringEqualHelper<CString>> m_excludedApps;
	CSimpleArray<CString, CStringEqualHelper<CString>> m_excludedWnds;
	BOOL m_bDisabled;
	CMenu m_menuConfig;
	CFontHandle m_fText;
	CFontHandle m_fCaption;
	CFontHandle m_fBtn;

	CString m_szDefaultSound;
	CString m_szErrorSound;
	CString m_szWarningSound;
	CString m_szInfoSound;
	CString m_szQuestionSound;

private:

	BOOL IsEvtSoundActive()
	{
		BOOL bRes = TRUE;
	    DWORD dwMute = 0;
		HRESULT hr = RegistryGetDWORD(HKEY_CURRENT_USER, L"ControlPanel\\Volume", L"Mute", &dwMute);
		if (SUCCEEDED(hr))
		{
			if ( ! (dwMute & MASK_EVENTMUTE) )
				bRes = FALSE;
		}

		return bRes;
	}
};




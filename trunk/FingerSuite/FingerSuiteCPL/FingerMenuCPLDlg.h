// FingerMenuCONFIGView.h : interface of the CFingerMenuCONFIGView class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "..\Common\Utils.h"
#include <Tlhelp32.h>

#define IDT_TMR_CTRL 10001
#define TMR_CTRL_INTERVAL 1000

class CColorViewImpl : public CWindowImpl<CColorViewImpl, CStatic>
{
public:

	CColorViewImpl ()
	{
		m_color = RGB(255, 255, 255);
	}

	void SetColor(COLORREF color)
	{
		m_color = color;
		InvalidateRect(NULL);
		UpdateWindow();
	}

	BEGIN_MSG_MAP(CColorViewImpl)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
	END_MSG_MAP()
 
	LRESULT OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		RECT rc; GetClientRect(&rc);
		CPaintDC dc(m_hWnd);
		dc.FillSolidRect(&rc, m_color);
		return 0;
	}

public:
	COLORREF m_color;
};

/********************************************************
 PAGE1
 ********************************************************/

class CFingerMenuCPLDlg1 :    public CPropertyPageImpl<CFingerMenuCPLDlg1>,
							  public CWinDataExchange<CFingerMenuCPLDlg1>,
							  public CUpdateUI<CFingerMenuCPLDlg1>
{
public:
	enum { IDD = IDD_PAGE1 };

	BOOL PreTranslateMessage(MSG* pMsg)
	{
		return CWindow::IsDialogMessage(pMsg);
	}

	BOOL OnIdle()
	{
		UIUpdateChildWindows();
		return FALSE;
	}

	void DoPaint(CDCHandle dc) {}


	BEGIN_MSG_MAP_EX(CFingerMenuCPLDlg1)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		CHAIN_MSG_MAP(CPropertyPageImpl<CFingerMenuCPLDlg1>)
	END_MSG_MAP()

    BEGIN_DDX_MAP(CFingerMenuCPLDlg1)
		DDX_CHECK(IDC_AUTOSTART_MENU, m_bAutostartMenu)
		DDX_CHECK(IDC_AUTOSTART_MSGBOX, m_bAutostartMsgbox)
    END_DDX_MAP()

	BEGIN_UPDATE_UI_MAP(CFingerMenuCPLDlg1)
    END_UPDATE_UI_MAP()

	int OnApply()
	{
		DoDataExchange(TRUE);
		SaveConfiguration();
		return PSNRET_NOERROR;
	}

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		UIAddChildWindowContainer(m_hWnd);

		// get startup folder
		WCHAR szValue[MAX_PATH];
		if (SHGetSpecialFolderPath(m_hWnd, szValue, CSIDL_STARTUP, FALSE))
		{
//		if (RegReadString(HKEY_LOCAL_MACHINE, L"System\\Explorer\\Shell Folders", L"StartUp", szValue))
//		{
			m_szStartupFolder.Format(L"%s", szValue);
		}
		else
		{
			OutputDebugString(ErrorString(GetLastError()));
			m_szStartupFolder = L"\\Windows\\StartUp";
		}

		ZeroMemory(szValue, sizeof(szValue));
		if (SHGetSpecialFolderPath(m_hWnd, szValue, CSIDL_PROGRAM_FILES, FALSE))
		{
		//if (RegReadString(HKEY_LOCAL_MACHINE, L"System\\Explorer\\Shell Folders", L"Program Files", szValue))
		//{
			m_szProgramFilesFolder.Format(L"%s", szValue);
		}
		else
		{
			OutputDebugString(ErrorString(GetLastError()));
			m_szProgramFilesFolder = L"\\Program Files";
		}


		if (!(m_bWithFingerMenu))
		{
			CWindow wnd1 = GetDlgItem(IDC_AUTOSTART_MENU);
			wnd1.EnableWindow(FALSE);

			CWindow wnd2 = GetDlgItem(IDC_LBL_AUTOSTART_MENU);
			wnd2.EnableWindow(FALSE);

		}

		LoadConfiguration();

		DoDataExchange(FALSE); 

		return TRUE;
	}

public:
	BOOL m_bWithFingerMenu;
private:
	BOOL m_bAutostartMenu;
	BOOL m_bAutostartMsgbox;
	CString m_szStartupFolder;
	CString m_szProgramFilesFolder;

	void LoadConfiguration()
	{
		CFindFile finder; 
		if (m_bWithFingerMenu)
			m_bAutostartMenu = finder.FindFile(m_szStartupFolder + L"\\FingerMenu.lnk");
		m_bAutostartMsgbox = finder.FindFile(m_szStartupFolder + L"\\FingerMsgbox.lnk");
	}

	void SaveConfiguration()
	{
		if (m_bWithFingerMenu)
		{
			if (m_bAutostartMenu)
			{
				HANDLE hLink = ::CreateFile(m_szStartupFolder + L"\\FingerMenu.lnk", 
											GENERIC_WRITE,
											0,
											NULL,
											CREATE_ALWAYS,
											FILE_ATTRIBUTE_NORMAL,
											NULL);
				if (hLink != INVALID_HANDLE_VALUE)
				{
					CString szPath;
					WCHAR szInstallDir[MAX_PATH];
					if (RegistryGetString(HKEY_LOCAL_MACHINE, L"Software\\FingerMenu", L"InstallDir", szInstallDir, MAX_PATH) == S_OK)
					{
						szPath.Format(L"%s\\FingerMenu.exe", szInstallDir);
					}
					else
					{
						szPath = m_szProgramFilesFolder + L"\\FingerMenu\\FingerMenu.exe";
					}


					//CHAR szLink[] = "42#\"\\Program Files\\FingerMenu\\FingerMenu.exe\"";
					CString szLink; szLink.Format(L"%d#\"%s\"", szPath.GetLength() + 2, szPath);
					CHAR* tmp = new CHAR[ szLink.GetLength() + 1 ];
					ZeroMemory(tmp, szLink.GetLength());
					for (int i = 0; i < szLink.GetLength(); i++)
						tmp[i] = (CHAR)szLink[i];

					DWORD dwNumberOfBytesWritten;
					::WriteFile(hLink, tmp, szLink.GetLength(), &dwNumberOfBytesWritten, NULL);
					CloseHandle(hLink);
				}
			}
			else
			{
				::DeleteFile(m_szStartupFolder + L"\\FingerMenu.lnk");
			}
		}

		if (m_bAutostartMsgbox)
		{
			HANDLE hLink = ::CreateFile(m_szStartupFolder + L"\\FingerMsgbox.lnk", 
				                        GENERIC_WRITE,
										0,
										NULL,
										CREATE_ALWAYS,
										FILE_ATTRIBUTE_NORMAL,
										NULL);
			if (hLink != INVALID_HANDLE_VALUE)
			{
				CString szPath;
				WCHAR szInstallDir[MAX_PATH];
				if (RegistryGetString(HKEY_LOCAL_MACHINE, L"Software\\FingerMsgbox", L"InstallDir", szInstallDir, MAX_PATH) == S_OK)
				{
					szPath.Format(L"%s\\FingerMsgbox.exe", szInstallDir);
				}
				else
				{
					szPath = m_szProgramFilesFolder + L"\\FingerMsgbox\\FingerMsgbox.exe";
				}

				CString szLink; szLink.Format(L"%d#\"%s\"", szPath.GetLength() + 2, szPath);
				CHAR* tmp = new CHAR[ szLink.GetLength() + 1 ];
				ZeroMemory(tmp, szLink.GetLength());
				for (int i = 0; i < szLink.GetLength(); i++)
					tmp[i] = (CHAR)szLink[i];

				DWORD dwNumberOfBytesWritten;
				::WriteFile(hLink, tmp, szLink.GetLength(), &dwNumberOfBytesWritten, NULL);
				CloseHandle(hLink);
			}
		}
		else
		{
			::DeleteFile(m_szStartupFolder + L"\\FingerMsgbox.lnk");
		}
	}
};

/********************************************************
 PAGE2
 ********************************************************/

class CFingerMenuCPLDlg2 :    public CPropertyPageImpl<CFingerMenuCPLDlg2>,
							  public CWinDataExchange<CFingerMenuCPLDlg2>,
							  public CUpdateUI<CFingerMenuCPLDlg2>
{
public:
	enum { IDD = IDD_PAGE2 };

	BOOL PreTranslateMessage(MSG* pMsg)
	{
		return CWindow::IsDialogMessage(pMsg);
	}

	BOOL OnIdle()
	{
		UIUpdateChildWindows();
		return FALSE;
	}

	void DoPaint(CDCHandle dc) {}


	BEGIN_MSG_MAP_EX(CFingerMenuCPLDlg2)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_HANDLER(IDC_BTN_BKGCOLORTRANSP, BN_CLICKED, OnBnClickedBtnBkgColorTransp)
		NOTIFY_HANDLER_EX(IDC_SPIN_TRANSLEVEL, UDN_DELTAPOS, OnSpinTransLevelDeltaPos)
		NOTIFY_HANDLER_EX(IDC_SPIN_MAXCOUNT, UDN_DELTAPOS, OnSpinMaxCount)
		NOTIFY_HANDLER_EX(IDC_SPIN_MAXVELOCITY, UDN_DELTAPOS, OnSpinMaxVelocity)
		CHAIN_MSG_MAP(CPropertyPageImpl<CFingerMenuCPLDlg2> )
	END_MSG_MAP()

    BEGIN_DDX_MAP(CFingerMenuCPLDlg2)
		DDX_UINT(IDC_TRANSLEVEL, m_dwTransparencyLevel)
		DDX_UINT(IDC_MAXCOUNT, m_dwMaxVisibleItemCount)
		DDX_UINT(IDC_MAXVELOCITY, m_dwMaxVelocity)
		DDX_CHECK(IDC_ENABLE_SIP, m_bEnableSIP)
		DDX_CHECK(IDC_ENABLE_TRANSPARENCY, m_bDisableTransparency)
		DDX_CHECK(IDC_DISABLE_BTNS, m_bDisableButtons)
		DDX_CHECK(IDC_ENABLE_ANIMATION, m_bEnableAnimation)
		DDX_CONTROL(IDC_CL_BKGCOLORTRANSP, m_ccBkgColorTransp)
    END_DDX_MAP()

	BEGIN_UPDATE_UI_MAP(CFingerMenuCPLDlg2)
    END_UPDATE_UI_MAP()

	int OnApply()
	{
		DoDataExchange(TRUE);
		SaveConfiguration();
		return PSNRET_NOERROR;
	}

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		//CMessageLoop* pLoop = _Module.GetMessageLoop();
		//ATLASSERT(pLoop != NULL);
		//pLoop->AddIdleHandler(this);
		UIAddChildWindowContainer(m_hWnd);

		LoadConfiguration();

		DoDataExchange(FALSE); 

		return TRUE;
	}

	LRESULT OnSpinTransLevelDeltaPos ( NMHDR* phdr )
	{
		NMUPDOWN* pm = (NMUPDOWN*)phdr;
		m_dwTransparencyLevel -= pm->iDelta;
		DoDataExchange(FALSE, IDC_TRANSLEVEL); 
		return 0;
	}

	LRESULT OnSpinMaxCount ( NMHDR* phdr )
	{
		NMUPDOWN* pm = (NMUPDOWN*)phdr;
		m_dwMaxVisibleItemCount -= pm->iDelta;
		DoDataExchange(FALSE, IDC_MAXCOUNT); 
		return 0;
	}

	LRESULT OnSpinMaxVelocity ( NMHDR* phdr )
	{
		NMUPDOWN* pm = (NMUPDOWN*)phdr;
		m_dwMaxVelocity -= pm->iDelta;
		DoDataExchange(FALSE, IDC_MAXVELOCITY); 
		return 0;
	}

	LRESULT OnBnClickedBtnBkgColorTransp(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		CColorDialog colorDlg;
		if (colorDlg.DoModal() == IDOK) // The user selected the "OK" button
		{
			m_ccBkgColorTransp.SetColor( colorDlg.GetColor() ); 
		}
		return 0;
	}

public:
	DWORD m_dwTransparencyLevel;
	BOOL  m_bEnableSIP;
	DWORD m_dwMaxVisibleItemCount;
	DWORD m_dwMaxVelocity;
	BOOL  m_bDisableTransparency;
	BOOL  m_bDisableButtons;
	BOOL  m_bEnableAnimation;
	CColorViewImpl m_ccBkgColorTransp;

private:
	void LoadConfiguration()
	{
		WCHAR keyName[] = L"Software\\FingerMenu";

		// load transparency level
		m_dwTransparencyLevel = 128;
		if (!(RegReadDWORD(HKEY_LOCAL_MACHINE, keyName, L"TransparencyLevel", m_dwTransparencyLevel)))
			wprintf(L"Unable to read TransparencyLevel from registry: %s\n", ErrorString(GetLastError()));


		m_bEnableSIP = TRUE;
		if (!(RegReadBOOL(HKEY_LOCAL_MACHINE, keyName, L"EnableSIP", m_bEnableSIP)))
			wprintf(L"Unable to read EnableSIP from registry: %s\n", ErrorString(GetLastError()));

		m_dwMaxVisibleItemCount = 0;
		if (!(RegReadDWORD(HKEY_LOCAL_MACHINE, keyName, L"MaxMenuItemCount", m_dwMaxVisibleItemCount)))
			wprintf(L"Unable to read MaxMenuItemCount from registry: %s\n", ErrorString(GetLastError()));

		m_dwMaxVelocity = 15;
		if (!(RegReadDWORD(HKEY_LOCAL_MACHINE, keyName, L"MaxVelocity", m_dwMaxVelocity)))
			wprintf(L"Unable to read MaxVelocity from registry: %s\n", ErrorString(GetLastError()));

		m_bDisableTransparency = FALSE;
		if (!(RegReadBOOL(HKEY_LOCAL_MACHINE, keyName, L"DisableTransparency", m_bDisableTransparency)))
			wprintf(L"Unable to read DisableTransparency from registry: %s\n", ErrorString(GetLastError()));

		m_bDisableButtons = FALSE;
		if (!(RegReadBOOL(HKEY_LOCAL_MACHINE, keyName, L"NeverShowButtons", m_bDisableButtons)))
			wprintf(L"Unable to read NeverShowButtons from registry: %s\n", ErrorString(GetLastError()));

		m_bEnableAnimation = TRUE;
		if (!(RegReadBOOL(HKEY_LOCAL_MACHINE, keyName, L"EnableAnimation", m_bEnableAnimation)))
			wprintf(L"Unable to read EnableAnimation from registry: %s\n", ErrorString(GetLastError()));

		RegReadColor(HKEY_LOCAL_MACHINE, keyName, L"NoTranspBkgColor", m_ccBkgColorTransp.m_color);

	}
public:
	void SaveConfiguration()
	{
		WCHAR szKeyName[] = L"Software\\FingerMenu";
		RegWriteDWORD(HKEY_LOCAL_MACHINE, szKeyName, L"TransparencyLevel", m_dwTransparencyLevel);
		RegWriteDWORD(HKEY_LOCAL_MACHINE, szKeyName, L"MaxMenuItemCount", m_dwMaxVisibleItemCount);
		RegWriteDWORD(HKEY_LOCAL_MACHINE, szKeyName, L"MaxVelocity", m_dwMaxVelocity);
		RegWriteBOOL(HKEY_LOCAL_MACHINE,  szKeyName, L"EnableSIP", m_bEnableSIP);
		RegWriteBOOL(HKEY_LOCAL_MACHINE,  szKeyName, L"DisableTransparency", m_bDisableTransparency);
		RegWriteBOOL(HKEY_LOCAL_MACHINE,  szKeyName, L"NeverShowButtons", m_bDisableButtons);
		RegWriteColor(HKEY_LOCAL_MACHINE, szKeyName, L"NoTranspBkgColor", m_ccBkgColorTransp.m_color);
		RegWriteBOOL(HKEY_LOCAL_MACHINE,  szKeyName, L"EnableAnimation", m_bEnableAnimation);
	}
};

/********************************************************
 PAGE3
 ********************************************************/

class CFingerMenuCPLDlg3 :    public CPropertyPageImpl<CFingerMenuCPLDlg3>,
							  public CWinDataExchange<CFingerMenuCPLDlg3>,
							  public CUpdateUI<CFingerMenuCPLDlg3>
{
public:
	enum { IDD = IDD_PAGE3 };

	BOOL PreTranslateMessage(MSG* pMsg)
	{
		return CWindow::IsDialogMessage(pMsg);
	}

	BOOL OnIdle()
	{
		UIUpdateChildWindows();
		return FALSE;
	}

	void DoPaint(CDCHandle dc) {}


	BEGIN_MSG_MAP_EX(CFingerMenuCPLDlg3)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_HANDLER(IDC_BTN_ADDPROGRAM, BN_CLICKED, OnBnClickedBtnAddProgram)
		COMMAND_HANDLER(IDC_BTN_REFRESH, BN_CLICKED, OnBnClickedBtnRefresh)
		COMMAND_HANDLER(IDC_BTN_REMOVEPROGRAMS, BN_CLICKED, OnBnClickedBtnRemove)
		CHAIN_MSG_MAP(CPropertyPageImpl<CFingerMenuCPLDlg3> )
	END_MSG_MAP()

    BEGIN_DDX_MAP(CFingerMenuCPLDlg3)
		DDX_CONTROL_HANDLE(IDC_CB_PROCESSES, m_cbProcesses)
		DDX_CONTROL_HANDLE(IDC_LB_PROCESSES, m_lbProcesses)
    END_DDX_MAP()

	BEGIN_UPDATE_UI_MAP(CFingerMenuCPLDlg)
    END_UPDATE_UI_MAP()

	int OnApply()
	{
		DoDataExchange(TRUE);
		SaveConfiguration();
		return PSNRET_NOERROR;
	}

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		//CMessageLoop* pLoop = _Module.GetMessageLoop();
		//ATLASSERT(pLoop != NULL);
		//pLoop->AddIdleHandler(this);
		UIAddChildWindowContainer(m_hWnd);

		//

		DoDataExchange(FALSE); 

		LoadConfiguration();

		RefreshProcessList();

		return TRUE;
	}



	LRESULT OnBnClickedBtnAddProgram(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		int i = m_cbProcesses.GetCurSel();
		if (i >= 0)
		{
			CString proc; m_cbProcesses.GetLBText(i, proc);
			int k = m_lbProcesses.FindString(0, proc);
			if (k < 0)
				m_lbProcesses.AddString(proc);
		}
		else
		{
			WCHAR proc[80]; ZeroMemory(proc, 80);
			m_cbProcesses.GetWindowText(proc, 80);
			if (proc[0] != '\0')
			{
				int k = m_lbProcesses.FindString(0, proc);
				if (k < 0)
					m_lbProcesses.AddString(proc);
			}
		}
		return 0;
	}

	LRESULT OnBnClickedBtnRefresh(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		RefreshProcessList();
		return 0;
	}

	LRESULT OnBnClickedBtnRemove(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		int i = m_lbProcesses.GetCurSel();
		if (i >= 0)
			m_lbProcesses.DeleteString(i);
		return 0;
	}

private:
	void RefreshProcessList()
	{
		m_cbProcesses.ResetContent();
		m_cbProcesses.Clear();
		// set compiler flag /EHa
		HANDLE snapShot = INVALID_HANDLE_VALUE;
		try
		{
		    snapShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS | TH32CS_SNAPNOHEAPS, 0);

		    if (snapShot != INVALID_HANDLE_VALUE)
			{
				// Build new list
				PROCESSENTRY32 processEntry;
				processEntry.dwSize = sizeof(PROCESSENTRY32);
				BOOL ret = Process32First(snapShot, &processEntry);
				while (ret == TRUE)
				{
					if (
						(lstrcmpi(processEntry.szExeFile, L"gwes.exe") != 0) &&
						(lstrcmpi(processEntry.szExeFile, L"nk.exe") != 0) &&
						(lstrcmpi(processEntry.szExeFile, L"connmgr.exe") != 0) &&
						(lstrcmpi(processEntry.szExeFile, L"device.exe") != 0) &&
						(lstrcmpi(processEntry.szExeFile, L"filesys.exe") != 0) &&
						(lstrcmpi(processEntry.szExeFile, L"services.exe") != 0) &&
						(lstrcmpi(processEntry.szExeFile, L"shell32.exe") != 0) 
					   )
					{
						m_cbProcesses.AddString(processEntry.szExeFile);
					}

					ret = Process32Next(snapShot, &processEntry);
				}
				CloseToolhelp32Snapshot(snapShot);
			}
		} catch (...)
		{
			// do nothing
			if (snapShot != INVALID_HANDLE_VALUE)
			{
				CloseToolhelp32Snapshot(snapShot);
			}
		}
	}

public:
	CSimpleArray<CString> m_szProcesses;
	CComboBox m_cbProcesses;
	CListBox m_lbProcesses;


private:
	void LoadConfiguration()
	{
		CSimpleArray<CString, CStringEqualHelper<CString>> arExcludedApp;
		LoadExclusionList(arExcludedApp, L"Software\\FingerMenu");

		m_lbProcesses.ResetContent();
		for (int i = 0; i < arExcludedApp.GetSize(); i ++)
		{
			m_lbProcesses.AddString(arExcludedApp[i]);
		}
	}
public:
	void SaveConfiguration()
	{
		CSimpleArray<CString, CStringEqualHelper<CString>> arrExcludedApps;

		for (int i = 0; i < m_lbProcesses.GetCount(); i++)
		{
			CString proc; m_lbProcesses.GetText(i, proc);
			arrExcludedApps.Add(proc);
		}

		SaveExclusionList(arrExcludedApps, L"Software\\FingerMenu");
	}
};



/********************************************************
 PAGE4
 ********************************************************/

class CFingerMenuCPLDlg4 :    public CPropertyPageImpl<CFingerMenuCPLDlg4>,
							  public CWinDataExchange<CFingerMenuCPLDlg4>,
							  public CUpdateUI<CFingerMenuCPLDlg4>
{
public:
	enum { IDD = IDD_PAGE4 };

	CComboBox m_cbSkinFingerMenu;
	CComboBox m_cbSkinFingerMsgbox;

	BOOL PreTranslateMessage(MSG* pMsg)
	{
		return CWindow::IsDialogMessage(pMsg);
	}

	BOOL OnIdle()
	{
		UIUpdateChildWindows();
		return FALSE;
	}

	void DoPaint(CDCHandle dc) {}


	BEGIN_MSG_MAP_EX(CFingerMenuCPLDlg4)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		CHAIN_MSG_MAP(CPropertyPageImpl<CFingerMenuCPLDlg4>)
	END_MSG_MAP()

    BEGIN_DDX_MAP(CFingerMenuCPLDlg4)
		DDX_CONTROL_HANDLE(IDC_CB_SKIN_FINGERMENU, m_cbSkinFingerMenu)
		DDX_CONTROL_HANDLE(IDC_CB_SKIN_FINGERMSGBOX, m_cbSkinFingerMsgbox)
    END_DDX_MAP()

	BEGIN_UPDATE_UI_MAP(CFingerMenuCPLDlg4)
    END_UPDATE_UI_MAP()

	int OnApply()
	{
		DoDataExchange(TRUE);
		SaveConfiguration();
		return PSNRET_NOERROR;
	}

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		UIAddChildWindowContainer(m_hWnd);

		DoDataExchange(FALSE); 

		if (!(m_bWithFingerMenu))
		{
			CWindow wnd1 = GetDlgItem(IDC_CB_SKIN_FINGERMENU);
			wnd1.EnableWindow(FALSE);

			CWindow wnd2 = GetDlgItem(IDC_LBL_CB_SKIN_FINGERMENU);
			wnd2.EnableWindow(FALSE);

		}

		LoadConfiguration();

		return TRUE;
	}

public:
	BOOL m_bWithFingerMenu;


private:
	void LoadConfiguration()
	{
		CString szProgramFilesFolder;
		WCHAR szValue[MAX_PATH]; 
		if (RegReadString(HKEY_LOCAL_MACHINE, L"System\\Explorer\\Shell Folders", L"Program Files", szValue))
		{
			szProgramFilesFolder.Format(L"%s", szValue);
		}

		// menu
		WCHAR szKeyName[] = L"Software\\FingerMenu";
		WCHAR skin[50];
		BOOL setDefault = RegReadString(HKEY_LOCAL_MACHINE, szKeyName, L"Skin", skin);

		m_cbSkinFingerMenu.ResetContent();

		CFindFile finder;
		BOOL bWorking = finder.FindFile(szProgramFilesFolder + L"\\FingerMenu\\skins\\*");
		BOOL bAdd = bWorking;
		int i = 0;
		while (bWorking)
		{
			if (bAdd)
			{
				m_cbSkinFingerMenu.AddString(finder.GetFileName());
				if ((setDefault) && (lstrcmpi(skin, finder.GetFileName()) == 0))
					m_cbSkinFingerMenu.SetCurSel(i);
				i++;
			}
			
			bWorking = finder.FindNextFile();
			if (bWorking)
			{
				if ( finder.IsDirectory() && (!(finder.IsDots())) )
				{
					bAdd = TRUE;
				}
				else
				{
					bAdd = FALSE;
				}
			}
		}

		// msgbox
		WCHAR szKeyName2[] = L"Software\\FingerMsgbox";
		setDefault = RegReadString(HKEY_LOCAL_MACHINE, szKeyName2, L"Skin", skin);

		m_cbSkinFingerMsgbox.ResetContent();

		bWorking = finder.FindFile(szProgramFilesFolder + L"\\FingerMsgbox\\skins\\*");
		bAdd = bWorking;
		i = 0;
		while (bWorking)
		{
			if (bAdd)		
			{
				m_cbSkinFingerMsgbox.AddString(finder.GetFileName());
				if ((setDefault) && (lstrcmpi(skin, finder.GetFileName()) == 0))
					m_cbSkinFingerMsgbox.SetCurSel(i);
				i++;
			}
			
			bWorking = finder.FindNextFile();
			if (bWorking)
			{
				if ( finder.IsDirectory() && (!(finder.IsDots())) )
				{
					bAdd = TRUE;
				}
				else
				{
					bAdd = FALSE;
				}
			}
		}
	}
public:
	void SaveConfiguration()
	{
		if (m_bWithFingerMenu)
		{
			WCHAR szKeyName[] = L"Software\\FingerMenu";
			int i = m_cbSkinFingerMenu.GetCurSel();
			CString skin;
			if (i >= 0)
				m_cbSkinFingerMenu.GetLBText(i, skin);
			else
				skin = L"default";
			RegWriteString(HKEY_LOCAL_MACHINE, szKeyName, L"Skin", skin, skin.GetLength());
		}

		{
			WCHAR szKeyName[] = L"Software\\FingerMsgbox";
			int i = m_cbSkinFingerMsgbox.GetCurSel();
			CString skin;
			if (i >= 0)
				m_cbSkinFingerMsgbox.GetLBText(i, skin);
			else
				skin = L"default";
			RegWriteString(HKEY_LOCAL_MACHINE, szKeyName, L"Skin", skin, skin.GetLength());
		}
	}
};


/********************************************************
 PAGE5
 ********************************************************/

class CFingerMenuCPLDlg5 :    public CPropertyPageImpl<CFingerMenuCPLDlg5>,
							  public CWinDataExchange<CFingerMenuCPLDlg5>,
							  public CUpdateUI<CFingerMenuCPLDlg5>
{
public:
	enum { IDD = IDD_PAGE5 };

	BOOL PreTranslateMessage(MSG* pMsg)
	{
		return CWindow::IsDialogMessage(pMsg);
	}

	BOOL OnIdle()
	{
		UIUpdateChildWindows();
		return FALSE;
	}

	void DoPaint(CDCHandle dc) {}


	BEGIN_MSG_MAP_EX(CFingerMenuCPLDlg5)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_HANDLER(IDC_BTN_BKGCOLORTRANSP, BN_CLICKED, OnBnClickedBtnBkgColorTransp)
		NOTIFY_HANDLER_EX(IDC_SPIN_TRANSLEVEL, UDN_DELTAPOS, OnSpinTransLevelDeltaPos)
		CHAIN_MSG_MAP(CPropertyPageImpl<CFingerMenuCPLDlg5> )
	END_MSG_MAP()

    BEGIN_DDX_MAP(CFingerMenuCPLDlg5)
		DDX_UINT(IDC_TRANSLEVEL, m_dwTransparencyLevel)
		DDX_CHECK(IDC_ENABLE_TRANSPARENCY, m_bDisableTransparency)
		DDX_CONTROL(IDC_CL_BKGCOLORTRANSP, m_ccBkgColorTransp)
    END_DDX_MAP()

	BEGIN_UPDATE_UI_MAP(CFingerMenuCPLDlg5)
    END_UPDATE_UI_MAP()

	int OnApply()
	{
		DoDataExchange(TRUE);
		SaveConfiguration();
		return PSNRET_NOERROR;
	}

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		//CMessageLoop* pLoop = _Module.GetMessageLoop();
		//ATLASSERT(pLoop != NULL);
		//pLoop->AddIdleHandler(this);
		UIAddChildWindowContainer(m_hWnd);

		LoadConfiguration();

		DoDataExchange(FALSE); 

		return TRUE;
	}

	LRESULT OnSpinTransLevelDeltaPos ( NMHDR* phdr )
	{
		NMUPDOWN* pm = (NMUPDOWN*)phdr;
		m_dwTransparencyLevel -= pm->iDelta;
		DoDataExchange(FALSE, IDC_TRANSLEVEL); 
		return 0;
	}

	LRESULT OnBnClickedBtnBkgColorTransp(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		CColorDialog colorDlg;
		if (colorDlg.DoModal() == IDOK) // The user selected the "OK" button
		{
			m_ccBkgColorTransp.SetColor( colorDlg.GetColor() ); 
		}
		return 0;
	}

public:
	DWORD m_dwTransparencyLevel;
	BOOL  m_bDisableTransparency;
	CColorViewImpl m_ccBkgColorTransp;

private:
	void LoadConfiguration()
	{
		WCHAR keyName[] = L"Software\\FingerMsgbox";

		// load transparency level
		m_dwTransparencyLevel = 128;
		if (!(RegReadDWORD(HKEY_LOCAL_MACHINE, keyName, L"TransparencyLevel", m_dwTransparencyLevel)))
			wprintf(L"Unable to read TransparencyLevel from registry: %s\n", ErrorString(GetLastError()));

		m_bDisableTransparency = FALSE;
		if (!(RegReadBOOL(HKEY_LOCAL_MACHINE, keyName, L"DisableTransparency", m_bDisableTransparency)))
			wprintf(L"Unable to read DisableTransparency from registry: %s\n", ErrorString(GetLastError()));

		RegReadColor(HKEY_LOCAL_MACHINE, keyName, L"NoTranspBkgColor", m_ccBkgColorTransp.m_color);

	}
public:
	void SaveConfiguration()
	{
		WCHAR szKeyName[] = L"Software\\FingerMsgbox";
		RegWriteDWORD(HKEY_LOCAL_MACHINE, szKeyName, L"TransparencyLevel", m_dwTransparencyLevel);
		RegWriteBOOL(HKEY_LOCAL_MACHINE,  szKeyName, L"DisableTransparency", m_bDisableTransparency);
		RegWriteColor(HKEY_LOCAL_MACHINE, szKeyName, L"NoTranspBkgColor", m_ccBkgColorTransp.m_color);
	}
};



/********************************************************
 PAGE6
 ********************************************************/

class CFingerMenuCPLDlg6 :    public CPropertyPageImpl<CFingerMenuCPLDlg6>,
							  public CWinDataExchange<CFingerMenuCPLDlg6>,
							  public CUpdateUI<CFingerMenuCPLDlg6>
{
public:
	enum { IDD = IDD_PAGE3 };

	BOOL PreTranslateMessage(MSG* pMsg)
	{
		return CWindow::IsDialogMessage(pMsg);
	}

	BOOL OnIdle()
	{
		UIUpdateChildWindows();
		return FALSE;
	}

	void DoPaint(CDCHandle dc) {}


	BEGIN_MSG_MAP_EX(CFingerMenuCPLDlg6)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_HANDLER(IDC_BTN_ADDPROGRAM, BN_CLICKED, OnBnClickedBtnAddProgram)
		COMMAND_HANDLER(IDC_BTN_REFRESH, BN_CLICKED, OnBnClickedBtnRefresh)
		COMMAND_HANDLER(IDC_BTN_REMOVEPROGRAMS, BN_CLICKED, OnBnClickedBtnRemove)
		CHAIN_MSG_MAP(CPropertyPageImpl<CFingerMenuCPLDlg6> )
	END_MSG_MAP()

    BEGIN_DDX_MAP(CFingerMenuCPLDlg6)
		DDX_CONTROL_HANDLE(IDC_CB_PROCESSES, m_cbProcesses)
		DDX_CONTROL_HANDLE(IDC_LB_PROCESSES, m_lbProcesses)
    END_DDX_MAP()

	BEGIN_UPDATE_UI_MAP(CFingerMenuCPLDlg6)
    END_UPDATE_UI_MAP()

	int OnApply()
	{
		DoDataExchange(TRUE);
		SaveConfiguration();
		return PSNRET_NOERROR;
	}

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		//CMessageLoop* pLoop = _Module.GetMessageLoop();
		//ATLASSERT(pLoop != NULL);
		//pLoop->AddIdleHandler(this);
		UIAddChildWindowContainer(m_hWnd);

		//

		DoDataExchange(FALSE); 

		LoadConfiguration();

		RefreshProcessList();

		return TRUE;
	}



	LRESULT OnBnClickedBtnAddProgram(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		int i = m_cbProcesses.GetCurSel();
		if (i >= 0)
		{
			CString proc; m_cbProcesses.GetLBText(i, proc);
			int k = m_lbProcesses.FindString(0, proc);
			if (k < 0)
				m_lbProcesses.AddString(proc);
		}
		else
		{
			WCHAR proc[80]; ZeroMemory(proc, 80);
			m_cbProcesses.GetWindowText(proc, 80);
			if (proc[0] != '\0')
			{
				int k = m_lbProcesses.FindString(0, proc);
				if (k < 0)
					m_lbProcesses.AddString(proc);
			}
		}
		return 0;
	}

	LRESULT OnBnClickedBtnRefresh(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		RefreshProcessList();
		return 0;
	}

	LRESULT OnBnClickedBtnRemove(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		int i = m_lbProcesses.GetCurSel();
		if (i >= 0)
			m_lbProcesses.DeleteString(i);
		return 0;
	}

private:
	void RefreshProcessList()
	{
		m_cbProcesses.ResetContent();
		m_cbProcesses.Clear();
		// set compiler flag /EHa
		HANDLE snapShot = INVALID_HANDLE_VALUE;
		try
		{
		    snapShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS | TH32CS_SNAPNOHEAPS, 0);

		    if (snapShot != INVALID_HANDLE_VALUE)
			{
				// Build new list
				PROCESSENTRY32 processEntry;
				processEntry.dwSize = sizeof(PROCESSENTRY32);
				BOOL ret = Process32First(snapShot, &processEntry);
				while (ret == TRUE)
				{
					if (
						(lstrcmpi(processEntry.szExeFile, L"gwes.exe") != 0) &&
						(lstrcmpi(processEntry.szExeFile, L"nk.exe") != 0) &&
						(lstrcmpi(processEntry.szExeFile, L"connmgr.exe") != 0) &&
						(lstrcmpi(processEntry.szExeFile, L"device.exe") != 0) &&
						(lstrcmpi(processEntry.szExeFile, L"filesys.exe") != 0) &&
						(lstrcmpi(processEntry.szExeFile, L"services.exe") != 0) &&
						(lstrcmpi(processEntry.szExeFile, L"shell32.exe") != 0) 
					   )
					{
						m_cbProcesses.AddString(processEntry.szExeFile);
					}

					ret = Process32Next(snapShot, &processEntry);
				}
				CloseToolhelp32Snapshot(snapShot);
			}
		} catch (...)
		{
			// do nothing
			if (snapShot != INVALID_HANDLE_VALUE)
			{
				CloseToolhelp32Snapshot(snapShot);
			}
		}
	}

public:
	CSimpleArray<CString> m_szProcesses;
	CComboBox m_cbProcesses;
	CListBox m_lbProcesses;


private:
	void LoadConfiguration()
	{
		CSimpleArray<CString, CStringEqualHelper<CString>> arExcludedApp;
		LoadExclusionList(arExcludedApp, L"Software\\FingerMsgbox");

		m_lbProcesses.ResetContent();
		for (int i = 0; i < arExcludedApp.GetSize(); i ++)
		{
			m_lbProcesses.AddString(arExcludedApp[i]);
		}
	}
public:
	void SaveConfiguration()
	{
		CSimpleArray<CString, CStringEqualHelper<CString>> arrExcludedApps;

		for (int i = 0; i < m_lbProcesses.GetCount(); i++)
		{
			CString proc; m_lbProcesses.GetText(i, proc);
			arrExcludedApps.Add(proc);
		}

		SaveExclusionList(arrExcludedApps, L"Software\\FingerMsgbox");
	}
};




/********************************************************
 PAGE7
 ********************************************************/

class CFingerMenuCPLDlg7 :    public CPropertyPageImpl<CFingerMenuCPLDlg7>,
							  public CWinDataExchange<CFingerMenuCPLDlg7>,
							  public CUpdateUI<CFingerMenuCPLDlg7>
{
public:
	enum { IDD = IDD_PAGE7 };

	BOOL PreTranslateMessage(MSG* pMsg)
	{
		return CWindow::IsDialogMessage(pMsg);
	}

	BOOL OnIdle()
	{
		UIUpdateChildWindows();
		return FALSE;
	}

	void DoPaint(CDCHandle dc) {}


	BEGIN_MSG_MAP_EX(CFingerMenuCPLDlg7)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_HANDLER(IDC_BTN_REMOVEPROGRAMS, BN_CLICKED, OnBnClickedBtnRemove)
		CHAIN_MSG_MAP(CPropertyPageImpl<CFingerMenuCPLDlg7> )
	END_MSG_MAP()

    BEGIN_DDX_MAP(CFingerMenuCPLDlg7)
		DDX_CONTROL_HANDLE(IDC_LB_PROCESSES, m_lbProcesses)
    END_DDX_MAP()

	BEGIN_UPDATE_UI_MAP(CFingerMenuCPLDlg7)
    END_UPDATE_UI_MAP()

	int OnApply()
	{
		DoDataExchange(TRUE);
		SaveConfiguration();
		return PSNRET_NOERROR;
	}

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		//CMessageLoop* pLoop = _Module.GetMessageLoop();
		//ATLASSERT(pLoop != NULL);
		//pLoop->AddIdleHandler(this);
		UIAddChildWindowContainer(m_hWnd);

		//

		DoDataExchange(FALSE); 

		LoadConfiguration();

		return TRUE;
	}

	LRESULT OnBnClickedBtnRemove(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		int i = m_lbProcesses.GetCurSel();
		if (i >= 0)
			m_lbProcesses.DeleteString(i);
		return 0;
	}

public:
	CSimpleArray<CString> m_szProcesses;
	CListBox m_lbProcesses;


private:
	void LoadConfiguration()
	{
		CSimpleArray<CString, CStringEqualHelper<CString>> arExcludedWnds;
		LoadWndExclusionList(arExcludedWnds, L"Software\\FingerMenu");

		m_lbProcesses.ResetContent();
		for (int i = 0; i < arExcludedWnds.GetSize(); i ++)
		{
			m_lbProcesses.AddString(arExcludedWnds[i]);
		}
	}
public:
	void SaveConfiguration()
	{
		CSimpleArray<CString, CStringEqualHelper<CString>> arExcludedWnds;

		for (int i = 0; i < m_lbProcesses.GetCount(); i++)
		{
			CString proc; m_lbProcesses.GetText(i, proc);
			arExcludedWnds.Add(proc);
		}

		SaveWndExclusionList(arExcludedWnds, L"Software\\FingerMenu");
	}
};


/********************************************************
 PAGE8
 ********************************************************/

class CFingerMenuCPLDlg8 :    public CPropertyPageImpl<CFingerMenuCPLDlg8>,
							  public CWinDataExchange<CFingerMenuCPLDlg8>,
							  public CUpdateUI<CFingerMenuCPLDlg8>
{
public:
	enum { IDD = IDD_PAGE7 };

	BOOL PreTranslateMessage(MSG* pMsg)
	{
		return CWindow::IsDialogMessage(pMsg);
	}

	BOOL OnIdle()
	{
		UIUpdateChildWindows();
		return FALSE;
	}

	void DoPaint(CDCHandle dc) {}


	BEGIN_MSG_MAP_EX(CFingerMenuCPLDlg8)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_HANDLER(IDC_BTN_REMOVEPROGRAMS, BN_CLICKED, OnBnClickedBtnRemove)
		CHAIN_MSG_MAP(CPropertyPageImpl<CFingerMenuCPLDlg8> )
	END_MSG_MAP()

    BEGIN_DDX_MAP(CFingerMenuCPLDlg8)
		DDX_CONTROL_HANDLE(IDC_LB_PROCESSES, m_lbProcesses)
    END_DDX_MAP()

	BEGIN_UPDATE_UI_MAP(CFingerMenuCPLDlg8)
    END_UPDATE_UI_MAP()

	int OnApply()
	{
		DoDataExchange(TRUE);
		SaveConfiguration();
		return PSNRET_NOERROR;
	}

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		//CMessageLoop* pLoop = _Module.GetMessageLoop();
		//ATLASSERT(pLoop != NULL);
		//pLoop->AddIdleHandler(this);
		UIAddChildWindowContainer(m_hWnd);

		//

		DoDataExchange(FALSE); 

		LoadConfiguration();

		return TRUE;
	}

	LRESULT OnBnClickedBtnRemove(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		int i = m_lbProcesses.GetCurSel();
		if (i >= 0)
			m_lbProcesses.DeleteString(i);
		return 0;
	}

public:
	CSimpleArray<CString> m_szProcesses;
	CListBox m_lbProcesses;


private:
	void LoadConfiguration()
	{
		CSimpleArray<CString, CStringEqualHelper<CString>> arExcludedWnds;
		LoadWndExclusionList(arExcludedWnds, L"Software\\FingerMsgbox");

		m_lbProcesses.ResetContent();
		for (int i = 0; i < arExcludedWnds.GetSize(); i ++)
		{
			m_lbProcesses.AddString(arExcludedWnds[i]);
		}
	}
public:
	void SaveConfiguration()
	{
		CSimpleArray<CString, CStringEqualHelper<CString>> arExcludedWnds;

		for (int i = 0; i < m_lbProcesses.GetCount(); i++)
		{
			CString proc; m_lbProcesses.GetText(i, proc);
			arExcludedWnds.Add(proc);
		}

		SaveWndExclusionList(arExcludedWnds, L"Software\\FingerMsgbox");
	}
};
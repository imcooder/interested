// FingerMsgBox.cpp : main source file for FingerMsgBox.exe
//

#include "stdafx.h"

#include <atlframe.h>
#include <atlctrls.h>
#include <atldlgs.h>

#include "resource.h"

#include "MainFrm.h"

CAppModule _Module;

int Run(LPTSTR /*lpstrCmdLine*/ = NULL, int nCmdShow = SW_SHOWNORMAL)
{
	CMessageLoop theLoop;
	_Module.AddMessageLoop(&theLoop);

	CMainFrame wndMain;

	if(wndMain.CreateEx() == NULL)
	{
		ATLTRACE(_T("Main window creation failed!\n"));
		return 0;
	}

	//wndMain.ShowWindow(nCmdShow);
	wndMain.ShowWindow(SW_MINIMIZE);

	int nRet = theLoop.Run();

	_Module.RemoveMessageLoop();
	return nRet;
}

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR lpstrCmdLine, int nCmdShow)
{
	HRESULT hRes = CMainFrame::ActivatePreviousInstance(hInstance);
	if(FAILED(hRes) || S_FALSE == hRes)
	{
		return hRes;
	}

	hRes = ::CoInitializeEx(NULL, COINIT_MULTITHREADED);
	ATLASSERT(SUCCEEDED(hRes));

	// Calling AtlInitCommonControls is not necessary to utilize picture,
	// static text, edit box, group box, button, check box, radio button, 
	// combo box, list box, or the horizontal and vertical scroll bars.
	// Calling AtlInitCommonControls with 0 is required to utilize the spin, 
	// progress, slider, list, tree, and tab controls.
	// Adding the ICC_DATE_CLASSES flag is required to initialize the 
	// date time picker and month calendar controls.
	// Add additional flags to support additoinal controls not mentioned above.
	AtlInitCommonControls(ICC_DATE_CLASSES);

	hRes = _Module.Init(NULL, hInstance);
	ATLASSERT(SUCCEEDED(hRes));

	int nRet = Run(lpstrCmdLine, nCmdShow);

	_Module.Term();
	::CoUninitialize();

	return nRet;
}

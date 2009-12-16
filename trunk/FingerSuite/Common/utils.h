#ifndef UTILS_H
#define UTILS_H

#pragma once

//#include "resource.h"
#include <snapi.h>
#include <regext.h>

#include "log\logger.h"

#ifndef __ATLMISC_H__
	#error utils.h requires atlmisc.h to be included first
#endif

template <class T>
class CStringEqualHelper
{
public:
	static bool IsEqual(const T& t1, const T& t2)
	{
		if (t1.Compare(t2) == 0)
			return true;
		else
			return false;
	}
};

//#ifdef __cplusplus
//extern "C" {
//#endif

inline CString ErrorString(DWORD err)
{
	CString Error;
	LPTSTR s;
	if(::FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		err,
		0,
		(LPTSTR)&s,
		0,
		NULL) == 0)
	{ /* failed */
		// Unknown error code %08x (%d)
		CString fmt = L"Unknown error 0x%08x (%d)\n";
		CString t;
		t.Format(fmt, err, LOWORD(err));
		Error = t;
	} /* failed */
	else
	{ /* success */
		LPTSTR p = _tcschr(s, _T('\r'));
		if(p != NULL)
		{ /* lose CRLF */
			*p = _T('\0');
		} /* lose CRLF */
		Error = s;
		::LocalFree(s);
	} /* success */
	return Error;
} // ErrorString


inline HBITMAP LoadImageFile( LPCTSTR szFileName )
{
	CString szImage = szFileName;
	CBitmapHandle hBmp = szImage.Find( L".bmp") != -1 ? 
		::SHLoadDIBitmap( szFileName) : ::SHLoadImageFile( szFileName);

	if( hBmp.IsNull())
	{
		wprintf(L"Cannot load image from: %s\n", szFileName);
	}
	return hBmp;
}	


inline int ParseTokens( CSimpleArray<CString>& result, CString szString, CString szTokens = ",;" )
{
	int iNum = 0;

	int iCurrPos= 0;
	CString subString;

	while( -1 != ( iCurrPos = szString.FindOneOf( szTokens ) ) )
	{
		iNum++;
		result.Add( szString.Left( iCurrPos ) );
		szString = szString.Right( szString.GetLength() - iCurrPos - 1 );
	}

	if ( szString.GetLength() > 0 )
	{
		// the last one...
		iNum++;
		result.Add( szString );
	}

	return iNum;
}



inline void DrawTransparent(HDC hdc, int x, int y, HBITMAP

					 hBitmap, COLORREF crColour)

{

	COLORREF crOldBack = SetBkColor(hdc, RGB(255, 255, 255));

	COLORREF crOldText = SetTextColor(hdc, RGB(0, 0, 0));

	HDC dcImage, dcTrans;


	// Create two memory dcs for the image and the mask

	dcImage=CreateCompatibleDC(hdc);

	dcTrans=CreateCompatibleDC(hdc);


	// Select the image into the appropriate dc

	HBITMAP pOldBitmapImage = (HBITMAP)SelectObject(dcImage, hBitmap);


	// Create the mask bitmap

	BITMAP bitmap;

	GetObject(hBitmap, sizeof(BITMAP), &bitmap);

	HBITMAP bitmapTrans=CreateBitmap(bitmap.bmWidth, bitmap.bmHeight, 1, 1, NULL);


	// Select the mask bitmap into the appropriate dc

	HBITMAP pOldBitmapTrans = (HBITMAP)SelectObject(dcTrans, bitmapTrans);


	// Build mask based on transparent colour

	SetBkColor(dcImage, crColour);

	BitBlt(dcTrans, 0, 0, bitmap.bmWidth, bitmap.bmHeight, dcImage, 0, 0, SRCCOPY);


	// Do the work - True Mask method - cool if not actual display

	BitBlt(hdc, x, y, bitmap.bmWidth, bitmap.bmHeight, dcImage, 0, 0, SRCINVERT);

	BitBlt(hdc, x, y, bitmap.bmWidth, bitmap.bmHeight, dcTrans, 0, 0, SRCAND);

	BitBlt(hdc, x, y, bitmap.bmWidth, bitmap.bmHeight, dcImage, 0, 0, SRCINVERT);


	// Restore settings

	SelectObject(dcImage, pOldBitmapImage);

	SelectObject(dcTrans, pOldBitmapTrans);

	SetBkColor(hdc, crOldBack);

	SetTextColor(hdc, crOldText);

}

/*
void FlipBitmap(CBitmap src)
{
	CDC dc; dc.CreateCompatibleDC(NULL);
	SIZE sz; src.GetSize(sz);
	CBitmap oldBmp = dc.SelectBitmap(src);
	dc.StretchBlt(0, 0, sz.cx, sz.cy, dc,  0,  sz.cy-1, sz.cx, -sz.cy, SRCCOPY );
	dc.SelectBitmap(oldBmp);
}
*/

inline BOOL RegReadDWORD(HKEY hKey, LPCTSTR lpszSubkey, LPCTSTR lpszName, DWORD& dwValue)
{
	BOOL bRet = TRUE;
	HKEY hConf;
	if (ERROR_SUCCESS == RegOpenKeyEx(hKey, 
								  lpszSubkey,
								  0,
								  0,
								  &hConf))
	{
		DWORD dwSize = sizeof(DWORD);
		DWORD dwType = REG_DWORD;
		DWORD dwNewValue;
		if (ERROR_SUCCESS == RegQueryValueEx(hConf, 
										lpszName,
										NULL,
										&dwType, 
										(LPBYTE)&dwNewValue,
										&dwSize))
		{
			// read dword 
			dwValue = dwNewValue;
		}
		else
			bRet = FALSE;
		RegCloseKey(hConf);
	}		
	else
		bRet = FALSE;
	return bRet;
}


inline BOOL RegReadBOOL(HKEY hKey, LPCTSTR lpszSubkey, LPCTSTR lpszName, BOOL& bValue)
{
	BOOL bRet = TRUE;
	HKEY hConf;
	if (ERROR_SUCCESS == RegOpenKeyEx(hKey, 
								  lpszSubkey,
								  0,
								  0,
								  &hConf))
	{
		DWORD dwSize = sizeof(DWORD);
		DWORD dwType = REG_DWORD;
		DWORD dwNewValue;
		if (ERROR_SUCCESS == RegQueryValueEx(hConf, 
										lpszName,
										NULL,
										&dwType, 
										(LPBYTE)&dwNewValue,
										&dwSize))
		{
			// read dword 
			bValue = (BOOL)dwNewValue;
		}
		else
			bRet = FALSE;
		RegCloseKey(hConf);
	}		
	else
		bRet = FALSE;
	return bRet;
}

inline BOOL RegReadColor(HKEY hKey, LPCTSTR lpszSubkey, LPCTSTR lpszName, COLORREF& clValue)
{
	BOOL bRet = TRUE;
	HKEY hConf;
	if (ERROR_SUCCESS == RegOpenKeyEx(hKey, 
								  lpszSubkey,
								  0,
								  0,
								  &hConf))
	{
		DWORD dwSize = 50;
		DWORD dwType = REG_SZ;
		BYTE data[50];
		ZeroMemory(data, 50);
		if (ERROR_SUCCESS == RegQueryValueEx(hConf, 
										lpszName,
										NULL,
										&dwType, 
										data,
										&dwSize))
		{
			// read sz 
			CString value = (WCHAR *)data;
			CSimpleArray<CString> colors;
			int n = ParseTokens(colors, value, " ");
			if (n == 3)
			{
				int r = _wtoi(  colors[0] );
				int g = _wtoi(  colors[1] );
				int b = _wtoi(  colors[2] );

				clValue = RGB(r,g,b);
			}
		}
		else
			bRet = FALSE;
		RegCloseKey(hConf);
	}		
	else
		bRet = FALSE;
	return bRet;
}


inline BOOL RegReadString(HKEY hKey, LPCTSTR lpszSubkey, LPCTSTR lpszName, LPWSTR szValue)
{
	BOOL bRet = TRUE;
	HKEY hConf;
	if (ERROR_SUCCESS == RegOpenKeyEx(hKey, 
								  lpszSubkey,
								  0,
								  0,
								  &hConf))
	{
		DWORD dwSize = MAX_PATH * 2;
		DWORD dwType = REG_SZ;
		BYTE data[MAX_PATH * 2];
		ZeroMemory(data, MAX_PATH * 2);
		if (ERROR_SUCCESS == RegQueryValueEx(hConf, 
										lpszName,
										NULL,
										&dwType, 
										data,
										&dwSize))
		{
			// read sz 
			lstrcpy(szValue, (WCHAR *)data);
		}
		else
			bRet = FALSE;
		RegCloseKey(hConf);
	}		
	else
		bRet = FALSE;
	return bRet;
}


inline void RegWriteColor(HKEY hKey, LPCTSTR lpszSubkey, LPCTSTR lpszName, COLORREF clValue)
{
	HKEY hConf;
	DWORD dwOptions = REG_OPTION_NON_VOLATILE;
	DWORD dwDisposition;
	if (ERROR_SUCCESS == RegCreateKeyEx(hKey, lpszSubkey, 0, NULL, dwOptions, 0, NULL, &hConf, &dwDisposition))
	{
		WCHAR buf[30];
		DWORD dwWrittenChar = wsprintf(buf, L"%d %d %d", GetRValue(clValue), GetGValue(clValue), GetBValue(clValue)) + 1;
		DWORD dwType = REG_SZ;
		RegSetValueEx(hConf, lpszName, 0, dwType, (BYTE*)buf, dwWrittenChar * sizeof(WCHAR));
		RegCloseKey(hConf);
	}
}

inline void RegWriteBOOL(HKEY hKey, LPCTSTR lpszSubkey, LPCTSTR lpszName, BOOL bValue)
{
	HKEY hConf;
	DWORD dwOptions = REG_OPTION_NON_VOLATILE;
	DWORD dwDisposition;
	if (ERROR_SUCCESS == RegCreateKeyEx(hKey, lpszSubkey, 0, NULL, dwOptions, 0, NULL, &hConf, &dwDisposition))
	{
		DWORD dwType = REG_DWORD;
		DWORD dwVal = (DWORD)bValue;
		RegSetValueEx(hConf, lpszName, 0, dwType, (CONST BYTE*)&dwVal, sizeof(DWORD));
		RegCloseKey(hConf);
	}
}

inline void RegWriteDWORD(HKEY hKey, LPCTSTR lpszSubkey, LPCTSTR lpszName, DWORD dwVal)
{
	HKEY hConf;
	DWORD dwOptions = REG_OPTION_NON_VOLATILE;
	DWORD dwDisposition;
	if (ERROR_SUCCESS == RegCreateKeyEx(hKey, lpszSubkey, 0, NULL, dwOptions, 0, NULL, &hConf, &dwDisposition))
	{
		DWORD dwType = REG_DWORD;
		RegSetValueEx(hConf, lpszName, 0, dwType, (CONST BYTE*)&dwVal, sizeof(DWORD));
		RegCloseKey(hConf);
	}
}

inline void RegWriteString(HKEY hKey, LPCTSTR lpszSubkey, LPCTSTR lpszName, LPCTSTR lpszValue, DWORD dwLength)
{
	HKEY hConf;
	DWORD dwOptions = REG_OPTION_NON_VOLATILE;
	DWORD dwDisposition;
	if (ERROR_SUCCESS == RegCreateKeyEx(hKey, lpszSubkey, 0, NULL, dwOptions, 0, NULL, &hConf, &dwDisposition))
	{
		DWORD dwType = REG_SZ;
		RegSetValueEx(hConf, lpszName, 0, dwType, (BYTE*)lpszValue, (dwLength + 1) * sizeof(WCHAR));
		RegCloseKey(hConf);
	}
}

inline void LoadExclusionList(CSimpleArray<CString, CStringEqualHelper<CString>> &list, LPTSTR pszKey)
{
	list.RemoveAll();

	HKEY hConf;
	if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, 
		                              pszKey, // L"Software\\FingerMenu",
									  0,
									  0,
									  &hConf))
	{
		long lRes;
		DWORD dwIndex = -1;
		CString szProcessName;
		CString szValueName;
		do
		{
			WCHAR valueName[1024];
			DWORD cchValueName = 1024;
			BYTE data[1024];
			DWORD dwSize = 1024;
			DWORD dwType = REG_SZ;		
			ZeroMemory(data, sizeof(data));
			ZeroMemory(valueName, sizeof(valueName));
			dwIndex ++;
			if (ERROR_SUCCESS == (lRes = RegEnumValue(hConf,
				                                      dwIndex,
													  valueName,
													  &cchValueName,
													  NULL,
													  &dwType,
													  data,
													  &dwSize) ))
			{
				szValueName = valueName;
				szProcessName = (PWCHAR)data;
				
				if ((dwType == REG_SZ) && (szValueName.Find(L"ExcludedApp", 0) == 0))
				{
					list.Add(szProcessName);
				}
			}
		} while (lRes == ERROR_SUCCESS);
	}
}

inline void SaveExclusionList(CSimpleArray<CString, CStringEqualHelper<CString>> &list, LPTSTR pszKey)
{
	//WCHAR szKeyName[] = L"Software\\FingerMenu";
	// delete all registry key
	HKEY hConf;
	if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, 
								  pszKey,
								  0,
								  0,
								  &hConf))
	{

		for (int i = 0; i < 200; i++)
		{
			CString name; name.Format(L"ExcludedApp%02d", i);
			RegDeleteValue(hConf, name);
		}
		RegCloseKey(hConf);
	}

	for (int i = 0; i < list.GetSize(); i++)
	{
		CString name; name.Format(L"ExcludedApp%02d", i);
		RegWriteString(HKEY_LOCAL_MACHINE, pszKey, name, list[i], list[i].GetLength());
	}
}


inline void LoadWndExclusionList(CSimpleArray<CString, CStringEqualHelper<CString>> &list, LPTSTR pszKey)
{
	list.RemoveAll();

	HKEY hConf;
	if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, 
		                              pszKey, // L"Software\\FingerMenu",
									  0,
									  0,
									  &hConf))
	{
		long lRes;
		DWORD dwIndex = -1;
		CString szProcessName;
		CString szValueName;
		do
		{
			WCHAR valueName[1024];
			DWORD cchValueName = 1024;
			BYTE data[1024];
			DWORD dwSize = 1024;
			DWORD dwType = REG_SZ;		
			ZeroMemory(data, sizeof(data));
			ZeroMemory(valueName, sizeof(valueName));
			dwIndex ++;
			if (ERROR_SUCCESS == (lRes = RegEnumValue(hConf,
				                                      dwIndex,
													  valueName,
													  &cchValueName,
													  NULL,
													  &dwType,
													  data,
													  &dwSize) ))
			{
				szValueName = valueName;
				szProcessName = (PWCHAR)data;
				
				if ((dwType == REG_SZ) && (szValueName.Find(L"ExcludedWnd", 0) == 0))
				{
					list.Add(szProcessName);
				}
			}
		} while (lRes == ERROR_SUCCESS);
	}
}

inline void SaveWndExclusionList(CSimpleArray<CString, CStringEqualHelper<CString>> &list, LPTSTR pszKey)
{
	//WCHAR szKeyName[] = L"Software\\FingerMenu";
	// delete all registry key
	HKEY hConf;
	if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, 
								  pszKey,
								  0,
								  0,
								  &hConf))
	{

		for (int i = 0; i < 200; i++)
		{
			CString name; name.Format(L"ExcludedWnd%02d", i);
			RegDeleteValue(hConf, name);
		}
		RegCloseKey(hConf);
	}

	for (int i = 0; i < list.GetSize(); i++)
	{
		CString name; name.Format(L"ExcludedWnd%02d", i);
		RegWriteString(HKEY_LOCAL_MACHINE, pszKey, name, list[i], list[i].GetLength());
	}
}


inline void SplitString(CString Source, CString Deliminator, CSimpleArray<CString>& AddIt, BOOL bAddEmpty)
{
	// initialize the variables
	CString		 newCString = Source;
	CString		 tmpCString = "";
	CString		 AddCString = "";

	int pos1 = 0;
	int pos = 0;

	if (Deliminator.IsEmpty()) {
		// Add default [comma] if empty!
		// acknowledgement: Guru Prasad [gprasad@rti.ie]
		Deliminator = ","; 
	}

	// do this loop as long as you have a deliminator
	do {
		// set to zero
		pos1 = 0;
		// position of deliminator starting at pos1 (0)
		pos = newCString.Find(Deliminator, pos1);
		// if the deliminator is found...
		if ( pos != -1 ) {
			// load a new var with the info left
			// of the position
			CString AddCString = newCString.Left(pos);
			if (!AddCString.IsEmpty()) {
				// if there is a string to add, then
				// add it to the Array
				AddIt.Add(AddCString);
			}
			else if (bAddEmpty) {
				// if empty strings are ok, then add them
				AddIt.Add(AddCString);
			}

			// make a copy of the of this var. with the info
			// right of the deliminator
			tmpCString = newCString.Mid(pos + Deliminator.GetLength());
			
			// reset this var with new info
			newCString = tmpCString;
		}
	} while ( pos != -1 );
	
	if (!newCString.IsEmpty()) {
		// as long as the variable is not emty, add it
		AddIt.Add(newCString);
	}

}

inline BOOL StringRGBToColor(LPCTSTR lpszColor, COLORREF &clValue)
{
	BOOL bRet = FALSE;

	if (lpszColor == NULL)
		return FALSE;

	CString value = lpszColor;
	CSimpleArray<CString> colors;
	int n = ParseTokens(colors, value, " ");
	if (n == 3)
	{
		int r = _wtoi(  colors[0] );
		int g = _wtoi(  colors[1] );
		int b = _wtoi(  colors[2] );

		bRet = TRUE;
		clValue = RGB(r,g,b);
	}
	return bRet;
}

inline BOOL StringToLogFont(LPCTSTR lpszFont, LOGFONT &lf)
{
	//HDC hDC = ::GetDC(0);
	BOOL bRet = TRUE;
	CDCHandle dc(::GetDC(0));

	CFont font = dc.GetCurrentFont();
	font.GetLogFont(lf);

	if (lpszFont == NULL)
	{
		return bRet;
	}
	
	CString value = lpszFont;
	CSimpleArray<CString> values;
	int n = ParseTokens(values, value, ",");

	//ZeroMemory(&lf, sizeof(LOGFONT));
	if (n >= 1)
	{

		// weight
		if (lstrcmpi(values[0], L"regular") == 0)
			lf.lfWeight = FW_NORMAL;
		if (lstrcmpi(values[0], L"italic") == 0)
		{
			lf.lfWeight = FW_NORMAL;
			lf.lfItalic = TRUE;
		}
		if (lstrcmpi(values[0], L"bold") == 0)
			lf.lfWeight = FW_BOLD;
		if (lstrcmpi(values[0], L"bold italic") == 0)
		{
			lf.lfWeight = FW_BOLD;
			lf.lfItalic = TRUE;
		}
	}

	if (n >= 2)
	{
		// height
		values[1].Replace(L"p", L" ");
		int l = _wtoi(values[1]);
		if (l > 0)
		{
			lf.lfHeight = -::MulDiv(l, dc.GetDeviceCaps(LOGPIXELSY), 72);
		}
	}


	// face
	if (n >= 3)
	{
		ZeroMemory(lf.lfFaceName, LF_FACESIZE);
		lstrcpy(lf.lfFaceName, values[2]);	
	}

	// charset
	/*
	if (lstrcmpi(values[3], L"ansi"        ) == 0) lf.lfCharSet = ANSI_CHARSET;
	if (lstrcmpi(values[3], L"baltic"      ) == 0) lf.lfCharSet = BALTIC_CHARSET;
	if (lstrcmpi(values[3], L"chinesebig5" ) == 0) lf.lfCharSet = CHINESEBIG5_CHARSET;
	if (lstrcmpi(values[3], L"default"     ) == 0) lf.lfCharSet = DEFAULT_CHARSET;
	if (lstrcmpi(values[3], L"easteurope"  ) == 0) lf.lfCharSet = EASTEUROPE_CHARSET;
	if (lstrcmpi(values[3], L"gb2312"      ) == 0) lf.lfCharSet = GB2312_CHARSET;
	if (lstrcmpi(values[3], L"greek"       ) == 0) lf.lfCharSet = GREEK_CHARSET;
	if (lstrcmpi(values[3], L"hangul"      ) == 0) lf.lfCharSet = HANGUL_CHARSET;
	if (lstrcmpi(values[3], L"mac"         ) == 0) lf.lfCharSet = MAC_CHARSET;
	if (lstrcmpi(values[3], L"oem"         ) == 0) lf.lfCharSet = OEM_CHARSET;
	if (lstrcmpi(values[3], L"russian"     ) == 0) lf.lfCharSet = RUSSIAN_CHARSET;
	if (lstrcmpi(values[3], L"shiftjis"    ) == 0) lf.lfCharSet = SHIFTJIS_CHARSET;
	if (lstrcmpi(values[3], L"symbol"      ) == 0) lf.lfCharSet = SYMBOL_CHARSET;
	if (lstrcmpi(values[3], L"turkish"     ) == 0) lf.lfCharSet = TURKISH_CHARSET;
	if (lstrcmpi(values[3], L"johab"       ) == 0) lf.lfCharSet = JOHAB_CHARSET;
	if (lstrcmpi(values[3], L"hebrew"      ) == 0) lf.lfCharSet = HEBREW_CHARSET;
	if (lstrcmpi(values[3], L"arabic"      ) == 0) lf.lfCharSet = ARABIC_CHARSET;
	if (lstrcmpi(values[3], L"thai"        ) == 0) lf.lfCharSet = THAI_CHARSET;
	*/
	return bRet;
}



inline LPWSTR LoadResourceString(UINT uID)
{

	LPWSTR lpOutput = new WCHAR[50];
	ZeroMemory(lpOutput, 50);
	int nCharCopied = ::LoadString(ModuleHelper::GetResourceInstance(), uID, lpOutput, 50);

	if (nCharCopied == 0)
		return NULL;

	return lpOutput;
}


inline BOOL IsDeviceLocked()
{
	BOOL bRes = FALSE;
	DWORD dwLockState = 0;
	RegistryGetDWORD(SN_LOCK_ROOT, SN_LOCK_PATH, SN_LOCK_VALUE, &dwLockState);
	if (dwLockState & SN_LOCK_BITMASK_KEYLOCKED)
		bRes = TRUE;

	return bRes;
}


// remote message
inline LPVOID PrepareRemoteMessage(LPHANDLE lphFile, DWORD dwBufferSize)
{
	LPVOID pViewMMFFile = NULL;

	*lphFile = ::CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, dwBufferSize + 1, L"SENDMESSAGEFILEMAP");              
	if ( *lphFile )
	{
		pViewMMFFile = MapViewOfFile(*lphFile, FILE_MAP_ALL_ACCESS, 0, 0, 0);                         
	}

	return pViewMMFFile;
}

inline void CloseRemoteMessage(HANDLE hFile, LPVOID pView)
{
	UnmapViewOfFile(pView);
	CloseHandle(hFile);
}


inline LRESULT SendMessageRemote(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, DWORD dwLParamSize = 0, UINT uTimeout = 0)
{
	LRESULT lResult = 0;
	DWORD dwResult;
	if (dwLParamSize == 0)
	{
		if (uTimeout == 0)
			lResult = ::SendMessage(hWnd, msg, wParam, lParam);
		else
			// TODO
			lResult = ::SendMessageTimeout(hWnd, msg, wParam, lParam, SMTO_NORMAL, uTimeout, &dwResult);
	}
	else
	{
		HANDLE hFileMMF = NULL;
		HANDLE pViewMMFFile = NULL;
		

		hFileMMF = ::CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, dwLParamSize + 1, L"SENDMESSAGEFILEMAP");              
		if ( hFileMMF )
		{
			pViewMMFFile = MapViewOfFile(hFileMMF, FILE_MAP_ALL_ACCESS, 0, 0, 0);                         
		}

		memcpy(pViewMMFFile, (LPVOID)lParam, dwLParamSize);

		if (uTimeout == 0)
			lResult = ::SendMessage(hWnd, msg, wParam, (LPARAM)pViewMMFFile);
		else
			// TODO
			lResult = ::SendMessageTimeout(hWnd, msg, wParam, (LPARAM)pViewMMFFile, SMTO_NORMAL, uTimeout, &dwResult); 

		UnmapViewOfFile(pViewMMFFile);
		CloseHandle(hFileMMF);

		hFileMMF = NULL;
		pViewMMFFile = NULL;
	}

	return lResult;
}

inline LRESULT CallRemoteWindowProc2(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, DWORD dwLParamSize = 0)
{
	LONG wndProc = ::GetWindowLong(hWnd, GWL_WNDPROC);
	LRESULT lResult = 0;

	if (dwLParamSize == 0)
	{
		lResult = ::CallWindowProc((WNDPROC)wndProc, hWnd, msg, wParam, lParam);
	}
	else
	{
		HANDLE hFileMMF = NULL;
		HANDLE pViewMMFFile = NULL;
		


		hFileMMF = ::CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, dwLParamSize + 1, L"SENDMESSAGEFILEMAP");              
		if ( hFileMMF )
		{
			pViewMMFFile = MapViewOfFile(hFileMMF, FILE_MAP_ALL_ACCESS, 0, 0, 0);                         
		}

		memcpy(pViewMMFFile, (LPVOID)lParam, dwLParamSize);

		lResult = ::CallWindowProc((WNDPROC)wndProc, hWnd, msg, wParam, (LPARAM)pViewMMFFile); 

		UnmapViewOfFile(pViewMMFFile);
		CloseHandle(hFileMMF);
	}
	return lResult;
}

//#ifdef __cplusplus
//}
//#endif

#endif //UTILS_H

#ifndef __ATLCEPLUS_H__
#define __ATLCEPLUS_H__

#pragma once

#ifndef __cplusplus
   #error ATL requires C++ compilation (use a .cpp suffix)
#endif

#ifndef __ATLAPP_H__
   #error atlceplus.h requires atlapp.h to be included first
#endif


/////////////////////////////////////////////////////////////////////////////
// Classes in this file
//
// CCompression
// CFindFile
// CFindProjectFile
// CFindFlashCard
//


/////////////////////////////////////////////////////////////////////////////
// CCompression - undocumented COREDLL compression methods

class CCompression
{
public:
   DWORD Compress(LPBYTE bufin, DWORD lenin, LPBYTE bufout, DWORD lenout)
   {
      typedef DWORD (WINAPI *PFNBINARYCOMPRESS)(LPBYTE, DWORD, LPBYTE, DWORD);
      PFNBINARYCOMPRESS lpf = 
         (PFNBINARYCOMPRESS) ::GetProcAddress(::GetModuleHandle(_T("COREDLL.DLL")), _T("BinaryCompress"));
      if( lpf == NULL ) return 0;
      return lpf(bufin, lenin, bufout, lenout);
   }
   DWORD Decompress(LPBYTE bufin, DWORD lenin, LPBYTE bufout, DWORD lenout, DWORD skip = 0)
   {
      typedef DWORD (WINAPI *PFNBINARYDECOMPRESS)(LPBYTE, DWORD, LPBYTE, DWORD, DWORD);
      PFNBINARYDECOMPRESS lpf = 
         (PFNBINARYDECOMPRESS) ::GetProcAddress(::GetModuleHandle(_T("COREDLL.DLL")), _T("BinaryDecompress"));
      if( lpf == NULL ) return 0;
      return lpf(bufin, lenin, bufout, lenout, skip);
   }
};


/////////////////////////////////////////////////////////////////////////////
// CFindFile - file search helper class

#include <projects.h>
#pragma comment(lib, "note_prj.lib")


class ATL_NO_VTABLE CFindFileBase
{
public:
   // Data members

   WIN32_FIND_DATA m_fd;
   TCHAR m_lpszRoot[MAX_PATH];
   TCHAR m_chDirSeparator;
   HANDLE m_hFind;
   BOOL m_bFound;

   // Constructor/destructor

   CFindFileBase() : m_hFind(NULL), m_chDirSeparator('\\'), m_bFound(FALSE)
   { 
   }
   ~CFindFileBase()
   {
      Close();
   }

   // Operations

   void Close()
   {
      m_bFound = FALSE;
      if( m_hFind != NULL && m_hFind != INVALID_HANDLE_VALUE ) {
         ::FindClose(m_hFind);
         m_hFind = NULL;
      }
   }

   // Attributes

   ULONGLONG GetFileSize() const
   {
      ATLASSERT(m_hFind!=NULL);
      ULARGE_INTEGER nFileSize;
      if( m_bFound ) {
         nFileSize.LowPart = m_fd.nFileSizeLow;
         nFileSize.HighPart = m_fd.nFileSizeHigh;
      }
      else {
         nFileSize.QuadPart = 0;
      }
      return nFileSize.QuadPart;
   }
   BOOL GetFileName(LPTSTR lpstrFileName, int cchLength) const
   {
      ATLASSERT(m_hFind!=NULL);
      if( (int) lstrlen(m_fd.cFileName) >= cchLength ) return FALSE;
      return (m_bFound && (::lstrcpy(lpstrFileName, m_fd.cFileName) != NULL));
   }
   BOOL GetFilePath(LPTSTR lpstrFilePath, int cchLength) const
   {
      ATLASSERT(m_hFind!=NULL);
      int nLen = lstrlen(m_lpszRoot);
      ATLASSERT(nLen > 0);
      if( nLen == 0 ) return FALSE;
      bool bAddSep = (m_lpszRoot[nLen - 1] != '\\' && m_lpszRoot[nLen - 1] != '/');
      if( ((int) lstrlen(m_lpszRoot) + (bAddSep ?  1 : 0)) >= cchLength ) return FALSE;
      BOOL bRet = (::lstrcpy(lpstrFilePath, m_lpszRoot) != NULL);
      if( bRet ) {
         TCHAR szSeparator[2] = { m_chDirSeparator, 0 };
         bRet = (::lstrcat(lpstrFilePath, szSeparator) != NULL);
         if( bRet ) bRet = (::lstrcat(lpstrFilePath, m_fd.cFileName) != NULL);
      }
      return bRet;
   }
   BOOL GetRoot(LPTSTR lpstrRoot, int cchLength) const
   {
      ATLASSERT(m_hFind!=NULL);
      if( (int) lstrlen(m_lpszRoot) >= cchLength ) return FALSE;
      return (::lstrcpy(lpstrRoot, m_lpszRoot) != NULL);
   }
#if defined(_WTL_USE_CSTRING) || defined(__ATLSTR_H__)
   CString GetFileName() const
   {
      ATLASSERT(m_hFind!=NULL);
      CString ret;
      if( m_bFound ) ret = m_fd.cFileName;
      return ret;
   }
   CString GetFilePath() const
   {
      ATLASSERT(m_hFind!=NULL);
      CString strResult = m_lpszRoot;
      if( strResult[strResult.GetLength() - 1] != '\\' &&
         strResult[strResult.GetLength() - 1] != '/' )
         strResult += m_chDirSeparator;
      strResult += GetFileName();
      return strResult;
   }
   CString GetRoot() const
   {
      ATLASSERT(m_hFind!=NULL);
      return m_lpszRoot;
   }
#endif //defined(_WTL_USE_CSTRING) || defined(__ATLSTR_H__)
   BOOL GetLastWriteTime(FILETIME* pTimeStamp) const
   {
      ATLASSERT(m_hFind!=NULL);
      ATLASSERT(pTimeStamp != NULL);
      if( m_bFound && pTimeStamp != NULL ) {
         *pTimeStamp = m_fd.ftLastWriteTime;
         return TRUE;
      }
      return FALSE;
   }
   BOOL GetLastAccessTime(FILETIME* pTimeStamp) const
   {
      ATLASSERT(m_hFind!=NULL);
      ATLASSERT(pTimeStamp != NULL);
      if( m_bFound && pTimeStamp != NULL ) {
         *pTimeStamp = m_fd.ftLastAccessTime;
         return TRUE;
      }
      return FALSE;
   }
   BOOL GetCreationTime(FILETIME* pTimeStamp) const
   {
      ATLASSERT(m_hFind!=NULL);
      if( m_bFound && pTimeStamp != NULL ) {
         *pTimeStamp = m_fd.ftCreationTime;
         return TRUE;
      }
      return FALSE;
   }
   BOOL MatchesMask(DWORD dwMask) const
   {
      ATLASSERT(m_hFind!=NULL);
      if( m_bFound ) return ((m_fd.dwFileAttributes & dwMask) != 0);
      return FALSE;
   }
   BOOL IsDots() const
   {
      ATLASSERT(m_hFind!=NULL);
      // return TRUE if the file name is "." or ".." and
      // the file is a directory
      BOOL bResult = FALSE;
      if( m_bFound && IsDirectory() ) {
         if( m_fd.cFileName[0] == '.' 
             && (m_fd.cFileName[1] == '\0' 
             || (m_fd.cFileName[1] == '.' && m_fd.cFileName[2] == '\0')) )
         {
            bResult = TRUE;
         }
      }
      return bResult;
   }

   BOOL IsReadOnly() const
   {
      return MatchesMask(FILE_ATTRIBUTE_READONLY);
   }
   BOOL IsDirectory() const
   {
      return MatchesMask(FILE_ATTRIBUTE_DIRECTORY);
   }
   BOOL IsCompressed() const
   {
      return MatchesMask(FILE_ATTRIBUTE_COMPRESSED);
   }
   BOOL IsSystem() const
   {
      return MatchesMask(FILE_ATTRIBUTE_SYSTEM);
   }
   BOOL IsHidden() const
   {
      return MatchesMask(FILE_ATTRIBUTE_HIDDEN);
   }
   BOOL IsTemporary() const
   {
      return MatchesMask(FILE_ATTRIBUTE_TEMPORARY);
   }
   BOOL IsNormal() const
   {
      return MatchesMask(FILE_ATTRIBUTE_NORMAL);
   }
   BOOL IsArchived() const
   {
      return MatchesMask(FILE_ATTRIBUTE_ARCHIVE);
   }
};


class CFindFile : public CFindFileBase
{
public:
   // Operations

   BOOL FindFile(LPCTSTR pstrPattern = NULL)
   {
      Close();
      if( pstrPattern == NULL ) pstrPattern = _T("*.*");
      ::lstrcpy(m_fd.cFileName, pstrPattern);
      m_hFind = ::FindFirstFile(pstrPattern, &m_fd);
      if( m_hFind == INVALID_HANDLE_VALUE ) return FALSE;
      ::ZeroMemory(m_lpszRoot, sizeof(m_lpszRoot));
      m_lpszRoot[0] = m_chDirSeparator;
      LPCTSTR pstr = NULL;
      LPCTSTR p = pstrPattern;
      while( *p ) {
         if( *p == m_chDirSeparator ) pstr = p;
         p = ::CharNext(p);
      }
      if( pstr ) ::lstrcpyn(m_lpszRoot, pstrPattern, pstr - pstrPattern + 1);
      m_bFound = TRUE;
      return TRUE;
   }
   BOOL FindNextFile()
   {
      ATLASSERT(m_hFind!=NULL);
      if( m_hFind == NULL ) return FALSE;
      if( !m_bFound ) return FALSE;
      m_bFound = ::FindNextFile(m_hFind, &m_fd);
      return m_bFound;
   }
};


class CFindProjectFile : public CFindFileBase
{
public:
   // Operations

   BOOL FindFile(LPCTSTR pstrPattern = NULL, DWORD dwOidFlash = 0, LPTSTR lpszProj = NULL)
   {
      Close();
      if( pstrPattern == NULL ) pstrPattern = _T("*.*");
      ::lstrcpy(m_fd.cFileName, pstrPattern);
      m_hFind = ::FindFirstProjectFile(pstrPattern, &m_fd, dwOidFlash, lpszProj);
      if( m_hFind == INVALID_HANDLE_VALUE ) return FALSE;
      ::ZeroMemory(m_lpszRoot, sizeof(m_lpszRoot));
      LPCTSTR pstr = NULL;
      LPCTSTR p = pstrPattern;
      while( *p ) {
         if( *p == m_chDirSeparator ) pstr = p;
         p = ::CharNext(p);
      }
      if( pstr ) ::lstrcpyn(m_lpszRoot, pstrPattern, pstr - pstrPattern); else ::lstrcpy(m_lpszRoot, _T("\\"));
      m_bFound = TRUE;
      return TRUE;
   }
   BOOL FindNextFile()
   {
      ATLASSERT(m_hFind!=NULL);
      if( m_hFind == NULL ) return FALSE;
      if( !m_bFound ) return FALSE;
      m_bFound = ::FindNextProjectFile(m_hFind, &m_fd);
      return m_bFound;
   }
};


class CFindFlashCard : public CFindFileBase
{
public:
   // Operations

   BOOL FindFile()
   {
      Close();
      m_hFind = ::FindFirstFlashCard(&m_fd);
      if( m_hFind == INVALID_HANDLE_VALUE ) return FALSE;
      ::ZeroMemory(m_lpszRoot, sizeof(m_lpszRoot));
      m_lpszRoot[0] = m_chDirSeparator;
      m_bFound = TRUE;
      return TRUE;
   }
   BOOL FindNextFile()
   {
      ATLASSERT(m_hFind!=NULL);
      if( m_hFind == NULL ) return FALSE;
      if( !m_bFound ) return FALSE;
      m_bFound = ::FindNextFlashCard(m_hFind, &m_fd);
      return m_bFound;
   }
};


#endif // __ATLCEPLUS_H__

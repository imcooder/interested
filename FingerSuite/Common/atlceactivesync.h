#if !defined(AFX_ATLCEACTIVESYNC_H__20040110_01FC_0634_DA35_0080AD509054__INCLUDED_)
#define AFX_ATLCEACTIVESYNC_H__20040110_01FC_0634_DA35_0080AD509054__INCLUDED_

/////////////////////////////////////////////////////////////////////////////
// atlceactivesync - Active Sync for Win CE implementation
//
// Written by Bjarke Viksoe (bjarke@viksoe.dk)
// Copyright (c) 2004 Bjarke Viksoe.
//
// This code may be used in compiled form in any way you desire. This
// file may be redistributed by any means PROVIDING it is 
// not sold for profit without the authors written consent, and 
// providing that this notice and the authors name is included. 
//
// This file is provided "as is" with no expressed or implied warranty.
// The author accepts no liability if it causes any damage to you or your
// computer whatsoever. It's free, so don't hassle me about it.
//
// Beware of bugs.
//

#pragma once

#include <cesync.h>


/////////////////////////////////////////////////////////////////////////////
// Macros

#ifndef _ASSERTE
   #define _ASSERTE(x)
#endif


#define PROP_TAG(ulPropType, ulPropID)    ((((ULONG)(ulPropID))<<16)|((ULONG)(ulPropType)))
#define PROP_TYPE(ulPropTag)              (((ULONG)(ulPropTag))&0x0000FFFF)


#define GET_STREAM_WORD(x)      x = * (LPWORD) pData; pData += sizeof(WORD); cbData -= sizeof(WORD)
#define GET_STREAM_DWORD(x)     x = * (LPDWORD) pData; pData += sizeof(DWORD); cbData -= sizeof(DWORD)
#define GET_STREAM_BYTES(x, n)  memcpy(x, pData, n); pData += (n); cbData -= (n)

#define PUT_STREAM_WORD(x)      * (LPWORD) pData = x; pData += sizeof(WORD); cbData += sizeof(WORD)
#define PUT_STREAM_DWORD(x)     * (LPDWORD) pData = x; pData += sizeof(DWORD); cbData += sizeof(DWORD)
#define PUT_STREAM_BYTES(x, n)  memcpy(pData, x, n); pData += (n); cbData += (n)


/////////////////////////////////////////////////////////////////////////////
// Debug Helpers

#ifdef DEBUG

inline void _cdecl Log(LPCWSTR lpszFormat, ...)
{
   // Format text
   va_list args;
   va_start(args, lpszFormat);
   WCHAR szBuffer[256] = { 0 };
   int nBuf = _vsnwprintf(szBuffer, (sizeof(szBuffer) / sizeof(WCHAR)) - 1, lpszFormat, args);
   va_end(args);
   // A truely crappy semaphore
   static volatile bool s_bLocked = false;
   DWORD dwTick = ::GetTickCount();
   while( s_bLocked && ::GetTickCount() - dwTick < 1000UL ) ::Sleep(0L);
   s_bLocked = true;
   // Append to file
   HANDLE hFile = ::CreateFile(L"\\sync.log", GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
   if( hFile != INVALID_HANDLE_VALUE ) {
      ::SetFilePointer(hFile, 0, NULL, FILE_END);
      DWORD dwWritten = 0;
      ::WriteFile(hFile, szBuffer, nBuf * sizeof(WCHAR), &dwWritten, NULL);
      ::FlushFileBuffers(hFile);
      ::CloseHandle(hFile);
   }
   s_bLocked = false;
}

#else

inline void _cdecl LogStub(LPCWSTR lpszFormat, ...) { };
#define Log 1 ? (void)0 : LogStub

#endif

/////////////////////////////////////////////////////////////////////////////
// Base Folder

class CReplFolder
{
public:
   CReplFolder(){};
   virtual ~CReplFolder() { };

public:
   virtual LPCWSTR GetName() const = 0;
   virtual BOOL Initialize(IReplObjHandler** ppObjHandler, UINT uPartnerBit) = 0;
   virtual BOOL UnInitialize() = 0;
   virtual HRESULT FindObjects(PFINDOBJINFO pFind) = 0;
   virtual BOOL ObjectNotify(POBJNOTIFY pObjNotify) = 0;
   virtual BOOL GetObjTypeInfo(POBJTYPEINFO pInfo) = 0;
   virtual BOOL SyncData(PSDREQUEST psd) = 0;
};


/////////////////////////////////////////////////////////////////////////////
// Store Implementation

class CReplStore
{
public:
   enum { MAX_FOLDERS = 10 };

   CReplFolder* m_aFolders[MAX_FOLDERS];
   int m_nFolders;

   CReplStore() : m_nFolders(0)
   {
   }
   virtual ~CReplStore()
   {
      for( int i = 0; i < m_nFolders; i++ ) delete m_aFolders[i];
   }

   virtual BOOL InitObjType(LPWSTR lpszObjType, IReplObjHandler **ppObjHandler, UINT uPartnerBit)
   {
      Log(L"CReplStore::InitObjType (%s)\r\n", lpszObjType == NULL ? L"<null>" : lpszObjType);
      if( lpszObjType == NULL ) 
      {
         // Uninitialize
         BOOL fRet = TRUE;
         for( int i = 0; i < m_nFolders; i++ ) {
            if( !m_aFolders[i]->UnInitialize() ) fRet = FALSE;
         }
         return fRet;
      }
      else 
      {
         // Initialize
         *ppObjHandler = NULL;
         CReplFolder* pFolder = GetFolderByName(lpszObjType);
         if( pFolder ) return pFolder->Initialize(ppObjHandler, uPartnerBit);
         return FALSE;
      }
   }
   virtual HRESULT FindObjects(PFINDOBJINFO pFind)
   {
      Log(L"CReplStore::FindObjects\r\n");
      if( pFind == NULL ) return E_INVALIDARG;
      CReplFolder* pFolder = GetFolderByName(pFind->szObjType);
      if( pFolder ) return pFolder->FindObjects(pFind);
      return E_INVALIDARG;
   }
   virtual BOOL ObjectNotify(POBJNOTIFY pNotify)
   {
      Log(L"CReplStore::ObjectNotify\r\n");
      if( pNotify == NULL ) return FALSE;
      CReplFolder* pFolder = GetFolderByName(pNotify->szObjType);
      if( pFolder ) return pFolder->ObjectNotify(pNotify);
      return FALSE;
   }
   virtual BOOL GetObjTypeInfo(POBJTYPEINFO pInfo)
   {
      Log(L"CReplStore::GetObjTypeInfo\r\n");
      if( pInfo == NULL ) return FALSE;
      CReplFolder* pFolder = GetFolderByName(pInfo->szObjType);
      if( pFolder ) return pFolder->GetObjTypeInfo(pInfo);
      return FALSE;
   }
   virtual BOOL ReportStatus(LPWSTR lpszObjType, UINT uCode, UINT uParam)
   {
      Log(L"CReplStore::ReportStatus %04X\r\n", uCode);
      return TRUE;
   }
   virtual BOOL SyncData(PSDREQUEST psd)
   {
      Log(L"CReplStore::SyncData\r\n");
      CReplFolder* pFolder = GetFolderByName(psd->szObjType);
      if( pFolder ) return pFolder->SyncData(psd);
      return FALSE;
   }

   CReplFolder* GetFolderByName(LPWSTR wszName) const
   {
      for( int i = 0; i < m_nFolders; i++ ) {
         if( wcscmp(m_aFolders[i]->GetName(), wszName) == 0 ) return m_aFolders[i];
      }
      return NULL;
   }
};


/////////////////////////////////////////////////////////////////////////////
// IReplObjHandler

template< class T >
class IReplObjHandlerImpl : public IReplObjHandler
{
public:
   enum { MAX_DATASIZE = 6000 };

   IReplObjHandlerImpl() : m_cRef(1)
   {
   }

   // IUnknown

   STDMETHODIMP_(ULONG) AddRef()
   {
      return ::InterlockedIncrement(&m_cRef);
   }
   STDMETHODIMP_(ULONG) Release()
   {
      ULONG urc;
      if( (urc = ::InterlockedDecrement(&m_cRef)) == 0 ) delete this;
      return urc;
   }
   STDMETHODIMP QueryInterface(REFIID iid, LPVOID* ppvObj)
   {      
      *ppvObj = NULL;
      return E_NOINTERFACE;
   }

   // IReplObjHandler

   STDMETHODIMP Setup(PREPLSETUP pSetup)
   {
      Log(L"IReplObjHandlerImpl::Setup\r\n");
      if( pSetup->fRead ) {
         m_pReadSetup = pSetup; 
         m_dwReadPos = 0;
         m_pData = (LPBYTE) ::LocalAlloc(LMEM_FIXED, MAX_DATASIZE);
         if( m_pData == NULL ) return E_OUTOFMEMORY;
      }
      else {
         m_pWriteSetup = pSetup;
         m_dwWritePos = 0;
      }
      return NOERROR;
   }
   STDMETHODIMP Reset(PREPLSETUP pSetup)
   {
      Log(L"IReplObjHandlerImpl::Reset\r\n");
      if( pSetup->fRead ) ::LocalFree(m_pData);
      return NOERROR;
   }

   LONG m_cRef;
   PREPLSETUP m_pReadSetup;
   PREPLSETUP m_pWriteSetup;
   DWORD m_dwReadPos;
   DWORD m_dwWritePos;
   LPBYTE m_pData;
};


/////////////////////////////////////////////////////////////////////////////
// CCeDatabase

class CCeDatabase
{
public:
   CEGUID m_Guid;
   CEOID m_Oid;
   HANDLE m_Db;
   bool m_bManaged;

   CCeDatabase() : m_Db(INVALID_HANDLE_VALUE), m_bManaged(false)
   {
      CREATE_INVALIDGUID(&m_Guid);
   }
   CCeDatabase(CEGUID Guid) : m_Db(INVALID_HANDLE_VALUE), m_Guid(Guid), m_bManaged(false)
   {
   }
   ~CCeDatabase()
   {
      Close();
      if( m_bManaged ) Unmount();
   }
   BOOL Create(LPWSTR pstrVolume, UINT uFlags = CREATE_ALWAYS)
   {
      _ASSERTE(pstrVolume);
      _ASSERTE(CHECK_INVALIDGUID(&m_Guid));
      return ::CeMountDBVol(&m_Guid, pstrVolume, uFlags);
   }
   BOOL MountObjectStore()
   {
      _ASSERTE(CHECK_INVALIDGUID(&m_Guid));
      CREATE_SYSTEMGUID(&m_Guid);
      m_bManaged = true;
      return TRUE;
   }
   BOOL Mount(LPWSTR pstrVolume)
   {
      _ASSERTE(pstrVolume);
      _ASSERTE(CHECK_INVALIDGUID(&m_Guid));
      BOOL bRes = ::CeMountDBVol(&m_Guid, pstrVolume, OPEN_EXISTING);
      m_bManaged = true;
      return bRes;
   }
   void Unmount()
   {
      if( CHECK_INVALIDGUID(&m_Guid) ) return;
      if( !CHECK_SYSTEMGUID(&m_Guid) ) ::CeUnmountDBVol(&m_Guid);
      CREATE_INVALIDGUID(&m_Guid);
      m_bManaged = false;
   }
   CEOID FindDatabase(LPWSTR pstrFile, DWORD dwType)
   {
      _ASSERTE(!CHECK_INVALIDGUID(&m_Guid));
      // Validate we're mounted
      if( CHECK_INVALIDGUID(&m_Guid) ) return 0;
      // First search for the database
      CEOID oid = 0;
      HANDLE hFind = ::CeFindFirstDatabaseEx(&m_Guid, dwType);
      if( hFind != INVALID_HANDLE_VALUE ) {
         while( true ) {
            CEOIDINFO oidInfo;
            oid = ::CeFindNextDatabase(hFind);   
            if( oid == 0
                || (::CeOidGetInfoEx(&m_Guid, oid, &oidInfo) 
                    && oidInfo.wObjType == OBJTYPE_DATABASE 
                    && wcscmp(oidInfo.infDatabase.szDbaseName, pstrFile) == 0) ) 
            {
               break;
            }
         }
         ::CloseHandle(hFind);
      }
      return oid;
   }
   BOOL Open(LPWSTR pstrFile, DWORD dwType, BOOL bCreateIfNotExists = TRUE)
   {
      _ASSERTE(!CHECK_INVALIDGUID(&m_Guid));
      // Validate we're mounted
      if( CHECK_INVALIDGUID(&m_Guid) ) return FALSE;
      // First search for the database
      CEOID oid = FindDatabase(pstrFile, dwType);
      // If not found, create a new database
      if( bCreateIfNotExists && oid == 0 ) {
         CEDBASEINFO info = { 0 };
         info.dwFlags = CEDB_VALIDNAME | CEDB_VALIDTYPE;
         ::lstrcpy(info.szDbaseName, pstrFile);
         info.dwDbaseType = dwType;
         oid = ::CeCreateDatabaseEx(&m_Guid, &info);
      }
      // Finally open the database
      if( oid ) {
         m_Db = ::CeOpenDatabaseEx(&m_Guid, &oid, NULL, NULL, 0, NULL);
         m_Oid = oid;
      }
      return m_Oid != 0 && m_Db != INVALID_HANDLE_VALUE;
   }
   BOOL Open(LPWSTR pstrFile, DWORD dwType, CEPROPID Prop, DWORD dwFlags = CEDB_AUTOINCREMENT, CENOTIFYREQUEST* pReq = NULL)
   {
      _ASSERTE(!CHECK_INVALIDGUID(&m_Guid));
      // Validate we're mounted
      if( CHECK_INVALIDGUID(&m_Guid) ) return FALSE;
      // First search for the database
      CEOID oid = FindDatabase(pstrFile, dwType);
      // Open the database
      if( oid ) {
         m_Db = ::CeOpenDatabaseEx(&m_Guid, &oid, NULL, Prop, dwFlags, pReq);
         m_Oid = oid;
      }
      return m_Oid != 0 && m_Db != INVALID_HANDLE_VALUE;
   }
   void Close()
   {
      if( m_Db != INVALID_HANDLE_VALUE ) {
         ::CloseHandle(m_Db);
         m_Db = INVALID_HANDLE_VALUE;
      }
   }
   BOOL DeleteDatabase()
   {
      _ASSERTE(!IsOpen());
      _ASSERTE(!CHECK_INVALIDGUID(&m_Guid));
      // Use Open() before calling this!
      Close();
      return ::CeDeleteDatabaseEx(&m_Guid, m_Oid);
   }
   BOOL IsOpen() const
   {
      return m_Db != INVALID_HANDLE_VALUE;
   }
   BOOL IsMounted() const
   {
      return CHECK_INVALIDGUID(&m_Guid);
   }
   BOOL IsVolume(CEGUID* pData, UINT cbData) const
   {
      // Compare data with GUID structure
      if( CHECK_SYSTEMGUID(&m_Guid) ) return pData == NULL || (cbData == sizeof(CEGUID) && CHECK_SYSTEMGUID(pData));
      return cbData == sizeof(CEGUID) && memcmp(pData, &m_Guid, sizeof(CEGUID)) == 0;
   }

   BOOL GetInfoEx(CEOIDINFO* oidInfo)
   {
      _ASSERTE(IsOpen());
      return ::CeOidGetInfoEx(&m_Guid, m_Oid, oidInfo);
   }

   DWORD Seek(DWORD dwType, DWORD dwValue, DWORD* pdwIndex = NULL)
   {
      _ASSERTE(IsOpen());
      DWORD dwTempIndex = 0;
      if( pdwIndex == NULL ) pdwIndex = &dwTempIndex;
      return ::CeSeekDatabase(m_Db, dwType, dwValue, pdwIndex);
   }
   DWORD SeekMatch(DWORD dwType, const CEPROPVAL* pValue, DWORD* pdwIndex = NULL)
   {
      return Seek(dwType, (DWORD) pValue, pdwIndex);
   }
   DWORD SeekOid(CEOID oid, DWORD* pdwIndex = NULL)
   {
      return Seek(CEDB_SEEK_CEOID, (DWORD) oid, pdwIndex);
   }
   DWORD SeekNext(DWORD* pdwIndex = NULL)
   {
      return Seek(CEDB_SEEK_CURRENT, 1, pdwIndex);
   }
   DWORD ReadRecord(LPBYTE* lplpBuffer, LPDWORD lpcbBuffer, LPWORD lpcPropID, CEPROPID* rgPropID = NULL, DWORD dwFlags = CEDB_ALLOWREALLOC)
   {
      _ASSERTE(IsOpen());
      _ASSERTE(lplpBuffer);
      _ASSERTE(lpcbBuffer);
      return ::CeReadRecordProps(m_Db, dwFlags, lpcPropID, rgPropID, lplpBuffer, lpcbBuffer);
   }
   DWORD ReadRecord(CEPROPVAL** lplpBuffer, LPDWORD lpcbBuffer, LPWORD lpcPropID, CEPROPID* rgPropID = NULL, DWORD dwFlags = CEDB_ALLOWREALLOC)
   {
      _ASSERTE(IsOpen());
      _ASSERTE(lplpBuffer);
      _ASSERTE(lpcbBuffer);
      return ::CeReadRecordProps(m_Db, dwFlags, lpcPropID, rgPropID, (LPBYTE*) lplpBuffer, lpcbBuffer);
   }
   DWORD UpdateRecord(CEOID oid, WORD cPropID, CEPROPVAL* rgPropVal)
   {
      _ASSERTE(IsOpen());
      _ASSERTE(rgPropVal);
      return ::CeWriteRecordProps(m_Db, oid, cPropID, rgPropVal);
   }
   DWORD CreateRecord(WORD cPropID, CEPROPVAL* rgPropVal)
   {
      _ASSERTE(IsOpen());
      _ASSERTE(rgPropVal);
      return ::CeWriteRecordProps(m_Db, 0, cPropID, rgPropVal);
   }
   BOOL DeleteRecord(CEOID oid)
   {
      _ASSERTE(IsOpen());
      return ::CeDeleteRecord(m_Db, oid);
   }

   operator HANDLE() const
   {
      return m_Db;
   }
   operator CEGUID() const
   {
      return m_Guid;
   }
   operator CEOID() const
   {
      return m_Oid;
   }
};


#endif // !defined(AFX_ATLCEACTIVESYNC_H__20040110_01FC_0634_DA35_0080AD509054__INCLUDED_)

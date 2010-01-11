
#include "stdafx.h"
#include "../Include/BlobStream.h"


CBlobStream::CBlobStream()
:	m_pBuffer	(NULL),
	m_nLength	(0),
	m_iPosRead	(0),
	m_iPosWrite	(0),
	m_nRef		(0),
	m_dbStatus	(DBSTATUS_S_OK)
{
	AddRef();
}


CBlobStream::~CBlobStream()
{
	Clear();
}


// CBlobStream::Clear
//
//		Clears the buffer
//
void CBlobStream::Clear()
{
	CoTaskMemFree(m_pBuffer);
	m_pBuffer = NULL;

	m_nLength	= 0;
	m_iPosRead	= 0;
	m_iPosWrite	= 0;
	m_dbStatus	= DBSTATUS_S_OK;
}


ULONG CBlobStream::AddRef()
{
	return ++m_nRef;
}


// CBlobStream::Release
//
//		Deletes the object when the reference count reaches zero
//
ULONG CBlobStream::Release()
{
	if(m_nRef)
	{
		--m_nRef;
		if(m_nRef == 0)
			delete this;
	}
	return m_nRef;
}


HRESULT CBlobStream::QueryInterface(REFIID riid, void** ppv)
{
	*ppv = NULL;

	if(riid == IID_IUnknown)
		*ppv = this;

	if(riid == IID_ISequentialStream)
		*ppv = this;

	if(*ppv)
	{
		((IUnknown*) *ppv)->AddRef();

		return S_OK;
	}

	return E_NOINTERFACE;
}


// CBlobStream::Read
//
//		Reads data from the BLOB stream
//
HRESULT CBlobStream::Read(void *pv, ULONG cb, ULONG* pcbRead)
{
	//
	// Check parameters.
	//
	if(pcbRead)
		*pcbRead = 0;

	if(!pv)
		return STG_E_INVALIDPOINTER;

	if(0 == cb)
		return S_OK; 

	//
	// Calculate bytes left and bytes to read.
	//
	ULONG cBytesLeft = m_nLength - m_iPosRead;
	ULONG cBytesRead = cb > cBytesLeft ? cBytesLeft : cb;

	//
	// Return bytes read to caller.
	//
	if(pcbRead)
		*pcbRead = cBytesRead;

	//
	// If no more bytes to retrive return S_FALSE.
	//
	if(0 == cBytesLeft)
		return S_OK;

	//
	// Copy to users buffer the number of bytes requested or remaining
	//
	memcpy(pv, (void*)(m_pBuffer + m_iPosRead), cBytesRead);
	m_iPosRead += cBytesRead;

	if(cb != cBytesRead)
		return S_FALSE; 

	return S_OK;
}


// CBlobStream::Write
//
//		Writes data to the BLOB stream
//
HRESULT CBlobStream::Write(const void *pv, ULONG cb, ULONG* pcbWritten)
{
	//
	// Check parameters.
	//
	if(!pv)
		return STG_E_INVALIDPOINTER;

	if(pcbWritten)
		*pcbWritten = 0;

	if(0 == cb)
		return S_OK;

	//
	// Enlarge the current buffer.
	//
	m_nLength += cb;

	//
	// Grow internal buffer to new size.
	//
	m_pBuffer = (BYTE*)CoTaskMemRealloc(m_pBuffer, m_nLength);

	//
	// Check for out of memory situation.
	//
	if(NULL == m_pBuffer) 
	{
		Clear();
		return E_OUTOFMEMORY;
	}

	//
	// Copy callers memory to internal bufffer and update write position.
	//
	memcpy((void*)(m_pBuffer + m_iPosWrite), pv, cb );
	m_iPosWrite += cb;

	//
	// Return bytes written to caller.
	//
	if(pcbWritten)
		*pcbWritten = cb;

	return S_OK;
}


// CBlobStream::WriteFromStream
//
//		Writes data to the blob stream from an open IStream
//
HRESULT CBlobStream::WriteFromStream(IStream *pStream, ULONG cb, ULONG *pcbWritten)
{
	HRESULT	hr;

	//
	// Check parameters.
	//
	if(!pStream)
		return STG_E_INVALIDPOINTER;

	if(pcbWritten)
		*pcbWritten = 0;

	if(0 == cb)
		return S_OK;

	//
	// Enlarge the current buffer.
	//
	m_nLength += cb;

	//
	// Grow internal buffer to new size.
	//
	m_pBuffer = (BYTE*)CoTaskMemRealloc(m_pBuffer, m_nLength);

	//
	// Check for out of memory situation.
	//
	if(NULL == m_pBuffer) 
	{
		Clear();
		return E_OUTOFMEMORY;
	}

	hr = pStream->Read((void*)(m_pBuffer + m_iPosWrite), cb, pcbWritten);
	if(SUCCEEDED(hr))
		m_iPosWrite += cb;

	return hr;
}


// CBlobStream::WriteFromStream
//
//		Writes data to the blob stream from an open ISequentialStream
//
HRESULT CBlobStream::WriteFromStream(ISequentialStream *pStream, ULONG cb, ULONG *pcbWritten)
{
	HRESULT	hr;

	//
	// Check parameters.
	//
	if(!pStream)
		return STG_E_INVALIDPOINTER;

	if(pcbWritten)
		*pcbWritten = 0;

	if(0 == cb)
		return S_OK;

	//
	// Enlarge the current buffer.
	//
	m_nLength += cb;

	//
	// Grow internal buffer to new size.
	//
	m_pBuffer = (BYTE*)CoTaskMemRealloc(m_pBuffer, m_nLength);

	//
	// Check for out of memory situation.
	//
	if(NULL == m_pBuffer) 
	{
		Clear();
		return E_OUTOFMEMORY;
	}

	hr = pStream->Read((void*)(m_pBuffer + m_iPosWrite), cb, pcbWritten);
	if(SUCCEEDED(hr))
		m_iPosWrite += cb;

	return hr;
}

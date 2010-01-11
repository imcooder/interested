

#include "stdafx.h"
#include "Currency.h"

using namespace OLEDBCLI;


//-------------------------------------------------------------------------
//
//	CDbValueRef
//
//-------------------------------------------------------------------------


CDbValueRef::CDbValueRef()
:	m_pLength	(NULL),
	m_pStatus	(NULL),
	m_wType		(DBTYPE_EMPTY),
	m_nMaxSize	(0),
	m_pValue	(NULL),
	m_bIsLong	(false)
{
}


CDbValueRef::CDbValueRef(BOUNDCOLUMN *pBoundColumn, BYTE *pData)
{
	ATLASSERT(pBoundColumn != NULL);
	ATLASSERT(pData != NULL);

	m_pLength	= (ULONG*)		(pData + pBoundColumn->obLength);
	m_pStatus	= (DBSTATUS*)	(pData + pBoundColumn->obStatus);
	m_pValue	= pData + pBoundColumn->obValue;
	m_wType		= pBoundColumn->wType;
	m_bIsLong	= (pBoundColumn->dwFlags & DBCOLUMNFLAGS_ISLONG) != 0;
	m_nMaxSize	= pBoundColumn->ulColumnSize;
}


CDbValueRef::CDbValueRef(DBTYPE wType, ULONG* pLength, DBSTATUS* pStatus, BYTE* pValue)
:	m_pLength	(pLength),
	m_pStatus	(pStatus),
	m_wType		(wType),
	m_nMaxSize	(0),
	m_pValue	(pValue),
	m_bIsLong	(false)
{
	ATLASSERT(pLength != NULL);
	ATLASSERT(pStatus != NULL);
	ATLASSERT(pValue != NULL);
}


void CDbValueRef::FreeStorage()
{
	if(*m_pStatus == DBSTATUS_S_OK)
	{
		IUnknown*	pUnknown = *(IUnknown**)m_pValue;

		if(pUnknown != NULL)
		{
			pUnknown->Release();
			*(IUnknown**)m_pValue = NULL;
		}
	}
}


bool CDbValueRef::GetValue(bool& bVal)
{
	ATLASSERT(m_pStatus != NULL);
	ATLASSERT(m_pValue != NULL);

	if(*m_pStatus == DBSTATUS_S_OK)
	{
		switch(m_wType)
		{
		case DBTYPE_BOOL:	bVal = *(VARIANT_BOOL*)m_pValue != 0;	return true;
		case DBTYPE_UI1:	bVal = *(BYTE*)m_pValue != 0;			return true;
		case DBTYPE_UI2:	bVal = *(USHORT*)m_pValue != 0;			return true;
		case DBTYPE_I2:		bVal = *(SHORT*)m_pValue != 0;			return true;
		case DBTYPE_UI4:	bVal = *(UINT*)m_pValue != 0;			return true;
		case DBTYPE_I4:		bVal = *(INT*)m_pValue != 0;			return true;
		}
	}
	return false;
}


// CDbValueRef::SetValue
//
//		Sets the column value as a boolean
//
bool CDbValueRef::SetValue(bool bVal)
{
	bool	bSet	= true;

	ATLASSERT(m_pStatus != NULL);
	ATLASSERT(m_pValue != NULL);

	switch(m_wType)
	{
	case DBTYPE_BOOL:	*(VARIANT_BOOL*)m_pValue	= bVal ? VARIANT_TRUE : VARIANT_FALSE;	break;
	case DBTYPE_UI1:	*(BYTE*)m_pValue			= bVal ? 1 : 0;							break;
	case DBTYPE_UI2:	*(USHORT*)m_pValue			= bVal ? 1 : 0;							break;
	case DBTYPE_I2:		*(SHORT*)m_pValue			= bVal ? 1 : 0;							break;
	case DBTYPE_UI4:	*(UINT*)m_pValue			= bVal ? 1 : 0;							break;
	case DBTYPE_I4:		*(int*)m_pValue				= bVal ? 1 : 0;							break;
	case DBTYPE_I8:		*(__int64*)m_pValue 		= bVal ? 1 : 0;							break;
	case DBTYPE_R4:		*(float*)m_pValue			= bVal ? 1.0f : 0.0f;					break;
	case DBTYPE_R8:		*(double*)m_pValue			= bVal ? 1.0 : 0.0;						break;

	default:
		bSet = false;
	}

	if(bSet)
		*m_pStatus = DBSTATUS_S_OK;
	return bSet;
}


// CDbValueRef::GetValue
//
//		Gets the column value as a short
//
bool CDbValueRef::GetValue(short& nVal)
{
	ATLASSERT(m_pStatus != NULL);
	ATLASSERT(m_pValue != NULL);

	if(*m_pStatus == DBSTATUS_S_OK)
	{
		switch(m_wType)
		{
		case DBTYPE_I8:		nVal = (short)(*(__int64*)m_pValue);	return true;
		case DBTYPE_UI4:	nVal = (short)(*(USHORT*)m_pValue);		return true;
		case DBTYPE_I4:		nVal = (short)(*(int*)m_pValue);		return true;
		case DBTYPE_UI2:	nVal = (short)(*(USHORT*)m_pValue);		return true;
		case DBTYPE_BOOL:
		case DBTYPE_I2:		nVal = *(short*)m_pValue;				return true;
		case DBTYPE_UI1:	nVal = (short)(*(BYTE*)m_pValue);		return true;
		case DBTYPE_R4:		nVal = (short)(*(float*)m_pValue);		return true;
		case DBTYPE_R8:		nVal = (short)(*(double*)m_pValue);		return true;
		case DBTYPE_CY:
			{
				CY	cy = *(CY*)m_pValue;
				nVal = short(cy.int64 / 10000);
			}
			return true;
		}
	}
	return false;
}


// CDbValueRef::GetValue
//
//		Gets the column value as an integer
//
bool CDbValueRef::GetValue(int &nVal)
{
	ATLASSERT(m_pStatus != NULL);
	ATLASSERT(m_pValue != NULL);

	if(*m_pStatus == DBSTATUS_S_OK)
	{
		switch(m_wType)
		{
		case DBTYPE_UI1:	nVal = (int)(*(BYTE*)m_pValue);		return true;
		case DBTYPE_UI2:	nVal = (int)(*(USHORT*)m_pValue);	return true;
		case DBTYPE_I2:		nVal = (int)(*(short*)m_pValue);	return true;
		case DBTYPE_UI4:	nVal = (int)(*(UINT*)m_pValue);		return true;
		case DBTYPE_I4:		nVal = *(int*)m_pValue;				return true;
		case DBTYPE_I8:		nVal = (int)(*(__int64*)m_pValue);	return true;
		case DBTYPE_R4:		nVal = (int)(*(float*)m_pValue);	return true;
		case DBTYPE_R8:		nVal = (int)(*(double*)m_pValue);	return true;
		case DBTYPE_CY:
			{
				CY	cy = *(CY*)m_pValue;
				nVal = int(cy.int64 / 10000);
			}
			return true;
		}
	}
	return false;
}


// CDbValueRef::SetValue
//
//		Sets the column value as an integer
//
bool CDbValueRef::SetValue(int nVal)
{
	bool	bSet	= true;

	ATLASSERT(m_pStatus != NULL);
	ATLASSERT(m_pValue != NULL);

	switch(m_wType)
	{
	case DBTYPE_UI1:	*(BYTE*)m_pValue	= (BYTE)nVal;	break;
	case DBTYPE_UI2:	*(USHORT*)m_pValue	= (USHORT)nVal;	break;
	case DBTYPE_I2:		*(SHORT*)m_pValue	= (SHORT)nVal;	break;
	case DBTYPE_UI4:	*(UINT*)m_pValue	= (UINT)nVal;	break;
	case DBTYPE_I4:		*(int*)m_pValue		= nVal;			break;
	case DBTYPE_I8:		*(__int64*)m_pValue = nVal;			break;
	case DBTYPE_R4:		*(float*)m_pValue	= (float)nVal;	break;
	case DBTYPE_R8:		*(double*)m_pValue	= nVal;			break;

	default:
		bSet = false;
	}

	if(bSet)
		*m_pStatus = DBSTATUS_S_OK;
	return bSet;
}


// CDbValueRef::GetValue
//
//		Gets the column value as an int64
//
bool CDbValueRef::GetValue(__int64& nVal)
{
	ATLASSERT(m_pStatus != NULL);
	ATLASSERT(m_pValue != NULL);

	if(*m_pStatus == DBSTATUS_S_OK)
	{
		switch(m_wType)
		{
		case DBTYPE_I8:		nVal = *(__int64*)m_pValue;				return true;
		case DBTYPE_UI4:	nVal = *(UINT*)m_pValue;				return true;
		case DBTYPE_I4:		nVal = *(int*)m_pValue;					return true;
		case DBTYPE_UI2:	nVal = *(USHORT*)m_pValue;				return true;
		case DBTYPE_I2:		nVal = *(short*)m_pValue;				return true;
		case DBTYPE_UI1:	nVal = *(BYTE*)m_pValue;				return true;
		case DBTYPE_R4:		nVal = (__int64)(*(float*)m_pValue);	return true;
		case DBTYPE_R8:		nVal = (__int64)(*(double*)m_pValue);	return true;
		}
	}
	return false;
}


// CDbValueRef::SetValue
//
//		Sets the column value as an int64
//
bool CDbValueRef::SetValue(__int64 nVal)
{
	bool	bSet	= true;

	ATLASSERT(m_pStatus != NULL);
	ATLASSERT(m_pValue != NULL);

	switch(m_wType)
	{
	case DBTYPE_UI1:	*(BYTE*)m_pValue	= (BYTE)nVal;	break;
	case DBTYPE_UI2:	*(USHORT*)m_pValue	= (USHORT)nVal;	break;
	case DBTYPE_I2:		*(short*)m_pValue	= (short)nVal;	break;
	case DBTYPE_UI4:	*(UINT*)m_pValue	= (UINT)nVal;	break;
	case DBTYPE_I4:		*(int*)m_pValue		= (int)nVal;	break;
	case DBTYPE_I8:		*(__int64*)m_pValue = nVal;			break;
	case DBTYPE_R4:		*(float*)m_pValue	= (float)nVal;	break;
	case DBTYPE_R8:		*(double*)m_pValue	= (double)nVal;	break;
	case DBTYPE_CY:
		{
			CY	cy;

			cy.int64		= nVal * 10000;
			*(CY*)m_pValue	= cy;
		}
		break;

	default:
		bSet = false;
	}

	if(bSet)
		*m_pStatus = DBSTATUS_S_OK;
	return bSet;
}


// CDbValueRef::GetValue
//
//		Gets the column value as a float
//
bool CDbValueRef::GetValue(float& fltVal)
{
	ATLASSERT(m_pStatus != NULL);
	ATLASSERT(m_pValue != NULL);

	if(*m_pStatus == DBSTATUS_S_OK)
	{
		switch(m_wType)
		{
		case DBTYPE_I8:		fltVal = (float)(*(__int64*)m_pValue);	return true;
		case DBTYPE_UI4:	fltVal = (float)(*(UINT*)m_pValue);		return true;
		case DBTYPE_I4:		fltVal = (float)(*(int*)m_pValue);		return true;
		case DBTYPE_UI2:	fltVal = (float)(*(USHORT*)m_pValue);	return true;
		case DBTYPE_I2:		fltVal = *(short*)m_pValue;				return true;
		case DBTYPE_UI1:	fltVal = *(BYTE*)m_pValue;				return true;
		case DBTYPE_R4:		fltVal = *(float*)m_pValue;				return true;
		case DBTYPE_R8:		fltVal = (float)(*(double*)m_pValue);	return true;
		case DBTYPE_CY:
			{
				CY	cy = *(CY*)m_pValue;

				fltVal = float((double)cy.int64 / 10000.0);
			}
			return true;
		}
	}
	return false;
}


// CDbValueRef::SetValue
//
//		Sets the column value as a float
//
bool CDbValueRef::SetValue(float fltVal)
{
	bool	bSet	= true;

	ATLASSERT(m_pStatus != NULL);
	ATLASSERT(m_pValue != NULL);

	switch(m_wType)
	{
	case DBTYPE_UI1:	*(BYTE*)m_pValue	= (BYTE)fltVal;		break;
	case DBTYPE_UI2:	*(USHORT*)m_pValue	= (USHORT)fltVal;	break;
	case DBTYPE_I2:		*(short*)m_pValue	= (short)fltVal;	break;
	case DBTYPE_UI4:	*(UINT*)m_pValue	= (UINT)fltVal;		break;
	case DBTYPE_I4:		*(int*)m_pValue		= (int)fltVal;		break;
	case DBTYPE_I8:		*(__int64*)m_pValue = (__int64)fltVal;	break;
	case DBTYPE_R4:		*(float*)m_pValue	= fltVal;			break;
	case DBTYPE_R8:		*(double*)m_pValue	= fltVal;			break;
	case DBTYPE_CY:		
		{
			CY	cyVal;

			cyVal.int64 = __int64(fltVal * 10000);
			*(CY*)m_pValue = cyVal;
		}
		break;

	default:
		bSet = false;
	}

	if(bSet)
		*m_pStatus = DBSTATUS_S_OK;
	return bSet;
}


// CDbValueRef::GetValue
//
//		Gets the column value as a double
//
bool CDbValueRef::GetValue(double& dblVal)
{
	ATLASSERT(m_pStatus != NULL);
	ATLASSERT(m_pValue != NULL);

	if(*m_pStatus == DBSTATUS_S_OK)
	{
		switch(m_wType)
		{
		case DBTYPE_I8:		dblVal = (double)(*(__int64*)m_pValue);	return true;
		case DBTYPE_UI4:	dblVal = (double)(*(UINT*)m_pValue);	return true;
		case DBTYPE_I4:		dblVal = (double)(*(int*)m_pValue);		return true;
		case DBTYPE_UI2:	dblVal = (double)(*(USHORT*)m_pValue);	return true;
		case DBTYPE_I2:		dblVal = *(short*)m_pValue;				return true;
		case DBTYPE_UI1:	dblVal = *(BYTE*)m_pValue;				return true;
		case DBTYPE_R4:		dblVal = *(float*)m_pValue;				return true;
		case DBTYPE_R8:		dblVal = *(double*)m_pValue;			return true;
		case DBTYPE_CY:
			{
				CY	cy = *(CY*)m_pValue;

				dblVal = double(cy.int64 / 10000.0);
			}
			return true;
		}
	}
	return false;
}


// CDbValueRef::SetValue
//
//		Sets the column value as double
//
bool CDbValueRef::SetValue(double dblVal)
{
	bool	bSet	= true;

	ATLASSERT(m_pStatus != NULL);
	ATLASSERT(m_pValue != NULL);

	switch(m_wType)
	{
	case DBTYPE_UI1:	*(BYTE*)m_pValue	= (BYTE)dblVal;		break;
	case DBTYPE_I2:		*(short*)m_pValue	= (short)dblVal;	break;
	case DBTYPE_I4:		*(int*)m_pValue		= (int)dblVal;		break;
	case DBTYPE_I8:		*(__int64*)m_pValue = (__int64)dblVal;	break;
	case DBTYPE_R4:		*(float*)m_pValue	= (float)dblVal;	break;
	case DBTYPE_R8:		*(double*)m_pValue	= dblVal;			break;
	case DBTYPE_CY:		
		{
			CY	cyVal;

			cyVal.int64 = __int64(dblVal * 10000);
			*(CY*)m_pValue = cyVal;
		}
		break;

	default:
		bSet = false;
	}

	if(bSet)
		*m_pStatus = DBSTATUS_S_OK;
	return bSet;
}


// CDbValueRef::GetValue
//
//		Gets the column value as a CY
//
bool CDbValueRef::GetValue(CY& cyVal)
{
	ATLASSERT(m_pStatus != NULL);
	ATLASSERT(m_pValue != NULL);

	if(*m_pStatus == DBSTATUS_S_OK)
	{
		switch(m_wType)
		{
		case DBTYPE_R4:
			{
				float val = *(float*)m_pValue;

				cyVal.int64 = __int64(val * 10000);
			}
			return true;

		case DBTYPE_R8:
			{
				double val = *(double*)m_pValue;

				cyVal.int64 = __int64(val * 10000);
			}
			return true;

		case DBTYPE_CY:
			cyVal = *(CY*)m_pValue;
			return true;
		}
	}
	return false;
}


// CDbValueRef::SetValue
//
//		Sets the column value as a CY
//
bool CDbValueRef::SetValue(CY cyVal)
{
	bool	bSet	= true;

	ATLASSERT(m_pStatus != NULL);
	ATLASSERT(m_pValue != NULL);

	switch(m_wType)
	{
	case DBTYPE_I2:
		*(short*)m_pValue = int(cyVal.int64 / 10000);
		break;

	case DBTYPE_I4:
		*(int*)m_pValue = int(cyVal.int64 / 10000);
		break;

	case DBTYPE_I8:
		*(__int64*)m_pValue = cyVal.int64 / 10000;
		break;

	case DBTYPE_R4:
		*(float*)m_pValue = float(cyVal.int64) / 10000;
		break;

	case DBTYPE_R8:
		*(double*)m_pValue = double(cyVal.int64) / 10000;
		break;

	case DBTYPE_CY:
		*(CY*)m_pValue = cyVal;
		break;

	default:
		bSet = false;
	}

	return bSet;
}


// CDbValueRef::GetValue
//
//		Gets the column value as a DB_NUMERIC
//
bool CDbValueRef::GetValue(DB_NUMERIC& numVal)
{
	ATLASSERT(m_pStatus != NULL);
	ATLASSERT(m_pValue != NULL);

	if(*m_pStatus == DBSTATUS_S_OK && m_wType == DBTYPE_NUMERIC)
	{
		numVal = *(DB_NUMERIC*)m_pValue;
		return true;
	}
	return false;
}


// CDbValueRef::GetValue
//
//		Gets the column value as a DBTIMESTAMP
//
bool CDbValueRef::GetValue(DBTIMESTAMP& dtVal)
{
	ATLASSERT(m_pStatus != NULL);
	ATLASSERT(m_pValue != NULL);

	if(*m_pStatus == DBSTATUS_S_OK && m_wType == DBTYPE_DBTIMESTAMP)
	{
		dtVal = *(DBTIMESTAMP*)m_pValue;
		return true;
	}
	return false;
}


// CDbValueRef::SetValue
//
//		Sets the column value as a DBTIMESTAMP
//
bool CDbValueRef::SetValue(DBTIMESTAMP& dtVal)
{
	ATLASSERT(m_pStatus != NULL);
	ATLASSERT(m_pValue != NULL);

	if(m_wType == DBTYPE_DBTIMESTAMP)
	{
		*(DBTIMESTAMP*)m_pValue	= dtVal;
		*m_pStatus				= DBSTATUS_S_OK;
		return true;
	}
	return false;
}


// CDbValueRef::GetValue
//
//		Gets the column value as a GUID
//
bool CDbValueRef::GetValue(GUID& guid)
{
	ATLASSERT(m_pStatus != NULL);
	ATLASSERT(m_pValue != NULL);

	if(*m_pStatus == DBSTATUS_S_OK && m_wType == DBTYPE_GUID)
	{
		guid = *(GUID*)m_pValue;
		return true;
	}
	return false;
}


// CDbValueRef::SetValue
//
//		Sets the column value as a GUID
//
bool CDbValueRef::SetValue(GUID& guid)
{
	ATLASSERT(m_pStatus != NULL);
	ATLASSERT(m_pValue != NULL);

	if(m_wType == DBTYPE_GUID)
	{
		*(GUID*)m_pValue	= guid;
		*m_pStatus			= DBSTATUS_S_OK;
		return true;
	}
	return false;
}


// CDbValueRef::GetValue
//
//		Gets the column value as a CString
//
bool CDbValueRef::GetValue(CString& strVal)
{
	ATLASSERT(m_pStatus != NULL);
	ATLASSERT(m_pValue != NULL);

	if(*m_pStatus == DBSTATUS_S_OK)
	{
		switch(m_wType)
		{
		case DBTYPE_UI1:
			strVal.Format(_T("%d"), *(BYTE*)m_pValue);
			return true;

		case DBTYPE_I2:
			strVal.Format(_T("%d"), *(short*)m_pValue);
			return true;

		case DBTYPE_I4:
			strVal.Format(_T("%d"), *(int*)m_pValue);
			return true;

		case DBTYPE_DBTIMESTAMP:
			{
				DBTIMESTAMP	dt = *(DBTIMESTAMP*)m_pValue;

				strVal.Format(_T("%d-%02d-%02d %02d:%02d:%02d"), dt.year, dt.month, dt.day, dt.hour, dt.minute, dt.second);
			}
			return true;

		case DBTYPE_CY:
			{
				CY			cy	= *(CY*)m_pValue;
				CCurrency	curr(cy);

				strVal = curr.Format(CCurrency::Currency);
			}
			return true;

		case DBTYPE_WSTR:
			if(m_bIsLong)
			{
				ISequentialStream*	pStream;
				TCHAR				buf[2048];
				ULONG				cb;
				
				pStream = *(ISequentialStream**)m_pValue;

				ATLASSERT(pStream != NULL);

				//
				// Enter a loop to read the string from the stream
				//
				do
				{
					pStream->Read(buf, sizeof(buf) - sizeof(TCHAR), &cb);

					if(cb > 0)
					{
						int	i = cb / sizeof(TCHAR);

						buf[i] = 0;
						strVal += buf;
					}
				} while(cb >= sizeof(buf));

				pStream->Release();
			}
			else
			{
				TCHAR*	pszText = (TCHAR*)m_pValue;
				ULONG	nLength	= *m_pLength / sizeof(wchar_t);

				pszText[nLength] = 0;

				strVal = pszText;
			}
			return true;
		}
	}
	else
		strVal.Empty();
	return false;
}


// CDbValueRef::SetValue
//
//		Sets the column value as a string
//
bool CDbValueRef::SetValue(LPCTSTR pszVal)
{
	ATLASSERT(m_pStatus != NULL);
	ATLASSERT(m_pValue != NULL);

	if(m_wType == DBTYPE_WSTR)
	{
		if(m_bIsLong)
		{
			CBlobStream* pBlobStream = new CBlobStream;

			if(pBlobStream == NULL)
				return false;

			ULONG	ulWritten;
			size_t	nStrLen	= wcslen(pszVal);
			HRESULT	hr		= pBlobStream->Write(pszVal, (nStrLen + 1) * sizeof(TCHAR), &ulWritten);

			if(SUCCEEDED(hr))
			{
				*(CBlobStream**)m_pValue	= pBlobStream;
				*m_pStatus					= DBSTATUS_S_OK;
				*m_pLength					= nStrLen;
			}
			else
			{
				delete pBlobStream;

				*(CBlobStream**)m_pValue	= NULL;
				*m_pStatus					= DBSTATUS_S_ISNULL;
				*m_pLength					= 0;
				return false;
			}
		}
		else
		{
			size_t	nStrLen = wcslen(pszVal);
			ULONG	nMaxLen = m_nMaxSize / sizeof(wchar_t);

			if(nStrLen > nMaxLen)
			{
				TCHAR*	pszBuf = (TCHAR*)m_pValue;

				wcsncpy(pszBuf, pszVal, nMaxLen);
				pszBuf[nMaxLen] = 0;
				nStrLen = nMaxLen;
			}
			else
			{
				wcscpy((TCHAR*)m_pValue, pszVal);
			}

			*m_pLength	= nStrLen;
			*m_pStatus	= DBSTATUS_S_OK;
		}

		return true;
	}
	return false;
}


// CDbValueRef::GetValue
//
//		Gets the column value into a byte buffer.
//		Does not work for BLOB columns.
//
bool CDbValueRef::GetValue(BYTE *pVal)
{
	ULONG	nLength;

	ATLASSERT(m_pStatus != NULL);
	ATLASSERT(m_pLength	!= NULL);
	ATLASSERT(m_pValue	!= NULL);
	ATLASSERT(pVal		!= NULL);

	nLength = * m_pLength;

	if(*m_pStatus == DBSTATUS_S_OK)
	{
		switch(m_wType)
		{
		case DBTYPE_I2:
		case DBTYPE_I4:
		case DBTYPE_I8:
		case DBTYPE_R4:
		case DBTYPE_R8:
		case DBTYPE_CY:
		case DBTYPE_NUMERIC:
		case DBTYPE_DBTIMESTAMP:
		case DBTYPE_GUID:
			memcpy(pVal, m_pValue, nLength);
			return true;

		case DBTYPE_WSTR:
			nLength *= sizeof(wchar_t);

		case DBTYPE_BYTES:
			if(!m_bIsLong)
			{
				memcpy(pVal, m_pValue, nLength);
				return true;
			}
		}
	}
	return false;
}


bool CDbValueRef::SetValue(BYTE *pVal, ULONG nLength)
{
	ATLASSERT(m_pStatus != NULL);
	ATLASSERT(m_pValue != NULL);

	if((m_wType == DBTYPE_BYTES) && m_bIsLong)
	{
		CBlobStream* pBlobStream = new CBlobStream;

		if(pBlobStream == NULL)
			return false;

		ULONG	ulWritten;
		HRESULT	hr		= pBlobStream->Write(pVal, nLength, &ulWritten);

		if(SUCCEEDED(hr))
		{
			*(CBlobStream**)m_pValue	= pBlobStream;
			*m_pStatus					= DBSTATUS_S_OK;
			*m_pLength					= nLength;
		}
		else
		{
			delete pBlobStream;

			*(CBlobStream**)m_pValue	= NULL;
			*m_pStatus					= DBSTATUS_S_ISNULL;
			*m_pLength					= 0;
			return false;
		}
	}
	else
	{
		if(nLength > m_nMaxSize)
			nLength = m_nMaxSize;

		memcpy(m_pValue, pVal, nLength);

		*m_pLength	= nLength;
		*m_pStatus	= DBSTATUS_S_OK;
	}


	return true;
}


// CDbValueRef::GetValue
//
//		Gets the column value as a BLOB
//
bool CDbValueRef::GetValue(CBlobStream& blob)
{
	ULONG	nLength,
			nWritten;
	HRESULT	hr;

	ATLASSERT(m_pStatus != NULL);
	ATLASSERT(m_pLength	!= NULL);
	ATLASSERT(m_pValue	!= NULL);

	blob.Clear();

	nLength = *m_pLength;

	if(*m_pStatus == DBSTATUS_S_OK)
	{
		switch(m_wType)
		{
		case DBTYPE_I2:
		case DBTYPE_I4:
		case DBTYPE_I8:
		case DBTYPE_R4:
		case DBTYPE_R8:
		case DBTYPE_CY:
		case DBTYPE_NUMERIC:
		case DBTYPE_DBTIMESTAMP:
		case DBTYPE_GUID:
			hr = blob.Write(m_pValue, nLength, &nWritten);
			if(SUCCEEDED(hr) && nWritten == nLength)
				return true;

		case DBTYPE_WSTR:
			nLength *= sizeof(wchar_t);

		case DBTYPE_BYTES:
			if(!m_bIsLong)
			{
				hr = blob.Write(m_pValue, nLength, &nWritten);
				if(SUCCEEDED(hr) && nWritten == nLength)
					return true;
			}
			else
			{
				ISequentialStream*	pStream = *(ISequentialStream**)m_pValue;

				hr = blob.WriteFromStream(pStream, nLength, &nWritten);
				pStream->Release();
				if(SUCCEEDED(hr) && nWritten == nLength)
					return true;
			}
		}
	}
	return false;
}


//-------------------------------------------------------------------------
//
//	CDbValue
//
//-------------------------------------------------------------------------


CDbValue::CDbValue()
:	CDbValueRef	(DBTYPE_EMPTY, &m_nLength, &m_status, (BYTE*)&m_val),
	m_nLength	(0),
	m_status	(DBSTATUS_S_ISNULL)
{
	memset(&m_val, 0, sizeof(m_val));
}


CDbValue::CDbValue(const CDbValue& dbVal)
{
	InternalCopy(dbVal);
}


CDbValue::CDbValue(DBTYPE type, ULONG nLength, DBSTATUS status, const void* pData)
:	CDbValueRef	(DBTYPE_EMPTY, &m_nLength, &m_status, (BYTE*)&m_val),
	m_nLength	(nLength),
	m_status	(status)
{
	memset(&m_val, 0, sizeof(m_val));

//	SetValue(type, nLength, status, pData);
}


CDbValue::~CDbValue()
{
	Clear();
}


void CDbValue::Initialize(DBTYPE type, ULONG nLength, ULONG nMaxLength, bool bIsLong)
{
	Clear();

	m_wType		= type;
	m_nMaxSize	= nMaxLength;
	m_nLength	= nLength;
	m_bIsLong	= bIsLong;
}


// CDbValue::InternalCopy
//
//		Copies two CDbValues
//
void CDbValue::InternalCopy(const CDbValue& dbVal)
{
	Clear();

	m_nLength	= dbVal.m_nLength;
	m_status	= dbVal.m_status;
	m_wType		= dbVal.m_wType;

	switch(m_wType)
	{
	case DBTYPE_WSTR:
		m_val.pszVal = new TCHAR[m_nLength + 1];
		if(m_val.pszVal != NULL)
			wcscpy(m_val.pszVal, dbVal.m_val.pszVal);
		else
			m_status = DBSTATUS_E_OUTOFSPACE;
		break;

	case DBTYPE_BYTES:
		m_val.pVal = new BYTE[m_nLength];
		if(m_val.pVal != NULL)
			memcpy(m_val.pVal, dbVal.m_val.pVal, m_nLength);
		else
			m_status = DBSTATUS_E_OUTOFSPACE;
		break;

	default:
		memcpy(&m_val, &dbVal.m_val, sizeof(m_val));
	}
}


// CDbValue::Clear
//
//		Clears the value
//
void CDbValue::Clear()
{
	if(m_wType == DBTYPE_WSTR)
	{
		delete [] m_val.pszVal;
		m_val.pszVal = NULL;
	}
	else if(m_wType == DBTYPE_BYTES)
	{
		delete [] m_val.pVal;
		m_val.pVal = NULL;
	}
	else
	{
		memset(&m_val, 0, sizeof(m_val));
	}

	m_nLength	= 0;
	m_status	= DBSTATUS_S_ISNULL;
}


// CDbValue::operator =
//
//		Assigns a CDbValue object to another
//
CDbValue& CDbValue::operator =(const OLEDBCLI::CDbValue &dbVal)
{
	if(this != &dbVal)
		InternalCopy(dbVal);
	return *this;
}


// CDbValue::SetValue
//
//		Sets a CDbValue from an OLE DB column definition
//
void CDbValue::SetValue(DBTYPE type, ULONG nLength, DBSTATUS status, const void* pData)
{
	Clear();

	m_wType		= type;
	m_nLength	= nLength;
	m_status	= status;

	if(status != DBSTATUS_S_OK)
		return;

	ATLASSERT(pData != NULL);

	switch(type)
	{
	case DBTYPE_UI1:	m_val.bVal		= *(BYTE*)pData;			break;
	case DBTYPE_I2:		m_val.shortVal	= *(short*)pData;			break;
	case DBTYPE_I4:		m_val.intVal	= *(int*)pData;				break;
	case DBTYPE_I8:		m_val.lngVal	= *(__int64*)pData;			break;
	case DBTYPE_R4:		m_val.fltVal	= *(float*)pData;			break;
	case DBTYPE_R8:		m_val.dblVal	= *(double*)pData;			break;
	case DBTYPE_CY:		m_val.cyVal		= *(CY*)pData;				break;
	case DBTYPE_GUID:	m_val.guidVal	= *(GUID*)pData;			break;
	case DBTYPE_BOOL:	m_val.boolVal	= *(VARIANT_BOOL*)pData;	break;

	case DBTYPE_DBTIMESTAMP:
		memcpy(&m_val.dtVal, pData, sizeof(m_val.dtVal));
		break;

	case DBTYPE_NUMERIC:
		memcpy(&m_val.numVal, pData, sizeof(m_val.numVal));
		break;

	case DBTYPE_BYTES:
		m_val.pVal = new BYTE[nLength];
		if(m_val.pVal != NULL)
		{
			memcpy(m_val.pVal, pData, nLength);
		}
		else
		{
			m_status	= DBSTATUS_S_ISNULL;
			m_nLength	= 0;
		}
		break;

	case DBTYPE_WSTR:
		m_val.pszVal = new TCHAR[nLength + 1];
		if(m_val.pszVal != NULL)
		{
			memset(m_val.pszVal, 0, sizeof(TCHAR) * (nLength + 1));
			memcpy(m_val.pszVal, pData, nLength * sizeof(TCHAR));
		}
		else
		{
			m_status	= DBSTATUS_S_ISNULL;
			m_nLength	= 0;
		}
		break;
	}
}


// CDbValue::SetValue
//
//		Sets a CDbValue from a BLOB
//
HRESULT CDbValue::SetValue(DBTYPE type, ULONG nLength, DBSTATUS status, ISequentialStream* pStream)
{
	HRESULT hr	= E_FAIL;

	Clear();

	m_wType		= type;
	m_nLength	= nLength;
	m_status	= status;

	if(status != DBSTATUS_S_OK)
		return S_OK;

	ATLASSERT(pStream != NULL);

	ULONG	nRead;

	if(type == DBTYPE_WSTR)
	{
		m_val.pszVal = new TCHAR[nLength + 1];
		if(m_val.pszVal != NULL)
		{
			memset(m_val.pszVal, 0, sizeof(TCHAR) * (nLength + 1));
			hr = pStream->Read(m_val.pszVal, nLength * sizeof(TCHAR), &nRead);
		}
		else
		{
			m_nLength	= 0;
			m_status	= DBSTATUS_S_ISNULL;
			hr			= E_OUTOFMEMORY;
		}
	}
	else if(type == DBTYPE_BYTES)
	{
		m_val.pVal = new BYTE[nLength];
		if(m_val.pVal != NULL)
		{
			hr = pStream->Read(m_val.pVal, nLength, &nRead);
		}
		else
		{
			m_nLength		= 0;
			m_status	= DBSTATUS_S_ISNULL;
			hr			= E_OUTOFMEMORY;
		}
	}

	return hr;
}


// CDbValue::SetValue
//
//		Sets a string value allowing the string buffer to increase its maximum size.
//		This is useful for command parameters where values are indirectly set through CDbValues.
//
bool CDbValue::SetValue(LPCTSTR pszVal)
{
	if(m_wType == DBTYPE_WSTR)
	{
		ULONG	nLength	= wcslen(pszVal) + 1;

		if(m_bIsLong)
		{
			SetValue(DBTYPE_WSTR, nLength, DBSTATUS_S_OK, pszVal);
			if(m_nMaxSize < nLength)
				m_nMaxSize = nLength;
		}
		else
		{
			if(nLength > m_nMaxSize)
				nLength = m_nMaxSize;
			SetValue(DBTYPE_WSTR, nLength, DBSTATUS_S_OK, pszVal);
		}

		return true;
	}
	return false;
}


bool CDbValue::SetValue(BYTE* pValue, ULONG nSize)
{
	if(m_wType == DBTYPE_BYTES)
	{
		if(m_bIsLong)
		{
			SetValue(DBTYPE_BYTES, nSize, DBSTATUS_S_OK, pValue);
			if(m_nMaxSize < nSize)
				m_nMaxSize = nSize;
		}
		else
		{
			if(nSize > m_nMaxSize)
				nSize = m_nMaxSize;
			SetValue(DBTYPE_BYTES, nSize, DBSTATUS_S_OK, pValue);
		}

		return true;
	}
	return false;
}


// CDbValue::GetDataPtr
//
//		Gets the underlying data pointer.
//
void* CDbValue::GetDataPtr()
{
	if(m_wType == DBTYPE_WSTR)
		return m_val.pszVal;

	if(m_wType == DBTYPE_BYTES)
		return m_val.pVal;

	return &m_val;
}

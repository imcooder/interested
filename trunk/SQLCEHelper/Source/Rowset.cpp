
#include "stdafx.h"
#include "Currency.h"

using namespace OLEDBCLI;


CRowset::CRowset()
:	m_pRowset			(NULL),
	m_pRowsetChange		(NULL),
	m_pRowsetUpdate		(NULL),
	m_pRowsetIndex		(NULL),
	m_pColAccessor		(NULL),
	m_pKeyAccessor		(NULL),
	m_hRow				(NULL),
	m_nColumns			(0),
	m_nKeyColumns		(0),
	m_nAccessors		(0),
	m_nRowSize			(0),
	m_nKeySize			(0),
	m_pBoundColumn		(NULL),
	m_pBoundKey			(NULL),
	m_pColumnNames		(NULL),
	m_phAccessor		(NULL),
	m_hKeyAccessor		(NULL),
	m_pBuffer			(NULL),
	m_pKeyBuffer		(NULL),
	m_pBindStatus		(NULL),
	m_bCustomBound		(false),
	m_pValueRef			(NULL),
	m_pKeyRef			(NULL)
{
}


CRowset::~CRowset()
{
	Close();
}


// CRowset::GetBlobCount
//
//		Counts the number of BLOB columns in the rowset.
//		This is required in order to know the number of accessor to allocate.
//
ULONG CRowset::GetBlobCount(DBCOLUMNINFO* pColumnInfo, ULONG nColumns)
{
	ULONG	nBlobs	= 0,
			iCol;

	for(iCol = 0; iCol < nColumns; ++iCol)
	{
		bool bIsBlob = (pColumnInfo[iCol].dwFlags & DBCOLUMNFLAGS_ISLONG) != 0;

		if(bIsBlob)
			++nBlobs;
	}
	return nBlobs;
}

DBCOLUMNINFO* CRowset::GetTableColumnInfo(LPCTSTR pszColumnName, DBCOLUMNINFO* pColumnInfo)
{
	DBCOLUMNINFO* pColInfo = pColumnInfo;

	if(!pColumnInfo)
		return NULL;

	for(ULONG iCol = 0; iCol< m_nColumns; ++iCol, ++pColInfo)
	{
		if(NULL != pColInfo->pwszName)
		{
			if(0 == _wcsicmp(pColInfo->pwszName, pszColumnName))
				return pColInfo;
		}
	}
	return NULL;
}



// CRowset::BindKeyColumns
//
//		Binds the key columns, if any
//
HRESULT CRowset::BindKeyColumns(DBCOLUMNINFO* pColumnInfo)
{
	HRESULT				hr;
	ULONG				nPropSet	= 0;
	DBPROPSET*			pPropSet	= NULL;
	ULONG				i;
	DBINDEXCOLUMNDESC*	pKeyColumnDesc;

	hr = m_pRowsetIndex->GetIndexInfo(&m_nKeyColumns,		// Number of index columns
									  &pKeyColumnDesc,		// Names of columns
									  &nPropSet,			// Don't care
									  &pPropSet);			// Don't care
	if(SUCCEEDED(hr))
	{
		CBindingArray	binding;
		DBBINDING*		pBinding		= NULL;

		m_pBoundKey = new BOUNDCOLUMN[m_nKeyColumns];

		if(binding.Allocate(m_nKeyColumns) && m_pBoundKey != NULL)
		{
			DWORD			dwOffset	= 0;
			BOUNDCOLUMN*	pBoundKey	= m_pBoundKey;

			memset(m_pBoundKey, 0, m_nKeyColumns * sizeof(BOUNDCOLUMN));

			m_nKeySize	= 0;
			pBinding	= binding[0];
			for(i = 0; i < m_nKeyColumns; ++i, ++pBinding, ++pBoundKey)
			{
				DBCOLUMNINFO*	pColInfo;

				pColInfo = GetTableColumnInfo(pKeyColumnDesc[i].pColumnID->uName.pwszName, pColumnInfo);

				pBinding->iOrdinal		= pColInfo->iOrdinal;
				pBinding->dwPart		= DBPART_VALUE | DBPART_STATUS | DBPART_LENGTH;
				pBinding->obLength		= dwOffset;                                     
				pBinding->obStatus		= AddOffset(pBinding->obLength, sizeof(ULONG));
				pBinding->obValue		= AddOffset(pBinding->obStatus, sizeof(DBSTATUS));
				pBinding->pObject		= NULL;
				pBinding->pBindExt		= NULL;
				pBinding->pTypeInfo		= NULL;
				pBinding->dwMemOwner	= DBMEMOWNER_CLIENTOWNED;
				pBinding->dwFlags		= 0;
				pBinding->wType			= pColInfo->wType;
				pBinding->bPrecision	= pColInfo->bPrecision;
				pBinding->bScale		= pColInfo->bScale;

				switch(pBinding->wType)
				{
				case DBTYPE_WSTR:		
					pBinding->cbMaxLen = sizeof(WCHAR)*(pColInfo->ulColumnSize + 1);	// Extra buffer for null terminator 
					break;

				default:
					pBinding->cbMaxLen = pColInfo->ulColumnSize; 
					break;
				}

				dwOffset = AddOffset(pBinding->obValue, pBinding->cbMaxLen);
	
				pBoundKey->bPrecision	= pBinding->bPrecision;
				pBoundKey->bScale		= pBinding->bScale;
				pBoundKey->dwFlags		= pColumnInfo->dwFlags;
				pBoundKey->iAccessor	= 0;
				pBoundKey->iOrdinal		= i + 1;//pBinding->iOrdinal;
				pBoundKey->obLength		= pBinding->obLength;
				pBoundKey->obStatus		= pBinding->obStatus;
				pBoundKey->obValue		= pBinding->obValue;
				pBoundKey->pwszName		= pColumnInfo->pwszName;
				pBoundKey->ulColumnSize	= pBinding->cbMaxLen;
				pBoundKey->wType		= pColInfo->wType;
			}

			// Allocate the key buffer
			m_pKeyBuffer = new BYTE[dwOffset * 2];
			if(m_pKeyBuffer != NULL)
			{
				m_nKeySize = dwOffset;

				// Allocate the CDbValueRef array
				m_pKeyRef = new CDbValueRef[m_nKeyColumns];
				if(m_pKeyRef != NULL)
				{
					ULONG iCol;

					// Assign all value refs
					for(iCol = 0; iCol < m_nKeyColumns; ++iCol)
					{
						pBoundKey = m_pBoundKey + iCol;

						m_pKeyRef[iCol] = CDbValueRef(pBoundKey, m_pKeyBuffer);
					}

					hr = m_pRowsetIndex->QueryInterface(IID_IAccessor, (void**)&m_pKeyAccessor);
					if(SUCCEEDED(hr))
					{
						hr = m_pKeyAccessor->CreateAccessor(DBACCESSOR_ROWDATA, m_nKeyColumns, binding[0], dwOffset * 2, &m_hKeyAccessor, NULL);
					}
				}
				else
					hr = E_OUTOFMEMORY;
			}
			else
				hr = E_OUTOFMEMORY;
		}
		else
			hr = E_OUTOFMEMORY;
	}

	//
	// Free the provider-allocated memory
	//
	if(pPropSet != NULL)
	{
		for(i = 0; i < nPropSet; ++i)
		{
			ULONG	iProp;

			for(iProp = 0; iProp < pPropSet[i].cProperties; ++iProp)
				VariantClear(&pPropSet[i].rgProperties[iProp].vValue);

			CoTaskMemFree(pPropSet[i].rgProperties);
		}
		CoTaskMemFree(pPropSet);
	}

	if(pKeyColumnDesc != NULL)
		CDbMemory::Free(pKeyColumnDesc, m_nKeyColumns);

	return hr;
}



// CRowset::Open
//
//		Opens the rowset using an existing IRowset pointer
//
HRESULT CRowset::Open(IRowset* pRowset)
{
	HRESULT					hr;
	CComPtr<IColumnsInfo>	spColumnsInfo;		// Automatically deleted at the end of the method
	ULONG					iCol,
							iAcc			= 0,
							nBlobs			= 0;
	BOUNDCOLUMN*			pBoundColumn	= NULL;
	DBCOLUMNINFO*			pColumnInfo		= NULL;
	DBBINDING*				pBinding		= NULL;
	DBBINDSTATUS*			pBindStatus		= NULL;
	ULONG					iBinding		= 0;
	CBindingArray			binding;
	CAtlArray<DBCOLUMNINFO>	columnInfo;

	Close();

	m_pRowset = pRowset;

	// Query for additional interfaces
	hr = m_pRowset->QueryInterface(IID_IRowsetChange, (void**)&m_pRowsetChange);
	if(FAILED(hr))
		m_pRowsetChange = NULL;
	hr = m_pRowset->QueryInterface(IID_IRowsetUpdate, (void**)&m_pRowsetUpdate);
	if(FAILED(hr))
		m_pRowsetUpdate = NULL;
	hr = m_pRowset->QueryInterface(IID_IRowsetIndex, (void**)&m_pRowsetIndex);
	if(FAILED(hr))
		m_pRowsetIndex = NULL;

	// Get the associated IColumnsInfo interface
	hr = m_pRowset->QueryInterface(IID_IColumnsInfo, (void**)&spColumnsInfo);
	if(FAILED(hr))
		return hr;

	// Get the the table's column names.
	// Note that the first column (iOrdinal = 0) is the bookmark - it has no name!
	hr = spColumnsInfo->GetColumnInfo(&m_nColumns, &pColumnInfo, &m_pColumnNames);
	if(FAILED(hr))
		return hr;

	// Copy the DBCOLUMNINFO array to our safe array
	if(!columnInfo.SetCount(m_nColumns))
	{
		CoTaskMemFree(pColumnInfo);
		return E_OUTOFMEMORY;
	}
	memcpy(columnInfo.GetData(), pColumnInfo, m_nColumns * sizeof(DBCOLUMNINFO));
	// Get the number of blob columns in the rowset.
	nBlobs = GetBlobCount(pColumnInfo, m_nColumns);
	CoTaskMemFree(pColumnInfo);

	// Allocate the bound columns array
	m_pBoundColumn = new BOUNDCOLUMN[m_nColumns];
	if(m_pBoundColumn == NULL)
		return E_OUTOFMEMORY;
	memset(m_pBoundColumn, 0, m_nColumns * sizeof(BOUNDCOLUMN));

	// Start the binding process by allocating the binding array
	if(!binding.Allocate(m_nColumns))
		return E_OUTOFMEMORY;

	m_nAccessors	= nBlobs + 1;
	m_nRowSize		= 0;		
	pBinding		= binding[0];
	pColumnInfo		= columnInfo.GetData();

	// Fill the binding array for the non-BLOB columns
	// All of these columns will be bound by the accessor handle at index zero
	for(iCol = 0; iCol < m_nColumns; ++iCol, ++pColumnInfo)
	{
		ULONG	nColumnSize = pColumnInfo->ulColumnSize;
		DBTYPE	wType		= pColumnInfo->wType;
		bool	bIsBlob		= (pColumnInfo->dwFlags & DBCOLUMNFLAGS_ISLONG) != 0;

		// Skip this column if it is a BLOB
		if(bIsBlob)
			continue;

		if(wType == DBTYPE_STR)
			++nColumnSize;		// Add the NULL terminator

		// If this is a UNICODE string, correct the size and allow for the NULL terminator
		if(wType == DBTYPE_WSTR)
			nColumnSize = (nColumnSize + 1) * sizeof(wchar_t);

		pBinding->iOrdinal		= pColumnInfo->iOrdinal;
		pBinding->wType			= wType;
		pBinding->bPrecision	= pColumnInfo->bPrecision;
		pBinding->bScale		= pColumnInfo->bScale;
		pBinding->dwPart		= DBPART_VALUE | DBPART_LENGTH | DBPART_STATUS;
		pBinding->dwMemOwner	= DBMEMOWNER_CLIENTOWNED;
		pBinding->cbMaxLen		= nColumnSize;

		pBinding->obLength		= m_nRowSize;
		m_nRowSize				= AddOffset(m_nRowSize, sizeof(ULONG));
		pBinding->obStatus		= m_nRowSize;
		m_nRowSize				= AddOffset(m_nRowSize, sizeof(ULONG));
		pBinding->obValue		= m_nRowSize;
		m_nRowSize				= AddOffset(m_nRowSize, nColumnSize);

		// Set the bound column info
		pBoundColumn = m_pBoundColumn + iCol;

		pBoundColumn->bPrecision	= pBinding->bPrecision;
		pBoundColumn->bScale		= pBinding->bScale;
		pBoundColumn->dwFlags		= pColumnInfo->dwFlags;
		pBoundColumn->iAccessor		= 0;
		pBoundColumn->iOrdinal		= pBinding->iOrdinal;
		pBoundColumn->obLength		= pBinding->obLength;
		pBoundColumn->obStatus		= pBinding->obStatus;
		pBoundColumn->obValue		= pBinding->obValue;
		pBoundColumn->pwszName		= pColumnInfo->pwszName;
		pBoundColumn->ulColumnSize	= nColumnSize;
		pBoundColumn->wType			= wType;

		++pBinding;
	}

	// Fill the binding array for the BLOB columns
	// Each column will be bound using its own accessor handle wich is stored in the accessor handle array.
	int iAccessor = 1;
	pColumnInfo	= columnInfo.GetData();
	for(iCol = 0; (m_nAccessors > 1) && (iCol < m_nColumns); ++iCol, ++pColumnInfo)
	{
		ULONG		nColumnSize = pColumnInfo->ulColumnSize;
		DBTYPE		wType		= pColumnInfo->wType;
		bool		bIsBlob		= (pColumnInfo->dwFlags & DBCOLUMNFLAGS_ISLONG) != 0;
		DBOBJECT*	pDbObject;

		// Skip this column if it is NOT a BLOB
		if(!bIsBlob)
			continue;

		// Allocate the DBOBJECT - this will be released by the Close method
		pDbObject = new DBOBJECT;
		if(pDbObject == NULL)
		{
			Close();
			return E_OUTOFMEMORY;
		}

		pDbObject->dwFlags	= STGM_READ;			// Check if this is enough
		pDbObject->iid		= IID_ISequentialStream;

		nColumnSize			= sizeof(IUnknown*);

		pBinding->iOrdinal		= pColumnInfo->iOrdinal;
		pBinding->wType			= DBTYPE_IUNKNOWN;
		pBinding->bPrecision	= 0;
		pBinding->bScale		= 0;
		pBinding->dwPart		= DBPART_VALUE | DBPART_LENGTH | DBPART_STATUS;
		pBinding->dwMemOwner	= DBMEMOWNER_CLIENTOWNED;
		pBinding->cbMaxLen		= nColumnSize;
		pBinding->pObject		= pDbObject;

		pBinding->obLength		= m_nRowSize;
		m_nRowSize				= AddOffset(m_nRowSize, sizeof(ULONG));
		pBinding->obStatus		= m_nRowSize;
		m_nRowSize				= AddOffset(m_nRowSize, sizeof(ULONG));
		pBinding->obValue		= m_nRowSize;
		m_nRowSize				= AddOffset(m_nRowSize, nColumnSize);

		// Set the bound column info
		pBoundColumn = m_pBoundColumn + iCol;

		pBoundColumn->bPrecision	= pBinding->bPrecision;
		pBoundColumn->bScale		= pBinding->bScale;
		pBoundColumn->dwFlags		= pColumnInfo->dwFlags;
		pBoundColumn->iAccessor		= iAccessor++;
		pBoundColumn->iOrdinal		= pBinding->iOrdinal;
		pBoundColumn->obLength		= pBinding->obLength;
		pBoundColumn->obStatus		= pBinding->obStatus;
		pBoundColumn->obValue		= pBinding->obValue;
		pBoundColumn->pwszName		= pColumnInfo->pwszName;
		pBoundColumn->ulColumnSize	= nColumnSize;
		pBoundColumn->wType			= wType;

		++pBinding;
	}

	// Allocate the data buffer
	m_pBuffer = new BYTE[m_nRowSize];
	if(m_pBuffer == NULL)
		return E_OUTOFMEMORY;
	memset(m_pBuffer, 0, m_nRowSize);

	// Allocate the DBBINDSTATUS array. We own this memory.
	m_pBindStatus = new DBBINDSTATUS[m_nColumns];
	if(m_pBindStatus == NULL)
		return E_OUTOFMEMORY;

	// Allocate the CDbValueRef array
	m_pValueRef = new CDbValueRef[m_nColumns];
	if(m_pValueRef == NULL)
		return E_OUTOFMEMORY;

	// Assign all value refs
	for(iCol = 0; iCol < m_nColumns; ++iCol)
	{
		pBoundColumn = m_pBoundColumn + iCol;

		m_pValueRef[iCol] = CDbValueRef(pBoundColumn, m_pBuffer);
	}

	// Get the IAccessor interface
	hr = m_pRowset->QueryInterface(IID_IAccessor, (void**)&m_pColAccessor);
	if(FAILED(hr))
		return hr;

	// Allocate the accessor handle array
	m_phAccessor = new HACCESSOR[m_nAccessors];
	if(m_phAccessor == NULL)
		return E_OUTOFMEMORY;

	// Bind the non-BLOB data first
	hr = m_pColAccessor->CreateAccessor(DBACCESSOR_ROWDATA, 
										m_nColumns - nBlobs, 
										binding[0], 
										m_nRowSize, 
										&m_phAccessor[0], 
										m_pBindStatus);
	if(FAILED(hr))
		return hr;

	ULONG nBlobBindSize = AddOffset(sizeof(IUnknown*), AddOffset(sizeof(ULONG), sizeof(ULONG)));

	iAccessor	= 1;
	pBinding	= binding[0]	+ m_nColumns - nBlobs;
	pBindStatus	= m_pBindStatus + m_nColumns - nBlobs;

	for(iCol = 0; SUCCEEDED(hr) && (iCol < nBlobs); ++iCol)
	{
		hr = m_pColAccessor->CreateAccessor(DBACCESSOR_ROWDATA, 
											1, 
											pBinding, 
											nBlobBindSize, 
											&m_phAccessor[iAccessor], 
											pBindStatus);

		++pBinding;
		++iAccessor;
		++pBindStatus;
	}

	// Finally, bind the key columns
	if(m_pRowsetIndex != NULL)
		hr = BindKeyColumns(columnInfo.GetData());

	return hr;
}


// CRowset::Close
//
//		Closes a rowset
//
void CRowset::Close()
{
	ULONG	i;

	ReleaseRow();

	if(m_pColAccessor != NULL)
	{
		if(m_phAccessor != NULL)
		{
			for(i = 0; i < m_nAccessors; ++i)
			{
				m_pColAccessor->ReleaseAccessor(m_phAccessor[i], NULL);
				m_phAccessor[i] = NULL;
			}
		}
		delete [] m_phAccessor;
		m_phAccessor = NULL;

		m_pColAccessor->Release();
		m_pColAccessor = NULL;
	}

	if(m_pKeyAccessor != NULL)
	{
		m_pKeyAccessor->ReleaseAccessor(m_hKeyAccessor, NULL);
		m_pKeyAccessor->Release();
		m_pKeyAccessor = NULL;
	}

	// Release the BOUNDCOLUMN arrays
	if(m_pBoundColumn != NULL)
	{
		delete [] m_pBoundColumn;
		m_pBoundColumn = NULL;
	}
	if(m_pBoundKey != NULL)
	{
		delete [] m_pBoundKey;
		m_pBoundKey = NULL;
	}

	if(m_pRowset != NULL)
	{
		m_pRowset->Release();
		m_pRowset = NULL;
	}

	if(m_pRowsetChange != NULL)
	{
		m_pRowsetChange->Release();
		m_pRowsetChange = NULL;
	}

	if(m_pRowsetUpdate != NULL)
	{
		m_pRowsetUpdate->Release();
		m_pRowsetUpdate = NULL;
	}

	if(m_pRowsetIndex != NULL)
	{
		m_pRowsetIndex->Release();
		m_pRowsetIndex = NULL;
	}

	if(m_pColumnNames != NULL)
	{
		CoTaskMemFree(m_pColumnNames);
		m_pColumnNames = NULL;
	}
	m_nColumns = 0;

	// Free the data buffer
	delete [] m_pBuffer;
	delete [] m_pKeyBuffer;
	delete [] m_pBindStatus;
	delete [] m_pValueRef;
	delete [] m_pKeyRef;
	m_pBuffer		= NULL;
	m_pKeyBuffer	= NULL;
	m_pBindStatus	= NULL;
	m_pValueRef		= NULL;
	m_pKeyRef		= NULL;
	m_nRowSize		= 0;
}


// CRowset::ReleaseRow
//
//		Releases the acquired row, if any
//
HRESULT CRowset::ReleaseRow()
{
	HRESULT	hr = S_OK;

	if(m_hRow != NULL && m_pRowset != NULL)
	{
		hr = m_pRowset->ReleaseRows(1, &m_hRow, NULL, NULL, NULL);
		if(SUCCEEDED(hr))
			m_hRow = NULL;
	}

	return hr;
}


// CRowset::MoveFirst
//
//		Moves to the first row in the rowset
//
HRESULT CRowset::MoveFirst()
{
	HRESULT	hr;

	ReleaseRow();
	hr = m_pRowset->RestartPosition(DB_NULL_HCHAPTER);
	if(FAILED(hr))
		return hr;
	return MoveNext();
}


// CRowset::MoveRelative
//
//		Moves the rowset position relative to the current position
//		Reads the next row
//
HRESULT CRowset::MoveRelative(LONG nOffset)
{
	HRESULT	hr;
	ULONG	nRows;
	HROW*	pRow = &m_hRow;

	ReleaseRow();

	hr = m_pRowset->GetNextRows(DB_NULL_HCHAPTER, nOffset - 1, 1, &nRows, &pRow);
	if(FAILED(hr))
		return hr;

	// Fetch the row
	hr = m_pRowset->GetData(m_hRow, m_phAccessor[0], m_pBuffer);
	return hr;
}


// CRowset::MoveToBookmark
//
//		Moves the rowset pointer to the given bookmark
//
HRESULT CRowset::MoveToBookmark(ULONG cbBookmark, const BYTE *pBookmark)
{
	HRESULT						hr;
	CComPtr<IRowsetBookmark>	spRowsetBookmark;

	hr = m_pRowset->QueryInterface(IID_IRowsetBookmark,(void**)&spRowsetBookmark);
	if(FAILED(hr))
		return hr;

	ReleaseRow();
	hr = spRowsetBookmark->PositionOnBookmark(DB_NULL_HCHAPTER, cbBookmark, pBookmark);
	if(FAILED(hr))
		return hr;

	// Move and read the row
	hr = MoveNext();

	return hr;
}


// CRowset::GetRowCount
//
//		Counts all the rows in the rowset by scrolling through all of them.
//
HRESULT CRowset::GetRowCount(ULONG *pcRows)
{
	HRESULT	hr;
	ULONG	nRows,
			nRowCount	= 0;
	HROW*	pRow		= &m_hRow;

	ReleaseRow();
	hr = m_pRowset->RestartPosition(DB_NULL_HCHAPTER);
	while(hr == S_OK)
	{
		hr = m_pRowset->GetNextRows(DB_NULL_HCHAPTER, 0, 1, &nRows, &pRow);
		if(hr == S_OK)
		{
			++nRowCount;
			ReleaseRow();
		}
	}

	if(SUCCEEDED(hr))
		*pcRows = nRowCount;

	return hr;
}


HRESULT CRowset::Update()
{
	DBROWSTATUS*	pRowStatus;
	HRESULT			hr;

	if(m_pRowsetUpdate == NULL)
		return E_NOINTERFACE;

	hr = m_pRowsetUpdate->Update(DB_NULL_HCHAPTER, 1, &m_hRow, NULL, NULL, &pRowStatus);
	if(SUCCEEDED(hr))
		CoTaskMemFree(pRowStatus);
	return hr;
}


HRESULT CRowset::Undo()
{
	DBROWSTATUS*	pRowStatus;
	HRESULT			hr;

	if(m_pRowsetUpdate == NULL)
		return E_NOINTERFACE;

	hr = m_pRowsetUpdate->Undo(DB_NULL_HCHAPTER, 1, &m_hRow, NULL, NULL, &pRowStatus);
	if(SUCCEEDED(hr))
		CoTaskMemFree(pRowStatus);
	return hr;
}


// CRowset::Insert
//
//		Inserts the current row
//
HRESULT CRowset::Insert()
{
	HRESULT	hr;

	if(m_pRowsetChange == NULL)
		return E_NOINTERFACE;

	ReleaseRow();
	hr = m_pRowsetChange->InsertRow(DB_NULL_HCHAPTER, m_phAccessor[0], m_pBuffer, &m_hRow);

	// Check for delayed update mode
	if(m_pRowsetUpdate != NULL && SUCCEEDED(hr))
	{
		ULONG i;

		// Set the data for all accessors (BLOBs)
		for(i = 1; SUCCEEDED(hr) && (i < m_nAccessors); ++i)
			hr = m_pRowsetChange->SetData(m_hRow, m_phAccessor[i], m_pBuffer);

		if(SUCCEEDED(hr))
			hr = Update();
		else
			Undo();
	}

	return hr;
}


// CRowset::Delete
//
//		Deletes the current row
//
HRESULT CRowset::Delete()
{
	HRESULT		hr;
	DBROWSTATUS	rowStatus	= 0;

	if(m_pRowsetChange == NULL)
		return E_NOINTERFACE;

	// Delete the rows
	hr = m_pRowsetChange->DeleteRows(DB_NULL_HCHAPTER, 1, &m_hRow, &rowStatus);

	// Check for delayed update mode
	if(m_pRowsetUpdate != NULL)
	{
		if(SUCCEEDED(hr))
			hr = Update();
		else
			Undo();
	}

	return hr;
}


// CRowset::SetData
//
//		Sets the current row data
//
HRESULT CRowset::SetData()
{
	HRESULT		hr	= S_OK;
	ULONG		i;

	if(m_pRowsetChange == NULL)
		return E_NOINTERFACE;

	for(i = 0; SUCCEEDED(hr) && (i < m_nAccessors); ++i)
		hr = m_pRowsetChange->SetData(m_hRow, m_phAccessor[i], m_pBuffer);

	// Check for delayed update mode
	if(m_pRowsetUpdate != NULL)
	{
		if(SUCCEEDED(hr))
			hr = Update();
		else
			Undo();
	}

	return hr;
}

/*
ULONG CRowset::GetColumnIndex(LPCTSTR pszColumnName)
{
	return 0;
}
*/


// CRowset::GetColumnIndex
//
//		Returns a column index given the column ordinal
//
ULONG CRowset::GetColumnIndex(ULONG iOrdinal)
{
	if(m_bCustomBound)
	{
		BOUNDCOLUMN*	pBoundColumn = m_pBoundColumn;
		ULONG			iCol;

		for(iCol = 0; iCol < m_nColumns; ++iCol, ++pBoundColumn)
		{
			if(pBoundColumn->iOrdinal == iOrdinal)
				return iCol;
		}
	}
	else
	{
		if(HasBookmark())
			return iOrdinal;
		return iOrdinal - 1;
	}
	return (ULONG)-1;
}


// CRowset::SeekRow
//
//		Seeks the previously set key values - does NOT retrieve data from the row.
//
HRESULT CRowset::SeekRow(int cKeyValues, DBSEEK dwSeekOptions)
{
	HRESULT	hr	= E_NOINTERFACE;

	if(m_pRowsetIndex != NULL)
	{
		ReleaseRow();
		hr = m_pRowsetIndex->Seek(m_hKeyAccessor, 
								  m_nKeyColumns, 
								  m_pKeyBuffer,
								  dwSeekOptions);

		if(hr == S_OK)
		{
			ULONG ulRowsFetched = 0;

			// Release a row if one is already around
			//ReleaseRows();

			// Get the row handle
			HROW* phRow = &m_hRow;
			hr = m_pRowset->GetNextRows(NULL, 0, 1, &ulRowsFetched, &phRow);
		}
	}

	return hr;
}


// CRowset::Seek
//
//		Seeks the previously set key values and retrieve data from the row, if found.
//
HRESULT CRowset::Seek(int cKeyValues, DBSEEK dwSeekOptions)
{
	HRESULT hr = SeekRow(cKeyValues, dwSeekOptions);
	if(hr == S_OK)
		hr = m_pRowset->GetData(m_hRow, m_phAccessor[0], m_pBuffer);
	return hr;
}

#include "stdafx.h"

using namespace OLEDBCLI;


// CCommand::CCommand
//
//		Constructor - builds the CCommand from the CSession object
//
CCommand::CCommand(OLEDBCLI::CSession &session)
:	m_pCommand		(NULL),
	m_nParams		(0),
	m_pszParamNames	(NULL),
	m_pBoundParam	(NULL),
	m_pBuffer		(NULL),
	m_nRowSize		(0),
	m_pBindStatus	(NULL),
	m_hParamAccessor(NULL),
	m_pParam		(NULL),
	m_pParamInfo	(NULL)
{
	IUnknown*					pUnknown = session;
	CComPtr<IDBCreateCommand>	spCreateCommand;
	HRESULT						hr;

	hr = pUnknown->QueryInterface(IID_IDBCreateCommand, (void**)&spCreateCommand);
	if(SUCCEEDED(hr))
	{
		hr = spCreateCommand->CreateCommand(NULL, IID_ICommandText, (IUnknown**)&m_pCommand);

		if(FAILED(hr))
			m_pCommand = NULL;
	}
}


// CCommand::~CCommand
//
//		Destructor
//
CCommand::~CCommand()
{
	ClearParameters();

	if(m_pCommand != NULL)
	{
		m_pCommand->Release();
		m_pCommand = NULL;
	}
}


void CCommand::ClearBinding()
{
	if(m_pBuffer != NULL)
	{
		delete [] m_pBuffer;
		m_pBuffer	= NULL;
		m_nRowSize	= 0;
	}

	if(m_pBoundParam != NULL)
	{
		delete [] m_pBoundParam;
		m_pBoundParam = NULL;
	}

	if(m_pBindStatus != NULL)
	{
		delete [] m_pBindStatus;
		m_pBindStatus = NULL;
	}

	if(m_hParamAccessor != NULL)
	{
		CComPtr<IAccessor>	spAccessor;
		HRESULT				hr = m_pCommand->QueryInterface(IID_IAccessor, (void**)&spAccessor);

		if(SUCCEEDED(hr))
		{
			spAccessor->ReleaseAccessor(m_hParamAccessor, NULL);
			m_hParamAccessor = NULL;
		}
	}
}


// CCommand::ClearParameters
//
//		Frees all parameter info.
//
void CCommand::ClearParameters()
{
	ClearBinding();

	if(m_pszParamNames != NULL)
	{
		CoTaskMemFree(m_pszParamNames);
		m_pszParamNames = NULL;
	}

	if(m_pParamInfo != NULL)
	{
		CoTaskMemFree(m_pParamInfo);
		m_pParamInfo = NULL;
	}

	if(m_pParam != NULL)
		delete [] m_pParam;
	m_pParam = NULL;

	m_nParams = 0;
}


// CCommand::SetText
//
//		Sets the command text
//
HRESULT CCommand::SetText(LPCTSTR pszText)
{
	HRESULT	hr;

	ATLASSERT(m_pCommand != NULL);
	ATLASSERT(pszText != NULL);

	hr = m_pCommand->SetCommandText(DBGUID_DEFAULT, pszText);
	return hr;
}


// CCommand::RebindParameters
//
//		Checks if the parameter buffer increased since last Bind.
//		Must return true for first execution.
//
bool CCommand::RebindParameters()
{
	ULONG	iParam;
	ULONG	nSize	= 0;

	for(iParam = 0; iParam < m_nParams; ++iParam)
	{
		nSize += sizeof(DBSTATUS) + sizeof(ULONG);

		if(m_pParam[iParam].GetType() == DBTYPE_WSTR)
			nSize += m_pParam[iParam].GetLength() * sizeof(wchar_t);
		else
			nSize += m_pParam[iParam].GetLength();
	}

	return nSize > m_nRowSize;
}


// CCommand::BindParameters
//
//		Binds command parameters
//
HRESULT CCommand::BindParameters()
{
	HRESULT					hr;
	CComPtr<IAccessor>		spAccessor;
	DBBINDING*				pBinding		= NULL;
	BOUNDCOLUMN*			pBoundParam		= NULL;
	ULONG					iParam;
	CBindingArray			binding;
	DBPARAMINFO*			pParamInfo		= m_pParamInfo;

	// Check if we need to bind the parameters
	if(!RebindParameters())
		return S_OK;

	// Start the binding process by allocating the binding array
	if(!binding.Allocate(m_nParams))
		return E_OUTOFMEMORY;

	// Allocate the BOUNDCOLUMN array for parameters
	m_pBoundParam = new BOUNDCOLUMN[m_nParams];
	if(m_pBoundParam == NULL)
		return E_OUTOFMEMORY;

	pBinding	= binding[0];
	pBoundParam	= m_pBoundParam;

	for(iParam = 0; iParam < m_nParams; ++iParam, ++pParamInfo, ++pBinding, ++pBoundParam)
	{
		ULONG		nParamSize	= m_pParam[iParam].GetLength();
		DBTYPE		wType		= pParamInfo->wType;
		DWORD		dwFlags		= pParamInfo->dwFlags;

		if(wType == DBTYPE_STR)
			++nParamSize;		// Add the NULL terminator

		// If this is a UNICODE string, correct the size and allow for the NULL terminator
		if(wType == DBTYPE_WSTR)
			nParamSize = (nParamSize + 1) * sizeof(wchar_t);

		pBinding->iOrdinal		= pParamInfo->iOrdinal;
		pBinding->wType			= wType;
		pBinding->bPrecision	= pParamInfo->bPrecision;
		pBinding->bScale		= pParamInfo->bScale;
		pBinding->dwPart		= DBPART_VALUE | DBPART_LENGTH | DBPART_STATUS;
		pBinding->dwMemOwner	= DBMEMOWNER_CLIENTOWNED;
		pBinding->cbMaxLen		= nParamSize;
		pBinding->pObject		= NULL;

		if(dwFlags & DBPARAMFLAGS_ISINPUT)
			pBinding->eParamIO |= DBPARAMIO_INPUT;
		if(dwFlags & DBPARAMFLAGS_ISOUTPUT)
			pBinding->eParamIO |= DBPARAMIO_OUTPUT;

		pBinding->obLength		= m_nRowSize;
		m_nRowSize				= AddOffset(m_nRowSize, sizeof(ULONG));
		pBinding->obStatus		= m_nRowSize;
		m_nRowSize				= AddOffset(m_nRowSize, sizeof(ULONG));
		pBinding->obValue		= m_nRowSize;
		m_nRowSize				= AddOffset(m_nRowSize, nParamSize);

		pBoundParam->bPrecision		= pBinding->bPrecision;
		pBoundParam->bScale			= pBinding->bScale;
		pBoundParam->dwFlags		= dwFlags;
		pBoundParam->iAccessor		= 0;
		pBoundParam->iOrdinal		= pBinding->iOrdinal;
		pBoundParam->obLength		= pBinding->obLength;
		pBoundParam->obStatus		= pBinding->obStatus;
		pBoundParam->obValue		= pBinding->obValue;
		pBoundParam->pwszName		= pParamInfo->pwszName;
		pBoundParam->ulColumnSize	= nParamSize;
		pBoundParam->wType			= wType;
	}

	// Allocate the data buffer
	m_pBuffer = new BYTE[m_nRowSize];
	if(m_pBuffer == NULL)
		return E_OUTOFMEMORY;

	// Allocate the DBBINDSTATUS array. We own this memory.
	m_pBindStatus = new DBBINDSTATUS[m_nParams];
	if(m_pBindStatus == NULL)
		return E_OUTOFMEMORY;

	hr = m_pCommand->QueryInterface(IID_IAccessor, (void**)&spAccessor);
	if(FAILED(hr))
		return hr;

	hr = spAccessor->CreateAccessor(DBACCESSOR_PARAMETERDATA, m_nParams, binding[0], m_nRowSize, &m_hParamAccessor, m_pBindStatus);

	return hr;
}


// CCommand::SetAllParameters
//
//		Copies all parameter values from the staging array to the parameter buffer
//
void CCommand::SetAllParameters()
{
	ULONG			iParam;
	BOUNDCOLUMN*	pBoundParam	= m_pBoundParam;

	for(iParam = 0; iParam < m_nParams; ++iParam, ++pBoundParam)
	{
		CDbValue&	value	= m_pParam[iParam];
		DBSTATUS*	pStatus	= (DBSTATUS*)	(m_pBuffer + pBoundParam->obStatus);
		ULONG*		pLength	= (ULONG*)		(m_pBuffer + pBoundParam->obLength);
		BYTE*		pValue	= (BYTE*)		(m_pBuffer + pBoundParam->obValue);

		*pStatus	= value.GetStatus();
		*pLength	= value.GetType() == DBTYPE_WSTR ? value.GetLength() * sizeof(wchar_t) : value.GetLength();

		if(*pStatus == DBSTATUS_S_OK && *pLength > 0)
			memcpy(pValue, value.GetDataPtr(), *pLength);
	}
}


// CCommand::CreateParameters
//
//		Creates te parameter value array to buffer data
//
HRESULT CCommand::CreateParameters(DBPARAMINFO* pParamInfo)
{
	ULONG	iParam;

	m_pParam = new CDbValue[m_nParams];
	if(m_pParam == NULL)
		return E_OUTOFMEMORY;

	for(iParam = 0; iParam < m_nParams; ++iParam, ++pParamInfo)
	{
		bool	bIsLong		= pParamInfo->ulParamSize > 8192;
		ULONG	nParamSize	= bIsLong ? 1024 : pParamInfo->ulParamSize;

		m_pParam[iParam].Initialize(pParamInfo->wType, nParamSize, nParamSize, bIsLong); 
	}

	return S_OK;
}


// CCommand::Prepare
//
//		Prepares the command for execution (compiles an execution plan)
//
HRESULT CCommand::Prepare(ULONG cExpectedRuns)
{
	CComPtr<ICommandPrepare>		spCommandPrepare;
	CComPtr<ICommandWithParameters> spCommandParams;
	HRESULT							hr;

	ATLASSERT(m_pCommand != NULL);

	hr = m_pCommand->QueryInterface(IID_ICommandPrepare, (void**)&spCommandPrepare);
	if(FAILED(hr))
		return hr;

	hr = spCommandPrepare->Prepare(cExpectedRuns);
	if(FAILED(hr))
		return hr;

	// Retrieve the command parameters, if any
	hr = m_pCommand->QueryInterface(IID_ICommandWithParameters, (void**)&spCommandParams);
	if(SUCCEEDED(hr))
	{
		ClearParameters();
		hr = spCommandParams->GetParameterInfo(&m_nParams, &m_pParamInfo, &m_pszParamNames);
		if(SUCCEEDED(hr) && m_nParams)
		{
			hr = CreateParameters(m_pParamInfo);
			if(SUCCEEDED(hr))
				hr = BindParameters();
		}
	}

	return hr;
}


// CCommand::Unprepare
//
//		Unprepares the command (releases the compiled execution plan)
//
HRESULT CCommand::Unprepare()
{
	CComPtr<ICommandPrepare>	spCommandPrepare;
	HRESULT						hr;

	ATLASSERT(m_pCommand != NULL);

	hr = m_pCommand->QueryInterface(IID_ICommandPrepare, (void**)&spCommandPrepare);
	if(FAILED(hr))
		return hr;

	hr = spCommandPrepare->Unprepare();
	if(SUCCEEDED(hr))
		ClearParameters();

	return hr;
}


// CCommand::Execute
//
//		Executes the command without returning a rowset.
//		Use for DDL commands.
//
HRESULT CCommand::Execute(LONG *pcRowsAffected)
{
	HRESULT		hr;
	DBPARAMS	dbParams;
	DBPARAMS*	pParams	= NULL;

	ATLASSERT(m_pCommand != NULL);

	hr = BindParameters();
	if(FAILED(hr))
		return hr;

	SetAllParameters();

	if(m_nParams)
	{
		dbParams.cParamSets	= 1;
		dbParams.hAccessor	= m_hParamAccessor;
		dbParams.pData		= m_pBuffer;

		pParams = &dbParams;
	}

	hr = m_pCommand->Execute(NULL, IID_NULL, &dbParams, pcRowsAffected, NULL);

	return hr;
}


// CCommand::Execute
//
//		Executes the command and returns a rowset.
//
HRESULT CCommand::Execute(CRowset &rowset, LONG *pcRowsAffected)
{
	HRESULT		hr;
	IRowset*	pRowset		= NULL;
	DBPARAMS	dbParams;
	DBPARAMS*	pParams		= NULL;

	ATLASSERT(m_pCommand != NULL);

	hr = BindParameters();
	if(FAILED(hr))
		return hr;

	SetAllParameters();

	if(m_nParams)
	{
		dbParams.cParamSets	= 1;
		dbParams.hAccessor	= m_hParamAccessor;
		dbParams.pData		= m_pBuffer;

		pParams = &dbParams;
	}

	hr = m_pCommand->Execute(NULL, IID_IRowset, pParams, pcRowsAffected, (IUnknown**)&pRowset);
	if(FAILED(hr))
		return hr;

	hr = rowset.Open(pRowset);
	return hr;
}


// CCommand::FindParameter
//
//		Returns a parameter entry index from the given ordinal or NULL if not found
//
ULONG CCommand::FindParameter(ULONG iOrdinal)
{
	ULONG			i;
	BOUNDCOLUMN*	p;

	for(i = 0, p = m_pBoundParam; i < m_nParams; ++i, ++p)
	{
		if(p->iOrdinal == iOrdinal)
			return i;
	}
	return (ULONG)-1;
}


#include "stdafx.h"

using namespace OLEDBCLI;


// CDataSource::CDataSource
//
//		Default constructor
//
CDataSource::CDataSource()
:	m_pInitialize(NULL)
{
}


// CDataSource::~CDataSource
//
//		Destructor
//
CDataSource::~CDataSource()
{
	Close();
}


// CDataSource::Open
//
//		Opens an existing data source
//
HRESULT CDataSource::Open(const CLSID &clsid, ULONG cPropSets, DBPROPSET rgPropSets[])
{
	HRESULT	hr;

	Close();

	hr = CoCreateInstance(clsid, NULL, CLSCTX_INPROC_SERVER, IID_IDBInitialize, (LPVOID*)&m_pInitialize);
	if(SUCCEEDED(hr))
	{
		CComPtr<IDBProperties>	spProperties;

		hr = m_pInitialize->QueryInterface(IID_IDBProperties, (void**)&spProperties);
		if(SUCCEEDED(hr))
		{
			hr = spProperties->SetProperties(cPropSets, rgPropSets);
			if(SUCCEEDED(hr))
				hr = m_pInitialize->Initialize();
		}
	}

	return hr;
}


// CDataSource::Create
//
//		Creates a new data source and optionally returns an open session
//
HRESULT CDataSource::Create(const CLSID& clsid, ULONG cPropSets, DBPROPSET rgPropSets[], CSession* pSession)
{
	HRESULT						hr;
	CComPtr<IDBDataSourceAdmin>	spAdmin;

	Close();

	hr = CoCreateInstance(clsid, NULL, CLSCTX_INPROC_SERVER, IID_IDBDataSourceAdmin, (void**)&spAdmin);
	if(FAILED(hr))
		return hr;

	if(pSession != NULL)
	{
		ISessionProperties* pSessionProperties = NULL;

		hr = spAdmin->CreateDataSource(cPropSets, rgPropSets, NULL, IID_ISessionProperties, (IUnknown**)&pSessionProperties);
		if(SUCCEEDED(hr))
			pSession->Open(pSessionProperties);
	}
	else
		hr = spAdmin->CreateDataSource(cPropSets, rgPropSets, NULL, IID_IUnknown, NULL);

	// Get the IDBInitialize pointer in order to keep the data source "open"
	if(SUCCEEDED(hr))
		spAdmin->QueryInterface(IID_IDBInitialize, (void**)&m_pInitialize);

	return hr;
}


// CDataSource::Close
//
//		Closes the data source
//
HRESULT CDataSource::Close()
{
	HRESULT hr = E_NOINTERFACE;

	if(m_pInitialize != NULL)
	{
		hr = m_pInitialize->Uninitialize();

		if(SUCCEEDED(hr))
			m_pInitialize->Release();
	}
	return hr;
}

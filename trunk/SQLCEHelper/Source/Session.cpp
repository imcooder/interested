
#include "stdafx.h"

using namespace OLEDBCLI;


CSession::CSession()
:	m_pSessionProps(NULL)
{
}


CSession::~CSession()
{
	Close();
}


HRESULT CSession::Open(ISessionProperties* pSessionProperties)
{
	Close();
	m_pSessionProps = pSessionProperties;

	return S_OK;
}


// CSession::Open
//
//		Open a new session on the data source
//
HRESULT CSession::Open(const CDataSource& dataSource)
{
	HRESULT						hr;
	IDBInitialize*				pInitialize	= dataSource;
	CComPtr<IDBCreateSession>	spCreateSession;

	if(pInitialize == NULL)
		return E_NOINTERFACE;

	Close();
	hr = pInitialize->QueryInterface(IID_IDBCreateSession, (void**)&spCreateSession);
	if(SUCCEEDED(hr))
		hr = spCreateSession->CreateSession(NULL, IID_ISessionProperties, (IUnknown**)&m_pSessionProps);

	return hr;
}


// CSession::Close
//
//		Closes the session
//
void CSession::Close()
{
	if(m_pSessionProps != NULL)
	{
		m_pSessionProps->Release();
		m_pSessionProps = NULL;
	}
}


// CSession::SetProperties
//
//		Set the session's properties
//
HRESULT CSession::SetProperties(ULONG cPropSets, DBPROPSET rgPropSets[])
{
	HRESULT	hr = E_NOINTERFACE;

	if(m_pSessionProps == NULL)
		return hr;

	hr = m_pSessionProps->SetProperties(cPropSets, rgPropSets);

	return hr;
}


#include "stdafx.h"

using namespace OLEDBCLI;


CSchemaRowset::CSchemaRowset(CSession& session)
:	m_pSchemaRowset(NULL)
{
	IUnknown* pUnknown = session;

	pUnknown->QueryInterface(IID_IDBSchemaRowset, (void**)&m_pSchemaRowset);
}

CSchemaRowset::~CSchemaRowset()
{
	if(m_pSchemaRowset != NULL)
	{
		m_pSchemaRowset->Release();
		m_pSchemaRowset = NULL;
	}
}


HRESULT	CSchemaRowset::Open(REFGUID rguidSchema, ULONG cRestrictions, const VARIANT rgRestrictions[], CRowset &rowset)
{
	HRESULT		hr;
	IRowset*	pRowset	= NULL;

	ATLASSERT(m_pSchemaRowset != NULL);

	hr = m_pSchemaRowset->GetRowset(NULL, rguidSchema, cRestrictions, rgRestrictions, IID_IRowset, 0, NULL, (IUnknown**)&pRowset);
	if(FAILED(hr))
		return hr;

	hr = rowset.Open(pRowset);
	return hr;
}


// CTablesRowset::Open
//
//		Opens the tables rowset
//
HRESULT CTablesRowset::Open(LPCTSTR pszCatalog, LPCTSTR pszSchema, LPCTSTR pszTable, LPCTSTR pszType, CRowset &rowset)
{
	HRESULT		hr;
	CComVariant	varParam[4];

	if(pszCatalog != NULL)
		varParam[0] = pszCatalog;
	if(pszSchema != NULL)
		varParam[1] = pszSchema;
	if(pszTable != NULL)
		varParam[2] = pszTable;
	if(pszType != NULL)
		varParam[3] = pszType;

	hr = CSchemaRowset::Open(DBSCHEMA_TABLES, 4, varParam, rowset);

	return hr;
}


HRESULT CViewsRowset::Open(LPCTSTR pszCatalog, LPCTSTR pszSchema, LPCTSTR pszTable, CRowset &rowset)
{
	HRESULT		hr;
	CComVariant	varParam[3];

	if(pszCatalog != NULL)
		varParam[0] = pszCatalog;
	if(pszSchema != NULL)
		varParam[1] = pszSchema;
	if(pszTable != NULL)
		varParam[2] = pszTable;

	hr = CSchemaRowset::Open(DBSCHEMA_VIEWS, 3, varParam, rowset);

	return hr;
}


// CColumnsRowset::Open
//
//		Opens the columns rowset with the given restrictions
//
HRESULT CColumnsRowset::Open(LPCTSTR	pszTableCatalog, 
							 LPCTSTR	pszTableSchema, 
							 LPCTSTR	pszTableName, 
							 LPCTSTR	pszColumnName,
							 CRowset&	rowset)
{
	HRESULT		hr;
	CComVariant	varParam[4];

	if(pszTableCatalog != NULL)
		varParam[0] = pszTableCatalog;
	if(pszTableSchema != NULL)
		varParam[1] = pszTableSchema;
	if(pszTableName != NULL)
		varParam[2] = pszTableName;
	if(pszColumnName != NULL)
		varParam[3] = pszColumnName;

	hr = CSchemaRowset::Open(DBSCHEMA_COLUMNS, 4, varParam, rowset);

	return hr;
}


// CIndexesRowset::Open
//
//		Opens the indexes rowset with the given restrictions
//
HRESULT CIndexesRowset::Open(LPCTSTR	pszCatalog, 
							 LPCTSTR	pszTableSchema, 
							 LPCTSTR	pszIndexName, 
							 LPCTSTR	pszType, 
							 LPCTSTR	pszTableName, 
							 CRowset&	rowset)
{
	HRESULT		hr;
	CComVariant	varParam[5];

	if(pszCatalog != NULL)
		varParam[0] = pszCatalog;
	if(pszTableSchema != NULL)
		varParam[1] = pszTableSchema;
	if(pszIndexName != NULL)
		varParam[2] = pszIndexName;
	if(pszType != NULL)
		varParam[3] = pszType;
	if(pszTableName != NULL)
		varParam[4] = pszTableName;

	hr = CSchemaRowset::Open(DBSCHEMA_INDEXES, 5, varParam, rowset);

	return hr;
}

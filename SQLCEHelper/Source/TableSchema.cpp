
#include "stdafx.h"

using namespace OLEDBCLI;

typedef CTable* CTablePtr;


CSchema::CSchema(CSession& session)
:	m_session	(session)
{
}


CSchema::~CSchema()
{
	Clear();
}


void CSchema::Clear()
{
	size_t i;

	for(i = 0; i < m_tables.GetCount(); ++i)
		delete m_tables[i];

	m_tables.RemoveAll();
}


// CSchema::Load
//
//		Loads all tables
//
HRESULT CSchema::Load()
{
	CTablesRowset		tables(m_session);		// http://msdn.microsoft.com/en-us/library/ms716980(VS.85).aspx
	HRESULT				hr;
	CRowset				rowset;

	Clear();

	// Open the tables rowset
	hr = tables.Open(NULL, NULL, NULL, NULL, rowset);
	if(SUCCEEDED(hr))
	{
		CString	strTable;

		for(hr = rowset.MoveFirst(); hr == S_OK; hr = rowset.MoveNext())
		{
			if(rowset.GetValue(3, strTable))
			{
				CTableSchema*	pTableSchema = new CTableSchema(strTable, this);

				if(pTableSchema != NULL)
					m_tables.Add(pTableSchema);
				else
					return E_OUTOFMEMORY;
			}
		}
		hr = S_OK;
	}

	return hr;
}


//-------------------------------------------------------------------------
//
//	CTableSchema - manipulates the table schema
//
//-------------------------------------------------------------------------

CTableSchema::CTableSchema()
:	m_pSchema		(NULL),
	m_bLoaded		(false)
{
}


CTableSchema::CTableSchema(LPCTSTR pszTableName, CSchema* pSchema)
:	m_pSchema		(pSchema),
	m_strName		(pszTableName),
	m_bLoaded		(false)
{
}


CTableSchema::CTableSchema(const CTableSchema& tableSchema)
:	m_bLoaded	(false)
{
	Copy(tableSchema);
}


CTableSchema::~CTableSchema()
{
	Clear();
}


void CTableSchema::Clear()
{
	size_t i;

	for(i = 0; i < m_columns.GetCount(); ++i)
		delete m_columns[i];
	m_columns.RemoveAll();

	for(i = 0; i < m_foreignKeys.GetCount(); ++i)
		delete m_foreignKeys[i];
	m_foreignKeys.RemoveAll();

	for(i = 0; i < m_indexes.GetCount(); ++i)
		delete m_indexes[i];
	m_indexes.RemoveAll();

	for(i = 0; i < m_uniques.GetCount(); ++i)
		delete m_uniques[i];
	m_uniques.RemoveAll();
}


bool CTableSchema::Copy(const OLEDBCLI::CTableSchema &tableSchema)
{
	Clear();

	m_columns		.Copy(tableSchema.m_columns);
	m_indexes		.Copy(tableSchema.m_indexes);
	m_foreignKeys	.Copy(tableSchema.m_foreignKeys);
	m_uniques		.Copy(tableSchema.m_uniques);
	
	m_strName = tableSchema.GetName();
	m_pSchema = tableSchema.GetSchema();
	m_bLoaded = tableSchema.IsLoaded();

	return true;
}


// CTableSchema::FindIndex
//
//		Seraches for an index in the list given a name and returns its position or -1 if not found.
//
int CTableSchema::FindIndex(LPCTSTR pszName)
{
	int	i, n = m_indexes.GetCount();

	for(i = 0; i < n; ++i)
	{
		if(wcsicmp(pszName, m_indexes[i]->GetName()) == 0)
			return i;
	}
	return -1;
}


// CTableSchema::FindUnique
//
//		Seraches for an unique constraint in the list given a name and returns its position or -1 if not found.
//
int CTableSchema::FindUnique(LPCTSTR pszName)
{
	int	i, n = m_uniques.GetCount();

	for(i = 0; i < n; ++i)
	{
		if(wcsicmp(pszName, m_uniques[i]->GetName()) == 0)
			return i;
	}
	return -1;
}


// CTableSchema::GetPrimaryKey
//
//		Gets the CIndex object that represents the PRIMARY KEY, if any
//
CIndex* CTableSchema::GetPrimaryKey()
{
	int	i, n = m_indexes.GetCount();

	for(i = 0; i < n; ++i)
	{
		if(m_indexes[i]->IsPrimaryKey())
			return m_indexes[i];
	}
	return NULL;
}


// CTableSchema::FindForeignKey
//
//		Searches for a foreign key in the list given a name and returns its position or -1 if not found.
//
int CTableSchema::FindForeignKey(LPCTSTR pszName)
{
	int i, n = m_foreignKeys.GetCount();

	for(i = 0; i < n; ++i)
	{
		if(wcsicmp(pszName, m_foreignKeys[i]->GetName()) == 0)
			return i;
	}
	return -1;
}


// CTableSchema::Load
//
//		Loads the table schema
//
HRESULT CTableSchema::Load(OLEDBCLI::CSession &session, LPCTSTR pszTableName)
{
	if(m_bLoaded)
		return S_OK;

	HRESULT				hr;
	CTableDefinition	tableDef;

	Clear();

	hr = tableDef.GetDefinition(session, m_strName);
	if(FAILED(hr))
		return hr;

	// Populate the column array
	tableDef.FillColumnArray(m_columns);

	// Populate the foreign key array
	tableDef.FillForeignKeyArray(m_foreignKeys);

	// Populate the foreign key array
	tableDef.FillUniqueArray(m_uniques);

	// Load and filter the indexes
	hr = LoadIndexes(session);

	m_bLoaded = true;

	return hr;
}


// CTableSchema::Load
//
//		Loads the schema from a table created through a CSchema.
//		These have a non-NULL m_pSchema.
//
HRESULT CTableSchema::Load()
{
	if(m_pSchema == NULL)
		return E_FAIL;

	return Load(m_pSchema->GetSession(), m_strName);
}


// CTableSchema::LoadIndexes
//
//		Loads the table's indexes
//
HRESULT CTableSchema::LoadIndexes(CSession& session)
{
	HRESULT			hr;
	CRowset			rowset;
	CIndexesRowset	indexes(session);

	// List all indexes for the given table
	hr = indexes.Open(NULL, NULL, NULL, NULL, m_strName, rowset);
	if(SUCCEEDED(hr))
	{
		CString	strName,
				strColumn;

		for(hr = rowset.MoveFirst(); hr == S_OK; hr = rowset.MoveNext())
		{
			int		iIndex,
					iOrdinal;
			short	nCollation;
			CIndex*	pIndex	= NULL;

			if(!rowset.GetValue(6, strName))
				return E_FAIL;

			// Skip known unique constraints
			if(FindUnique(strName) != -1)
				continue;

			if(!rowset.GetValue(17, iOrdinal))
				return E_FAIL;
			if(!rowset.GetValue(18, strColumn))
				return E_FAIL;
			if(!rowset.GetValue(21, nCollation))
				return E_FAIL;

			iIndex = FindIndex(strName);
			if(iIndex == -1)
			{
				bool	bPrimary, bUnique;

				rowset.GetValue(7, bPrimary);
				rowset.GetValue(8, bUnique);

				pIndex = new CIndex(strName, bUnique, bPrimary);

				if(pIndex == NULL)
					return E_OUTOFMEMORY;

				m_indexes.Add(pIndex);
				iIndex = 0;
			}

			if(pIndex == NULL)
				pIndex = m_indexes[iIndex];

			pIndex->AddColumn(strColumn, nCollation, (ULONG)iOrdinal);
		}
		hr = S_OK;
	}

	return hr;
}


#include "stdafx.h"

using namespace OLEDBCLI;


CTable::CTable(CSession& session, LPCTSTR pszTableName)
:	m_session		(session),
	m_strName		(pszTableName)
{
}


// CTable::~CTable
//
//		Destructor
//
CTable::~CTable()
{
}


// CTable::Open
//
//		Open a table cursor using the optional index name
//
HRESULT CTable::Open(ULONG cPropSets, DBPROPSET rgPropSets[], LPCTSTR pszIndexName, CRowset &rowset)
{
	HRESULT					hr;
	DBID					dbidTable,
							dbidIndex;
	DBID*					pIndexID	= NULL;
	CComPtr<IRowset>		spRowset;
	CComPtr<IUnknown>		spUnknown(m_session);
	CComPtr<IOpenRowset>	spOpenRowset;

	hr = spUnknown->QueryInterface(IID_IOpenRowset, (void**)&spOpenRowset);
	if(FAILED(hr))
		return hr;

	dbidTable.eKind				= DBKIND_NAME;
	dbidTable.uName.pwszName	= (LPOLESTR)GetName();

	if(pszIndexName != NULL)
	{
		dbidIndex.eKind				= DBKIND_NAME;
		dbidIndex.uName.pwszName	= (LPOLESTR)pszIndexName;
		pIndexID = &dbidIndex;
	}

	// Get the IRowset interface
	hr = spOpenRowset->OpenRowset(NULL, &dbidTable, pIndexID, IID_IRowset, cPropSets, rgPropSets, (IUnknown**)&spRowset);
	if(FAILED(hr))
		return hr;

	hr = rowset.Open(spRowset);
	return hr;
}


//-------------------------------------------------------------------------
//
//	CTableDefinition class
//
//-------------------------------------------------------------------------


CTableDefinition::CTableDefinition()
:	m_nColumns		(0),
	m_nPropSets		(0),
	m_nConstraints	(0),
	m_pColumns		(NULL),
	m_pConstraints	(NULL),
	m_pStrings		(NULL),
	m_pPropSets		(NULL)
{
}


CTableDefinition::~CTableDefinition()
{
	Clear();
}


// CTableDefinition::Clear
//
//		Clears the stored table definition data
//
void CTableDefinition::Clear()
{
	DBCOLUMNDESC*		pColCur;
	DBCONSTRAINTDESC*	pConCur;
	DBPROPSET*			pSetCur;
	ULONG				i, j;

	// Free the column items
	for(pColCur = m_pColumns, i = 0; i < m_nColumns; ++i, ++pColCur)
	{
		if(pColCur->pTypeInfo != NULL)
			CoTaskMemFree(pColCur->pTypeInfo);

		if(pColCur->pclsid != NULL)
			CoTaskMemFree(pColCur->pclsid);

		CDbMemory::Free(pColCur->dbcid);

		for(j = 0; j < pColCur->cPropertySets; ++j)
			CDbMemory::Free(&pColCur->rgPropertySets[j]);

		if(pColCur->rgPropertySets != NULL)
			CoTaskMemFree(pColCur->rgPropertySets);
	}

	// Free the constraint items
	for(pConCur = m_pConstraints, i = 0; i < m_nConstraints; ++i, ++pConCur)
	{
		for(j = 0; j < pConCur->cColumns; ++j)
			CDbMemory::Free(pConCur->rgColumnList[j]);
		if(pConCur->cColumns)
			CoTaskMemFree(pConCur->rgColumnList);

		for(j = 0; j < pConCur->cForeignKeyColumns; ++j)
			CDbMemory::Free(pConCur->rgForeignKeyColumnList[j]);
		if(pConCur->cForeignKeyColumns)
			CoTaskMemFree(pConCur->rgForeignKeyColumnList);

		CDbMemory::Free(*pConCur->pConstraintID);
		if(pConCur->pReferencedTableID)
			CDbMemory::Free(*pConCur->pReferencedTableID);
	}

	// Free the property sets
	for(pSetCur = m_pPropSets, i = 0; i < m_nPropSets; ++i, ++pSetCur)
		CDbMemory::Free(pSetCur);

	// Free the column array
	if(m_pColumns != NULL)
		CoTaskMemFree(m_pColumns);
	m_pColumns = NULL;

	// Free the constraints array
	if(m_pConstraints != NULL)
		CoTaskMemFree(m_pConstraints);
	m_pConstraints = NULL;

	// Free the property sets array
	if(m_pPropSets != NULL)
		CoTaskMemFree(m_pPropSets);
	m_pPropSets = NULL;

	// Free the strings memory
	if(m_pStrings != NULL)
		CoTaskMemFree(m_pStrings);
	m_pStrings = NULL;
}


// CTableDefinition::GetDefinition
//
//		Retrieves the table definition for a given table name
//
HRESULT CTableDefinition::GetDefinition(CSession& session, LPCTSTR pszTableName)
{
	CComPtr<ITableCreation>	spTableCreation;
	CComPtr<IUnknown>		spUnknown(session);
	HRESULT					hr;
	DBID					dbidTable;

	hr = spUnknown->QueryInterface(IID_ITableCreation, (void**)&spTableCreation);
	if(FAILED(hr))
		return hr;

	Clear();

	// Set the table name as the DBID
	dbidTable.eKind				= DBKIND_NAME;
	dbidTable.uName.pwszName	= (LPOLESTR)pszTableName;

	// Get the table definition
	hr = spTableCreation->GetTableDefinition(&dbidTable, 
											 &m_nColumns, &m_pColumns, 
											 &m_nPropSets, &m_pPropSets, 
											 &m_nConstraints, &m_pConstraints, 
											 &m_pStrings);
	return hr;
}


// CTableDefinition::FillColumnArray
//
//		Populates a column list from the table definition
//
bool CTableDefinition::FillColumnArray(CColumnArray& columns)
{
	ULONG	i;
	size_t	c, nColumns = columns.GetCount();

	// Clear the column array
	for(c = 0; c < nColumns; ++c)
		delete columns[c];
	columns.RemoveAll();

	if(m_nColumns == 0)
		return true;

	for(i = 0; i < m_nColumns; ++i)
	{
		CColumn* pColumn = new CColumn(m_pColumns + i, i + 1);

		if(pColumn == NULL)
			return false;

		columns.Add(pColumn);
	}
	return true;
}


// CTableDefinition::FillForeignKeyArray
//
//		Populates a foreign key array from the table definition
//
bool CTableDefinition::FillForeignKeyArray(CForeignKeyArray& foreignKeys)
{
	ULONG	i;
	size_t	c, n = foreignKeys.GetCount();

	// Clear the foreign key array
	for(c = 0; c < n; ++c)
		delete foreignKeys[c];
	foreignKeys.RemoveAll();

	for(i = 0; i < m_nConstraints; ++i)
	{
		if(m_pConstraints[i].ConstraintType == DBCONSTRAINTTYPE_FOREIGNKEY)
		{
			CForeignKey* pForeignKey = new CForeignKey(m_pConstraints + i);

			if(pForeignKey == NULL)
				return false;
			foreignKeys.Add(pForeignKey);
		}
	}
	return true;
}


// CTableDefinition::FillUniqueArray
//
//		Fills an index array with the UNIQUE constraints
//
bool CTableDefinition::FillUniqueArray(CIndexArray& uniques)
{
	ULONG	i;
	size_t	c, n = uniques.GetCount();

	// Clear the unique constraint array
	for(c = 0; c < n; ++c)
		delete uniques[c];
	uniques.RemoveAll();

	for(i = 0; i < m_nConstraints; ++i)
	{
		if(m_pConstraints[i].ConstraintType == DBCONSTRAINTTYPE_UNIQUE)
		{
			CIndex* pIndex = new CIndex(m_pConstraints + i);

			if(pIndex == NULL)
				return false;
			uniques.Add(pIndex);
		}
	}
	return true;
}


CIndex* CTableDefinition::GetPrimaryKey()
{
	ULONG i;
	for(i = 0; i < m_nConstraints; ++i)
	{
		if(m_pConstraints[i].ConstraintType == DBCONSTRAINTTYPE_PRIMARYKEY)
		{
			CIndex*	pKey = new CIndex(m_pConstraints + i);

			return pKey;
		}
	}
	return NULL;
}

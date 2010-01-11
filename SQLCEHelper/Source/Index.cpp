
#include "stdafx.h"

using namespace OLEDBCLI;


//-------------------------------------------------------------------------
//
//	CIndex - manipulates the table index schema
//
//-------------------------------------------------------------------------


// CIndex::CIndex
//
//		Default constructor
//
CIndex::CIndex()
:	m_bUnique		(false),
	m_bPrimary		(false),
	m_bConstraint	(false)
{
}


CIndex::CIndex(LPCTSTR pszIndexName, bool bUnique, bool bPrimary)
:	m_strName		(pszIndexName),
	m_bUnique		(bUnique),
	m_bPrimary		(bPrimary),
	m_bConstraint	(false)
{
}


// CIndex::CIndex
//
//		Initialize a CIndex from an OLE DB constraint description.
//		This constructor is used for UNIQUE constraints.
//
CIndex::CIndex(DBCONSTRAINTDESC* pConstraintDesc)
:	m_bUnique		(true),
	m_bPrimary		(false),
	m_bConstraint	(false)
{
	ULONG	i;

	ATLASSERT(pConstraintDesc != NULL);

	m_strName		= pConstraintDesc->pConstraintID->uName.pwszName;
	m_bConstraint	= pConstraintDesc->ConstraintType == DBCONSTRAINTTYPE_UNIQUE;
	m_bPrimary		= pConstraintDesc->ConstraintType == DBCONSTRAINTTYPE_PRIMARYKEY;

	for(i = 0; i < pConstraintDesc->cColumns; ++i)
		AddColumn(pConstraintDesc->rgColumnList[i].uName.pwszName, DB_COLLATION_ASC, i + 1);
}


CIndex::~CIndex()
{
	size_t i, n = m_columns.GetCount();

	for(i = 0; i < n; ++i)
		delete m_columns[i];
}


// CIndex::AddColumn
//
//		Adds a new index column to the list
//
void CIndex::AddColumn(LPCTSTR pszColumnName, short nCollation, ULONG nOrdinal)
{
	CIndexColumn* pIndexColumn = new CIndexColumn(pszColumnName, nCollation, nOrdinal);

	if(pIndexColumn != NULL)
		m_columns.Add(pIndexColumn);
}


// CIndex::FindColumn
//
//		Finds an index column given its name.
//		Returns the zero-based index of the column or -1 if not found.
//
int CIndex::FindColumn(LPCTSTR pszColumnName)
{
	int i, n = int(m_columns.GetCount());

	for(i = 0; i < n; ++i)
	{
		if(wcsicmp(pszColumnName, m_columns[i]->GetName()) == 0)
			return i;
	}
	return -1;
}

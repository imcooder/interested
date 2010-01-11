
#include "stdafx.h"

using namespace OLEDBCLI;


//-------------------------------------------------------------------------
//
//	CForeignKey - table foreign key schema
//
//-------------------------------------------------------------------------


CForeignKey::CForeignKey(DBCONSTRAINTDESC* pConstraintDesc)
{
	ULONG	i;

	ATLASSERT(pConstraintDesc != NULL);
	ATLASSERT(pConstraintDesc->ConstraintType == DBCONSTRAINTTYPE_FOREIGNKEY);
	ATLASSERT(pConstraintDesc->cColumns == pConstraintDesc->cForeignKeyColumns);

	m_strName		= pConstraintDesc->pConstraintID->uName.pwszName;
	m_strRefTable	= pConstraintDesc->pReferencedTableID->uName.pwszName;
	m_match			= pConstraintDesc->MatchType;
	m_deleteRule	= pConstraintDesc->DeleteRule;
	m_updateRule	= pConstraintDesc->UpdateRule;
	m_deferrability	= pConstraintDesc->Deferrability;

	for(i = 0; i < pConstraintDesc->cColumns; ++i)
	{
		LPCTSTR				pszRefColumn	= pConstraintDesc->rgColumnList[i].uName.pwszName,
							pszKeyColumn	= pConstraintDesc->rgForeignKeyColumnList[i].uName.pwszName;
		CForeignKeyPair*	pPair			= new CForeignKeyPair(pszRefColumn, pszKeyColumn);

		if(pPair != NULL)
			m_pairs.Add(pPair);
	}
}


CForeignKey::~CForeignKey()
{
	Clear();
}


void CForeignKey::Clear()
{
	size_t	i, n = m_pairs.GetCount();

	for(i = 0; i < n; ++i)
		delete m_pairs[i];
	m_pairs.RemoveAll();
}

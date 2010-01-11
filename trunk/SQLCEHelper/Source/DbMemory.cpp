
#include "stdafx.h"

using namespace OLEDBCLI;


// CDbMemory::Free(DBPROP* pProp)
//
//		Frees a single DBPROP value
//
void CDbMemory::Free(DBPROP* pProp)
{
	ATLASSERT(pProp != NULL);

	VariantClear(&pProp->vValue);
	Free(pProp->colid);
}


// CDbMemory::Free
//
//		Frees a provider-allocated DBPROPSET
//
void CDbMemory::Free(DBPROPSET* pPropSet)
{
	ULONG iProp;

	ATLASSERT(pPropSet != NULL);

	for(iProp = 0; iProp < pPropSet->cProperties; ++iProp)
		Free(&pPropSet->rgProperties[iProp]);

	if(pPropSet->rgProperties != NULL)
		CoTaskMemFree(pPropSet->rgProperties);
}


void CDbMemory::Free(DBID &dbid)
{
	switch(dbid.eKind)
	{
	case DBKIND_GUID_NAME:
		if(dbid.uName.pwszName != NULL)
			CoTaskMemFree(dbid.uName.pwszName);
		break;

	case DBKIND_NAME:
		if(dbid.uName.pwszName != NULL)
			CoTaskMemFree(dbid.uName.pwszName);
		break;

	case DBKIND_PGUID_NAME:
		if(dbid.uName.pwszName != NULL)
			CoTaskMemFree(dbid.uName.pwszName);
		if(dbid.uGuid.pguid != NULL)
			CoTaskMemFree(dbid.uGuid.pguid);
		break;

	case DBKIND_PGUID_PROPID:
		if(dbid.uGuid.pguid != NULL)
			CoTaskMemFree(dbid.uGuid.pguid);
		break;

	}
}


void CDbMemory::Free(DBINDEXCOLUMNDESC *pKeyColumnDesc, ULONG nKeys)
{
	ULONG	i;

	for(i = 0; i < nKeys; ++i)
		Free(*pKeyColumnDesc[i].pColumnID);
	CoTaskMemFree(pKeyColumnDesc);
}

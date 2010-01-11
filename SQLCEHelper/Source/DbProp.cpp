
#include "stdafx.h"

using namespace OLEDBCLI;


CDbProp::CDbProp()
{
	dwPropertyID	= 0;
	dwOptions		= 0;
	dwStatus		= 0;
	memset(&colid, 0, sizeof(colid));
	VariantInit(&vValue);
}

CDbProp::CDbProp(const DBPROP &prop)
{
	Copy(prop);
}

CDbProp::~CDbProp()
{
	Clear();
}


void CDbProp::Copy(const DBPROP &prop)
{
	dwPropertyID	= prop.dwPropertyID;
	dwOptions		= prop.dwOptions;
	dwStatus		= prop.dwStatus;

	memcpy(&colid, &prop.colid, sizeof(DBID));
	VariantCopy(&vValue, const_cast<VARIANT*>(&prop.vValue));
}

void CDbProp::Clear()
{
	dwPropertyID	= 0;
	dwOptions		= 0;
	dwStatus		= 0;
	memset(&colid, 0, sizeof(colid));
	VariantClear(&vValue);
}

CDbProp& CDbProp::operator = (const CDbProp& rhs)
{
	if(this != &rhs)
		Copy(rhs);
	return *this;
}

HRESULT CDbProp::CopyTo(DBPROP *pProp) const
{
	ATLASSERT(pProp != NULL);

	HRESULT	hr;

	pProp->dwPropertyID	= dwPropertyID;
	pProp->dwOptions	= dwOptions;
	pProp->dwStatus		= dwStatus;

	memcpy(&pProp->colid, &colid, sizeof(DBID));
	VariantInit(&pProp->vValue);
	hr = VariantCopy(&pProp->vValue, (VARIANT*)&vValue);
	return hr;
}

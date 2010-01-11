
#include "stdafx.h"

using namespace OLEDBCLI;


CDbPropSet::CDbPropSet(GUID guid)
{
	guidPropertySet	= guid;
	cProperties		= 0;
	rgProperties	= NULL;
}


CDbPropSet::~CDbPropSet()
{
	Clear();
}


void CDbPropSet::Clear()
{
	ULONG	i;

	for(i = 0; i < cProperties; ++i)
		VariantClear(&rgProperties[i].vValue);

	cProperties = 0;
	CoTaskMemFree(rgProperties);
	rgProperties = NULL;
}


bool CDbPropSet::AddProperty(const CDbProp &prop)
{
	DBPROP*	pProps = (DBPROP*)CoTaskMemRealloc(rgProperties, (cProperties + 1) * sizeof(DBPROP));

	if(pProps != NULL)
	{
		prop.CopyTo(pProps + cProperties);
		rgProperties = pProps;
		++cProperties;
		return true;
	}
	return false;
}


bool CDbPropSet::AddProperty(DBPROPID id, int propVal)
{
	CDbProp prop;

	prop.dwPropertyID	= id;
	prop.dwOptions		= DBPROPOPTIONS_REQUIRED;
	prop.dwStatus		= DBPROPSTATUS_OK;
	prop.colid			= DB_NULLID;
	prop.vValue.vt		= VT_I4;
	prop.vValue.intVal	= propVal;

	return AddProperty(prop);
}


bool CDbPropSet::AddProperty(DBPROPID id, bool propVal)
{
	CDbProp prop;

	prop.dwPropertyID	= id;
	prop.dwOptions		= DBPROPOPTIONS_REQUIRED;
	prop.dwStatus		= DBPROPSTATUS_OK;
	prop.colid			= DB_NULLID;
	prop.vValue.vt		= VT_BOOL;
	prop.vValue.boolVal	= propVal ? VARIANT_TRUE : VARIANT_FALSE;

	return AddProperty(prop);
}


bool CDbPropSet::AddProperty(DBPROPID id, LPCTSTR propVal)
{
	CDbProp prop;

	prop.dwPropertyID	= id;
	prop.dwOptions		= DBPROPOPTIONS_REQUIRED;
	prop.dwStatus		= DBPROPSTATUS_OK;
	prop.colid			= DB_NULLID;
	prop.vValue.vt		= VT_BSTR;
	prop.vValue.bstrVal	= SysAllocString(propVal);

	if(prop.vValue.bstrVal == NULL)
		return false;

	return AddProperty(prop);
}

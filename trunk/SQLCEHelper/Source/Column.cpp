
#include "stdafx.h"

using namespace OLEDBCLI;


//-------------------------------------------------------------------------
//
//	CColumn class
//
//-------------------------------------------------------------------------


CColumn::CColumn()
:	m_wType		(DBTYPE_EMPTY),
	m_ulOrdinal	(0),
	m_ulSize	(0),
	m_dwFlags	(0),
	m_bPrecision(0),
	m_bScale	(0),
	m_bRowGuid	(false),
	m_bIdentity	(false)
{
}


CColumn::CColumn(DBCOLUMNDESC* pColumnDesc, ULONG iOrdinal)
:	m_wType		(DBTYPE_EMPTY),
	m_ulOrdinal	(0),
	m_ulSize	(0),
	m_dwFlags	(0),
	m_bPrecision(0),
	m_bScale	(0),
	m_bRowGuid	(false),
	m_bIdentity	(false)
{
	ATLASSERT(pColumnDesc != NULL);

	m_strName		= pColumnDesc->dbcid.uName.pwszName;
	m_wType			= pColumnDesc->wType;
	m_ulSize		= pColumnDesc->ulColumnSize;
	m_bPrecision	= pColumnDesc->bPrecision;
	m_bScale		= pColumnDesc->bScale;
	m_dwFlags		= 0;
	m_ulOrdinal		= iOrdinal;
	m_bRowGuid		= false;
	m_bIdentity		= false;
	m_nSeed			= 0;
	m_nIncrement	= 0;

	ParseProperties(pColumnDesc);
}


CColumn::~CColumn()
{
}


// CColumn::ParseProperties
//
//		Parses the column description properties
//
void CColumn::ParseProperties(DBCOLUMNDESC* pColumnDesc)
{
	ULONG		cs,
				cp;
	DBPROPSET*	pPropSet = pColumnDesc->rgPropertySets;

	for(cs = 0; cs < pColumnDesc->cPropertySets; ++cs, ++pPropSet)
	{
		if(pPropSet->guidPropertySet == DBPROPSET_SSCE_COLUMN)
		{
			DBPROP*	pProp = pPropSet->rgProperties;

			for(cp = 0; cp < pPropSet->cProperties; ++cp, ++pProp)
			{
				if(pProp->dwPropertyID == DBPROP_SSCE2_COL_ROWGUID ||
				   pProp->dwPropertyID == DBPROP_SSCE3_COL_ROWGUID)
					m_bRowGuid = (pProp->vValue.boolVal == VARIANT_TRUE);
			}
		}

		if(pPropSet->guidPropertySet == DBPROPSET_COLUMN)
		{
			DBPROP*	pProp = pPropSet->rgProperties;
			
			for(cp = 0; cp < pPropSet->cProperties; ++cp, ++pProp)
			{
				switch(pProp->dwPropertyID)
				{
				case DBPROP_COL_DEFAULT:
					{
						VARIANT	varStr;
						HRESULT	hr;

						VariantInit(&varStr);
						hr = VariantChangeType(&varStr, &pProp->vValue, 0, VT_BSTR);
						if(SUCCEEDED(hr))
							m_strDefault = (LPCTSTR)varStr.bstrVal;
						else
							m_strDefault = _T("VariantChangeType failed.");
						VariantClear(&varStr);
					}
					break;

				case DBPROP_COL_AUTOINCREMENT:
					m_bIdentity = (pProp->vValue.boolVal == VARIANT_TRUE);
					break;

				case DBPROP_COL_SEED:
					m_nSeed = pProp->vValue.intVal;
					break;

				case DBPROP_COL_INCREMENT:
					m_nIncrement = pProp->vValue.intVal;
					break;

				case DBPROP_COL_NULLABLE:
					if(pProp->vValue.boolVal == VARIANT_TRUE)
						m_dwFlags |= DBCOLUMNFLAGS_ISNULLABLE;
					break;

				case DBPROP_COL_ISLONG:
					if(pProp->vValue.boolVal == VARIANT_TRUE)
						m_dwFlags |= DBCOLUMNFLAGS_ISLONG;
					break;

				case DBPROP_COL_FIXEDLENGTH:
					if(pProp->vValue.boolVal == VARIANT_TRUE)
						m_dwFlags |= DBCOLUMNFLAGS_ISFIXEDLENGTH;
					break;

				default:
					break;
				}
			}
		}

	}
}

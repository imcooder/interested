
#include "stdafx.h"

using namespace OLEDBCLI;


CDbException::CDbException(HRESULT hr)
:	m_hr			(hr),
	m_pErrorRecords	(NULL)
{
	CComPtr<IErrorInfo>	spErrorInfo = NULL;

	hr = GetErrorInfo(0, &spErrorInfo);
	if(SUCCEEDED(hr))
		spErrorInfo->QueryInterface(IID_IErrorRecords, (void**)&m_pErrorRecords);
}

CDbException::~CDbException()
{
	if(m_pErrorRecords != NULL)
	{
		m_pErrorRecords->Release();
		m_pErrorRecords = NULL;
	}
}


ULONG CDbException::GetErrorCount()
{
	HRESULT hr		= S_OK;
	ULONG	nCount	= 0;

	if(m_pErrorRecords != NULL)
		hr = m_pErrorRecords->GetRecordCount(&nCount);
	return SUCCEEDED(hr) ? nCount : 0;
}


HRESULT CDbException::GetBasicErrorInfo(ULONG nError, ERRORINFO* pErrorInfo)
{
	HRESULT	hr = E_NOINTERFACE;

	if(m_pErrorRecords != NULL)
		hr = m_pErrorRecords->GetBasicErrorInfo(nError, pErrorInfo);
	return hr;
}


HRESULT CDbException::GetErrorParameters(ULONG nError, DISPPARAMS* pDispParams)
{
	HRESULT	hr = E_NOINTERFACE;

	if(m_pErrorRecords != NULL)
		hr = m_pErrorRecords->GetErrorParameters(nError, pDispParams);
	return hr;
}


HRESULT CDbException::GetErrorSource(ULONG nError, BSTR* pBstrSource, LCID lcid)
{
	HRESULT	hr = E_NOINTERFACE;

	if(m_pErrorRecords != NULL)
	{
		CComPtr<IErrorInfo>	spErrorInfo;

		hr = m_pErrorRecords->GetErrorInfo(nError, lcid, &spErrorInfo);
		if(SUCCEEDED(hr))
			hr = spErrorInfo->GetSource(pBstrSource);
	}
	return hr;
}


HRESULT CDbException::GetErrorSource(ULONG nError, CString& strSource, LCID lcid)
{
	HRESULT	hr;
	BSTR	bstr;

	hr = GetErrorSource(nError, &bstr, lcid);
	if(SUCCEEDED(hr))
	{
		strSource = bstr;
		SysFreeString(bstr);
	}
	return hr;
}


HRESULT CDbException::GetErrorDescription(ULONG nError, BSTR* pBstrDescription, LCID lcid)
{
	HRESULT	hr = E_NOINTERFACE;

	if(m_pErrorRecords != NULL)
	{
		CComPtr<IErrorInfo>	spErrorInfo;

		hr = m_pErrorRecords->GetErrorInfo(nError, lcid, &spErrorInfo);
		if(SUCCEEDED(hr))
			hr = spErrorInfo->GetDescription(pBstrDescription);
	}
	return hr;
}


HRESULT CDbException::GetErrorDescription(ULONG nError, CString& strDescription, LCID lcid)
{
	HRESULT	hr;
	BSTR	bstr;

	hr = GetErrorDescription(nError, &bstr, lcid);
	if(SUCCEEDED(hr))
	{
		strDescription = bstr;
		SysFreeString(bstr);
	}
	return hr;
}

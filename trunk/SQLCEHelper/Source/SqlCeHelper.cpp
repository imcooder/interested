#include "StdAfx.h"
#include "../Include/SqlCeHelper.h"

/*
TO-DO-LIST
**Support to create database file programmatically
**Support to open the database with password
**Support to specify the Open Mode
**Support to load Ole Db without COM registry.
	//http://social.msdn.microsoft.com/Forums/en-US/sqlce/thread/35db5fb8-443c-4b87-bda7-68fc7df6c6e6
**Support Transactions
**Support Parameters

*/

//http://www.microsoft.com/downloads/details.aspx?FamilyID=1ff0529a-eb1f-4044-b4b7-40b00710f7b7&displaylang=en
//Microsoft SQL Server Compact 3.5 Books Online and Samples

//C:\Program Files\Microsoft SQL Server Compact Edition\v3.5\Devices\wce500\armv4i
//sqlce.ppc.wce5.armv4i.CAB
//sqlce.repl.ppc.wce5.armv4i.CAB


SqlCeHelper::SqlCeHelper(void):
isOpen(false),
currentRowCount(0)
{
}

SqlCeHelper::~SqlCeHelper(void)
{
	Close();
}


bool SqlCeHelper::CreateDatabase(const CString& dbPath)
{
	HRESULT hr =		-1;
	DBPROP				dbprop[1];					// property used in property set to initialize provider
	DBPROPSET			dbpropset[1];				// Property Set used to initialize provider

	// Initialize environment
	//
	hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
	if(FAILED(hr))
	{
         goto Exit;
	}

	VariantInit(&dbprop[0].vValue);	

	HANDLE				hFind;							// File handle
	WIN32_FIND_DATA		FindFileData;					// The file structure description 
	hFind = FindFirstFile(dbPath, &FindFileData);
	if (INVALID_HANDLE_VALUE != hFind)
	{
		FindClose(hFind);
		DeleteDatabase(dbPath);
	}

	// Initialize a property with name of database
	//
    dbprop[0].dwPropertyID	= DBPROP_INIT_DATASOURCE;
	dbprop[0].dwOptions		= DBPROPOPTIONS_REQUIRED;
    dbprop[0].vValue.vt		= VT_BSTR;
	dbprop[0].vValue.bstrVal= SysAllocString(dbPath);
	if(NULL == dbprop[0].vValue.bstrVal)
	{
		goto Exit;
	}

	// Initialize the property set
	//
	dbpropset[0].guidPropertySet = DBPROPSET_DBINIT;
	dbpropset[0].rgProperties	 = dbprop;
	dbpropset[0].cProperties	 = sizeof(dbprop)/sizeof(dbprop[0]);

	hr = dataSource.Create(CLSID_SQLSERVERCE, 1, dbpropset, &session);
	if(FAILED(hr))
	{
		goto Exit;
	}

Exit:
	VariantClear(&dbprop[0].vValue);
	return (hr >= 0);
}

bool SqlCeHelper::DeleteDatabase(const CString& dbPath)
{
	BOOL r = DeleteFile(dbPath);
	return r?true:false;
}

bool SqlCeHelper::Open(const CString& dbPath)
{
	HRESULT hr =		-1;
	DBPROP				dbprop[1];					// property used in property set to initialize provider
	DBPROPSET			dbpropset[1];				// Property Set used to initialize provider

	// Initialize environment
	//
	hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
	if(FAILED(hr))
	{
         goto Exit;
	}
	
	HANDLE				hFind;							// File handle
	WIN32_FIND_DATA		FindFileData;					// The file structure description 
	hFind = FindFirstFile(dbPath, &FindFileData);
	if (INVALID_HANDLE_VALUE == hFind)
	{
		goto Exit;
	}

	// Initialize a property with name of database
	//
    dbprop[0].dwPropertyID	= DBPROP_INIT_DATASOURCE;
	dbprop[0].dwOptions		= DBPROPOPTIONS_REQUIRED;
    dbprop[0].vValue.vt		= VT_BSTR;
	dbprop[0].vValue.bstrVal= SysAllocString(dbPath);
	if(NULL == dbprop[0].vValue.bstrVal)
	{
		goto Exit;
	}

	// Initialize the property set
	//
	dbpropset[0].guidPropertySet = DBPROPSET_DBINIT;
	dbpropset[0].rgProperties	 = dbprop;
	dbpropset[0].cProperties	 = sizeof(dbprop)/sizeof(dbprop[0]);

	hr = dataSource.Open(CLSID_SQLSERVERCE, 1, dbpropset);
	if(FAILED(hr))
	{
		goto Exit;
	}
	
	hr = session.Open(dataSource);
	if(FAILED(hr))
	{
		goto Exit;
	}
	
Exit:
	VariantClear(&dbprop[0].vValue);
	return (hr >= 0);
}

void SqlCeHelper::Close()
{
	if(!isOpen)
	{
		return;
	}

	CloseReader();
	session.Close();
	dataSource.Close();
	
	// Uninitialize the environment
	CoUninitialize();
	isOpen = false;
}

int SqlCeHelper::ExecuteNonQuery(const CString& sql)
{
	CCommand command(session);
	command.SetText(sql);
	LONG rowsAffected;
	command.Execute(&rowsAffected);
	
	return (int)rowsAffected;
}


void SqlCeHelper::BeginTransaction()
{
}

void SqlCeHelper::CommitTransaction()
{
}

void SqlCeHelper::RollbackTransaction()
{
}

//Changed from
//ms-help://MS.SSC.v35/MS.SSC.v35.EN/ssctechref/html/a25fafe1-e90a-4545-8836-749dd44135c0.htm
CString SqlCeHelper::GetErrorsMessage()
{
    static CString sErrIErrorInfo     = L"IErrorInfo interface";
    static CString sErrIErrorRecords  = L"IErrorRecords interface";
    static CString sErrRecordCount    = L"error record count";
    static CString sErrInfo           = L"ERRORINFO structure";
    static CString sErrStandardInfo   = L"standard error info";
    static CString sErrDescription    = L"standard error description";
    static CString sErrNoSource       = L"error source";

    HRESULT hr                          = S_OK;
    IErrorInfo       *pIErrorInfo       = NULL;
    IErrorRecords    *pIErrorRecords    = NULL;
    ERRORINFO        errorInfo          = { 0 };
    IErrorInfo       *pIErrorInfoRecord = NULL;
	
	CString message = L"";
	char str[255];
    try
    {
        // This interface supports returning error information.
        // Get the error object from the system for the current
        // thread.
        hr = GetErrorInfo(0, &pIErrorInfo);
        if ( hr == S_FALSE )
        {
            message = "No error occured.";
            return message;
        }

        if(FAILED(hr) || NULL == pIErrorInfo)
            throw sErrIErrorInfo;

        // The error records are retrieved from the IIErrorRecords
        // interface, which can be obtained from the IErrorInfo
        // interface.
        hr = pIErrorInfo->QueryInterface(IID_IErrorRecords,
            (void **) &pIErrorRecords);
        if ( FAILED(hr) || NULL == pIErrorRecords )
            throw sErrIErrorRecords;

        // The IErrorInfo interface is no longer required because
        // we have the IErrorRecords interface, relase it.
        pIErrorInfo->Release();
        pIErrorInfo = NULL;

        ULONG ulNumErrorRecs = 0;

        // Determine the number of records in this error object
        hr = pIErrorRecords->GetRecordCount(&ulNumErrorRecs);
        if ( FAILED(hr) )
            throw sErrRecordCount;


        // Loop over each error record in the error object to display 
        // information about each error. Errors are returned. 
        for (DWORD dwErrorIndex = 0;
             dwErrorIndex < ulNumErrorRecs;
             dwErrorIndex++)
        {
            // Retrieve basic error information for this error.
            hr = pIErrorRecords->GetBasicErrorInfo(dwErrorIndex,
              &errorInfo);
            if ( FAILED(hr) )
                throw sErrInfo;

            TCHAR szCLSID[64]  = { 0 };
            TCHAR szIID[64]    = { 0 };
            TCHAR szDISPID[64] = { 0 };

            StringFromGUID2(errorInfo.clsid, (LPOLESTR)szCLSID,
                sizeof(szCLSID));
            StringFromGUID2(errorInfo.iid, (LPOLESTR)szIID,
                sizeof(szIID));

            sprintf(str, "HRESULT           = %lx\n", errorInfo.hrError);
            message += str;
			sprintf(str, "clsid             = %S\n", szCLSID);
            message += str;
            sprintf(str, "iid               = %S\n", szIID);
            message += str;
            sprintf(str, "dispid            = %ld\n", errorInfo.dispid);
            message += str;
            sprintf(str, "Native Error Code = %lx\n", errorInfo.dwMinor);
            message += str;

            // Retrieve standard error information for this error.
            hr = pIErrorRecords->GetErrorInfo(dwErrorIndex, NULL,
                &pIErrorInfoRecord);

            if ( FAILED(hr) )
                throw sErrStandardInfo;

            BSTR bstrDescriptionOfError;
            BSTR bstrSourceOfError;

            // Get the description of the error.
            hr = pIErrorInfoRecord->GetDescription(
                   &bstrDescriptionOfError);
            if ( FAILED(hr) )
                throw sErrDescription;

            sprintf(str, "Description = %S\n", bstrDescriptionOfError);
            message += str;
			
            // Get the source of the error.
            hr = pIErrorInfoRecord->GetSource(&bstrSourceOfError);
            if ( FAILED(hr) )
                throw sErrNoSource;

            sprintf(str, "Description = %S\n", bstrSourceOfError);
            message += str;
			
            // This interface variable will be used the next time 
            // though this loop. In the last error case this interface 
            // is no longer needed so we must release it.
            if(NULL != pIErrorInfoRecord)
                pIErrorInfoRecord->Release();
            pIErrorInfoRecord = NULL;
        }
    }
	catch( CString& szMsg )
    {
        message = L"Failed to retrieve " + szMsg;
    }

    if( pIErrorInfoRecord )
        pIErrorInfoRecord->Release();

    if ( pIErrorInfo )
        pIErrorInfo->Release();

    if ( pIErrorRecords )
        pIErrorRecords->Release();

    return message;
}

int SqlCeHelper::ExecuteReader(const CString& sql)
{
	CCommand command(session);
	command.SetText(sql);
	ULONG rows;
	command.Execute(rowset);
	rowset.GetRowCount(&rows);
	currentRowCount = 0;
	totalRowCount = (int)rows;

	rowset.MoveFirst();
	return totalRowCount;
}


bool SqlCeHelper::IsEndOfRecordSet()
{
	return (currentRowCount == totalRowCount);
}

void SqlCeHelper::MoveNext()
{
	++currentRowCount;
	rowset.MoveNext();
}

void SqlCeHelper::CloseReader()
{
	currentRowCount = 0;
	totalRowCount = 0;
	rowset.Close();
}

int SqlCeHelper::GetRowInt(const long ordinal)
{
	int i;
	rowset.GetValue(ordinal, i);
	return i;
}

double SqlCeHelper::GetRowDouble(const long ordinal)
{	
	double d;
	rowset.GetValue(ordinal, d);
	return d;
}

CString SqlCeHelper::GetRowStr(const long ordinal)
{
	CString s;
	rowset.GetValue(ordinal, s);
	return s;
}

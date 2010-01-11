#pragma once


#include "../Include/SqlCeHelper.h"

#define DB_FILE_NAME L"SqlCeHelperTest.sdf"
int max = 100;

TEST(SqlCeHelper, SelectDataAfterDelete)
{
	printf("\n====After Delete.====\n");
	SqlCeHelper db;
	HRESULT hr = db.Open(DB_FILE_NAME);
	if(hr < 0)
	{
		CString cstr = db.GetErrorsMessage();
		char* str = new char[cstr.GetLength()+1];
		sprintf(str, "%S", cstr);
		FAIL(str);
		delete str;
	}
	
	CString sqlStr;
	sqlStr.Format(L"SELECT * FROM T1");
	hr = db.ExecuteReader(sqlStr);
	if(hr < 0)
	{
		CString cstr = db.GetErrorsMessage();
		char* str = new char[cstr.GetLength()+1];
		sprintf(str, "%S", cstr);
		FAIL(str);
		delete str;
	}
	
	while(!db.IsEndOfRecordSet())
	{
		wprintf(L"F1=[%d], F2=[%s], F3=[%s]\n", db.GetRowInt(1), db.GetRowStr(2), db.GetRowStr(3));
		db.MoveNext();
	}

	db.CloseReader();
	db.Close();
}

TEST(SqlCeHelper, DeleteData)
{
	SqlCeHelper db;
	HRESULT hr = db.Open(DB_FILE_NAME);
	if(hr < 0)
	{
		CString cstr = db.GetErrorsMessage();
		char* str = new char[cstr.GetLength()+1];
		sprintf(str, "%S", cstr);
		FAIL(str);
		delete str;
	}

	CString sqlStr;
	for(int i=0; i<max; ++i)
	{
		sqlStr.Format(L"DELETE FROM T1 WHERE F1=%d", i);
		hr = db.ExecuteNonQuery(sqlStr);
		if(hr < 0)
		{
			CString cstr = db.GetErrorsMessage();
			char* str = new char[cstr.GetLength()+1];
			sprintf(str, "%S", cstr);
			FAIL(str);
			delete str;
		}
	}

	db.Close();
}

TEST(SqlCeHelper, SelectDataBeforeDelete)
{
	printf("\n====Before Delete.====\n");
	SqlCeHelper db;
	HRESULT hr = db.Open(DB_FILE_NAME);
	if(hr < 0)
	{
		CString cstr = db.GetErrorsMessage();
		char* str = new char[cstr.GetLength()+1];
		sprintf(str, "%S", cstr);
		FAIL(str);
		delete str;
	}
	
	CString sqlStr;
	sqlStr.Format(L"SELECT * FROM T1");
	hr = db.ExecuteReader(sqlStr);
	if(hr < 0)
	{
		CString cstr = db.GetErrorsMessage();
		char* str = new char[cstr.GetLength()+1];
		sprintf(str, "%S", cstr);
		FAIL(str);
		delete str;
	}
	
	while(!db.IsEndOfRecordSet())
	{
		wprintf(L"F1=[%d], F2=[%s], F3=[%s]\n", db.GetRowInt(1), db.GetRowStr(2), db.GetRowStr(3));
		db.MoveNext();
	}

	db.CloseReader();
	db.Close();
}


TEST(SqlCeHelper, UpdateData)
{
	SqlCeHelper db;
	HRESULT hr = db.Open(DB_FILE_NAME);
	if(hr < 0)
	{
		CString cstr = db.GetErrorsMessage();
		char* str = new char[cstr.GetLength()+1];
		sprintf(str, "%S", cstr);
		FAIL(str);
		delete str;
	}

	CString sqlStr;
	for(int i=0; i<max; ++i)
	{
		SYSTEMTIME currentTime;	
		GetLocalTime(&currentTime);
		sqlStr.Format(L"UPDATE T1 SET F2='STR%d', F3='%d-%d-%d %d:%d:%d' WHERE F1=%d", 
			max-i, currentTime.wYear, currentTime.wMonth, currentTime.wDay, currentTime.wHour, currentTime.wMinute, currentTime.wSecond, i);
		hr = db.ExecuteNonQuery(sqlStr);
		if(hr < 0)
		{
			CString cstr = db.GetErrorsMessage();
			char* str = new char[cstr.GetLength()+1];
			sprintf(str, "%S", cstr);
			FAIL(str);
			delete str;
		}
	}

	db.Close();
}

TEST(SqlCeHelper, SelectDataBeforeUpdate)
{
	printf("\n====Before Update.====\n");
	SqlCeHelper db;
	HRESULT hr = db.Open(DB_FILE_NAME);
	if(hr < 0)
	{
		CString cstr = db.GetErrorsMessage();
		char* str = new char[cstr.GetLength()+1];
		sprintf(str, "%S", cstr);
		FAIL(str);
		delete str;
	}
	
	CString sqlStr;
	sqlStr.Format(L"SELECT * FROM T1");
	hr = db.ExecuteReader(sqlStr);
	if(hr < 0)
	{
		CString cstr = db.GetErrorsMessage();
		char* str = new char[cstr.GetLength()+1];
		sprintf(str, "%S", cstr);
		FAIL(str);
		delete str;
	}
	
	while(!db.IsEndOfRecordSet())
	{
		wprintf(L"F1=[%d], F2=[%s], F3=[%s]\n", db.GetRowInt(1), db.GetRowStr(2), db.GetRowStr(3));
		db.MoveNext();
	}
	db.CloseReader();
	db.Close();
}

TEST(SqlCeHelper, InsertData)
{
	SqlCeHelper db;
	HRESULT hr = db.Open(DB_FILE_NAME);
	if(hr < 0)
	{
		CString cstr = db.GetErrorsMessage();
		char* str = new char[cstr.GetLength()+1];
		sprintf(str, "%S", cstr);
		FAIL(str);
		delete str;
	}

	CString sqlStr;
	for(int i=0; i<max; ++i)
	{
		SYSTEMTIME currentTime;	
		GetLocalTime(&currentTime);
		sqlStr.Format(L"INSERT INTO T1 (F1, F2, F3) VALUES(%d, 'STR%d', '%d-%d-%d %d:%d:%d')", 
			i, i, currentTime.wYear, currentTime.wMonth, currentTime.wDay, currentTime.wHour, currentTime.wMinute, currentTime.wSecond);
		hr = db.ExecuteNonQuery(sqlStr);
		if(hr < 0)
		{
			CString cstr = db.GetErrorsMessage();
			char* str = new char[cstr.GetLength()+1];
			sprintf(str, "%S", cstr);
			FAIL(str);
			delete str;
		}
	}

	db.Close();
}

TEST(SqlCeHelper, SelectDataBeforeInsert)
{
	printf("\n====Before Insert.====\n");
	SqlCeHelper db;
	HRESULT hr = db.Open(DB_FILE_NAME);
	if(hr < 0)
	{
		CString cstr = db.GetErrorsMessage();
		char* str = new char[cstr.GetLength()+1];
		sprintf(str, "%S", cstr);
		FAIL(str);
		delete str;
	}
	
	CString sqlStr;
	sqlStr.Format(L"SELECT * FROM T1");
	hr = db.ExecuteReader(sqlStr);
	if(hr < 0)
	{
		CString cstr = db.GetErrorsMessage();
		char* str = new char[cstr.GetLength()+1];
		sprintf(str, "%S", cstr);
		FAIL(str);
		delete str;
	}
	
	while(!db.IsEndOfRecordSet())
	{
		wprintf(L"F1=[%d], F2=[%s], F3=[%s]\n", db.GetRowInt(1), db.GetRowStr(2), db.GetRowStr(3));
		db.MoveNext();
	}
	db.CloseReader();
	db.Close();
}

TEST(SqlCeHelper, OpenDatabase)
{
	SqlCeHelper db;
	bool b = db.Open(DB_FILE_NAME);
	if(!b)
	{
		CString cstr = db.GetErrorsMessage();
		char* str = new char[cstr.GetLength()+1];
		sprintf(str, "%S", cstr);
		FAIL(str);
		delete str;
	}
	
	//if(hr == -1)
	//{
	//	FAIL("The database file doesn't exist.");
	//}
	//else if(hr == REGDB_E_CLASSNOTREG)
	//{
	//	FAIL("SqlCe 3.5 is not installed or the sqlceoledb35.dll is not registered.");
	//}
	//else if(hr < 0)
	//{
	//	FAIL(db.GetErrorsMessage().c_str());
	//	//FAIL("Cannot open the database.");
	//}

	db.Close();
}
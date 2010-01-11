// SqlCeHelper class
// based on OLE DB Client class library, João Paulo Figueira
// Copyright (C) 2009 by Jake Lin
//
//

#pragma once

#include "oledbcli.h"
using namespace OLEDBCLI;

class SqlCeHelper
{
public:
	SqlCeHelper(void);
	~SqlCeHelper(void);

	bool Open(const CString& dbPath);
	void Close();

	//If database exists, delete it and create new one.
	bool CreateDatabase(const CString& dbPath);
	bool DeleteDatabase(const CString& dbPath);

	int	 ExecuteReader(const CString& sql);
	bool IsEndOfRecordSet();
	void MoveNext();
	void CloseReader();

	int		GetRowInt(const long ordinal);
	double	GetRowDouble(const long ordinal);
	CString	GetRowStr(const long ordinal);

	int  ExecuteNonQuery(const CString& sql);

	void BeginTransaction();
	void CommitTransaction();
	void RollbackTransaction();

	CString GetErrorsMessage();

private:
	bool		isOpen;
	CDataSource	dataSource;
	CSession	session;
	CRowset		rowset;
	int			totalRowCount;
	int			currentRowCount;
};

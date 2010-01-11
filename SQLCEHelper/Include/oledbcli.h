// OLE DB Client class library
//
// Copyright (C) 2008,2009 by João Paulo Figueira
//

#ifndef __OLEDBCLI_H__
#define __OLEDBCLI_H__
#include "sqlce_oledb.h"
#include "sqlce_ex.h"
#include "BlobStream.h"


namespace OLEDBCLI 
{
	class CSession;
	class CRowset;
	class CSchema;


	// CDbMemory
	//
	//		Static class to manage OLE DB memory
	//
	class CDbMemory
	{
	public:
		static void	Free(DBPROP* pProp);
		static void Free(DBPROPSET* pPropSet);
		static void Free(DBID &dbid);
		static void Free(DBINDEXCOLUMNDESC* pKeyColumnDesc, ULONG nKeys);
	};


	// CDbProp
	//
	//		Wraps the DBPROP structure
	//
	class CDbProp : public tagDBPROP
	{
	private:
		void Copy(const DBPROP &prop);

	public:
		CDbProp();
		CDbProp(const DBPROP &prop);
		~CDbProp();

		void Clear();

		CDbProp& operator = (const CDbProp& rhs);

		HRESULT CopyTo(DBPROP *pProp) const;
	};

	// CDbPropSet
	//
	//		A set of CDbProp objects
	//
	class CDbPropSet : public tagDBPROPSET
	{
	public:
		CDbPropSet(GUID guid);
		~CDbPropSet();

		void Clear		();
		bool AddProperty(const CDbProp &prop);
		bool AddProperty(DBPROPID id, int propVal);
		bool AddProperty(DBPROPID id, bool propVal);
		bool AddProperty(DBPROPID id, LPCTSTR propVal);

	};


	// CDbException
	//
	//		OLE DB exception
	//
	class CDbException
	{
	protected:
		HRESULT			m_hr;
		IErrorRecords*	m_pErrorRecords;

	public:
		CDbException(HRESULT hr);
		virtual ~CDbException();

		ULONG	GetErrorCount		();
		HRESULT GetBasicErrorInfo	(ULONG nError, ERRORINFO* pErrorInfo);
		HRESULT GetErrorParameters	(ULONG nError, DISPPARAMS* pDispParams);
		HRESULT GetErrorSource		(ULONG nError, BSTR* pBstrSource, LCID lcid = LOCALE_SYSTEM_DEFAULT);
		HRESULT GetErrorSource		(ULONG nError, CString& strSource, LCID lcid = LOCALE_SYSTEM_DEFAULT);
		HRESULT GetErrorDescription	(ULONG nError, BSTR* pBstrDescription, LCID lcid = LOCALE_SYSTEM_DEFAULT);
		HRESULT GetErrorDescription	(ULONG nError, CString& strDescription, LCID lcid = LOCALE_SYSTEM_DEFAULT);
	};


	// CBindingArray
	//
	//		A "managed" DBBINDING array. Note the special requirement when cleaning up.
	//
	class CBindingArray
	{
	private:
		DBBINDING*	m_pBinding;
		ULONG		m_nItems;

	public:
		CBindingArray() : m_pBinding(NULL), m_nItems(0) { }

		~CBindingArray()
		{
			Clear();
		}

		void Clear()
		{
			ULONG		i;
			DBBINDING*	p;

			for(i = 0, p = m_pBinding; i < m_nItems; ++i, ++p)
			{
				if(p->pObject != NULL)		// DBOBJECT allocated?
					delete p->pObject;		// Delete it...
			}
			delete [] m_pBinding;

			m_pBinding	= NULL;
			m_nItems	= 0;
		}

		bool Allocate(ULONG nItems)
		{
			Clear();

			m_pBinding = new DBBINDING[nItems];
			if(m_pBinding != NULL)
			{
				memset(m_pBinding, 0, nItems * sizeof(DBBINDING));
				m_nItems = nItems;
				return true;
			}
			return false;
		}

		DBBINDING* operator[] (ULONG i)
		{
			ATLASSERT(i < m_nItems);
			return m_pBinding + i;
		}

	};


	// BOUNDCOLUMN
	//
	//		Stores information about a bound column
	//
	struct BOUNDCOLUMN
	{
		LPOLESTR		pwszName;			// Column name
		ULONG			iOrdinal;			// Column ordinal
		ULONG			obValue;			// Binding value offset
		ULONG			obLength;			// Binding length offset
		ULONG			obStatus;			// Binding status offset
		int				iAccessor;			// Accessor handle index
		DBCOLUMNFLAGS	dwFlags;			// Column flags
		ULONG			ulColumnSize;		// Column size
		DBTYPE			wType;				// Column type (DBTYPE_*)
		BYTE			bPrecision;			// Column precision
		BYTE			bScale;				// Column scale
	};


	// CDbValueRef
	//
	//		Value reference
	//
	class CDbValueRef
	{
	protected:
		ULONG*		m_pLength;
		DBSTATUS*	m_pStatus;
		DBTYPE		m_wType;
		ULONG		m_nMaxSize;
		BYTE*		m_pValue;
		bool		m_bIsLong;

	public:
		CDbValueRef();
		CDbValueRef(BOUNDCOLUMN* pBoundColumn, BYTE* pData);
		CDbValueRef(DBTYPE wType, ULONG* pLength, DBSTATUS* pStatus, BYTE* pValue);

		void		FreeStorage	();

		ULONG		GetLength	()	{ return *m_pLength;	}
		ULONG		GetMaxLength()	{ return m_nMaxSize;	}
		DBSTATUS	GetStatus	()	{ return *m_pStatus;	}
		DBTYPE		GetType		()	{ return m_wType;		}
		bool		IsLong		()	{ return m_bIsLong;		}

		void		SetStatus	(DBSTATUS status)	{ *m_pStatus = status;	}
		void		SetLength	(ULONG nLength)		{ *m_pLength = nLength;	}

		bool		GetValue	(bool&			bVal);
		bool		SetValue	(bool			bVal);
		bool		GetValue	(BYTE&			bVal);
		bool		SetValue	(BYTE			bVal)	{ return SetValue(int(bVal));	}
		bool		GetValue	(int&			nVal);
		bool		SetValue	(int			nVal);
		bool		GetValue	(short&			nVal);
		bool		SetValue	(short			nVal)	{ return SetValue(int(nVal));	}
		bool		GetValue	(__int64&		nVal);
		bool		SetValue	(__int64		nVal);
		bool		GetValue	(double&		dblVal);
		bool		SetValue	(double			dblVal);
		bool		GetValue	(float&			fltVal);
		bool		SetValue	(float			fltVal);
		bool		GetValue	(CString&		strVal);
		bool		SetValue	(LPCTSTR		pszVal);
		bool		SetValue	(CString&		strVal)	{ return SetValue((LPCTSTR)strVal);	}
		bool		GetValue	(BYTE*			pVal);
		bool		SetValue	(BYTE*			pVal, ULONG nLength);
		bool		GetValue	(GUID&			guid);
		bool		SetValue	(GUID&			guid);
		bool		GetValue	(CY&			cyVal);
		bool		SetValue	(CY				cyVal);
		bool		GetValue	(DBTIMESTAMP&	dtVal);
		bool		SetValue	(DBTIMESTAMP&	dtVal);
		bool		GetValue	(DB_NUMERIC&	numVal);
		bool		SetValue	(DB_NUMERIC&	numVal);
		bool		SetValue	(CBlobStream&	blob);
		bool		GetValue	(CBlobStream&	blob);
	};


	// CDbValue
	//
	//		Value
	//
	class CDbValue : public CDbValueRef
	{
	private:
		union val
		{
			BYTE			bVal;
			int				intVal;
			__int64			lngVal;
			short			shortVal;
			double			dblVal;
			float			fltVal;
			CY				cyVal;
			GUID			guidVal;
			VARIANT_BOOL	boolVal;
			DBTIMESTAMP		dtVal;
			DB_NUMERIC		numVal;
			BYTE*			pVal;
			TCHAR*			pszVal;
		}			m_val;				// Value
		ULONG		m_nLength;			// Length
		DBSTATUS	m_status;			// Status

		void	InternalCopy	(const CDbValue& dbVal);

	public:
		CDbValue();
		CDbValue(const CDbValue &dbVal);
		CDbValue(DBTYPE type, ULONG nSize, DBSTATUS status, const void* pData);
		~CDbValue();

		void	Initialize	(DBTYPE type, ULONG nLength, ULONG nMaxLength, bool bIsLong);
		void	SetStatus	(DBSTATUS status) { m_status = status; }
		void	Clear		();
		void*	GetDataPtr	();

		void	SetValue	(DBTYPE type, ULONG nLength, DBSTATUS status, const void* pData);
		HRESULT	SetValue	(DBTYPE type, ULONG nLength, DBSTATUS status, ISequentialStream* pStream);

		bool	SetValue	(LPCTSTR pszVal);
		bool	SetValue	(BYTE* pVal, ULONG nSize);

		CDbValue&	operator = (const CDbValue& dbVal);
	};


	// CDataSource
	//
	//		Models an OLE DB Data Source object
	//
	class CDataSource
	{
	protected:
		IDBInitialize*	m_pInitialize;

	public:
		CDataSource();
		virtual ~CDataSource();

		HRESULT Open	(const CLSID& clsid, ULONG cPropSets, DBPROPSET rgPropSets[]);
		HRESULT	Create	(const CLSID& clsid, ULONG cPropSets, DBPROPSET rgPropSets[], CSession *pSession = NULL);
		HRESULT Close	();

		operator IDBInitialize*() const { return m_pInitialize; }
	};


	// CSession
	//
	//		Models an OLE DB Session object
	//
	class CSession
	{
	protected:
		ISessionProperties*	m_pSessionProps;

	public:
		CSession();
		virtual ~CSession();

		operator IUnknown*() const { return m_pSessionProps; }

		HRESULT	Open		(ISessionProperties* pSessionProperties);
		HRESULT Open		(const CDataSource& dataSource);
		void	Close		();

		HRESULT	SetProperties	(ULONG cPropSets, DBPROPSET rgPropSets[]);
	};


	// CForeignKeyPair
	//
	//		A pair of columns matched in a FOREIGN KEY
	//
	class CForeignKeyPair
	{
	private:
		CString		m_strRefName,
					m_strKeyName;

	public:
		CForeignKeyPair(LPCTSTR pszRefName, LPCTSTR pszKeyName)
			:	m_strRefName(pszRefName),
				m_strKeyName(pszKeyName)
		{
		}

		LPCTSTR		GetRefName	()	{ return m_strRefName;	}
		LPCTSTR		GetKeyName	()	{ return m_strKeyName;	}
	};

	typedef CAtlArray<CForeignKeyPair*>	CForeignKeyPairArray;


	// CForeignKey
	//
	//		A foreign-key definition
	//
	class CForeignKey
	{
	private:
		CString					m_strName,
								m_strRefTable;
		CForeignKeyPairArray	m_pairs;
		DBMATCHTYPE				m_match;
		DBUPDELRULE				m_deleteRule,
								m_updateRule;
		DBDEFERRABILITY			m_deferrability;

		void Clear();

	public:
		CForeignKey(DBCONSTRAINTDESC* pConstraintDesc);
		~CForeignKey();

		LPCTSTR			GetName			()	{ return m_strName;			}
		LPCTSTR			GetRefTableName	()	{ return m_strRefTable;		}
		DBMATCHTYPE		GetMatchType	()	{ return m_match;			}
		DBUPDELRULE		GetDeleteRule	()	{ return m_deleteRule;		}
		DBUPDELRULE		GetUpdateRule	()	{ return m_updateRule;		}
		DBDEFERRABILITY	GetDeferrability()	{ return m_deferrability;	}

		bool				AddPair			(LPCTSTR pszRefName, LPCTSTR pszKeyName);
		int					GetPairCount	()				{ return m_pairs.GetCount();	}
		CForeignKeyPair*	GetPair			(int iIndex)	{ return m_pairs[iIndex];		}
	};

	typedef CAtlArray<CForeignKey*>	CForeignKeyArray;


	// CIndexColumn
	//
	//		Contains an index column
	//
	class CIndexColumn
	{
	private:
		CString		m_strName;
		short		m_nCollation;
		ULONG		m_nOrdinal;

	public:
		CIndexColumn() : m_nCollation(DB_COLLATION_ASC), m_nOrdinal(0) { }
		CIndexColumn(LPCTSTR pszColumnName, short nCollation, ULONG nOrdinal)
			:	m_strName	(pszColumnName),
				m_nCollation(nCollation),
				m_nOrdinal	(nOrdinal)
		{
		}

		LPCTSTR		GetName		()	{ return m_strName;		}
		short		GetCollation()	{ return m_nCollation;	}
		ULONG		GetOrdinal	()	{ return m_nOrdinal;	}
	};

	typedef CAtlArray<CIndexColumn*> CIndexColumnArray;


	// CIndex
	//
	//		Contains information about a table index
	//
	class CIndex
	{
	private:
		CString				m_strName;
		bool				m_bUnique,
							m_bPrimary,
							m_bConstraint;
		CIndexColumnArray	m_columns;

	public:
		CIndex();
		CIndex(LPCTSTR pszIndexName, bool bUnique, bool bPrimary);
		CIndex(DBCONSTRAINTDESC* pConstraintDesc);
		~CIndex();

		void		SetName				(LPCTSTR pszName)	{ m_strName = pszName;	}
		LPCTSTR		GetName				()	const			{ return m_strName;		}

		int				GetColumnCount	()				{ return int(m_columns.GetCount());	}
		CIndexColumn*	GetColumn		(int iColumn)	{ return m_columns[iColumn];		}
		void			AddColumn		(LPCTSTR pszColumnName, short nCollation, ULONG nOrdinal);
		int				FindColumn		(LPCTSTR pszColumnName);
		
		bool		IsUnique			() const	{ return m_bUnique;		}
		bool		IsPrimaryKey		() const	{ return m_bPrimary;	}
		bool		IsConstraint		() const	{ return m_bConstraint;	}
	};

	typedef CAtlArray<CIndex*> CIndexArray;


	// CColumn
	//
	//		Contains column schema information
	//
	class CColumn
	{
	private:
		CString			m_strName,			// Name
						m_strDefault;		// Default value
		DBTYPE			m_wType;			// Type
		ULONG			m_ulOrdinal,		// Ordinal within table
						m_ulSize;			// Size
		INT				m_nSeed,			// Identity seed
						m_nIncrement;		// Identity increment 
		DBCOLUMNFLAGS	m_dwFlags;			// Flags
		BYTE			m_bPrecision,		// Precision
						m_bScale;			// Scale
		bool			m_bRowGuid,			// Is this a ROWGUIDCOL?
						m_bIdentity;		// Is this an IDENTITY column?

		void ParseProperties(DBCOLUMNDESC* pColumnDesc);

	public:
		CColumn();
		CColumn(DBCOLUMNDESC* pColumnDesc, ULONG iOrdinal);
		~CColumn();

		LPCTSTR			GetName			()	{ return m_strName;		}
		LPCTSTR			GetDefault		()	{ return m_strDefault;	}
		DBTYPE			GetDbType		()	{ return m_wType;		}
		ULONG			GetSize			()	{ return m_ulSize;		}
		ULONG			GetOrdinal		()	{ return m_ulOrdinal;	}
		BYTE			GetPrecision	()	{ return m_bPrecision;	}
		BYTE			GetScale		()	{ return m_bScale;		}
		DBCOLUMNFLAGS	GetFlags		()	{ return m_dwFlags;		}
		bool			IsNullable		()	{ return (m_dwFlags & DBCOLUMNFLAGS_ISNULLABLE) != 0;		}
		bool			IsLong			()	{ return (m_dwFlags & DBCOLUMNFLAGS_ISLONG) != 0;			}
		bool			IsRowVersion	()	{ return (m_dwFlags & DBCOLUMNFLAGS_ISROWVER) != 0;			}
		bool			IsFixedLength	()	{ return (m_dwFlags & DBCOLUMNFLAGS_ISFIXEDLENGTH) != 0;	}
		bool			IsRowGuid		()	{ return m_bRowGuid;	}
		bool			IsIdentity		()	{ return m_bIdentity;	}
	};

	typedef CAtlArray<CColumn*> CColumnArray;


	// CTableDefinition
	//
	//		Helper class that retrieves the table schema definitions
	//
	class CTableDefinition
	{
	protected:
		ULONG				m_nColumns,			// Number of columns
							m_nPropSets,		// Number of property sets
							m_nConstraints;		// Number of constraints
		DBCOLUMNDESC*		m_pColumns;			// Column array
		DBCONSTRAINTDESC*	m_pConstraints;		// Constraint array
		OLECHAR*			m_pStrings;			// Strings
		DBPROPSET*			m_pPropSets;		// Property set array

		void Clear();

	public:
		CTableDefinition();
		~CTableDefinition();

		bool		FillColumnArray		(CColumnArray& columns);
		bool		FillForeignKeyArray	(CForeignKeyArray& foreignKeys);
		bool		FillUniqueArray		(CIndexArray& uniques);
		CIndex*		GetPrimaryKey		();

		HRESULT		GetDefinition(CSession& session, LPCTSTR pszTableName);
	};


	// CTable
	//
	//		Opens a table cursor and renames an existing table.
	//
	class CTable
	{
	protected:
		CSession&	m_session;
		CString		m_strName;

	public:
		CTable(CSession& session, LPCTSTR pszTableName);
		virtual ~CTable();

		LPCTSTR	GetName	()	{ return m_strName;		}
		HRESULT	Rename	(LPCTSTR pszNewName);
		HRESULT	Open	(ULONG cPropSets, DBPROPSET rgPropSets[], LPCTSTR pszIndexName, CRowset& rowset);
	};


	// CTableSchema
	//
	//		Contains table schema information
	//
	class CTableSchema
	{
	private:
		CSchema*			m_pSchema;		// Containing CSchema, if any.
		CString				m_strName;
		CColumnArray		m_columns;
		CIndexArray			m_indexes,
							m_uniques;
		CForeignKeyArray	m_foreignKeys;
		bool				m_bLoaded;

		HRESULT LoadIndexes	(CSession& session);
		bool	Copy		(const CTableSchema& tableSchema);

	public:
		CTableSchema();
		CTableSchema(LPCTSTR pszTableName, CSchema* pSchema = NULL);
		CTableSchema(const CTableSchema& tableSchema);
		~CTableSchema();

		CTableSchema& operator =(const CTableSchema& tableSchema);

		HRESULT	Load	();
		HRESULT	Load	(CSession& session, LPCTSTR pszTableName);
		void	Clear	();

		LPCTSTR	GetName				() const	{ return m_strName;						}
		int		GetColumnCount		() const	{ return int(m_columns.GetCount());		}
		int		GetIndexCount		() const	{ return int(m_indexes.GetCount());		}
		int		GetUniqueCount		() const	{ return int(m_uniques.GetCount());		}
		int		GetForeignKeyCount	() const	{ return int(m_foreignKeys.GetCount());	}
		bool	IsLoaded			() const	{ return m_bLoaded;		}
		int		FindColumn			(LPCTSTR pszName);
		int		FindIndex			(LPCTSTR pszName);
		int		FindUnique			(LPCTSTR pszName);
		int		FindForeignKey		(LPCTSTR pszName);

		CColumn*		GetColumn		(int iItem)	{ return m_columns[iItem];		}
		CIndex*			GetIndex		(int iItem)	{ return m_indexes[iItem];		}
		CIndex*			GetUnique		(int iItem)	{ return m_uniques[iItem];		}
		CIndex*			GetPrimaryKey	();
		CForeignKey*	GetForeignKey	(int iItem)	{ return m_foreignKeys[iItem];	}
		CSchema*		GetSchema		() const	{ return m_pSchema;				}
	};

	typedef CAtlArray<CTableSchema*> CTableSchemaArray;


	// CSchema
	//
	//		Contains the database schema information
	//
	class CSchema
	{
	protected:
		CTableSchemaArray	m_tables;
		CSession&			m_session;

	public:
		CSchema(CSession& m_session);
		virtual ~CSchema();

		HRESULT			Load			();
		void			Clear			();
		CTableSchema*	GetTableSchema	(int iTable)	{ return m_tables[iTable];			}
		int				GetTableCount	()				{ return int(m_tables.GetCount());	}
		CSession&		GetSession		()				{ return m_session;					}
	};


	// CCommand
	//
	//		OLE DB Command class
	//
	class CCommand
	{
	protected:
		ICommandText*	m_pCommand;			// OLE DB Command interface pointer
		ULONG			m_nParams;			// Number of parameters
		LPOLESTR		m_pszParamNames;	// Parameter names
		BOUNDCOLUMN*	m_pBoundParam;		// Bound parameter
		BYTE*			m_pBuffer;			// Parameter data buffer
		ULONG			m_nRowSize;			// Parameter data buffer size
		DBBINDSTATUS*	m_pBindStatus;		// Binding status (useful for debugging)
		HACCESSOR		m_hParamAccessor;	// Unique accessor handle used to bind parameters
		CDbValue*		m_pParam;
		DBPARAMINFO*	m_pParamInfo;

		void			SetAllParameters();
		HRESULT			CreateParameters(DBPARAMINFO* pParamInfo);
		bool			RebindParameters();
		void			ClearParameters	();
		void			ClearBinding	();
		HRESULT			BindParameters	();
		ULONG			FindParameter	(ULONG iOrdinal);

	public:
		CCommand(CSession& session);
		virtual ~CCommand();

		ULONG		GetParamCount()	{ return m_nParams; }

		HRESULT		SetText		(LPCTSTR pszText);
		HRESULT		Prepare		(ULONG cExpectedRuns = 0);
		HRESULT		Unprepare	();
		HRESULT		Execute		(LONG* pcRowsAffected = NULL);
		HRESULT		Execute		(CRowset& rowset, LONG* pcRowsAffected = NULL);

		template <typename T>
		bool SetParam(ULONG iOrdinal, T value)
		{
			ULONG iParam = FindParameter(iOrdinal);

			if(iParam == (ULONG)-1)
				return false;
			return m_pParam[iParam].SetValue(value);
		}

		bool SetParam(ULONG iOrdinal, BYTE* pValue, ULONG nSize)
		{
			ULONG iParam = FindParameter(iOrdinal);

			if(iParam == (ULONG)-1)
				return false;
			return m_pParam[iParam].SetValue(pValue, nSize);
		}

		bool GetStatus(ULONG iOrdinal, DBSTATUS& status)
		{
			ULONG iParam = FindParameter(iOrdinal);

			if(iParam == (ULONG)-1)
				return false;
			status = m_pParam[iParam].GetStatus();
			return true;
		}
	};


	// CShemaRowset
	//
	//		Base schema rowset
	//
	class CSchemaRowset
	{
	protected:
		IDBSchemaRowset*	m_pSchemaRowset;

	public:
		CSchemaRowset(CSession& session);
		virtual ~CSchemaRowset();

		HRESULT	Open(REFGUID rguidSchema, ULONG cRestrictions, const VARIANT rgRestrictions[], CRowset &rowset);
	};


	// CTablesRowset
	//
	//		Tables schema rowset
	//
	class CTablesRowset : public CSchemaRowset
	{
	public:
		CTablesRowset(CSession& session) : CSchemaRowset(session) { }

		HRESULT Open(LPCTSTR pszCatalog, LPCTSTR pszSchema, LPCTSTR pszTable, LPCTSTR pszType, CRowset& rowset);
	};

	// CColumnsRowset
	//
	//		Columns rowset
	//
	class CColumnsRowset : public CSchemaRowset
	{
	public:
		CColumnsRowset(CSession& session) : CSchemaRowset(session) { }

		HRESULT Open(LPCTSTR pszTableCatalog, LPCTSTR pszTableSchema, LPCTSTR pszTableName, LPCTSTR pszColumnName, CRowset& rowset);
	};


	// CViewsRowset
	//
	//		Views schema rowset
	//
	class CViewsRowset : public CSchemaRowset
	{
	public:
		CViewsRowset(CSession& session) : CSchemaRowset(session) { }

		HRESULT Open(LPCTSTR pszCatalog, LPCTSTR pszSchema, LPCTSTR pszTable, CRowset& rowset);
	};


	class CIndexesRowset : public CSchemaRowset
	{
	public:
		CIndexesRowset(CSession& session) : CSchemaRowset(session) { }

		HRESULT Open(LPCTSTR pszCatalog, LPCTSTR pszTableSchema, LPCTSTR pszIndexName, LPCTSTR pszType, LPCTSTR pszTableName, CRowset& rowset);
	};


	// CRowset
	//
	//		Implements a generic rowset
	//
	class CRowset
	{
	protected:
		IRowset*			m_pRowset;
		IRowsetChange*		m_pRowsetChange;
		IRowsetUpdate*		m_pRowsetUpdate;
		IRowsetIndex*		m_pRowsetIndex;
		IAccessor*			m_pColAccessor;
		IAccessor*			m_pKeyAccessor;

		HROW				m_hRow;				// Current row handle
		ULONG				m_nColumns,			// Number of data columns
							m_nKeyColumns,		// Number of key columns
							m_nAccessors,		// Number of accessor handles
							m_nRowSize,			// Data buffer size
							m_nKeySize;			// Key data buffer size
		BOUNDCOLUMN*		m_pBoundColumn;
		BOUNDCOLUMN*		m_pBoundKey;
		OLECHAR*			m_pColumnNames;
		HACCESSOR*			m_phAccessor;		// Accessor handle array for column values
		HACCESSOR			m_hKeyAccessor;		// Accessor handle for key values
		BYTE*				m_pBuffer;			// Data buffer
		BYTE*				m_pKeyBuffer;		// Key data buffer
		DBBINDSTATUS*		m_pBindStatus;		// Binding status (useful for debugging)
		bool				m_bCustomBound;		// Columns were custom bound
		CDbValueRef*		m_pValueRef;
		CDbValueRef*		m_pKeyRef;

		ULONG	GetBlobCount(DBCOLUMNINFO* pColumnInfo, ULONG nColumns);

		DBCOLUMNINFO*	GetTableColumnInfo	(LPCTSTR pszColumnName, DBCOLUMNINFO* pColumnInfo);
		HRESULT			BindKeyColumns		(DBCOLUMNINFO* pColumnInfo);

	public:
		CRowset();
		virtual ~CRowset();

		HRESULT	Open	(IRowset* pRowset);
		void	Close	();
		bool	IsOpen	()	{ return m_pColAccessor != NULL;	}

		HRESULT	ReleaseRow		();
		HRESULT	GetRowCount		(ULONG* pcRows);

		HRESULT	MoveRelative	(LONG nOffset);
		HRESULT	MoveFirst		();
		HRESULT	MoveNext		()	{ return MoveRelative(1);	}
		HRESULT	MovePrev		()	{ return MoveRelative(-1);	}
		HRESULT	MoveToBookmark	(ULONG cbBookmark, const BYTE* pBookmark);

		HRESULT	Insert			();
		HRESULT	Delete			();
		HRESULT	SetData			();

		HRESULT Update			();
		HRESULT	Undo			();

		HRESULT SeekRow			(int cKeyValues, DBSEEK dwSeekOptions = DBSEEK_FIRSTEQ);
		HRESULT Seek			(int cKeyValues, DBSEEK dwSeekOptions = DBSEEK_FIRSTEQ);

		template <typename T>
		bool GetValue(ULONG iOrdinal, T& value)
		{
			ULONG iColumn = GetColumnIndex(iOrdinal);

			if(iColumn == (ULONG)-1)
				return false;

			CDbValueRef& valRef = m_pValueRef[iColumn];
			
			// Is this a BLOB?
			if(valRef.IsLong())
			{
				int		iAccessor	= m_pBoundColumn[iColumn].iAccessor;								// Get the accessor handle index
				HRESULT hr			= m_pRowset->GetData(m_hRow, m_phAccessor[iAccessor], m_pBuffer);	// Get the BLOB data
				if(FAILED(hr))
					return false;
			}
			return valRef.GetValue(value);
		}

		template <typename T>
		bool SetValue(ULONG iOrdinal, T value)
		{
			ULONG iColumn = GetColumnIndex(iOrdinal);

			if(iColumn == (ULONG)-1)
				return false;

			return m_pValueRef[iColumn].SetValue(value);
		}

		bool SetValue(ULONG iOrdinal, BYTE* pValue, ULONG nSize)
		{
			ULONG iColumn = GetColumnIndex(iOrdinal);

			if(iColumn == (ULONG)-1)
				return false;

			return m_pValueRef[iColumn].SetValue(pValue, nSize);
		}

		template <typename T>
		bool SetKeyValue(ULONG iOrdinal, T value)
		{
			ATLASSERT(iOrdinal > 0 && iOrdinal <= m_nKeyColumns);
			return m_pKeyRef[iOrdinal-1].SetValue(value);
		}


		// Column accessors via column index (low-level access)
		ULONG    _GetLength(ULONG iColumn) { return *(ULONG*)(m_pBuffer + m_pBoundColumn[iColumn].obLength); }
		DBSTATUS _GetStatus(ULONG iColumn) { return *(DBSTATUS*)(m_pBuffer + m_pBoundColumn[iColumn].obStatus); }
		BYTE*	 _GetValPtr(ULONG iColumn) { return m_pBuffer + m_pBoundColumn[iColumn].obValue; }

		void _SetLength(ULONG iColumn, ULONG nLength)
		{
			*(ULONG*)(m_pBuffer + m_pBoundColumn[iColumn].obLength) = nLength;
		}

		void _SetStatus(ULONG iColumn, DBSTATUS status)
		{
			*(DBSTATUS*)(m_pBuffer + m_pBoundColumn[iColumn].obStatus) = status;
		}

		ULONG	GetColumnCount()	{ return m_nColumns;	}
		ULONG	GetRowSize	()		{ return m_nRowSize;	}
		BYTE*	GetRowBuffer()		{ return m_pBuffer;		}

		BOUNDCOLUMN*	GetColumnInfo(ULONG iColumn) { return m_pBoundColumn + iColumn; }

		DBSTATUS	GetStatus(ULONG iOrdinal);
		ULONG		GetLength(ULONG iOrdinal);
		BYTE*		GetValPtr(ULONG iOrdinal);

		void	SetStatus(ULONG iOrdinal, DBSTATUS status);
		void	SetLength(ULONG iOrdinal, ULONG nLength);

		bool	HasBookmark		();
		ULONG	GetColumnIndex	(ULONG iOrdinal);
//		ULONG	GetColumnIndex	(LPCTSTR pszColumnName);
	};


	//---------------------------------------------------------------------
	//
	//	Inline methods
	//
	//---------------------------------------------------------------------

	inline DBSTATUS CRowset::GetStatus(ULONG iOrdinal)
	{
		ULONG iColumn = GetColumnIndex(iOrdinal);

		return *(DBSTATUS*)(m_pBuffer + m_pBoundColumn[iColumn].obStatus);
	}

	inline ULONG CRowset::GetLength(ULONG iOrdinal)
	{
		ULONG iColumn = GetColumnIndex(iOrdinal);

		return *(ULONG*)(m_pBuffer + m_pBoundColumn[iColumn].obLength);
	}

	inline BYTE* CRowset::GetValPtr(ULONG iOrdinal)
	{
		ULONG iColumn = GetColumnIndex(iOrdinal);

		return (BYTE*)(m_pBuffer + m_pBoundColumn[iColumn].obValue);
	}

	inline void CRowset::SetStatus(ULONG iOrdinal, DBSTATUS status)
	{
		ULONG		iColumn	= GetColumnIndex(iOrdinal);
		DBSTATUS*	pStatus = (DBSTATUS*)(m_pBuffer + m_pBoundColumn[iColumn].obStatus);
		
		*pStatus = status;
	}

	inline void CRowset::SetLength(ULONG iOrdinal, ULONG nLength)
	{
		ULONG	iColumn	= GetColumnIndex(iOrdinal);
		ULONG*	pLength = (ULONG*)(m_pBuffer + m_pBoundColumn[iColumn].obLength);
		
		*pLength = nLength;
	}

	// Returns true if the rowset has a bound bookmark.
	inline bool CRowset::HasBookmark()
	{
		ATLASSERT(m_pBoundColumn != NULL);
		return m_pBoundColumn[0].iOrdinal == 0;
	}

};	// namespace OLEDBCLI

#endif // __OLEDBCLI_H__

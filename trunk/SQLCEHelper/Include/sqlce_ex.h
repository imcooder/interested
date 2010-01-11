#ifndef __sqlce_ex_h__
#define __sqlce_ex_h__

#ifdef __cplusplus
extern "C"{
#endif 

/***************************************************************************
****************************************************************************
        SQL CE specific GUIDS
***************************************************************************
***************************************************************************/


// Microsoft SQL Server for Windows CE 1.0 Provider (Microsoft.SQLSERVER.OLEDB.CE.1.0)
//
extern const OLEDBDECLSPEC GUID CLSID_SQLSERVERCE_1_0 = {0x7D0703CB,0x4C23,0x11d2,{0x88,0x82,0x00,0xC0,0x4F,0xD9,0x37,0xF0}};

// Microsoft SQL Server for Windows CE 2.0 Provider (Microsoft.SQLSERVER.OLEDB.CE.2.0)
//
extern const OLEDBDECLSPEC GUID CLSID_SQLSERVERCE_2_0 = {0x76A85B2E,0x9DE0,0x4ded,{0x8E,0x69,0x4D,0xEF,0xDB,0x9C,0x09,0x17}};

// Microsoft SQL Server Lite for Windows 3.0 Provider (Microsoft.SQLSERVER.OLEDB.CE.3.0)
//
// {32CE2952-2585-49a6-AEFF-1732076C2945}
//
extern const OLEDBDECLSPEC GUID CLSID_SQLSERVERCE_3_0 = {0x32ce2952, 0x2585, 0x49a6, {0xae, 0xff, 0x17, 0x32, 0x7, 0x6c, 0x29, 0x45}};

/*

// Microsoft SQL Server Lite for Windows 3.5 Provider (Microsoft.SQLSERVER.CE.OLEDB.3.5)
//
// {F49C559D-E9E5-467C-8C18-3326AAE4EBCC}
//
extern const OLEDBDECLSPEC GUID CLSID_SQLSERVERCE_3_5 = {0xf49c559d, 0xe9e5, 0x467c, {0x8c, 0x18, 0x33, 0x26, 0xaa, 0xe4, 0xeb, 0xcc}};

// PUBLISHED: Provider Specific Property Sets
//
// Provider-Specific DBInit Property Set
// {2B9AB5BA-4F6C-4ddd-BF18-24DD4BD41848}
//
//extern const OLEDBDECLSPEC GUID DBPROPSET_SSCE_DBINIT = {0x2b9ab5ba, 0x4f6c, 0x4ddd, {0xbf, 0x18, 0x24, 0xdd, 0x4b, 0xd4, 0x18, 0x48}};

// Provider-Specific Column Property Set
// {352CC8D5-9181-11d3-B27B-00C04F68DBFF}
//
//extern const OLEDBDECLSPEC GUID DBPROPSET_SSCE_COLUMN = {0x352cc8d5, 0x9181, 0x11d3, {0xb2, 0x7b, 0x0, 0xc0, 0x4f, 0x68, 0xdb, 0xff}};

// Provider-Specific Rowset Property Set
// {5C17C602-A107-11d3-B27B-00C04F68DBFF}
//
//extern const OLEDBDECLSPEC GUID DBPROPSET_SSCE_ROWSET = {0x5c17c602, 0xa107, 0x11d3, {0xb2, 0x7b, 0x0, 0xc0, 0x4f, 0x68, 0xdb, 0xff}};

// Provider-Specific Session Property Set
// {22FE7D33-5E5C-4a45-B723-8BED2374A06B}
//
//extern const OLEDBDECLSPEC GUID DBPROPSET_SSCE_SESSION = {0x22fe7d33, 0x5e5c, 0x4a45, {0xb7, 0x23, 0x8b, 0xed, 0x23, 0x74, 0xa0, 0x6b}};

*/

#ifndef GUIDS_ONLY

#undef DBPROP_SSCE_COL_ROWGUID
#undef DBPROP_SSCE_MAXBUFFERSIZE
#undef DBPROP_SSCE_DBPASSWORD
#undef DBPROP_SSCE_ENCRYPTDATABASE
#undef DBPROP_SSCE_TEMPFILE_DIRECTORY


// PUBLISHED Provider specific properties
//
#define DBPROP_SSCE3_COL_ROWGUID					0x1F9L      // SSCE_COLUMN
#define DBPROP_SSCE3_MAXBUFFERSIZE					0x1FAL      // SSCE_DBINIT
#define DBPROP_SSCE3_DBPASSWORD						0x1FBL      // SSCE_DBINIT
#define DBPROP_SSCE3_ENCRYPTDATABASE				0x1FCL      // SSCE_DBINIT
#define DBPROP_SSCE3_TEMPFILE_DIRECTORY				0x1FEL      // SSCE_DBINIT


// FOR SQL CE 2.0 ONLY
//
#define DBPROP_SSCE_TBL_DISTINCT					0x64
#define DBPROP_SSCE_COL_TTKEYASCENDING				0x65
#define DBPROP_SSCE_COL_TTKEYORDINAL				0x66
#define DBPROP_SSCE_TBL_TTTYPE						0x67
#define DBPROP_SSCE_PARTIALSETDATA					0x68

#define DBPROP_SSCE2_COL_ROWGUID					0x69
#define DBPROP_SSCE2_MAXBUFFERSIZE					0x70
#define DBPROP_SSCE2_DBPASSWORD						0x71
#define DBPROP_SSCE2_ENCRYPTDATABASE				0x72
#define DBPROP_SSCE2_TEMPFILE_DIRECTORY				0x73

#endif

/*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif // __sqlce_ex_h__

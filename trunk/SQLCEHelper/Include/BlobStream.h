#ifndef __BLOBSTREAM_H__
#define __BLOBSTREAM_H__

class CBlobStream : public ISequentialStream
{
public:
	CBlobStream();
	virtual ~CBlobStream();

	void Clear();

	//
	// ISequentialStream interface implementation.
	//
    virtual HRESULT STDMETHODCALLTYPE QueryInterface( 
        /* [in] */ REFIID riid,
        /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
    
    virtual ULONG STDMETHODCALLTYPE AddRef( void);
    
    virtual ULONG STDMETHODCALLTYPE Release( void);
 
    virtual /* [local] */ HRESULT STDMETHODCALLTYPE Read( 
        /* [length_is][size_is][out] */ void __RPC_FAR *pv,
        /* [in] */ ULONG cb,
        /* [out] */ ULONG __RPC_FAR *pcbRead);
    
    virtual /* [local] */ HRESULT STDMETHODCALLTYPE Write( 
        /* [size_is][in] */ const void __RPC_FAR *pv,
        /* [in] */ ULONG cb,
        /* [out] */ ULONG __RPC_FAR *pcbWritten);

	HRESULT WriteFromStream(IStream *pStream, ULONG cb, ULONG *pcbWritten);
	HRESULT WriteFromStream(ISequentialStream *pStream, ULONG cb, ULONG *pcbWritten);

	ULONG	GetSize	()	{ return m_nLength;	}
	BYTE*	GetData	()	{ return m_pBuffer;	}

protected:
	BYTE*		m_pBuffer;
	ULONG		m_nLength,
				m_iPosRead,
				m_iPosWrite,
				m_nRef;
	DBSTATUS	m_dbStatus;
};

#endif
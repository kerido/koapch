#pragma once




class StreamImpl_Resource : public IStream
{
protected:
    virtual HRESULT __stdcall Seek(
		LARGE_INTEGER       dlibMove,
		DWORD               dwOrigin,
		ULARGE_INTEGER *    plibNewPosition) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE SetSize(ULARGE_INTEGER libNewSize) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE CopyTo( 
        IStream *           pstm,
        ULARGE_INTEGER      cb,
        ULARGE_INTEGER *    pcbRead,
        ULARGE_INTEGER *    pcbWritten) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE Commit( 
        DWORD               grfCommitFlags) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE Revert() = 0;
    
    virtual HRESULT STDMETHODCALLTYPE LockRegion( 
        ULARGE_INTEGER      libOffset,
        ULARGE_INTEGER      cb,
        DWORD               dwLockType) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE UnlockRegion( 
        ULARGE_INTEGER      libOffset,
        ULARGE_INTEGER      cb,
        DWORD               dwLockType) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE Stat( 
        STATSTG *           pstatstg,
        DWORD               grfStatFlag) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE Clone( 
        IStream **           ppstm) = 0;

//IUnknown members
protected:

}:
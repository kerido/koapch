#pragma once

#include "sdk_IconExtractor.h"

#include "LogicCommon.h"
#include "UtilPidlFunc.h"

//////////////////////////////////////////////////////////////////////////////////////////////

class UtilShellFunc
{
public:
	static HRESULT SHBindToParentEx (LPCITEMIDLIST, REFIID, void ** , LPCITEMIDLIST *);
	
	static bool GetDirectoryName(IShellFolder * theSF, LPCITEMIDLIST thePidl, TCHAR * theOut, int theOutSize);

	//TODO: conform to buffer
	static void StrRetToBuf
		(
			LPCITEMIDLIST theItemPidl, //!<
			const STRRET & theValue,   //!< 
			TCHAR * theOutput,         //!< 
			int theSize                //!< 
		) throw();


	//! 
	static HRESULT GetDisplayName
		(
			IShellFolder * theSF,  //!< 
			LPCITEMIDLIST thePidl, //!< 
			SHGDNF theFlags,       //!< 
			TCHAR * theOutput,     //!< 
			int theSize            //!< 
		) throw();


	static HRESULT ParseDisplayName
		(
			const TCHAR * theName,     //!< 
			int theLength,             //!< 
			IShellFolder ** theOutSF,  //!< 
			LPITEMIDLIST * theOutPidl  //!< 
		) throw();

	static HRESULT GetIconNew(IShellFolder *, LPCITEMIDLIST, IconExtractParams *, void **);

	static HRESULT GetIconAsync (IconExtractParams *, void *);

	static HRESULT GetIconOverlay(IShellFolder *, LPCITEMIDLIST, IconExtractParams *, void **);

	static HRESULT GetIconOverlay(IShellIconOverlay *, LPCITEMIDLIST, IconExtractParams *);

	static LPITEMIDLIST GetAbsolutePidl(IShellFolder * theSF, LPCITEMIDLIST thePidl);

	static HRESULT ReleaseAsyncArg(void * theAsyncArg);

	static HRESULT ShellExecuteRetValToHResult(int theShellExecRet);
};

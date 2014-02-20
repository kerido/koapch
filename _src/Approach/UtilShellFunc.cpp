#include "stdafx.h"

#include "sdk_MemoryWriter.h"
#include "sdk_CharConversion.h"

#include "LogicCommon.h"
#include "UtilShellFunc.h"
#include "Application.h"
#include "Framework.h"
#include "Trace.h"
#include "ShellItemGenericIconManager.h"
#include "Preferences.h"

//////////////////////////////////////////////////////////////////////////////////////////////

struct ShellIconExtractAsyncArg
{
	int                 Flags;           //<! One or more of IIconAcceptor::IconChangeFlags
	LPITEMIDLIST        AbsolutePidl;

	ShellIconExtractAsyncArg()
	{
		Trace("ShellIconExtractAsyncArg::ShellIconExtractAsyncArg. this=[0x%8x]\n", this);

		SecureZeroMemory(this, sizeof ShellIconExtractAsyncArg);
	}

	~ShellIconExtractAsyncArg()
	{
		Trace("ShellIconExtractAsyncArg::~ShellIconExtractAsyncArg. this=[0x%8x]\n", this);

		if (AbsolutePidl != 0)
			CoTaskMemFree(AbsolutePidl);
	}

	static ShellIconExtractAsyncArg * Ensure(void ** theAddr)
	{
		ATLASSERT(theAddr != 0);

		if (*theAddr == 0)
			*theAddr = new ShellIconExtractAsyncArg();

		return reinterpret_cast<ShellIconExtractAsyncArg *>(*theAddr);
	};

	static void * operator new (size_t theSize)
	{
		return CoTaskMemAlloc(sizeof ShellIconExtractAsyncArg);
	}

	static void operator delete(void * theObj)
	{
		return CoTaskMemFree(theObj);
	}
};

//////////////////////////////////////////////////////////////////////////////////////////////

class FullPidlExtractor
{
public:
	virtual LPCITEMIDLIST GetAbsolutePidl() = 0;
};

class StaticFullPidlExtractor : public FullPidlExtractor
{
private:
	LPCITEMIDLIST myPidl;

public:
	StaticFullPidlExtractor(LPCITEMIDLIST thePidl)
		: myPidl(thePidl)
	{ }

public:
	LPCITEMIDLIST GetAbsolutePidl() { return myPidl; }
};

class DynamicFullPidlExtractor : public FullPidlExtractor
{
private:
	IShellFolder * mySF;
	LPCITEMIDLIST  myRelativePidl;
	LPITEMIDLIST   myAbsolutePidl;

public:

	DynamicFullPidlExtractor(IShellFolder * theSF, LPCITEMIDLIST theRelativePidl)
		: mySF(theSF), myRelativePidl(theRelativePidl), myAbsolutePidl(0)
	{ }

	~DynamicFullPidlExtractor()
	{
		if (myAbsolutePidl != 0)
			CoTaskMemFree(myAbsolutePidl);
	}

public:
	LPCITEMIDLIST GetAbsolutePidl()
	{
		if (myAbsolutePidl == 0)
			myAbsolutePidl = UtilShellFunc::GetAbsolutePidl(mySF, myRelativePidl);

		return myAbsolutePidl;
	}
};

//////////////////////////////////////////////////////////////////////////////////////////////

class IconExtractTraitsA : public CharConversionTraitsA<MAX_PATH>
{
public:
	typedef IExtractIconA Extractor;
};

class IconExtractTraitsW : public CharConversionTraitsW<MAX_PATH>
{
public:
	typedef IExtractIconW Extractor;
};

template<typename tTraits>
class IconExtraction
{
public:
	static HRESULT Run(IShellFolder * theSF, LPCITEMIDLIST theItemPidl, IconExtractParams * theParams, void ** theOutAsyncArg)
	{
		CComPtr<typename tTraits::Extractor> aIE;

		HRESULT aRes = theSF->GetUIObjectOf(NULL, 1, &theItemPidl, __uuidof(typename tTraits::Extractor), 0, (void **) &aIE);

		if ( FAILED(aRes) )
			return aRes;
		else if (aIE == 0)
			return E_FAIL;
		else
			return Perform(theSF, theItemPidl, theParams, aIE);
	}

	static HRESULT Perform(IShellFolder * theSF, LPCITEMIDLIST theItemPidl, IconExtractParams * theParams, typename tTraits::Extractor * theExtractor)
	{
		DynamicFullPidlExtractor aFPE(theSF, theItemPidl);

		return Perform(aFPE, theParams, theExtractor);
	}

	static HRESULT Perform(FullPidlExtractor & thePidlExtr, IconExtractParams * theParams, typename tTraits::Extractor * theExtractor)
	{
		bool aPerformFastExtract = (theParams->Flags & EXTRACT_PREFERFAST) != 0;

		UINT aExtractIconFlags = aPerformFastExtract ? GIL_FORSHELL|GIL_ASYNC : GIL_FORSHELL;
		int aIndex;
		typename tTraits::Char aPath[MAX_PATH];
		UINT aRetFlags = 0;
		HRESULT aRes = theExtractor->GetIconLocation(aExtractIconFlags, aPath, MAX_PATH, &aIndex, &aRetFlags);

		if ( FAILED(aRes) )
			return aRes;

#ifdef _DEBUG
		typename tTraits::AsciiTrans aTrans;
		tTraits::CharToAscii(aPath, -1, aTrans, MAX_PATH);

		const char * aAsciiPath = aTrans;

		Trace("  The icon location is: %s,%d. GIL_NOTFILENAME=%d\n",
			aAsciiPath,
			aIndex,
			(aRetFlags & GIL_NOTFILENAME) ? 1 : 0 );

#endif	//_DEBUG

		typename tTraits::WideTrans aPathWide;
		tTraits::CharToWide(aPath, -1, aPathWide, MAX_PATH);	// obtain guaranteed Unicode icon path

		bool aUseCaching = (aRetFlags & GIL_DONTCACHE) == 0 && (theParams->Flags & CACHE_IGNORE) == 0;

		IIconCache * aCache = Application::Instance().GetIconCache();

		bool aHasEntry = aUseCaching ? aCache->Lookup(aPathWide, aIndex, *theParams) : false;

		if (aHasEntry)
			return S_OK;

		// According to the docs, since the object did not return E_PENDING,
		// it's not guaranteed to be thread-safe. However, our implementation
		// does not use the object between threads. Instead, we will clone the
		// absolute PIDL and then re-create icon extractors in a background thread.
		else if (aPerformFastExtract && (aRetFlags & GIL_PERINSTANCE) != 0)
			return E_PENDING;


		// The icon is not in the cache, but we can perform extraction here
		// either because we can afford slow extractions (aPerformFastExtract is false)
		// or because it looks like extraction can be relatively fast
		// (aRetFlags does not contain the GIL_PERINSTANCE flag)

		HICON aSmall = 0, aLarge = 0;

		int aW1 = GetSystemMetrics(SM_CXICON);
		int aW2 = GetSystemMetrics(SM_CXSMICON);

		aRes = theExtractor->Extract( aPath, aIndex, &aLarge, &aSmall, MAKELONG(aW1, aW2) );

		//Trace("  After calling IExtractIconW::Extract the icon is: %d, theRes is 0x%8x, \n", aIcn, aRes);


		if (aSmall != 0)
		{
			// we don't need the large icon

			if (aLarge != aSmall)
				DestroyIcon(aLarge);

			aLarge = 0;
		}
		else if (aLarge != 0)	//aSmall is null
		{
			aSmall = aLarge;
			aLarge = 0;
		}
		else if ( (aRetFlags & GIL_NOTFILENAME) == 0)	//both icons are nulls -- try ExtractIconEx
		{
			if (aIndex == -1) 
				aIndex = 0;

			ExtractIconExW(aPathWide, aIndex, NULL, &aSmall, 1);

			//Trace("  After calling ExtractIconExW the icon is: %d\n", aIcn);
		}
		else	//finally try SHGetFileInfo (used when extracting icons for EXE files without an icon resource)
		{
			SHFILEINFO aFI;
			DWORD_PTR aRet = SHGetFileInfo( (LPCWSTR) thePidlExtr.GetAbsolutePidl(), FILE_ATTRIBUTE_NORMAL, &aFI, sizeof(SHFILEINFO), SHGFI_PIDL|SHGFI_ICON|SHGFI_SMALLICON);

			if (aRet)
				aSmall = aFI.hIcon;
		}

		if (aSmall == 0)	//still unable to get a reasonable icon
			return E_FAIL;

		aRes = S_OK;

		//////////////////////////////////////////////////////////////
		// Cache icon if possible

		if (aUseCaching)
		{
			//Trace("  Inserting the icon into the cache. Index: %d\n", aImageIndex);
			aCache->Add(aPathWide, aIndex, aSmall, *theParams);
		}
		else
		{
			//Trace("  Inserting the icon into the instance list. Index: %d\n", aImageIndex);
			theParams->IconIndex = ImageList_ReplaceIcon(theParams->ImageList, -1, aSmall);
		}

		DestroyIcon(aSmall);
		aSmall = 0;


		ATLASSERT(aSmall == 0 && aLarge == 0);

		return aRes;
	}
};


typedef IconExtraction<IconExtractTraitsA> IconExtractionA;
typedef IconExtraction<IconExtractTraitsW> IconExtractionW;

//////////////////////////////////////////////////////////////////////////////////////////////
//this function is taken from the internet

HRESULT UtilShellFunc::SHBindToParentEx (LPCITEMIDLIST pidl, REFIID riid, void ** ppv, LPCITEMIDLIST * ppidlLast)
{
#ifdef _USE_APPROACH
	if (!pidl || !ppv)
	{
		Trace ("  SHBindToParentEx -- ERR: Invalid Pidl and/or invalid destination pointer.\n");
		return E_POINTER;
	}
	
	UINT aNumItemsInPidl = UtilPidlFunc::Count (pidl);
	if (aNumItemsInPidl == 0)	//invalid pidl
	{
		Trace ("  SHBindToParentEx -- ERR: Empty Pidl.\n");
		return E_POINTER;
	}

	HRESULT aRes = S_OK;
	IShellFolder * aDesktopSF = NULL;
	SHGetDesktopFolder (&aDesktopSF);

	if (aNumItemsInPidl == 1)	// desktop pidl
	{
		aRes = aDesktopSF->QueryInterface(riid, ppv);

		if ( SUCCEEDED(aRes) )
			if (ppidlLast) 
				*ppidlLast = UtilPidlFunc::Copy<MemoryWriter_Crt> (pidl);

		aDesktopSF->Release ();

		Trace ("  SHBindToParentEx -- STS: Single-level Pidl = 0x%x, DesktopSF returned 0x%x.\n",
			ppidlLast, aRes);

		return aRes;
	}

	LPBYTE pRel = UtilPidlFunc::GetByPos (pidl, aNumItemsInPidl - 1);

	UINT aCount = (UINT) (pRel - (LPBYTE) pidl);

	LPITEMIDLIST pidlParent = UtilPidlFunc::Copy<MemoryWriter_Crt> (pidl, aCount);
	IShellFolder * psfFolder = NULL;

	aRes = aDesktopSF->BindToObject (pidlParent, NULL, riid, (void **) &psfFolder);
	
	if ( FAILED(aRes) )
	{
		Trace ("  SHBindToParentEx -- ERR: Failed to bind to object. Multiple-level Pidl = 0x%x.\n", pidlParent);

		CoTaskMemFree(pidlParent);
		aDesktopSF->Release ();
		return aRes;
	}

	if (ppidlLast)
		*ppidlLast = UtilPidlFunc::Copy<MemoryWriter_Crt> ((LPCITEMIDLIST) pRel);

	CoTaskMemFree(pidlParent);
	aDesktopSF->Release ();

	*ppv = psfFolder;

	Trace ("  SHBindToParentEx -- STS: Returning 0x%x", aRes);

	return aRes;

#else
	return SHBindToParent(pidl, riid, ppv, ppidlLast);
#endif	//_USE_APPROACH
}

//////////////////////////////////////////////////////////////////////////////////////////////

HRESULT UtilShellFunc::GetIconNew(IShellFolder * theSF,
                                  LPCITEMIDLIST theItemPidl,
                                  IconExtractParams * theParams,
                                  void ** theOutAsyncArg)
{
	const ApplicationSettings * aSt = Application::InstanceC().Prefs();

	bool aUseGenericIcon = aSt->GetUseGenericIcons();
	bool aUseOverlays    = aSt->GetUseIconOverlays();
	bool aShowDimmedIcon = aSt->GetDimHiddenItems();

	HRESULT aRes = S_OK;
	int aAsyncFlags = 0;


	////////////////////////////////////////////////////////////////////////
	// Step 0. Determine whether the icon is dimmed.

	if (aShowDimmedIcon)
	{
		SFGAOF aAttribs = SFGAO_HIDDEN;
		theSF->GetAttributesOf(1, &theItemPidl, &aAttribs);

		if ( (aAttribs & SFGAO_HIDDEN) != 0 )
			theParams->IsIconDimmed = true;
	}


	if (!aUseGenericIcon)
	{
		////////////////////////////////////////////////////////////////////////
		// Step 1. Try to extract from the UNICODE version of the IExtractIcon
		HRESULT aIconRes = IconExtractionW::Run(theSF, theItemPidl, theParams, theOutAsyncArg);


		////////////////////////////////////////////////////////////////////////
		// Step 2. Try to extract from the ASCII version of the IExtractIcon
		if ( FAILED(aIconRes) && aIconRes != E_PENDING )
			aIconRes = IconExtractionA::Run(theSF, theItemPidl, theParams, theOutAsyncArg);

		if (aIconRes == E_PENDING)
			aAsyncFlags = IIconAcceptor::CHANGE_ICON;

		else if ( SUCCEEDED(aIconRes) )
			aRes = MAKE_HRESULT(0, 0, IIconAcceptor::CHANGE_ICON);


		////////////////////////////////////////////////////////////////////////
		// Step 3. Extract icon overlay information
		if (aUseOverlays)
		{
			if (aRes != E_PENDING)
			{
				HRESULT aOverlayRes = GetIconOverlay(theSF, theItemPidl, theParams, theOutAsyncArg);

				if (aOverlayRes == E_PENDING)
					aAsyncFlags |= IIconAcceptor::CHANGE_OVERLAY;

				else if (SUCCEEDED(aOverlayRes))
					aRes |= IIconAcceptor::CHANGE_OVERLAY;
			}
			else
				aAsyncFlags |= IIconAcceptor::CHANGE_OVERLAY;
		}

		// as a result of this block, we will return E_PENDING if either icon extraction
		// or icon overlay inquiry returned E_PENDING. The context necessary to continue
		// asynchronously will be saved in theOutAsyncArg
	}

	if (aAsyncFlags != 0)
	{
		ShellIconExtractAsyncArg * aArg = ShellIconExtractAsyncArg::Ensure(theOutAsyncArg);
		aArg->AbsolutePidl = UtilShellFunc::GetAbsolutePidl(theSF, theItemPidl);
		aArg->Flags = aAsyncFlags;
		aRes = E_PENDING;
	}

	////////////////////////////////////////////////////////////////////////
	// Step 4. Use generic icon
	if (theParams->IconIndex == -1 || aUseGenericIcon)
	{
		const ShellItemGenericIconManager * aMan = Application::InstanceC().GetShellItemGenericIconManager();

		int aGenericIconType = ShellItemGenericIconManager::GENERIC_ICON_DOCUMENT;

		HRESULT aRes1 = aMan->GetIconType(theSF, theItemPidl);

		if ( SUCCEEDED(aRes1) )
			aGenericIconType = HRESULT_CODE(aRes1);

		aRes1 = aMan->GetIconLocation(aGenericIconType, theParams);

		if (aRes != E_PENDING)
			aRes |= MAKE_HRESULT(0, 0, IIconAcceptor::CHANGE_ICON);
	}

	return aRes;	// retain the return value (especially required if it is E_PENDING)
}

//////////////////////////////////////////////////////////////////////////////////////////////

HRESULT UtilShellFunc::GetIconAsync(IconExtractParams * theParams, void * theArg)
{
	const ShellIconExtractAsyncArg * aArg = reinterpret_cast<const ShellIconExtractAsyncArg *>(theArg);

	ATLASSERT(aArg != 0);
	ATLASSERT(aArg->Flags != 0);
	ATLASSERT(aArg->AbsolutePidl != 0);

	StaticFullPidlExtractor aExt(aArg->AbsolutePidl);

	CComPtr<IShellFolder> aParent;
	LPCITEMIDLIST aChildPidl = 0;

	HRESULT aRes = SHBindToParentEx(aArg->AbsolutePidl, IID_IShellFolder, (void**) &aParent, &aChildPidl);

	if ( FAILED(aRes) )
		return aRes;

	aRes = MAKE_HRESULT(0, 0, 0);

	if ( aArg->Flags & IIconAcceptor::CHANGE_ICON)
	{
		if ( SUCCEEDED(aRes) )
		{
			// Step 1. IExtractIconW
			CComPtr<IExtractIconW> aEIW;
			aRes = aParent->GetUIObjectOf(NULL, 1, &aChildPidl, IID_IExtractIconW, 0, (void **) &aEIW);

			if ( SUCCEEDED(aRes))
				aRes = IconExtractionW::Perform(aExt, theParams, aEIW);

			// Step 2. IExtractIconA
			if ( FAILED(aRes) )
			{
				CComPtr<IExtractIconA> aEIA;
				aRes = aParent->GetUIObjectOf(NULL, 1, &aChildPidl, IID_IExtractIconA, 0, (void **) &aEIA);

				if ( SUCCEEDED(aRes))
					aRes = IconExtractionA::Perform(aExt, theParams, aEIA);
			}

			if (SUCCEEDED(aRes))
				aRes = MAKE_HRESULT(0, 0, IIconAcceptor::CHANGE_ICON);
		}

		if ( FAILED(aRes) )
			Trace("Unable to perform async icon extraction\n");
	}


	if (aArg->Flags & IIconAcceptor::CHANGE_OVERLAY)
	{
		CComQIPtr<IShellIconOverlay> aSIO(aParent);

		if (aSIO != 0)
		{
			HRESULT aRes_OL = GetIconOverlay(aSIO, aChildPidl, theParams);
			if (SUCCEEDED(aRes_OL))
				aRes |= IIconAcceptor::CHANGE_OVERLAY;
		}
	}

	return aRes;
}

//////////////////////////////////////////////////////////////////////////////////////////////

bool UtilShellFunc::GetDirectoryName(IShellFolder * theSF, LPCITEMIDLIST thePidl, TCHAR * theOut, int theOutSize)
{
	STRRET aStrDispName;
	theSF->GetDisplayNameOf(thePidl, SHGDN_NORMAL|SHGDN_FORADDRESSBAR|SHGDN_FORPARSING, &aStrDispName);
	StrRetToBuf( thePidl, aStrDispName, theOut, theOutSize-1);

	//find the last backslash or slash
	int aLastSlash = -1;
	int aLength = lstrlen(theOut);

	for (int i = aLength-1; i >= 0; i--)
		if (theOut[i] == _T('/') || theOut[i] == _T('\\') )
			{ aLastSlash = i+1; break; }	//TEMP

	if (aLastSlash > 0)
	{
		theOut[aLastSlash] = 0;
		return true;
	}
	else
		return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////

HRESULT UtilShellFunc::GetDisplayName(IShellFolder * theSF, LPCITEMIDLIST thePidl, SHGDNF theFlags, TCHAR * theOutput, int theSize) throw()
{
	STRRET aDispName;
	HRESULT aRes = theSF->GetDisplayNameOf(thePidl, theFlags, &aDispName);
	if ( FAILED(aRes) )
		return aRes;

	StrRetToBuf(thePidl, aDispName, theOutput, theSize);
	return aRes;
}

//////////////////////////////////////////////////////////////////////////////////////////////

void UtilShellFunc::StrRetToBuf( LPCITEMIDLIST theItemPidl, const STRRET & theValue, TCHAR * theOutput, int theSize ) throw()
{
	int aLen;

	switch( theValue.uType )
	{
	////////////////////////////////////////////////////////////////////////////////////////////
	case STRRET_CSTR:	// The value is a C string

#ifndef _UNICODE
		aLen = lstrlenA(theValue.cStr);
		lstrcpynA(theOutput, theValue.cStr, __min(aLen+1, theSize) );
#else
		aLen = MultiByteToWideChar(CP_ACP, 0, theValue.cStr, -1, theOutput, theSize);
		theOutput[aLen] = 0;
#endif	//!_UNICODE
		break;
		//NOTE: cStr is not released -- this is according to the documentation


	////////////////////////////////////////////////////////////////////////////////////////////
	case STRRET_WSTR: // The value is on OLE string
#ifdef _UNICODE
		aLen =  lstrlenW(theValue.pOleStr);
		lstrcpynW(theOutput, theValue.pOleStr, __min(aLen+1, theSize) );
#else
		aLen = WideCharToMultiByte(CP_ACP, 0, theValue.pOleStr, -1, theOutput, theSize, NULL, NULL );
		theOutput[aLen] = 0;
#endif
		CoTaskMemFree( theValue.pOleStr );
		break;


	////////////////////////////////////////////////////////////////////////////////////////////
	case STRRET_OFFSET:  // The value is an offset within the PIDL
#ifdef _UNICODE
		lstrcpynW(theOutput, LPCWSTR( PBYTE(theItemPidl) + theValue.uOffset), theSize);
#else
		lstrcpynA(theOutput, LPCSTR( PBYTE(theItemPidl) + theValue.uOffset), theSize);
#endif	//_UNICODE
		break;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////

HRESULT UtilShellFunc::ParseDisplayName(const TCHAR * theName, int theLength, IShellFolder ** theOutSF, LPITEMIDLIST * theOutPidl) throw()
{
	// create a temporary non-const buffer for the string
	// this is necessary because ParseDisplayName modifies the buffer
	TCHAR * aTemp = new TCHAR[theLength+1];
	lstrcpyn(aTemp, theName, theLength+1);

	HRESULT aRes = SHGetDesktopFolder(theOutSF);

	if ( SUCCEEDED(aRes) )
		aRes = (*theOutSF)->ParseDisplayName(NULL, NULL, aTemp, NULL, theOutPidl, NULL);

	delete [] aTemp;

	return aRes;
}

//////////////////////////////////////////////////////////////////////////////////////////////

HRESULT UtilShellFunc::GetIconOverlay(IShellFolder * theSF, LPCITEMIDLIST theItemPidl, IconExtractParams * theParams, void ** theOutAsyncArg)
{
	HRESULT aRes = E_FAIL;
	CComQIPtr<IShellIconOverlay> aSIO(theSF);

	if (aSIO == 0)
		return aRes;

	return GetIconOverlay(aSIO, theItemPidl, theParams);
}

//////////////////////////////////////////////////////////////////////////////////////////////

HRESULT UtilShellFunc::GetIconOverlay(IShellIconOverlay * theSIO, LPCITEMIDLIST theItemPidl, IconExtractParams * theParams)
{
	int aOverlayIndex = 0;

	if ( (theParams->Flags & EXTRACT_PREFERFAST) != 0 )
		aOverlayIndex = OI_ASYNC;

	HRESULT aRes = theSIO->GetOverlayIndex(theItemPidl, &aOverlayIndex);

	if ( SUCCEEDED(aRes) )
	{
		if (aOverlayIndex == 1)
			theParams->OverlayIndex = 3;	//fix the bug with shared folders in Windows 7
		else
		{
			aOverlayIndex = 0;

			aRes = theSIO->GetOverlayIconIndex(theItemPidl, &aOverlayIndex);

			if (aRes == S_OK)
				theParams->OverlayIndex = aOverlayIndex;
		}
	}

	return aRes;
}

//////////////////////////////////////////////////////////////////////////////////////////////

LPITEMIDLIST UtilShellFunc::GetAbsolutePidl(IShellFolder * theSF, LPCITEMIDLIST thePidl)
{
	CComQIPtr<IPersistFolder2> aPF2(theSF);

	if (aPF2 == 0)
		return 0;

	UINT aSize_Child = UtilPidlFunc::PidlSize(thePidl);

	LPITEMIDLIST aPidl_ParentOnly = 0;	//needs to be freed
	HRESULT aRes = aPF2->GetCurFolder(&aPidl_ParentOnly);

	if ( FAILED(aRes) || aPidl_ParentOnly == 0)
		return 0;

	LPITEMIDLIST aRet = 0;
	if (thePidl != 0)
	{
		aRet = UtilPidlFunc::PidlAppend<MemoryWriter_Crt>(aPidl_ParentOnly, thePidl);
		CoTaskMemFree(aPidl_ParentOnly);	//finalize
	}
	else
		aRet = aPidl_ParentOnly;	//need not free

	return aRet;
}

//////////////////////////////////////////////////////////////////////////////////////////////

HRESULT UtilShellFunc::ReleaseAsyncArg(void * theAsyncArg)
{
	ShellIconExtractAsyncArg * aArg = reinterpret_cast<ShellIconExtractAsyncArg *>(theAsyncArg);

	if (aArg == 0)
		return S_FALSE;

	delete aArg;
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////////////////////////

HRESULT UtilShellFunc::ShellExecuteRetValToHResult(int theShellExecRet)
{
	switch (theShellExecRet)
	{
	case 0:                        return E_OUTOFMEMORY;
	case ERROR_FILE_NOT_FOUND:
	case ERROR_PATH_NOT_FOUND:
	case ERROR_BAD_FORMAT:         return E_INVALIDARG;
	case SE_ERR_ACCESSDENIED:      return E_ACCESSDENIED;
	case SE_ERR_ASSOCINCOMPLETE:
	case SE_ERR_DDEBUSY:
	case SE_ERR_DDEFAIL:
	case SE_ERR_DDETIMEOUT:
	case SE_ERR_DLLNOTFOUND:
	case SE_ERR_NOASSOC:
	case SE_ERR_OOM:
	case SE_ERR_SHARE:             return E_FAIL;
	default:                       return S_OK;
	}
}
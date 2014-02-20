#include "stdafx.h"

#include "sdk_MemoryWriter.h"

#include "ShellItem.h"
#include "UtilPidlFunc.h"
#include "UtilShellFunc.h"
#include "Trace.h"
#include "InterProcessContextMenuInfo.h"
#include "Application.h"

//////////////////////////////////////////////////////////////////////////////////////////////

ShellItem::ShellItem() : myPidl(NULL), myParentSF(NULL)
{
#ifdef _DEBUG
	SecureZeroMemory(myDbgFullName, MAX_PATH * sizeof TCHAR); 
#endif	//DEBUG
}

//////////////////////////////////////////////////////////////////////////////////////////////

ShellItem::ShellItem(LPITEMIDLIST theInitPidl, IShellFolder * theInitSF /* = NULL*/)
{
	myPidl = theInitPidl;

	if (!theInitSF) SHGetDesktopFolder(&myParentSF);
	else            { myParentSF = theInitSF; myParentSF->AddRef(); }

	Init();
}

//////////////////////////////////////////////////////////////////////////////////////////////

ShellItem::ShellItem(const TCHAR * thePath)
{
	int aLen = lstrlen(thePath);

	UtilShellFunc::ParseDisplayName(thePath, aLen, &myParentSF, &myPidl);

	Init();
}

//////////////////////////////////////////////////////////////////////////////////////////////

ShellItem::~ShellItem()
{
	if (myPidl != NULL)
		CoTaskMemFree(myPidl);

	myPidl = NULL;

	//added 2006-02-19
	if (myParentSF != 0)
		myParentSF->Release();
}

//////////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP ShellItem::GetDisplayName(bool theFullName, TCHAR * theOutName, int * theSize)
{
	UtilShellFunc::GetDisplayName(myParentSF, myPidl, GetDisplayNameFlags(theFullName), theOutName, *theSize);
	//TODO: move to UtilShellFunc
	*theSize = lstrlen(theOutName);
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////////////////////////

void ShellItem::Init()
{
	myRefs = 1UL;

#ifdef _DEBUG
	UtilShellFunc::GetDisplayName
	(
		myParentSF,
		myPidl,
		SHGDN_NORMAL|SHGDN_FORADDRESSBAR|SHGDN_FORPARSING,
		myDbgFullName,
		MAX_PATH
	);
#endif	//_DEBUG
}

//////////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP ShellItem::DoLaunch(HWND theParentWnd, ULONG theOptions)
{
	IItemContextMenuHandler * aH = Application::InstanceC().GetItemContextMenuHandler();

	if (aH == 0)
		return E_FAIL;

	bool aDefaultOnly = (theOptions == RUN_DEFAULT_ACTION);
	return aH->DoHandle(this, theParentWnd, aDefaultOnly);
}

//////////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP ShellItem::RetrieveIconData(IconExtractParams * theParams, void ** theOutAsyncArg)
{
	return UtilShellFunc::GetIconNew(myParentSF, myPidl, theParams, theOutAsyncArg);
}

//////////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP ShellItem::RetrieveIconDataAsync(IconExtractParams * theParams, void * theAsyncArg)
{
	return UtilShellFunc::GetIconAsync(theParams, theAsyncArg);
}

//////////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP ShellItem::ReleaseAsyncArg(void * theAsyncArg)
{
	return UtilShellFunc::ReleaseAsyncArg(theAsyncArg);
}

//////////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP ShellItem::GetShellItemData(PidlType theType, IShellFolder ** theOutSF, LPITEMIDLIST * theOutPidl)
{
	if (theOutPidl == 0)
		return E_POINTER;
	else if (myParentSF == 0)
		return E_FAIL;


	switch (theType)
	{
	case PRIMARY_RELATIVE:
	case SECONDARY_RELATIVE:
		{
			if (theOutSF == 0)
				return E_POINTER;

			*theOutSF = myParentSF;
			myParentSF->AddRef();

			*theOutPidl = UtilPidlFunc::Copy<MemoryWriter_Crt>(myPidl);

			return S_OK;
		}


	case PRIMARY_FULL:
	case SECONDARY_FULL:
		{
			LPITEMIDLIST aAbs = UtilShellFunc::GetAbsolutePidl(myParentSF, myPidl);

			if (aAbs == 0)
				return E_FAIL;
			else
			{
				*theOutPidl = aAbs;

				if (theOutSF != 0)
					return SHGetDesktopFolder(theOutSF);
				else
					return S_OK;
			}
		}

	case PARENTFOLDER_ABSOLUTE:
		{
			LPITEMIDLIST aAbs = UtilShellFunc::GetAbsolutePidl(myParentSF, NULL);

			if (aAbs == 0)
				return E_FAIL;
			else
			{
				*theOutPidl = aAbs;

				if (theOutSF != 0)
					return SHGetDesktopFolder(theOutSF);
				else
					return S_OK;
			}
		}

	default:
		return E_INVALIDARG;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////

SHGDNF ShellItem::GetDisplayNameFlags( bool theFullName )
{
	return theFullName ?
		SHGDN_NORMAL|SHGDN_FORADDRESSBAR|SHGDN_FORPARSING : 
		SHGDN_NORMAL;
}

//////////////////////////////////////////////////////////////////////////////////////////////

ULONG STDMETHODCALLTYPE ShellItem::AddRef()
{
	return ++myRefs;
}

//////////////////////////////////////////////////////////////////////////////////////////////

ULONG STDMETHODCALLTYPE ShellItem::Release()
{
	ULONG aNumRefs = --myRefs;

	if (aNumRefs == 0UL)
		delete this;

	return aNumRefs;
}

//////////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP ShellItem::QueryInterface(REFIID theIID, void ** theOut)
{
	if (theIID == IID_ILaunchable)
		*theOut = static_cast<ILaunchable *>(this);

	else if (theIID == IID_IIconAcceptor)
		*theOut = static_cast<IIconAcceptor *>(this);

	else if (theIID == IID_IApproachShellItem)
		*theOut = static_cast<IApproachShellItem *>(this);

	else if (theIID == IID_IUnknown)
		*theOut = static_cast<IUnknown *>( static_cast<Item *>(this) );

	else
		return E_NOINTERFACE;

	AddRef();
	return S_OK;
}

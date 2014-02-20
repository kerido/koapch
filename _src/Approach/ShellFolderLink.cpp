#include "stdafx.h"

#include "sdk_MemoryWriter.h"

#include "ShellFolderLink.h"
#include "UtilShellFunc.h"
#include "Application.h"

//////////////////////////////////////////////////////////////////////////////////////////////

ShellFolderLink::ShellFolderLink(IShellFolder * theParent_Trg, LPITEMIDLIST thePidl_Trg,
																 IShellFolder * theParent_Lnk, LPITEMIDLIST theItem_Lnk)
																 : ShellFolder(thePidl_Trg, theParent_Trg)
{
	myNameSF = theParent_Lnk;
	myNamePidl = theItem_Lnk;
}

//////////////////////////////////////////////////////////////////////////////////////////////

ShellFolderLink::~ShellFolderLink()
{
	if (myNamePidl != NULL)
		CoTaskMemFree(myNamePidl);
}

//////////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP ShellFolderLink::GetDisplayName(bool theFullName, TCHAR * theOutName, int * theSize)
{
	UtilShellFunc::GetDisplayName(myNameSF, myNamePidl, GetDisplayNameFlags(theFullName), theOutName, *theSize);
	//TODO: move to UtilShellFunc
	*theSize = lstrlen(theOutName);
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP ShellFolderLink::RetrieveIconData(IconExtractParams * theParams, void ** theOutAsyncArg)
{
	return UtilShellFunc::GetIconNew(myNameSF, myNamePidl, theParams, theOutAsyncArg);
}

//////////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP ShellFolderLink::GetShellItemData(PidlType theType, IShellFolder ** theOutSF, LPITEMIDLIST * theOutPidl)
{
	if (theOutPidl == 0)
		return E_POINTER;
	else if (myParentSF == 0)
		return E_FAIL;

	switch (theType)
	{
	case PRIMARY_RELATIVE:
		if ( theOutSF != 0 )
		{
			*theOutSF = myNameSF;
			myNameSF->AddRef();
		}

		*theOutPidl = UtilPidlFunc::Copy<MemoryWriter_Crt>(myNamePidl);

		return S_OK;

	case PRIMARY_FULL:
		{
			LPITEMIDLIST aAbs = UtilShellFunc::GetAbsolutePidl(myNameSF, myNamePidl);

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
		return ShellItem::GetShellItemData(theType, theOutSF, theOutPidl);
	}
}

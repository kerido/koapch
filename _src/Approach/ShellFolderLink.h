#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////

#include "ShellFolder.h"

//////////////////////////////////////////////////////////////////////////////////////////////

class ShellFolderLink : public ShellFolder
{
// Fields
private:
	IShellFolder * myNameSF;
	LPITEMIDLIST   myNamePidl;


// Constructors, Destructor
public:
	ShellFolderLink(IShellFolder * theParent_Trg, LPITEMIDLIST thePidl_Trg,
									IShellFolder * theParent_Lnk, LPITEMIDLIST theItem_Lnk);

	virtual ~ShellFolderLink();

protected:
	LPCITEMIDLIST GetPidl() const { return myNamePidl; }


// Item Members
protected:
	STDMETHODIMP GetDisplayName(bool theFullName, TCHAR * theOutName, int * theSize);


// IIconAcceptor Members
protected:
	STDMETHODIMP RetrieveIconData(IconExtractParams * theParams, void ** theOutAsyncArg);


//IApproachShellItem members
protected:
	STDMETHODIMP GetShellItemData(PidlType theType, IShellFolder ** theOutSF, LPITEMIDLIST * theOutPidl);
};

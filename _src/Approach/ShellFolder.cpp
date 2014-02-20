#include "stdafx.h"

#include "ShellFolder.h"
#include "ShellFolderLink.h"
#include "ShellItemFactory.h"
#include "ShellItemEnumerator.h"
#include "MenuManager.h"

#include "UtilPidlFunc.h"

/////////////////////////////////////////////////////////////////////////////

ShellFolder::ShellFolder(LPITEMIDLIST theInitPIDL, IShellFolder * theParentSF /* = NULL*/)
	: ShellItem( theInitPIDL, theParentSF )
{
	//TODO: Bugs in ShellFolder constructor when calling InitShellFolder (VISTA)
	InitShellFolder();
}

//////////////////////////////////////////////////////////////////////////////////////////////

ShellFolder::ShellFolder(const TCHAR * thePath) : ShellItem(thePath)
{
	InitShellFolder();
}

//////////////////////////////////////////////////////////////////////////////////////////////

ShellFolder::ShellFolder(int theCSIDL)
{
	HRESULT aRes = SHGetDesktopFolder(& myParentSF);
	aRes = SHGetSpecialFolderLocation(NULL, theCSIDL, &myPidl);

	InitShellFolder();
	Init();
}

//////////////////////////////////////////////////////////////////////////////////////////////

ShellFolder::~ShellFolder()
{
	if (mySF)
		mySF->Release();

	// TODO: need to check whether myParentSF points to a desktop folder.
	// if it does, need to release it; otherwise it will be released when a parent
	// ShellFolder or ShellFolderLink will be deleted.
}

//////////////////////////////////////////////////////////////////////////////////////////////

void ShellFolder::InitShellFolder()
{
	// When this func is called, myParentSF and myPidl members are already initialized.
	// So we can safely browse for myPidl and extract mySF. However, we are not guaranteed
	// to succeed in BindToObject, so we explicitly check if the result is successful.

	HRESULT aRes;

	if ( UtilPidlFunc::IsZero(myPidl) )
		aRes = myParentSF->QueryInterface(IID_IShellFolder, (void**) & mySF);

	else
		aRes = myParentSF->BindToObject(myPidl, NULL, IID_IShellFolder, (void**) & mySF);

	if ( FAILED(aRes) )
		mySF = 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP ShellFolder::EnumItems(ULONG theOptions, IEnumItems ** theOutEnum)
{
	return ShellItemList::Enum(mySF, theOptions, theOutEnum);
}

//////////////////////////////////////////////////////////////////////////////////////////////

ULONG STDMETHODCALLTYPE ShellFolder::AddRef()
{
	return ShellItem::AddRef();
}

//////////////////////////////////////////////////////////////////////////////////////////////

ULONG STDMETHODCALLTYPE ShellFolder::Release()
{
	return ShellItem::Release();
}

//////////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP ShellFolder::QueryInterface( REFIID theIID, void ** theOut )
{
	if (theIID == IID_IItemProvider)
		* theOut = static_cast<IItemProvider *>(this);

	else
		return ShellItem::QueryInterface(theIID, theOut);

	AddRef();
	return S_OK;
}
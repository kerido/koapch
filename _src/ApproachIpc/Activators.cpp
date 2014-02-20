#include "Common.h"

#include "Main.h"
#include "Trace.h"

//////////////////////////////////////////////////////////////////////////////////////////////

extern GetCtxHandler    gFnGetCtxMenuHandler;

//////////////////////////////////////////////////////////////////////////////////////////////

//! 
BaseShellClickData * Common_Activate_WinXP2003VistaWin7(HWND theShellViewWnd, HWND theListViewWnd, int theItemIndex)
{
	IUnknown * aSF = 0;		     // These two values will be returned
	LPITEMIDLIST aPidl = 0;    // in the form if BaseShellClickData


#pragma warning(disable : 4312)
	IShellView * aSV = (IShellView *)GetWindowLongPtr(theShellViewWnd, 0);
#pragma warning(default : 4312)

	if (aSV == 0)
		return 0;

	IFolderView * aFV = 0;
	HRESULT aRes = aSV->QueryInterface(IID_IFolderView, (void**) &aFV);

	if ( FAILED(aRes) || aFV == 0 )
		return 0;


	//Step 1. Shell Folder -- always returned

	aRes = aFV->GetFolder(IID_IUnknown, (void **) &aSF);

	if ( FAILED(aRes) )
	{
		aFV->Release();
		return 0;
	}


	// Step 2. Item Pidl -- returned only if theListViewWnd and theItemIndex are supplied

	if (theListViewWnd != 0 && theItemIndex >= 0)
		aRes = aFV->Item(theItemIndex, &aPidl);


	Trace("Creating BaseShellClickData...\n");
	BaseShellClickData * aStr = new BaseShellClickData
	(
		aSF,
		aPidl,
		gFnGetCtxMenuHandler(theShellViewWnd),
		true
	);

	aFV->Release();

	return aStr;
}

//////////////////////////////////////////////////////////////////////////////////////////////

//OSSPECIFIC: Common_Activate_Win2K extracts undocumented data
BaseShellClickData * Common_Activate_Win2K(HWND theShellViewWnd, HWND theListViewWnd, int theItemIndex)
{
	IUnknown * aSF = 0;
	LPITEMIDLIST aPidl = 0;


	//Step 1. Shell Folder -- always returned

#pragma warning(disable : 4312)

	void * aRawPtr = (void *)GetWindowLongPtr(theShellViewWnd, 0);
	aSF = *(IUnknown **)( (char *) aRawPtr + 0x90 );

#pragma warning(default : 4312)



	// Step 2. Item Pidl -- returned only if theListViewWnd and theItemIndex are supplied
	//         This is because this routine can also be called from a Titlebar Menus event
	//         where theListViewWnd is zero and theItemIndex is -1

	if (theListViewWnd != 0 && theItemIndex >= 0)
	{
		LVITEM aLVItem;
		aLVItem.mask = LVIF_PARAM;
		aLVItem.iItem = theItemIndex;

		BOOL aRes = ListView_GetItem(theListViewWnd, &aLVItem);

		if (aRes != FALSE)
			aPidl = (LPITEMIDLIST) aLVItem.lParam;
	}


	Trace("Creating BaseShellClickData...\n");
	BaseShellClickData * aStr = new BaseShellClickData
		(
			aSF,
			aPidl,
			gFnGetCtxMenuHandler(theShellViewWnd),
			false
		);

	return aStr;
}


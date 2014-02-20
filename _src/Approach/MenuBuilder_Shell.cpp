#include "stdafx.h"

#include "sdk_Theme.h"
#include "sdk_ItemProvider.h"
#include "sdk_Item.h"

#include "MenuBuilder_Shell.h"
#include "MenuWindow_Shell.h"
#include "MenuManager.h"

//////////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP MenuBuilder_Shell::CanBuild (Item * theItem, UINT * theOut)
{
	if (theOut == 0)
		return E_POINTER;

	IItemProvider * aPr = 0;
	HRESULT aRes = theItem->QueryInterface(IID_IItemProvider, (void**) &aPr);

	if ( SUCCEEDED(aRes) && aPr != 0)
	{
		*theOut = EXPANDABLE_BASIC_FOLDER;
		aPr->Release();
	}
	else
		*theOut = IMenuBuilder::NON_EXPANDABLE;

	return aRes;
}

//////////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP MenuBuilder_Shell::GetArrowBitmap(UINT theExpCode, bool theSelected, const Theme * theTheme, BitmapInfo * theInfo)
{
	return E_NOTIMPL;
}

//////////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP MenuBuilder_Shell::Build(Item * theItem, const MenuBuilderData * theBuildData, MenuWindow ** theOutMenu)
{
	if (theOutMenu == 0)
		return E_POINTER;

	IItemProvider * aPr = 0;
	HRESULT aRes = theItem->QueryInterface(IID_IItemProvider, (void**) &aPr);

	if ( FAILED(aRes) || aPr == 0)
	{
		*theOutMenu = 0;
		return aRes;
	}

	MenuWindow_Shell * aNewWindow = new MenuWindow_Shell(theBuildData->Agent);

	aNewWindow->SetTheme(theBuildData->Theme);
	aNewWindow->PopulateAndMeasure(aPr);
	aNewWindow->SetRestorableData(theItem, theBuildData->Rect);

	aPr->Release();

	*theOutMenu = aNewWindow;
	return S_OK;
}

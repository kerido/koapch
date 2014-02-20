#pragma once

#include "sdk_MenuBuilder.h"

//////////////////////////////////////////////////////////////////////////////////////////////

//! Creates instances of the MenuWindow_Shell class based on the IItemProvider interface.
class MenuBuilder_Shell : public IMenuBuilder
{
private:
	ULONG myNumRefs;

public:
	MenuBuilder_Shell() : myNumRefs(1UL) { }


public:
	STDMETHODIMP Build (Item * theItem, const MenuBuilderData * theBuildData, MenuWindow ** theOutMenu);

	STDMETHODIMP CanBuild (Item * theItem, UINT * theOut);

	STDMETHODIMP GetArrowBitmap (UINT theExpCode, bool theSelected, const Theme * theTheme, BitmapInfo * theOutInfo);


public:
	STDMETHODIMP QueryInterface(REFIID theIID, void ** theOut)
	{
		if (theIID == IID_IUnknown)
			*theOut = static_cast<IUnknown *>(this);

		if (theIID == IID_IMenuBuilder)
			*theOut = static_cast<IMenuBuilder *>(this);
		else
			return E_NOINTERFACE;

		AddRef();
		return S_OK;
	}

	ULONG STDMETHODCALLTYPE AddRef()
	{
		return ++myNumRefs;
	}

	ULONG STDMETHODCALLTYPE Release()
	{
		ULONG aNumRefs = --myNumRefs;

		if (aNumRefs == 0UL)
			delete this;

		return aNumRefs;
	}
};

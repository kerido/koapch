#pragma once

#include "sdk_Item.h"
#include "sdk_ItemProvider.h"
#include "sdk_Launchable.h"
#include "sdk_ItemFactory.h"
#include "sdk_IconExtractor.h"

#include "ItemDesignerBase.h"

//////////////////////////////////////////////////////////////////////////////////////////////

//! Represents a logical item which, upon click, displays the standard Shell Run
//! dialog box.
class LogicalItem_Run_Root :
	public Item,
	public ILaunchable,
	public IItemProvider,
	public IIconAcceptor
{
private:
	class RunItemsEnumerator;


private:
	ULONG myRefs;                   //!< Stores the number of references to the current object.


// Constructors, destructor
public:
	LogicalItem_Run_Root();


// Item members
protected:
	STDMETHODIMP GetDisplayName(bool theFullName, TCHAR * theOutName, int * theSize);


// IItemProvider Members
protected:
	STDMETHODIMP EnumItems(ULONG theOptions, IEnumItems ** theOut);


// ILaunchable Members
protected:
	STDMETHODIMP DoLaunch(HWND theParent, ULONG theOptions);


// IIconAcceptor Members
protected:
	STDMETHODIMP RetrieveIconData(IconExtractParams * theParams, void ** theOptAsyncArg);

	STDMETHODIMP RetrieveIconDataAsync(IconExtractParams * theParams, void * theAsyncArg)
		{ return E_NOTIMPL; }

	STDMETHODIMP ReleaseAsyncArg(void * theAsyncArg)
		{ return E_NOTIMPL; }


// IUnknown Members
protected:
	ULONG STDMETHODCALLTYPE AddRef();
	ULONG STDMETHODCALLTYPE Release();
	STDMETHODIMP QueryInterface(REFIID theIID, void ** theOut);
};

//////////////////////////////////////////////////////////////////////////////////////////////

//! Represents an entry in the Shel Run MRU list.
class LogicalItem_Run_Child :
	public Item,
	public ComEntry2<ILaunchable, IIconAcceptor>
{
protected:
	std::wstring myString;     //!< The display name of the item.


// Constructors, destructor
public:
	LogicalItem_Run_Child(const std::wstring & theString);


// ILaunchable Members
protected:
	STDMETHODIMP DoLaunch(HWND theMenuParent, ULONG theOptions);


// Item Members
protected:
	STDMETHODIMP GetDisplayName(bool theFullName, TCHAR * theOutName, int * theSize);


// IIconAcceptor Members
protected:
	STDMETHODIMP RetrieveIconData(IconExtractParams * theParams, void ** theOptAsyncArg);

	STDMETHODIMP RetrieveIconDataAsync(IconExtractParams * theParams, void * theAsyncArg)
		{ return E_NOTIMPL; }

	STDMETHODIMP ReleaseAsyncArg(void * theAsyncArg)
		{ return E_NOTIMPL; }
};

//////////////////////////////////////////////////////////////////////////////////////////////

class RunItemFactory : public ComEntry2<IItemFactory, IItemDesigner>
{
// IItemFactory Members
protected:
	STDMETHODIMP CreateItem (const BYTE * theData, int theDataSize, Item ** theOutItem);


// IItemDesigner Members
protected:
	STDMETHODIMP GetNumItems (int * theOut);

	STDMETHODIMP GetItemData (int theIndex, MenuSetItemData * theOut);

	STDMETHODIMP GetItemName (const MenuSetItemData * theItem, TCHAR * theOut, int theBufSize);

	STDMETHODIMP EditData    (MenuSetItemData * theItem, HWND theOwner, DWORD theParam1, LPARAM theParam2)
		{ return E_NOTIMPL; }


// IRuntimeObject Members
protected:
	STDMETHODIMP GetTypeGuid(GUID * theOut);
};

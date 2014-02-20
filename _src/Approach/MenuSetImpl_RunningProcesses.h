#pragma once

#include "sdk_Item.h"
#include "sdk_ItemProvider.h"
#include "sdk_Launchable.h"
#include "sdk_ItemFactory.h"
#include "sdk_IconExtractor.h"

#include "ItemDesignerBase.h"

//////////////////////////////////////////////////////////////////////////////////////////////

class LogicalItem_Processes_Root :
	public Item,
	public ComEntry3<IItemProvider, IIconAcceptor, ILaunchable>
{
// Constructors, Destructor
public:
	LogicalItem_Processes_Root();

// Item Members
protected:
	STDMETHODIMP GetDisplayName(bool theFullName, TCHAR * theOutName, int * theSize);


// IItemProvider Members
protected:
	STDMETHODIMP EnumItems(ULONG theOptions, IEnumItems ** theOut);


// IIconAcceptor Members
protected:
	STDMETHODIMP RetrieveIconData(IconExtractParams * theParams, void ** theOptAsyncArg);

	STDMETHODIMP RetrieveIconDataAsync (IconExtractParams * theParams, void * theAsyncArg)
		{ return E_NOTIMPL; }

	STDMETHODIMP ReleaseAsyncArg       (void * theAsyncArg)
		{ return E_NOTIMPL; }


	// ILaunchable Members
protected:
	STDMETHODIMP DoLaunch(HWND theParent, ULONG theOptions);
};

//////////////////////////////////////////////////////////////////////////////////////////////

class LogicalItem_Processes_Child :
	public Item,
	public ComEntry3<IItemProvider, IIconAcceptor, ILaunchable>
{
protected:
	const static int ProcessNameSize = 1024;

public:
	enum ProcessItemFlags
	{
		NameValid   = 1,
		NameChecked = 2,
		CanQuit     = 4,
		CanEdit     = 8
	};

protected:
	TCHAR myProcessName[ProcessNameSize];
	TCHAR * myProcessShortName;
	int myActualLength;
	DWORD myProcessID;
	HANDLE myHProcess;
	int myFlags;


public:
	LogicalItem_Processes_Child(DWORD theProcessID, HANDLE theHProcess);

	~LogicalItem_Processes_Child();


// Interface
public:
	const TCHAR * GetFullName() const
		{ return myProcessName; }

	const TCHAR * GetShortName() const
		{ return myProcessShortName; }

	HANDLE GetProcessHandle() const
		{ return myHProcess; }

	bool HasAllFlags(int theFlags) const
		{ return (myFlags & theFlags) == theFlags; }

	bool HasOneFlag(int theFlags) const
		{ return (myFlags & theFlags) != 0; }

// Item Members
protected:
	STDMETHODIMP GetDisplayName(bool theFullName, TCHAR * theOutName, int * theSize);


// IItemProvider Members
protected:
	STDMETHODIMP EnumItems(ULONG theOptions, IEnumItems ** theOut);


// IIconAcceptor Members
protected:
	STDMETHODIMP RetrieveIconData(IconExtractParams * theParams, void ** theOptAsyncArg);

	STDMETHODIMP RetrieveIconDataAsync(IconExtractParams * theParams, void * theAsyncArg);

	STDMETHODIMP ReleaseAsyncArg(void * theAsyncArg);


// ILaunchable Members
protected:
	STDMETHODIMP DoLaunch(HWND theParent, ULONG theOptions);
};


class LogicalItem_Processes_Process : public ComInstance<LogicalItem_Processes_Child>
{
public:
	LogicalItem_Processes_Process(DWORD theProcessID, HANDLE theHProcess)
		: ComInstance<LogicalItem_Processes_Child>(theProcessID, theHProcess) { }

protected:
	STDMETHODIMP QueryInterface(REFIID theIID,  void ** theOut)
	{
		if ( theIID == __uuidof(IItemProvider) && !HasOneFlag(NameValid|CanQuit) )
			return E_NOINTERFACE;
		else
			return ComInstance<LogicalItem_Processes_Child>::QueryInterface(theIID, theOut);
	}
};

//////////////////////////////////////////////////////////////////////////////////////////////

class LogicalItem_Processes_Task :
	public Item,
	public ComEntry2<ILaunchable, IIconAcceptor>
{
public:
	enum ItemType
	{
		Reveal = 0,
		Quit = 1
	};

private:
	const LogicalItem_Processes_Child * myProcessItem;
	ItemType myType;


public:
	LogicalItem_Processes_Task(const LogicalItem_Processes_Child * theProcessItem, ItemType theType)
		: myProcessItem(theProcessItem), myType(theType) { }


// Item Members
protected:
	STDMETHODIMP GetDisplayName(bool theFullName, TCHAR * theOutName, int * theSize);


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
};

//////////////////////////////////////////////////////////////////////////////////////////////

class ItemFactory_Processes : public ComEntry2<IItemFactory, IItemDesigner>
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

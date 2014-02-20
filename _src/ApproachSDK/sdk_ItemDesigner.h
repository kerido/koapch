#pragma once

#include "sdk_RuntimeObject.h"

//////////////////////////////////////////////////////////////////////////////////////////////

struct MenuSetItemData
{
	//! Represents the type of the Menu Set Entry.
	enum EntryType
	{
		ITEM,               //!< Specifies that the entry represents a single Item.
		LIST,               //!< Specifies that the entry represents a single Item list.
		SEPARATOR,          //!< Specifies that the entry is a separator element.
		DISPITEM            //!< Specifies that the entry is a display item.
	};

	size_t    Size;       //!< The size of the structure, must be sizeof(MenuSetItemData)þ
	EntryType Type;       //!< The type of the entry.
	GUID      ObjectID;   //!< The GUID of the Content Provider which enumerates the contents of the entry.
	int       IconIndex;  //!< TODO: Stores the index of the icon representing the item.
	TCHAR *   Name;       //!< Stores a pointer to the buffer that holds the item's name.
	int       NameSize;   //!< Stores the size of the buffer that holds the item's name.
	BYTE *    Param;      //!< Stores an arbitrary parameter sent to the IItemFactory or IItemList.
	int       ParamSize;  //!< Stores the length of the parameter buffer.
};

//////////////////////////////////////////////////////////////////////////////////////////////

MIDL_INTERFACE("B91FC1D4-7B4A-4657-AD3F-B525C3481D68") IItemDesigner : public IRuntimeObject
{
	STDMETHOD (GetNumItems) (int * theOut) = 0;
	STDMETHOD (GetItemData) (int theIndex, MenuSetItemData * theOut) = 0;
	STDMETHOD (GetItemName) (const MenuSetItemData * theItem, TCHAR * theOut, int theBufSize) = 0;
	STDMETHOD (EditData)    (MenuSetItemData * theItem, HWND theOwner, DWORD theParam1, LPARAM theParam2) = 0;
};

// {B91FC1D4-7B4A-4657-AD3F-B525C3481D68}
DFN_GUID(IID_IItemDesigner,
         0xB91FC1D4, 0x7B4A, 0x4657, 0xAD, 0x3F, 0xB5, 0x25, 0xC3, 0x48, 0x1D, 0x68);

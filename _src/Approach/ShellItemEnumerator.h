#pragma once

#include "sdk_ItemProvider.h"

#include "ItemDesignerBase.h"

//////////////////////////////////////////////////////////////////////////////////////////////

struct ShellItemProviderParam
{
	enum Type
	{
		FullPath,
		FullPidl,
	};

	const static DWORD SiPp = 0x70506953;

	DWORD Magic;
	Type ParamType;

	union
	{
		LPITEMIDLIST Pidl;
		TCHAR Path[MAX_PATH];
	};


	ShellItemProviderParam() : Magic(SiPp)
	{ }

	ShellItemProviderParam(const TCHAR * thePath) : Magic(SiPp), ParamType(FullPath)
	{
		lstrcpy(Path, thePath);
	}
};

//////////////////////////////////////////////////////////////////////////////////////////////

class ShellItemList : public ComEntry2<IItemList, IItemDesigner>
{
// IRuntimeObject members
protected:
	STDMETHODIMP GetTypeGuid(GUID * theOut);


// IItemList members
protected:
	STDMETHODIMP EnumItems(const BYTE * theData, int theDataSize, ULONG theOptions, IEnumItems ** theOutEnum);


// IItemDesigner Members
protected:
	STDMETHODIMP GetNumItems (int * theOut)
		{ return E_NOTIMPL; }

	STDMETHODIMP GetItemData (int theIndex, MenuSetItemData * theOut)
		{ return E_NOTIMPL; }

	STDMETHODIMP GetItemName (const MenuSetItemData * theItem, TCHAR * theOut, int theBufSize)
		{ return E_NOTIMPL; }

	STDMETHODIMP EditData    (MenuSetItemData * theItem, HWND theOwner, DWORD theParam1, LPARAM theParam2)
		{ return E_NOTIMPL; }


public:
	static DWORD GetEnumFlags();

	static HRESULT Enum(IShellFolder * theSF, ULONG theOptions, IEnumItems ** theOutEnum);
};

DFN_GUID(OBJ_ShellItemList,
         0x7429B826, 0x7E0A, 0x438A, 0xA5, 0xE0, 0x07, 0xCC, 0x6D, 0x7D, 0xD8, 0x07);
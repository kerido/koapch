#pragma once

class Item;

//////////////////////////////////////////////////////////////////////////////////////////////

MIDL_INTERFACE("71048925-7DAF-44C7-A895-5742AED654D4") IContextMenuExecutor : public IUnknown
{
	enum Flags
	{
		NORMAL       = 0,
		DEFAULT_ONLY = 1
	};

	STDMETHOD (PopulateMenu)  (HMENU theOutMenu, UINT theMinID, UINT theMaxID, ULONG theFlags) = 0;
	STDMETHOD (InvokeCommand) (UINT theID, HWND theOwner, ULONG theExt) = 0;
	STDMETHOD (HandleMessage) (UINT theMsg, WPARAM theWParam, LPARAM theLParam, LRESULT * theRes) = 0;
};

// {71048925-7DAF-44C7-A895-5742AED654D4}
DFN_GUID(IID_IContextMenuExecutor,
         0x71048925, 0x7DAF, 0x44C7, 0xA8, 0x95, 0x57, 0x42, 0xAE, 0xD6, 0x54, 0xD4);

//////////////////////////////////////////////////////////////////////////////////////////////

MIDL_INTERFACE("C3F6F4DC-A440-45F6-BE0A-3BA0A2DE7555") IContextMenuExtension : public IUnknown
{
	STDMETHOD (QueryExecutor) (Item * theItem, ULONG theExt1, ULONG theExt2, IContextMenuExecutor ** theOut) = 0;
};

// {C3F6F4DC-A440-45F6-BE0A-3BA0A2DE7555}
DFN_GUID(IID_IContextMenuExtension,
         0xC3F6F4DC, 0xA440, 0x45F6, 0xBE, 0xA, 0x3B, 0xA0, 0xA2, 0xDE, 0x75, 0x55);

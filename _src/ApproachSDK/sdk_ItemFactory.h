#pragma once

#include "sdk_RuntimeObject.h"

//////////////////////////////////////////////////////////////////////////////////////////////

class Item;

MIDL_INTERFACE("457F87FE-FC89-4786-9309-228A82159CA5") IItemFactory : public IRuntimeObject
{
public:
	STDMETHOD (CreateItem) (const BYTE * theData, int theDataSize, Item ** theOutItem) = 0;
};

// {457F87FE-FC89-4786-9309-228A82159CA5}
DFN_GUID(IID_IItemFactory,
         0x457F87FE, 0xFC89, 0x4786, 0x93, 0x09, 0x22, 0x8A, 0x82, 0x15, 0x9C, 0xA5);

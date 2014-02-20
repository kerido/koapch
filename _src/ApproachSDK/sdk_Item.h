#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////

//! Serves as a base class for logical items
class DECLSPEC_NOVTABLE Item : public IUnknown
{
// Interface members
public:
	STDMETHOD (GetDisplayName) (bool theFullName, TCHAR * theOutName, int * theSize) = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////////


//! Extracts IShellFolder and PIDL information from a Shell item.
MIDL_INTERFACE("CECC84E2-C335-4409-A90B-436305BEB0DF") IApproachShellItem : public IUnknown
{
	//!Represents a Pidl type.
	enum PidlType
	{
		PRIMARY_RELATIVE,     //!< The relative Pidl of the object itself.
		PRIMARY_FULL,         //!< The absolute (relative to the Desktop folder) Pidl of the object itself.
		SECONDARY_RELATIVE,   //!< The relative Pidl of the object target. Only makes sense for folder shortcuts.
		SECONDARY_FULL,       //!< The absolute Pidl of the object target. Only makes sense for folder shortcuts.
		PARENTFOLDER_ABSOLUTE //!< The absolute Pidl of the folder that contains the object.
	};


	STDMETHOD (GetShellItemData) (PidlType theType, IShellFolder ** theOutSF, LPITEMIDLIST * theOutPidl) = 0;
};

// {CECC84E2-C335-4409-A90B-436305BEB0DF}
DFN_GUID(IID_IApproachShellItem,
         0xCECC84E2, 0xC335, 0x4409, 0xA9, 0x0B, 0x43, 0x63, 0x05, 0xBE, 0xB0, 0xDF);

#pragma once

#include "sdk_RuntimeObject.h"

//////////////////////////////////////////////////////////////////////////////////////////////

class Item;

MIDL_INTERFACE("A5A79705-38D8-4617-B278-8D019BB25625") IEnumItems : public IUnknown
{
	STDMETHOD (Next)  (ULONG theNumItems, Item ** theOutEnum, ULONG * theNumExtracted) = 0;
	STDMETHOD (Skip)  (ULONG theNumItems) = 0;
	STDMETHOD (Reset) () = 0;
	STDMETHOD (Clone) (IEnumItems ** theOut) = 0;
};

// {A5A79705-38D8-4617-B278-8D019BB25625}
DFN_GUID(IID_IEnumItems,
         0xA5A79705, 0x38D8, 0x4617, 0xB2, 0x78, 0x8D, 0x01, 0x9B, 0xB2, 0x56, 0x25);


//////////////////////////////////////////////////////////////////////////////////////////////

//! Specifies options for item enumeration.
enum EnumItemsOptions
{
	ENUMOPTIONS_FOLDERS_ON_TOP = 1,    //!< Specifies that folders should precede files.
	ENUMOPTIONS_FOLDERS_ON_BTM = 2,    //!< Specifies that files should precede folders.
	ENUMOPTIONS_SORT_ITEMS     = 4     //!< Specifies that items should be sorted.
};

//////////////////////////////////////////////////////////////////////////////////////////////

//! \brief When implemented, represents an object that can create an Item enumeration object.
//! 
//! \details
//! This interface must be implemented by Item classes that can create lists of
//! child items.
MIDL_INTERFACE("EFC4DACF-FFD3-424E-B76E-86998A26A7F3") IItemProvider : public IUnknown
{
	//! Creates an Item enumeration object.
	//! 
	//! \param [in] theOptions
	//!     Zero or a combination of OrderingOptions flags.
	//! 
	//! \param [out] theOutEnum
	//!     The pointer to the object that will be created. The caller
	//!     must call Release() on the object when it is no longer needed.
	//! 
	//! \return
	//!     S_OK when the object was successfully created;
	//!     otherwise, a COM error value.
	STDMETHOD (EnumItems) (ULONG theOptions, IEnumItems ** theOutEnum) = 0;
};

// {EFC4DACF-FFD3-424E-B76E-86998A26A7F3}
DFN_GUID(IID_IItemProvider,
         0xEFC4DACF, 0xFFD3, 0x424E, 0xB7, 0x6E, 0x86, 0x99, 0x8A, 0x26, 0xA7, 0xF3);


//////////////////////////////////////////////////////////////////////////////////////////////

//! \brief When implemented, represents a predefined list of Items.
MIDL_INTERFACE("3991E0BB-9803-4AA2-A05D-08087CAF2580") IItemList : public IRuntimeObject
{
	//! Creates an Item enumeration object based on the specified binary parameter.
	//! \param [in]  theData      The array of bytes representing the enumeration parameter.
	//! \param [in]  theDataSize  The size of the array contained in \a theData
	//! \param [in]  theOptions   Zero or a combination of OrderingOptions flags.
	//! \param [out] theOutEnum   The pointer to the Item enumeration object to be created.
	//! \return   S_OK if the object was successfully created; otherwise, a COM error value.
	//! \remarks
	//!     The data contained in \a theData must be first parsed by the implementor.
	//!     If the format of the data is not consistent with that expected,
	//!     the function must return E_INVALIDARG.
	//!     The caller must call Release on the Item enumeration object contained in
	//!     \a theOutEnum when it is no longer needed.
	STDMETHOD (EnumItems) (const BYTE * theData, int theDataSize, ULONG theOptions, IEnumItems ** theOutEnum) = 0;
};

DFN_GUID(IID_IItemList, 0x3991E0BB, 0x9803, 0x4AA2, 0xA0, 0x5D, 0x08, 0x08, 0x7C, 0xAF, 0x25, 0x80);

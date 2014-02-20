#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////

//! When implemented by an Item, allows the object to be visually selected. The
//! implementation must decide whether it wants to redraw itself after being highlighted.
MIDL_INTERFACE("6C04F2D4-6F98-42F1-9812-6877E6D48AA2") ISelectable : public IUnknown
{
	//! Specifies the action to perform on an item after it has been selected or deselected.
	enum OnSelectResult
	{
		DEFAULT = 0,       //!< No action must be performed.
		REDRAW = 1         //!< The item must be redrawn.
	};


	//! Identifies the operation that occurs on an item.
	enum OnSelectFlags
	{
		DESELECTED = 0,    //!< Specifies that the item is being deselected.
		SELECTED   = 1,    //!< Specifies that the item is being selected.

		NOACTIVATE = 2     //!< Specifies that no child window activation must take place.
		                   //!  Only makes sense in combination with #SELECTED flag.
		                   //!  The menu sets this flag when the item is selected by keyboard.
	};


	//! Called when the item is being selected or deselected.
	//! 
	//! \param theFlags
	//!     A combination of one or more values from the OnSelectFlags enumeration.
	//!
	//! \return
	//!     If the item returns a success status, the HRESULT CODE must be equal to
	//!     one or more values from the OnSelectResult enumeration. The item can user
	//!     the MAKE_HRESULT macro with the third parameter set to the OnSelectResult value.
	//!     The item can also return a COM error value.
	STDMETHOD (OnSelect) (ULONG theFlags) = 0;
};

// {6C04F2D4-6F98-42F1-9812-6877E6D48AA2}
DFN_GUID(IID_ISelectable,
         0x6C04F2D4, 0x6F98, 0x42F1, 0x98, 0x12, 0x68, 0x77, 0xE6, 0xD4, 0x8A, 0xA2);

#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////

//! When implemented, represents an object that can be launched separately from Approach Menus.
MIDL_INTERFACE("F4727C51-7168-422D-A214-1F385BBCE513") ILaunchable : public IUnknown
{
	//! Specifies launch options.
	enum Options
	{
		RUN_DEFAULT_ACTION = 0, //!< Perform only default action. Used when the item is double-clicked.
		SHOW_CONTEXT_MENU       //!< Display context menu. Used when the item is right-clicked.
	};


	//! Performs object launching such as opening a window, starting a process
	//! or opening a context menu.
	//! 
	//! \param theParent
	//!     The handle of the Approach Menu which is currently being browsed.
	//! 
	//! \param theOptions
	//!     One or more values from the Options enumeration.
	//! 
	//! \return
	//!   - S_OK if the object was launched successfully and menus must be destroyed.
	//!   - S_FALSE if an object was launches successfully, but menus must not be destroyed.
	//!   - COM error value otherwise.
	STDMETHOD (DoLaunch) (HWND theParent, ULONG theOptions) = 0;
};


// {F4727C51-7168-422D-A214-1F385BBCE513}
DFN_GUID(IID_ILaunchable,
         0xF4727C51, 0x7168, 0x422D, 0xA2, 0x14, 0x1F, 0x38, 0x5B, 0xBC, 0xE5, 0x13);

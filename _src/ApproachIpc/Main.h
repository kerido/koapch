#pragma once

#include "Prefs.h"

//////////////////////////////////////////////////////////////////////////////////////////////

//! \brief Contains information about a Shell object from which an Approach menu
//! must be constructed.
//! 
//! Instances of this class are always created in the address space of the process monitored
//! by Titlebar Menus or Folder Menus. The data is then serialized into shared memory
//! and transferred into the address space of KO Approach.
//! 
//! Since instances are created as a result of executing injected code (namely, one of the
//! installed Hook Procedures), it is important to keep the logic as simple as possible.
//! For example, it is undesirable to use exception handling and other advanced C++ features.
class BaseShellClickData
{
public:
	IUnknown * SF;               //!< TODO
	LPITEMIDLIST Pidl;           //!< TODO
	HWND BaseInvokeWindow;       //!< TODO
	bool MustReleaseExplicitly;  //!< True if the object should destroy Pidl and SF in the destructor.

private:
	ULONG myOldVTable;           //!< TODO
	DWORD myOldPidlSize;         //!< TODO

// Constructors, Descructor
public:
	BaseShellClickData(IUnknown * theSF, LPITEMIDLIST thePidl, HWND theBaseWnd, bool theMustReleaseExplicitly)		 throw()
		:	SF(theSF), Pidl(thePidl), BaseInvokeWindow(theBaseWnd), MustReleaseExplicitly(theMustReleaseExplicitly)
	{
		myOldVTable = GetVTable();

		if ( thePidl != 0)
			myOldPidlSize = Pidl->mkid.cb;
		else
			myOldPidlSize = 0;
	}

	~BaseShellClickData() throw()
	{
		if (!MustReleaseExplicitly)
			return;

		if (SF != 0)
			SF->Release();

		if (Pidl != 0)
			CoTaskMemFree(Pidl);
	}

public:
	bool Validate() const throw()
	{
		if (myOldPidlSize != 0)
			return
				( myOldVTable   == GetVTable()   ) &&
				( myOldPidlSize == Pidl->mkid.cb );
		else
			return myOldVTable == GetVTable();
	}

private:
	//HACK
	ULONG GetVTable() const throw()
	{
		return *(ULONG*)(SF);
	}

// New, Delete Operator Overrides
public:

	//! Allocates memory for a class instance without referencing CRT.
	static void * operator new(size_t theSize) throw()
	{
		return HeapAlloc(GetProcessHeap(), 0, theSize);
	}
	
	//! Frees memory occupied by a class instance without referencing CRT.
	static void operator delete(void * thePtr) throw()
	{
		HeapFree(GetProcessHeap(), 0, thePtr);
	}
};

//////////////////////////////////////////////////////////////////////////////////////////////

//! \brief Represents a function that is called when a Context Menu command
//! needs to be executed on a Shell Item.
typedef HWND                 (*GetCtxHandler) ( HWND );


//! \brief Represents a function that is called when an Approach menu needs to be
//! displayed in response to either (1) an Explorer Icon click (or hover) or
//! (2) a CTRL+click on an Explorer window's Title Bar.
//! 
//! The main goal of this function is extracting data identifying
//! either (1) the currently selected item in Windows explorer (for Folder Menus)
//! or (2) the shell folder that is currently being browsed (for Titlebar Menus).
//!
//! \param theShellViewWnd   The handle of the Shell View window (typically of SHELLDLL_DefView class).
//! 
//! \param theListViewWnd    The handle to the implementation of the Shell View window
//!                          (ListView or DirectUIHWND classes).
//! 
//! \param theItemIndex      The index of the currently selected item. If this item is negative, a menu for
//!                          Titlebar Menus feature is being contructed.
typedef BaseShellClickData * (*Activator) (HWND theShellViewWnd, HWND theListViewWnd, int theItemIndex);




//! TODO
typedef int (*ItemFromCoordsTranslator)(HWND theWnd, int theX, int theY);

//////////////////////////////////////////////////////////////////////////////////////////////
// Common routines

void                 SetGlobalPrefs(const PrefsSerz & thePrefs);

BaseShellClickData * Common_Activate_Win2K(HWND, HWND, int);
BaseShellClickData * Common_Activate_WinXP2003VistaWin7(HWND, HWND, int);

HWND                 Common_GetCtxHandlerWndFromShellView_Win2kXp2003(HWND);
HWND                 Common_GetCtxHandlerWndFromShellView_WinVista(HWND);


//////////////////////////////////////////////////////////////////////////////////////////////
// FolderMenus- related routines

void                 FolderMenus_SetReactFlag(LONG theReact, bool theCheck = true);
void                 FolderMenus_DoProcessHookMouse (DWORD theMouseMsg, const MOUSEHOOKSTRUCT * theHookStruct);
void                 FolderMenus_DoProcessHookMouse1(DWORD theRemoveCode, MSG * theHookStruct);
void                 FolderMenus_DoProcessHookGetMsg(const MSG * theHookStruct);


//////////////////////////////////////////////////////////////////////////////////////////////
// TitlebarMenus- related routines

void                 TitlebarMenus_ProcessTitleBarClick(HWND thePrimaryWindow, int theX, int theY);


void MenuActivity_DoProcess_Post(WPARAM wParam, LPARAM lParam);
void MenuActivity_DoProcess_Get(WPARAM wParam, LPARAM lParam);

/*
(c) 2005 Kirill Osipov

MenuManager.h: contains the interface of the MenuManager class.
This class controls the behavior of the menus. It is responsible for showing,
hiding, drawing the menus.
On startup: register classes necessary for menuwindow display
On deletion: unregister them
*/

#pragma once

#include "Prefs.h"

//////////////////////////////////////////////////////////////////////////////////////////////

interface IMenuManagerAgent;
class MenuWindow;
class Theme;
class Item;

//////////////////////////////////////////////////////////////////////////////////////////////

class MenuManager
{
public:
	enum MenuPositioning
	{
		POS_DEFAULT = 0,
		POS_CENTER_IN_RECT = 1
	};

private:
	class MenuManagerAgentImpl;
	class HelperWnd;

	typedef std::list <MenuWindow *>        ActiveMenusList;
	typedef ActiveMenusList::iterator       ActiveMenusIter;
	typedef ActiveMenusList::const_iterator ActiveMenusIterC;

// Fields
public:
	static ActiveMenusList ActiveMenus;   //!< Stores top-level menus.
	static WNDCLASS MenuWndClass;         //!< Window class for all menu windows.
	static HWND ActivateBaseWnd;          //!< Stores a handle to the Shell view window that originated a KO Approach menu operation.
	static HWND HoverMenuWnd;             //!< Stores a handle to the menu under the mouse cursor that will receive mouse wheel events.
	static HelperWnd * HelperWindow;      //!< Monitors menu foreground state.
	static Theme * DefaultTheme;          //!< Stores a pointer to the menu theme.
	static DWORD OperationInProgress;     //!< TODO
	static IMenuManagerAgent * Agent;     //!< Stores a pointer to an object providing menu integration.
	static bool IsDraggingAndDropping;    //!< TODO
	static int FGSetMode;                 //!< Specifies how menus should be brought to foreground:
	                                      //!  1 -- via a simulated mouse click; 2- via thread input.  \sa SetFGWindowMode.


public:
	static MenuWindow * Resolve(HWND);

	static bool IsMenuWindow(HWND);

	static HRESULT CreateMenuWindow(MenuWindow * theObj, HWND * theOutHwnd);

	//! Destroys the parent-to-child menu chain starting from the specified instance.
	static HRESULT DestroyMenuChain(MenuWindow * theStartAt);

	//! Destroys non-floating menus, that is, menus that have not been locked on the screen.
	static void DestroyNonFloatingMenus();

	static bool HasNonFloatingMenus();

	//! Positions the menu on the screen and makes it visible.
	//! \param theConstraintRect  Specifies the alignment rectangle, is in screen coordinates.
	static void AdjustPosition(MenuWindow * theTarget, RECT * theConstraintRect, int theFlags);

	//! Returns a handle to the window which was spied by Approach and initially caused
	//! an Approach Menu to appear.
	//! 
	//! \remarks
	//! Because the Approach process may not be the owner of
	//! this window, the implementation must neither directly nor indirectly call SendMessage
	//! on this window. For example, if a Menu is appearing as a result of a Folder Menus
	//! click, this parameter stores the handle of the Shell View window.
	//!
	//! \return
	//!     A valid window handle or a NULL handle.
	static HWND GetIpcShellViewWnd()             { return ActivateBaseWnd; }


	//! Stores the handle to the window which is being spied by KO Approach.
	//! 
	//! \param [in] theWnd      The window handle being stored.
	static void SetIpcShellViewWnd(HWND theWnd)  { ActivateBaseWnd = theWnd; }


	//! Creates a menu from a given logical item.
	//! 
	//! \param [in] theDataItem
	//!     The logical item for which a menu needs to be created.
	//! 
	/// \param [in] theParent
	//!     The menu that will become the parent for the newly constructed menu.
	//! 
	//! \param [in] theAlignRect
	//!     The rectangle in screen coordinates that MenuManager uses for aligning the new menu.
	//!     MenuManager tries to align a client corner of the window with one of the corners of
	//!     the passed rectangle. If the parameter is null, the cursor position is used for alignment.
	//! 
	/// \param [in] theShow
	//!     Specifies if the menu must be shown immediately after construction.
	//! 
	//! \return
	//!     A pointer to the newly created menu or a null pointer in case of failure<param>
	static MenuWindow * ConstructByDataItem(Item * theDataItem, MenuWindow * theParent = 0, RECT * theAlignRect = 0, bool theShow = true);

	static void SetFGWindowMode(int theMode)           { FGSetMode = theMode; }

	static Theme * GetTheme()                          { return DefaultTheme; }

	static IMenuManagerAgent * GetAgent()              { return Agent; }

	static void SetOperationInProgress(DWORD theProgr) { OperationInProgress = theProgr; }

	static bool IsOperationInProgress()                { return OperationInProgress != 0; }

	static void SetDragDropMode(bool theIsDragDrop)    { IsDraggingAndDropping = theIsDragDrop; }

	static void Init(Theme * theTheme);

	//! Called when the application is exiting to free any resources used.
	static void Dispose();

	static UINT GetNavKey(int theKeyType);


// Implementation Details
private:
	static LRESULT CALLBACK MenuWindowProc(HWND, UINT, WPARAM, LPARAM);

	static LRESULT OnDestroy(HWND, WPARAM, LPARAM);

	static void DefaultMouseTrack(HWND, DWORD);

	static void BringMenuToForeground(MenuWindow * theMenu);

	static void DestroyMenuChainRecursive(MenuWindow * theStartAt);

	static HWND GetTopmostMenu();

	static int GetDirection(const MenuWindow * theWnd);
};


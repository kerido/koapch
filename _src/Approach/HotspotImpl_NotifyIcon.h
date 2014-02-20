#pragma once

#include "sdk_Thread.h"
#include "sdk_ContextMenuExtension.h"

#include "ItemDesignerBase.h"
#include "HotspotNew.h"
#include "ShellFolder.h"
#include "IpcGuids.h"
#include "Application.h"
#include "RootWindow.h"

#include "resource.h"
#include "auto_LocResource.h"


class MenuWindow_DisplayItemList;


// this macro is for performing a transition from 'Approach Items' folder
// to XML-based storage of Approach Items data
//#define USE_SHELLCHANGENOTIFY

//////////////////////////////////////////////////////////////////////////////////////////////

//! Represents the Approach Items program feature
class HotspotImpl_NotifyIcon :
	public Hotspot,
	public CMessageMap,
	public ComEntry3<IContextMenuExtension, IItemDesigner, IItemList>
{
	class ContextMenuExecutor;

// Windows messages
private:
	static const UINT WM_USER_TRAYNOTIFY                      = RootWindow::WM_USER_TRAYNOTIFY;
	static const UINT WM_USER_SHELLNOTIFY_APPROACHITEMS       = RootWindow::WM_USER_FIRST_APCHITEMS + 0;


// Data members
private:
	RootWindow * myRootWindow;                     //!< Stores a pointer to the Approach root window providing Win32 messaging functionality.

	ShellItem * myItem;                            //!< Contains the contents of the Approach Items Folder set.

	//! Stores the path to the Approach Items folder. The path is typically
	//! <em>C:\\Documents and Settings\\[user]\\%Application Data\\KO Approach Items</em>
	//! under operating systems prior to Windows Vista/Windows Server 2003
	//! or
	//! <em>C:\\users\\[user]\\%Application Data\\KO Approach Items</em> on most newer
	//! operating systems.
	TCHAR myFolderPath[MAX_PATH];

	IShellFolder * myDesktopSF;                    //!< Stores a pointer to the desktop folder.
	IShellFolder * myApproachItems;                //!< Stores a pointer to the 'KO Approach Items' folder.

	LPITEMIDLIST myApproachItemsPidl;              //!< Stores an absolute PIDL representing the 'KO Approach Items' folder.

	bool myHasTargetItem;                          //!< True if _Target.lnk exists in the 'KO Approach Items' folder; otherwise, false.

	ULONG myChangeNotifyID_Main;                   //!< Stores the ID of the Shell change-notify event for the 'KO Approach Items' folder.

	UINT myTimerID_ProcessApproachItemsChange;     //!< TODO


// Constructors, destructor
public:
	HotspotImpl_NotifyIcon();
	~HotspotImpl_NotifyIcon();


// Interface
public:
	//! Opens the KO Approach Items directory in Windows Explorer.
	void ShowItems();

	const TCHAR * GetApproachItemsPath(bool theEnsureExists)
	{
		if (theEnsureExists)
			EnsureDirectoryExists();

		return myFolderPath;
	}


// Hotspot members
protected:
	void OnToggle(bool theEnable);
	void GetGuid(GUID & theOut) const { theOut = GUID_Hotspot_ApproachItems; }


// IContextMenuExtension Members
protected:
	STDMETHODIMP QueryExecutor(Item * theItem, ULONG theExt1, ULONG theExt2, IContextMenuExecutor ** theOut);


// IItemList Members
protected:
	STDMETHODIMP EnumItems(const BYTE * theData, int theDataSize, ULONG theOptions, IEnumItems ** theOutEnum);


// IItemDesigner Members
protected:
	STDMETHODIMP GetNumItems (int * theOut);

	STDMETHODIMP GetItemData (int theIndex, MenuSetItemData * theOut);

	STDMETHODIMP GetItemName (const MenuSetItemData * theItem, TCHAR * theOut, int theBufSize);

	STDMETHODIMP EditData    (MenuSetItemData * theItem, HWND theOwner, DWORD theParam1, LPARAM theParam2)
		{ return E_NOTIMPL; }


// IRuntimeObject Members
protected:
	STDMETHODIMP GetTypeGuid(LPGUID theOutGuid);


//WTL Windowing
	BEGIN_MSG_MAP(HotspotImpl_NotifyIcon)
		MESSAGE_HANDLER(WM_TIMER,                                MsgHandler_Timer)
		MESSAGE_HANDLER(WM_USER_TRAYNOTIFY,                      MsgHandler_TrayNotify)

		MESSAGE_HANDLER(WM_USER_SHELLNOTIFY_APPROACHITEMS,       MsgHandler_ShNotifApchItems)
	END_MSG_MAP()



private:
	LRESULT MsgHandler_Timer(UINT theMsg, WPARAM theWParam, LPARAM theLParam, BOOL & theHandled);

	LRESULT MsgHandler_TrayNotify(UINT theMsg, WPARAM theWParam, LPARAM theLParam, BOOL & theHandled);

	LRESULT MsgHandler_ShNotifApchItems(UINT theMsg, WPARAM theWParam, LPARAM theLParam, BOOL & theHandled);


private:

	//! Displays the Approach Items menu.
	void ShowMenu();

	//! Sets #myItem and #myHasTargetItem.
	void UpdateEffectiveItem();

	//! Checks if the 'KO Approach Items' folder exists in the user's 'Application Data' folder.
	void EnsureDirectoryExists();

	//! Called when the feature is enabled.
	void Enable();

	//! Called when the feature is disabled.
	void Disable();
};


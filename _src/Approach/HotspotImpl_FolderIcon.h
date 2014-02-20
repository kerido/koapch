#pragma once

#include "HotspotNew.h"
#include "IpcGuids.h"
#include "RootWindow.h"

//////////////////////////////////////////////////////////////////////////////////////////////

class IpcModule;

//////////////////////////////////////////////////////////////////////////////////////////////

//! \brief Represents the Folder Menus program feature.
//! 
//! This feature monitors clicks on folder icons in Shell View windows
//! (such as Explorer windows and Open/Save dialog boxes).
//! If the click lasts long enough, a menu is displayed that
//! mimics the structure of the folder being clicked on.
//! 
//! This class monitors window hierarchy for presence of Shell View
//! windows. Threads owning such windows are then installed a
//! special Hook Procedure upon. The latter intercepts user actions
//! and notifies the program back in case a menu needs to be shown.
//! 
//! Only one instance of this class exists in the program.
class HotspotImpl_FolderIcon :
	public Hotspot,
	public CMessageMap,
	public IPreferencesEventProcessor,
	public IIpcIoProcessor,
	public IShellHookProcessor
{
	//! A map that stores thread monitoring data. The key is the thread ID; the value is an HHOOK.
	typedef ATL::CSimpleMap<DWORD, HHOOK> SpiedThreadsMap;

	//! A map that stores threads affected by one search session. The key is the thread ID;
	//! the value is always true.
	typedef ATL::CSimpleMap<DWORD, bool>  ThreadMaskMap;

private:
	class NavigationContext
	{
	private:
		HotspotImpl_FolderIcon & myOwner;
		ThreadMaskMap & myThreadMaskMap;
		HWND mySearchStartWnd;

	public:
		NavigationContext(HotspotImpl_FolderIcon & theOwner, ThreadMaskMap & theThreadMaskMap, HWND theSearchStartWnd = 0)
			: myOwner(theOwner), myThreadMaskMap(theThreadMaskMap), mySearchStartWnd(theSearchStartWnd)
		{ }

		//! Called as a result of window hierarchy traversal.
		//! \return   True if the traversal should continue; otherwise, false.
		bool operator () (HWND theIter_Wnd)
		{
			myOwner.EnsureWindowMonitoring(theIter_Wnd, myThreadMaskMap);
			return mySearchStartWnd == 0;
		}
	};

protected:
	ShellItem    * myItem;          //!< TODO
	IpcModule    * myIpcModule;     //!< Pointer to the IPC Module.
	RootWindow   * myRootWindow;    //!< Application main window.
	UINT           myTimerID;       //!< ID of the timer to fully update the spied threads map every 5 seconds.

	bool  myDblClkOpensIcons;       //!< true if Explorer icons are activated with a double-click;
	                                //!  false -- with a single-click.

	SpiedThreadsMap mySpiedThreads; //!< \brief   The map of threads which are currently being monitored.
	                                //!  \details  Monitored threads are those for which a Folder Menus Hook
	                                //!  is installed.


// Constructors, Destructor
public:
	HotspotImpl_FolderIcon();

	virtual ~HotspotImpl_FolderIcon();

// Hotspot Members
protected:

	//! Called when the feature is being enabled/disabled.
	//! \details This function calls #Start or #Stop depending
	//! on the value of \a theEnable parameter.
	//! \param theEnable   True if the feature is being enabled; otherwise, false.
	virtual void OnToggle(bool theEnable);

	//! Gets the GUID of the current program feature.
	//! \param theOut out    The reference of the memory block to receive the value.
	virtual void GetGuid(GUID & theOut) const
		{ theOut = GUID_Hotspot_FolderMenus; }


// IPreferencesEventProcessor Members
protected:

	//! Processes the application preferences event.
	//! 
	//! \param theEvent  The event that occurred on the application settings.
	//! 
	//! \param thePrefs  The present application settings.
	//! 
	//! \details
	//!     With no regard to the actual event, updates the shared memory block that
	//!     stores preferences for all processes whose threads are being monitored.
	void OnPreferencesEvent(PreferencesEvent theEvent, ApplicationSettings & thePrefs);


// IIpcIoProcessor Members
protected:
	void ProcessIpcIo(const void * theMemory);


// IShellHookProcessor Members
protected:

	//! Called by the Shell when a top level window is created or destroyed.
	void ProcessShellHook(WPARAM theWParam, LPARAM theLParam);


// WTL Windowing
protected:
	BEGIN_MSG_MAP(HotspotImpl_FolderIcon)
		MESSAGE_HANDLER(WM_TIMER,         MsgHandler_Timer)
		MESSAGE_HANDLER(WM_SETTINGCHANGE, MsgHandler_SettingChange)
	END_MSG_MAP()


// Message Handlers
private:
	LRESULT MsgHandler_Timer(UINT theMsg, WPARAM theWParam, LPARAM theLParam, BOOL & theHandled);
	LRESULT MsgHandler_SettingChange(UINT theMsg, WPARAM theWParam, LPARAM theLParam, BOOL & theHandled);


// Implementation Details
private:

	//! Called when the feature is being enabled.
	void Start();


	//! Called when the feature is being disabled.
	void Stop();


	//! Requests the icon click mode from the Control Panel.
	//! \return True if the icon click mode was changed.
	bool SetIconClickMode();


	//! Requests the icon click mode from the Control Panel and updates
	//! associated monitoring mechanisms.
	void UpdateIconClickMode();


	//! \brief Scans the window hierarchy for presence of Shell View windows.
	//! The search begins either with a concrete window or the Desktop window.
	//!
	//! \param theParentWnd   The parent window to begin scan with. It this parameter
	//!                       is zero, the entire window hierarchy will be scanned.
	void ReEnumerate(HWND theParentWnd);


	//! \brief Ensures that the thread owning a window is being monitored.
	//!
	//! \remarks This function also helps collect unused threads (i.e., finished
	//! threads that are still stored in the monitored thread map - #mySpiedThreads).
	//!
	//! \param theWnd             The window that has been identified as a Shell View.
	//! \param theMarkedThreads   The map of threads that have been affected by a
	//!                           particular search session. If the thread is to be
	//!                           monitored, an entry is added to this map where thread ID
	//!                           is the key and the value is always true.
	void EnsureWindowMonitoring(HWND theWnd, ThreadMaskMap & theMarkedThreads);


	//! Detaches all Hooks from processes currently being monitored.
	//! \remarks  Calling this function alone does not disable the
	//! program feature itself.
	void RemoveAllMonitoring();


	//! Detaches the Folder Menus Hook from the thread that is no more in use
	//! (e.g. finished or terminated).
	void RemoveUnusedThreadMonitoring(DWORD theThreadId);
};

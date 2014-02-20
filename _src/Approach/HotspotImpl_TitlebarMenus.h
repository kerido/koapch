#pragma once

#include "sdk_ItemProvider.h"

#include "LogicCommon.h"
#include "HotspotNew.h"
#include "RootWindow.h"

//////////////////////////////////////////////////////////////////////////////////////////////

class IpcModule;

//! \brief Represents the Titlebar Menus program feature. Only one instance of this class
//! exists in the program.
//! 
//! This feature monitors CTRL+clicks on the titlebar of Explorer windows.
//! Upon CTRL+click, a menu listing the entire navigable folder hierarchy is displayed.
//!
//! For example, if the user CTRL+clicks the title bar of the window displaying the
//! contents of the C: drive, the menu will look as follows:
//!
//! - Desktop
//! - My Computer
//! - C:\
//!
//! This feature can also be instructed to display the contents in reverse order, i.e.:
//!
//! - C:\
//! - My Computer
//! - Desktop
class HotspotImpl_TitlebarMenus :
	public Hotspot,
	public CMessageMap,
	public IIpcIoProcessor,
	public IShellHookProcessor
{
// Type Definitions
protected:
	typedef ATL::CSimpleMap<DWORD, HHOOK> SpiedThreadsMap;


// Fields
private:
	SpiedThreadsMap mySpiedThreads;    //!< Map of threads already monitored.
	IpcModule     * myIpcModule;       //!< Pointer to the IPC Module.
	RootWindow    * myRootWindow;      //!< Pointer to the application root window.
	UINT            myTimerID_Infreq;  //!< ID of the timer to fully update the spied threads map every 5 seconds.
	UINT            myTimerID_Shell;   //!< ID of the timer to queue a full update of the spied threads map as a result of a Shell Hook event.


// Constructors, Destructor
public:
	HotspotImpl_TitlebarMenus();
	~HotspotImpl_TitlebarMenus();


public:
	//! Called as a result of window hierarchy traversal.
	//! \param theIter_Wnd   The window currently being enumerated.
	//! \return   True if the traversal should continue; otherwise, false.
	bool operator () (HWND theIter_Wnd);


// Hotspot Members
protected:

	//! Called when the feature is being enabled/disabled.
	//! \param theEnable   True if the feature is being enabled; otherwise, false.
	virtual void OnToggle(bool theEnable);
	

	//! TODO
	virtual void GetGuid(GUID & theOut) const;


// IIpcIoProcessor Members
protected:
	void ProcessIpcIo(const void * theMemory);


// IShellHookProcessor Members
protected:
	void ProcessShellHook(WPARAM theWParam, LPARAM theLParam);


// WTL Windowing
protected:
	BEGIN_MSG_MAP(HotspotImpl_TitlebarMenus)
		MESSAGE_HANDLER(WM_TIMER, MsgHandler_Timer)
	END_MSG_MAP()


// Win32 Message Handlers
private:
	LRESULT MsgHandler_Timer(UINT theMsg, WPARAM theWParam, LPARAM theLParam, BOOL & theHandled);


// Implementation Details
private:

	void Start();

	void Stop();

	void ReEnumerate();

	void RemoveAllMonitoring();
};


#pragma once

#include "sdk_GuidComparer.h"
#include "sdk_Thread.h"
#include "sdk_CriticalSection.h"

#include "IpcGuids.h"

struct PrefsSerz;
class InterProcessContextMenuInfo;

//////////////////////////////////////////////////////////////////////////////////////////////

//! Represents a mechanism of interacting with ApproachIpc.dll.
class IpcModule : Thread
{
private:
	struct WindowMessageData
	{
		HWND Window;
		UINT Message;

		WindowMessageData(HWND theWindow = 0, UINT theMessage = 0)
			: Window(theWindow), Message(theMessage)
		{ }

		void Set(HWND theWnd, UINT theMsg)
		{ Window = theWnd; Message = theMsg; }

		void Clear()
		{ Window = 0; Message = 0; }
	};

public:
	typedef void (*SETPREFS) (const PrefsSerz & thePreferences);
	typedef void (*EXECFN)   (DWORD theThreadID, InterProcessContextMenuInfo * theInfo);


// Fields
private:
	HMODULE myModuleHandle;         //!< Handle to the dynamically loaded instance of ApproachIpc.dll.
	HANDLE  myEvent;                //!< Synchronization event.
	HANDLE  myMemMap;               //!< Shared memory handle for reading event data.

	SETPREFS mfSetPrefs;            //!< Address of the function that sets preferences on all
	                                //!  loaded instances of ApproachIpc.dll.

	EXECFN  mfExecuteContextMenu;   //!< Address of the function that executes a Context Menu
	                                //!  command in the Shell thread.

	HOOKPROC mfHkPrc_FM_DblClkMode; //!< Address of the Hook Procedure installed by
	                                //!  Folder Menus to monitor Shell Icon clicks
	                                //!  in Double-Click-To-Open-Icons mode.

	HOOKPROC mfHkPrc_FM_SglClkMode; //!< Address of the Hook Procedure installed by
	                                //!  Folder Menus to monitor Shell Icon clicks
	                                //!  in Single-Click-To-Open-Icons mode.

	HOOKPROC mfHkPrc_TM;            //!< Address of the Hook Procedure installed by
	                                //!  Titlebar Menus to monitor CTRL+clicks
	                                //!  on Explorer window captions.

	HOOKPROC mfHkPrc_Menu_Post;     //!< TODO

	HOOKPROC mfHkPrc_Menu_Get;      //!< TODO

	LONG myTerminated;              //!< 0 if the waiter thread on the Synchronization
	                                //!  event should continue; 1 if the thread should
	                                //!  finish.

	CriticalSection mySync;         //!< Synchronizes access to the IPC event subscriber map.

	WindowMessageData myIpcDt;      //!< Stores data necessary to notify a window of an IPC event.


public:
	IpcModule() throw() :
			myEvent(0), myMemMap(0), myModuleHandle(0), myTerminated(0L)
	{ }

	~IpcModule() throw()
	{
		Terminate();

		if (myEvent != 0)
		{
			SetEvent(myEvent); //fire the event so that any waiter thread is signaled
			CloseHandle(myEvent);
		}

		if (myMemMap != 0)
			CloseHandle(myMemMap);

		if (myModuleHandle != 0)
		{
			ReleaseTraceCode(IPCMODULE_UNLOAD);
			FreeLibrary(myModuleHandle);
		}
	}


public:
	HANDLE GetSharedMemoryHandle() throw()
	{
		EnsureModuleLoaded();
		return myMemMap;
	}

	HINSTANCE GetModuleInstance() throw()
	{
		EnsureModuleLoaded();
		return (HINSTANCE) myModuleHandle;
	}

	HOOKPROC GetHookAddress_TitlebarMenus() throw()
	{
		EnsureModuleLoaded();
		return mfHkPrc_TM;
	}

	HOOKPROC GetHookAddress_FolderMenus_DoubleClickMode() throw()
	{
		EnsureModuleLoaded();
		return mfHkPrc_FM_DblClkMode;
	}

	HOOKPROC GetHookAddress_FolderMenus_SingleClickMode() throw()
	{
		EnsureModuleLoaded();
		return mfHkPrc_FM_SglClkMode;
	}

	HOOKPROC GetHookAddress_Menu_Post() throw()
	{
		EnsureModuleLoaded();
		return mfHkPrc_Menu_Post;
	}

	HOOKPROC GetHookAddress_Menu_Get() throw()
	{
		EnsureModuleLoaded();
		return mfHkPrc_Menu_Get;
	}

	//! \brief Stores data necessary to dispatch IPC events in the main UI thread.
	//! 
	//! \param theWindow   The windowed handle that will receive a known message when an IPC event occurs.
	//! \param theMsg      The message that will be posted to window.
	//!
	//! Message notification always occurs through calling the PostMessage
	//! API function on the window.
	void SetIoHandlerData(HWND theWindow, UINT theMsg)
	{
		EnsureIpcConnection();

		mySync.Enter();

		myIpcDt.Set(theWindow, theMsg);

		mySync.Leave();
	}


	//! Clears IPC dispatch data previously set via a call to #SetIoHandlerData
	//! and stops listening to IPC events.
	void ClearIoHandlerData()
	{
		mySync.Enter();

		myIpcDt.Clear();

		mySync.Leave();

		Terminate();	// stop waiting for the IPC event
	}

	//! Writes preferences into a block of memory shared by all
	//! processes that have ApproachIpc.dll loaded.
	void LLSetPreferences(const PrefsSerz & thePreferences)
	{
		EnsureModuleLoaded();
		mfSetPrefs(thePreferences);
	}

	//! Installs a temporary hook on the specified thread, writes
	//! the data of the Context Menu command to shared memory,
	//! and sends a dummy message to the thread in order for the temporary
	//! hook to be invoked and immediately removed.
	void LLExecuteContextMenu(DWORD theThreadID, InterProcessContextMenuInfo * theInfo)
	{
		EnsureModuleLoaded();
		mfExecuteContextMenu(theThreadID, theInfo);
	}


protected:

	//! Creates a wait loop on the IPC event and, upon receiving the event,
	//! dispatches notifications to subscribed objects.
	virtual DWORD Run()
	{
		Assert(myEvent == 0);

		while (true)
		{
			DWORD aRes = WaitForSingleObject(myEvent, INFINITE);

			if (aRes == WAIT_FAILED)
			{
				Trace("Failed while waiting for IPC event to fire\n");
				return 1;
			}
			else if (aRes == WAIT_OBJECT_0)
			{
				if (myTerminated)
				{
					InterlockedExchange(&myTerminated, 0);	// prepare for the next run
					return 0;
				}
				else
				{
					mySync.Enter();
						PostMessage(myIpcDt.Window, myIpcDt.Message, 0, 0);
					mySync.Leave();

					ResetEvent(myEvent);
				}
			}
		}

		return 0;
	}

	//! Notifies the IPC event wait loop to finish execution.
	virtual void Terminate()
	{
		InterlockedExchange(&myTerminated, 1L);
		SetEvent(myEvent);
	}


private:

	//! Called from other methods to perform actual loading of ApproachIpc.dll.
	void EnsureModuleLoaded()
	{
		if (myModuleHandle == 0)
		{
			myModuleHandle = LoadLibrary( _T("ApproachIpc.dll") );

			if (myModuleHandle != 0)
				ReleaseTraceCode(IPCMODULE_LOAD_SUCCESS);
			else
				ReleaseTraceCode(IPCMODULE_LOAD_FAIL);
		}

		// Set up exported procedure addresses
		mfSetPrefs             = (SETPREFS) GetProcAddress(myModuleHandle, "SetGlobalPrefs");
		mfExecuteContextMenu   = (EXECFN)   GetProcAddress(myModuleHandle, "ExecuteContextMenu");

		mfHkPrc_FM_DblClkMode  = (HOOKPROC) GetProcAddress(myModuleHandle, "Hook_Mouse");
		mfHkPrc_FM_SglClkMode  = (HOOKPROC) GetProcAddress(myModuleHandle, "Hook_GetMsg");
		mfHkPrc_TM             = (HOOKPROC) GetProcAddress(myModuleHandle, "Hook_Title");
		mfHkPrc_Menu_Post      = (HOOKPROC) GetProcAddress(myModuleHandle, "Hook_Menu_Post");
		mfHkPrc_Menu_Get       = (HOOKPROC) GetProcAddress(myModuleHandle, "Hook_Menu_Get");
	}

	//! Obtains addresses of Hook procedures and sets up the IPC interaction mechanism.
	void EnsureIpcConnection()
	{
		EnsureModuleLoaded();

		// Set up shared memory and notification event
		if (myEvent == 0)
			myEvent = CreateEvent
			(
				NULL,
				FALSE,
				FALSE,
				Event_IpcSignal
			);

		if (myMemMap == 0)
			myMemMap = CreateFileMapping
			(
				INVALID_HANDLE_VALUE,
				NULL,
				PAGE_READWRITE,
				0,
				4096,
				File_IpcExchange
			);

		// Since we've opened the IPC channel, we must also launch the waiter thread
		// for monitoring events.
		Thread::Run(this);
	}
};
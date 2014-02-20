#pragma once

#include "sdk_GuidComparer.h"
#include "sdk_Localization.h"

#include "LogicCommon.h"
#include "HotspotNew.h"
#include "IpcRootWindow.h"
#include "UpdateCheck.h"

#include "util_Localization.h"

//////////////////////////////////////////////////////////////////////////////////////////////

class IIpcIoProcessor
{
public:

	//! Processes an event from an IPC module.
	//! \param [in] theMemory     The raw memory associated with the event.
	virtual void ProcessIpcIo(const void * theMemory) = 0;
};


class IShellHookProcessor
{
public:
	virtual void ProcessShellHook(WPARAM theWParam, LPARAM theLParam) = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////////

//! processes messages from the notification icon
//!  Only one instance exists in the program
class RootWindow :
	public CWindowImpl<RootWindow, CWindow, CNullTraits>,
	public IHotspotToggleProcessor,
	public ILocalizationEventProcessor,
	public IPreferencesEventProcessor,
	public IpcRootWindow,
	public IAsyncUpdateCheckResultHandler
{

// Constants
public:
	static const UINT WM_USER_TRAYNOTIFY                 = WM_USER_FIRST +  0;
	static const UINT WM_USER_IPC                        = WM_USER_FIRST +  1;
	static const UINT WM_USER_DISPLAYOPTIONS             = WM_USER_FIRST +  2;
	static const UINT WM_USER_LOADLOCALE                 = WM_USER_FIRST +  3;

	//! Posted to the Root Window when the key is restored in a secondary process
	//! wParam is nonzero if the key was successfully restored. Otherwise, it is zero.
	static const UINT WM_USER_KEYRESTORERESULT           = WM_USER_FIRST +  4;

	//! Posted by the Root Window itself when an update check result is available.
	static const UINT WM_USER_HANDLEUPDATE               = WM_USER_FIRST +  5;

	static const UINT WM_USER_FIRST_APCHITEMS            = WM_USER_FIRST + 64;

	static const UINT TimerID_TrayLeftButtonClick        = 1;
	static const UINT TimerID_LocaleChangeReact          = 2;
	static const UINT TimerID_UpdateCheckBackground      = 3;
	static const UINT TimerID_Max                        = 0x0100;


// Nested Classes
protected:

	struct MessageMapData
	{
		CMessageMap * Map;
		DWORD         MapID;

		MessageMapData(CMessageMap * theMap, DWORD theMapID = (DWORD)-1)
			: Map(theMap), MapID(theMapID)
		{ }

		bool operator == (const MessageMapData & theObj) const
			{ return Map == theObj.Map; }

		bool operator != (const MessageMapData & theObj) const
			{ return Map != theObj.Map; }
	};

	class MessageMapList :
		public std::list<MessageMapData>,
		public CMessageMap
	{
	public:
		BOOL ProcessWindowMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& lResult, DWORD dwMsgMapID)
		{
			// broadcast the message to all handlers, with no regard to BOOL & bHandled
			bool aIndividualMessage = (uMsg != WM_SETTINGCHANGE && uMsg != WM_THEMECHANGED);

			for (reverse_iterator aIt = rbegin(); aIt != rend(); aIt++)
			{
				MessageMapData & aDt = *aIt;
				DWORD aMapID = aDt.MapID == ((DWORD)-1) ? dwMsgMapID : aDt.MapID;

				BOOL aRet = aDt.Map->ProcessWindowMessage(hWnd, uMsg, wParam, lParam, lResult, aMapID);

				if (aIndividualMessage && aRet == TRUE)
					return aRet;
			}

			return !aIndividualMessage;
		}
	};


// Type Definitions
private:
	typedef std::map<GUID, IIpcIoProcessor *, GuidComparer> IpcIoProcMap;
	typedef IpcIoProcMap::iterator                          IpcIoProcIter;
	typedef IpcIoProcMap::const_iterator                    IpcIoProcIterC;
	typedef IpcIoProcMap::value_type                        IpcIoProcPair;

	typedef std::list<IShellHookProcessor *>                ShellHookProcList;
	typedef ShellHookProcList::iterator                     ShellHookProcIter;
	typedef ShellHookProcList::const_iterator               ShellHookProcIterC;

	typedef MessageMapList::iterator                        MessageMapIter;
	typedef MessageMapList::const_iterator                  MessageMapIterC;
	typedef MessageMapList::reverse_iterator                MessageMapRiter;
	typedef MessageMapList::const_reverse_iterator          MessageMapRiterC;


// Fields
private:

	const UINT WM_SHELLHOOK;                 //!< A dynamically obtained Windows Message ID
	                                         //!  that is sent when a Shell Hook event occurs.

	const UINT WM_TASKBARCREATED;            //!< Used to create a tray icon if for some reason Explorer will re-launch
	                                         //!  while Approach is working. Explorer sends this message globally to all
	                                         //!  top-level windows in order to notify them of its presence.

	IpcIoProcMap        myProcs;             //!< Stores objects that subscribe to IPC events.
	ShellHookProcList   myShellHooks;        //!< Stores objects that respond to Shell Hook events.
	MessageMapList      myChildMaps;         //!< Stores objects that override the object's message handling.

	UINT myCurTimerID;                       //!< Stores the ID of the timer which can be used in subclasses.

	UINT myCurResendMsg;                     //!< Stores the ID of the notification area message that is sent
	                                         //   to the child message maps when the message is not processed by the object.
	WPARAM myCurResendWParam;                //!< Stores the WPARAM of notification area message.
	LPARAM myCurResendLParam;                //!< Stores the LPARAM notification area message.

	NOTIFYICONDATA myData;                   //!< Stores data necessary to construct (and remove) the Approach notification icon.
	HMENU     myPopupMenu;                   //!< Stores the handle to the popup menu that is displayed when
	                                         //!  the user right-clicks the Approach notification icon.

	POINT myCursorPos;                       //!< Stores cursor position (in screen coordinates) when processing a notification area message.

	bool myAlreadyClicked;                   //!< TODO
	bool myShowingOptionsDlg;                //!< True if the Options window is currently visible; otherwise, false.
	bool myIsProcessingLocaleChange;         //!< True if the object is currently processing a file change notification.

	LocalizationManagerPtr myLocMan;

	UpdateCheckPackage myUpdateCheckPackage; //<! TODO
	UINT_PTR myUpdateCheckTimerID;           //!< TODO


	// Constructors, Destructor
public:
	RootWindow();                           //!< Creates a default object instance.
	~RootWindow();                          //!< Destroys an object instance.


// Interface
public:
	UINT GetAvailableTimerID() { return myCurTimerID++; }

	void AddIpcProcessor(REFGUID theGuid, IIpcIoProcessor * theProcessor);

	void RemoveIpcProcessor(REFGUID theGuid);

	void AddShellHookProcessor(IShellHookProcessor * theProcessor);

	void RemoveShellHookProcessor(IShellHookProcessor * theProcessor);

	void AddMessageMap(CMessageMap * theMsgMap, DWORD theMsgMapID = -1);

	void RemoveMessageMap(CMessageMap * theMsgMap);

	void ProcessStartupOptions();

	void GetCursorPositionForMenu(POINT * theOutPoint);

	void SafeToggleHotspot(const GUID & theGuid);

	//TODO: this will be unneeded once we order global data in order
	void WatchAfterHotspots();

	//TODO: this will be unneeded once we order global data in order
	void StopWatchingAfterHotspots();

public:

	static void NotifyAnotherInstance();

	static void SecondaryProcess(LPCWSTR theCmdLineArg, bool theElevate);


// WTL Windowing
public:
	DECLARE_WND_CLASS( _T("ApproachRootWnd") )

	BEGIN_MSG_MAP(RootWindow)
		MESSAGE_HANDLER(WM_CREATE,                     MsgHandler_Create)
		MESSAGE_HANDLER(WM_TIMER,                      MsgHandler_Timer)

		MESSAGE_HANDLER(WM_USER_IPC,                   MsgHandler_UserIpc)
		MESSAGE_HANDLER(WM_USER_TRAYNOTIFY,            MsgHandler_UserTrayNotify)
		MESSAGE_HANDLER(WM_USER_DISPLAYOPTIONS,        MsgHandler_UserDisplayOptions)
		MESSAGE_HANDLER(WM_USER_QUIT,                  MsgHandler_UserQuit)
		MESSAGE_HANDLER(WM_USER_LOADLOCALE,            MsgHandler_UserLoadLocale)
		MESSAGE_HANDLER(WM_USER_HANDLEUPDATE,          MsgHandler_UserHandleUpdate)

		MESSAGE_HANDLER(WM_SHELLHOOK,                  MsgHandler_ShellHook)
		MESSAGE_HANDLER(WM_TASKBARCREATED,             MsgHandler_TaskbarCreated)

		CHAIN_MSG_MAP_ALT_MEMBER(myChildMaps, 0)
	END_MSG_MAP()



// IHotspotToggleProcessor members
protected:
	void OnHotspotToggle(const GUID & theIndex, bool theEnable, bool theResult);


// ILocalizationEventProcessor members
protected:
	void OnLocalizationChanged(ILocalization *, const TCHAR *, int)
		{ BindToLocalization(); }


// IAsyncUpdateCheckResultHandler
protected:
	void HandleUpdateCheckResult(UpdateCheckTask * theTask);


// IPreferencesEventProcessor Members
protected:
	void OnPreferencesEvent(PreferencesEvent, class ApplicationSettings &)
		{ BindToApplicationSettings(); }


// Message Handlers
private:
	LRESULT MsgHandler_Create(UINT theMsg, WPARAM theWParam, LPARAM theLParam, BOOL & theHandled);

	LRESULT MsgHandler_Timer(UINT theMsg, WPARAM theWParam, LPARAM theLParam, BOOL & theHandled);

	LRESULT MsgHandler_UserIpc(UINT theMsg, WPARAM theWParam, LPARAM theLParam, BOOL & theHandled);

	LRESULT MsgHandler_UserTrayNotify(UINT theMsg, WPARAM theWParam, LPARAM theLParam, BOOL & theHandled);

	LRESULT MsgHandler_UserDisplayOptions(UINT theMsg, WPARAM theWParam, LPARAM theLParam, BOOL & theHandled);

	LRESULT MsgHandler_UserQuit(UINT theMsg, WPARAM theWParam, LPARAM theLParam, BOOL & theHandled);

	LRESULT MsgHandler_UserLoadLocale(UINT theMsg, WPARAM theWParam, LPARAM theLParam, BOOL & theHandled);

	LRESULT MsgHandler_UserHandleUpdate(UINT theMsg, WPARAM theWParam, LPARAM theLParam, BOOL & theHandled);

	LRESULT MsgHandler_ShellHook(UINT theMsg, WPARAM theWParam, LPARAM theLParam, BOOL & theHandled);

	LRESULT MsgHandler_TaskbarCreated(UINT theMsg, WPARAM theWParam, LPARAM theLParam, BOOL & theHandled);


private:
	void BindToLocalization();

	void BindToApplicationSettings();

	void ShowOptionsDlg();

	void HandlePopupMenu();

	void CheckForUpdatesInBackground();

	void SetUpdateCheckTimer();

	void KillUpdateCheckTimer();

	static void Restart();

	static void KeyRestore();
};

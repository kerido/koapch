#include "stdafx.h"

#include "RootWindow.h"
#include "Application.h"
#include "IpcModule.h"
#include "DlgOptions.h"
#include "HtmlHelpProvider.h"
#include "FilePathProvider.h"
#include "MenuManager.h"

#include "util_Warning.h"
#include "util_Localization.h"

#include "resource.h"
#include "auto_LocResource.h"

//////////////////////////////////////////////////////////////////////////////////////////////

RootWindow::RootWindow() :
	myCurTimerID(TimerID_Max),
	myAlreadyClicked(false),
	myShowingOptionsDlg(false),
	myIsProcessingLocaleChange(false),
	WM_SHELLHOOK( RegisterWindowMessage( _T("SHELLHOOK") ) ),
	WM_TASKBARCREATED( RegisterWindowMessage( _T("TaskbarCreated") ) ),
	myUpdateCheckTimerID(0)
{
	Application & aApp = Application::Instance();

	myCursorPos.x = myCursorPos.y = -1;

	//create menu
	myPopupMenu = LoadMenu( (HMODULE) _Module.GetModuleInstance(), MAKEINTRESOURCE(IDR_TRAYMENU) );
	myPopupMenu = ::GetSubMenu(myPopupMenu, 0);

	SetMenuDefaultItem(myPopupMenu, IDP_MAIN_OPTIONS, FALSE);

	// initialize notification icon data
	SecureZeroMemory(&myData, sizeof NOTIFYICONDATA);
	myData.cbSize = sizeof NOTIFYICONDATA;
	myData.hIcon = aApp.GetIconManager().GetApplicationIconSmall();

	lstrcpy(myData.szTip, _T("KO Approach") );
	myData.uID = 1;
	myData.uCallbackMessage = WM_USER_TRAYNOTIFY;
	myData.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;

	// notification data will be used for creating a tray icon
	// upon processing the WM_CREATE message

	// because we display a pop-up menu that has localizable strings
	// we subscribe to localization events 
	aApp.RegisterLocalizationEventProcessor(this);

	aApp.AddPreferencesEventProcessor(this);

	Create(NULL);
}

//////////////////////////////////////////////////////////////////////////////////////////////

RootWindow::~RootWindow()
{
	Application & aApp = Application::Instance();

	aApp.RemovePreferencesEventProcessor(this);
	aApp.UnregisterLocalizationEventProcessor(this);

	myData.uFlags = NIF_MESSAGE;
	Shell_NotifyIcon(NIM_DELETE, &myData);

	ATLASSERT( myProcs.empty() );
	ATLASSERT( myShellHooks.empty() );
	ATLASSERT( myChildMaps.empty() );

	DestroyWindow();
}

//////////////////////////////////////////////////////////////////////////////////////////////

void RootWindow::AddIpcProcessor(REFGUID theGuid, IIpcIoProcessor * theProcessor)
{
	bool aNeedToRegister = myProcs.empty();

	myProcs[theGuid] = theProcessor;

	if (aNeedToRegister)
		Application::Instance().GetIpcModule()->SetIoHandlerData(*this, WM_USER_IPC);
}

//////////////////////////////////////////////////////////////////////////////////////////////

void RootWindow::RemoveIpcProcessor(REFGUID theGuid)
{
	myProcs.erase(theGuid);

	if (myProcs.empty() )
		Application::Instance().GetIpcModule()->ClearIoHandlerData();
}

//////////////////////////////////////////////////////////////////////////////////////////////

void RootWindow::AddShellHookProcessor( IShellHookProcessor * theProcessor )
{
	bool aMustRegister = myShellHooks.empty();

	myShellHooks.push_back(theProcessor);

	if (aMustRegister)
		RegisterShellHookWindow(*this);
}

//////////////////////////////////////////////////////////////////////////////////////////////

void RootWindow::RemoveShellHookProcessor(IShellHookProcessor * theProcessor)
{
	ShellHookProcIterC aIt = std::find(myShellHooks.begin(), myShellHooks.end(), theProcessor);

	if ( aIt == myShellHooks.end() )
		return;

	myShellHooks.erase(aIt);

	if ( myShellHooks.empty() )
		DeregisterShellHookWindow(*this);
}

//////////////////////////////////////////////////////////////////////////////////////////////

void RootWindow::GetCursorPositionForMenu(POINT * theOutPoint)
{
	*theOutPoint = myCursorPos;
}

//////////////////////////////////////////////////////////////////////////////////////////////

void RootWindow::AddMessageMap(CMessageMap * theMsgMap, DWORD theMsgMapID/*= -1*/)
{
	MessageMapData aDt(theMsgMap, theMsgMapID);
	myChildMaps.push_back(aDt);
}

//////////////////////////////////////////////////////////////////////////////////////////////

void RootWindow::RemoveMessageMap(CMessageMap * theMsgMap)
{
	MessageMapIterC aIt = std::find(myChildMaps.begin(), myChildMaps.end(), theMsgMap);

	if ( aIt != myChildMaps.end() )
		myChildMaps.erase(aIt);
}

//////////////////////////////////////////////////////////////////////////////////////////////

void RootWindow::ProcessStartupOptions()
{
	ApplicationSettings * aS = Application::Instance().Prefs();

	bool aNeedToShowOptions = !aS->GetAlreadyLaunched();

	if (aNeedToShowOptions)
	{
		//FLAW: no changeable entity events will be fired
		aS->SetAlreadyLaunched(true);
		aS->Save();
	}

	if ( !aNeedToShowOptions )
	{
		TCHAR * aCmdLine = GetCommandLine();

		if ( _tcsstr(aCmdLine, _T("-options") ) != 0)
			aNeedToShowOptions = true;
	}

	if (aNeedToShowOptions)
		ShowOptionsDlg();
}

//////////////////////////////////////////////////////////////////////////////////////////////

void RootWindow::NotifyAnotherInstance()
{
	const TCHAR * aCmdLine = GetCommandLine();

	if ( _tcsstr(aCmdLine, _T("-restart") ) != 0 )
		Restart();

	else if ( _tcsstr(aCmdLine, _T("-quit") ) != 0 )
		Quit();

	else if ( _tcsstr(aCmdLine, _T("-keyrestore") ) != 0 )
		KeyRestore();

	else
	{
		HWND aWnd = FindSingleInstance();

		if (aWnd != 0)
			::PostMessage(aWnd, WM_USER_DISPLAYOPTIONS, 0, 0);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////

void RootWindow::SecondaryProcess(LPCWSTR theCmdLineArg, bool theElevate)
{
	TCHAR aFileName[1000];
	GetModuleFileName(NULL, aFileName, 1000-1);

	LPCWSTR aVerb = theElevate ? L"runas" : L"open";
	ShellExecute(NULL, aVerb, aFileName, theCmdLineArg, NULL, SW_SHOW);
}

//////////////////////////////////////////////////////////////////////////////////////////////

void RootWindow::SafeToggleHotspot(const GUID & theGuid)
{
	HotspotManager * aMan = Application::Instance().GetHotspotManager();

	ATLASSERT(aMan != 0);

	if ( !aMan->ToggleHotspot(theGuid) )
		WarningMessageUtility::Error_FeatureLocked(NULL, myLocMan);
}

//////////////////////////////////////////////////////////////////////////////////////////////

void RootWindow::WatchAfterHotspots()
{
	Application::Instance().GetHotspotManager()->RegisterProcessor(this);
}

//////////////////////////////////////////////////////////////////////////////////////////////

void RootWindow::StopWatchingAfterHotspots()
{
	Application::Instance().GetHotspotManager()->UnregisterProcessor(this);
}

//////////////////////////////////////////////////////////////////////////////////////////////

void RootWindow::OnHotspotToggle(const GUID & theGuid, bool theEnable, bool theResult)
{
	UINT aFlags = theEnable ^ !theResult ? MF_BYCOMMAND|MF_CHECKED : MF_BYCOMMAND|MF_UNCHECKED;

	if      (theGuid == GUID_Hotspot_FolderMenus)
		CheckMenuItem(myPopupMenu, IDP_MAIN_ENBL_FLDMNUS, aFlags);

	else if (theGuid == GUID_Hotspot_ApproachItems)
		CheckMenuItem(myPopupMenu, IDP_MAIN_ENBL_APCHITMS, aFlags);

	else if (theGuid == GUID_Hotspot_TitlebarMenus)
		CheckMenuItem(myPopupMenu, IDP_MAIN_ENBL_TTLBARMNUS, aFlags);
}

//////////////////////////////////////////////////////////////////////////////////////////////

void RootWindow::HandleUpdateCheckResult(UpdateCheckTask * theTask)
{
	PostMessage(WM_USER_HANDLEUPDATE);
}

//////////////////////////////////////////////////////////////////////////////////////////////

LRESULT RootWindow::MsgHandler_Create( UINT theMsg, WPARAM theWParam, LPARAM theLParam, BOOL & theHandled )
{
	SetWindowText( GetWindowCaption() );

	SetForegroundWindow(m_hWnd);
	PostMessage(WM_NULL, 0, 0);

	myData.hWnd = m_hWnd;
	Shell_NotifyIcon(NIM_ADD, &myData);

	BindToApplicationSettings();

	return 0L;
}

//////////////////////////////////////////////////////////////////////////////////////////////

LRESULT RootWindow::MsgHandler_Timer(UINT theMsg, WPARAM theWParam, LPARAM theLParam, BOOL & theHandled)
{
	if (theWParam == TimerID_TrayLeftButtonClick)
	{
		myAlreadyClicked = false;
		KillTimer(TimerID_TrayLeftButtonClick);

		LRESULT aRes = 0;
		myChildMaps.ProcessWindowMessage(m_hWnd, myCurResendMsg, myCurResendWParam, myCurResendLParam, aRes, 0);
	}

#ifdef _USE_LOCALIZATION_MONITORING
	else if (theWParam == TimerID_LocaleChangeReact)
	{
		KillTimer(TimerID_LocaleChangeReact);

		Application::Instance().GetLocalizationManager()->ProcessChangeEvent();
		myIsProcessingLocaleChange = false;
	}
#endif

	else if (theWParam == TimerID_UpdateCheckBackground)
	{
		CheckForUpdatesInBackground();
	}

	else
		theHandled = FALSE;

	return 0L;
}

//////////////////////////////////////////////////////////////////////////////////////////////

LRESULT RootWindow::MsgHandler_UserIpc(UINT theMsg, WPARAM theWParam, LPARAM theLParam, BOOL & theHandled)
{
	HANDLE aH = Application::Instance().GetIpcModule()->GetSharedMemoryHandle();

	char * aBuf = (char *)MapViewOfFile(aH, FILE_MAP_ALL_ACCESS, 0, 0, 4096);

	GUID aG;
	CopyMemory(&aG, aBuf, sizeof GUID);

	IpcIoProcIter aIt = myProcs.find(aG);

	if (aIt != myProcs.end() )
	{
		IIpcIoProcessor * aPr = aIt->second;
		aPr->ProcessIpcIo(aBuf + sizeof GUID);
	}

	UnmapViewOfFile(aBuf);	//equivalent to reader destruction

	return 0L;
}

//////////////////////////////////////////////////////////////////////////////////////////////

LRESULT RootWindow::MsgHandler_UserTrayNotify( UINT theMsg, WPARAM theWParam, LPARAM theLParam, BOOL & theHandled )
{
	switch (theLParam)
	{
	case WM_RBUTTONUP:
		//TODO: check modifier keys
		HandlePopupMenu();
		break;


	case WM_LBUTTONDBLCLK:
		GetCursorPos(&myCursorPos);

		myAlreadyClicked = false;

		KillTimer(TimerID_TrayLeftButtonClick);
		ShowOptionsDlg();
		break;


	case WM_LBUTTONDOWN:
		GetCursorPos(&myCursorPos);

		if (myAlreadyClicked)
		{
			myAlreadyClicked = false;

			KillTimer(TimerID_TrayLeftButtonClick);
			ShowOptionsDlg();
		}
		else
		{
			myCurResendMsg = theMsg;
			myCurResendWParam = theWParam;
			myCurResendLParam = theLParam;

			myAlreadyClicked = true;
			SetTimer(TimerID_TrayLeftButtonClick, GetDoubleClickTime(), NULL);
		}

		break;

	default:
		break;
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////

LRESULT RootWindow::MsgHandler_UserDisplayOptions( UINT theMsg, WPARAM theWParam, LPARAM theLParam, BOOL & theHandled )
{
	SetForegroundWindow(m_hWnd);
	ShowOptionsDlg();
	return 0L;
}

//////////////////////////////////////////////////////////////////////////////////////////////

LRESULT RootWindow::MsgHandler_UserQuit( UINT theMsg, WPARAM theWParam, LPARAM theLParam, BOOL & theHandled )
{
	PostQuitMessage(0);
	return 0L;
}

//////////////////////////////////////////////////////////////////////////////////////////////

LRESULT RootWindow::MsgHandler_UserLoadLocale( UINT theMsg, WPARAM theWParam, LPARAM theLParam, BOOL & theHandled )
{
#ifdef _USE_LOCALIZATION_MONITORING

	if (!myIsProcessingLocaleChange)
	{
		myIsProcessingLocaleChange = true;
		SetTimer(TimerID_LocaleChangeReact, 500, NULL);
	}

#endif

	return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////

LRESULT RootWindow::MsgHandler_UserHandleUpdate(UINT, WPARAM, LPARAM, BOOL &)
{
	if ( myShowingOptionsDlg || MenuManager::HasNonFloatingMenus() )
		return 0L;

	UpdateCheckTask::SaveUpdateCheckResult();

	if ( myUpdateCheckPackage.GetData().HasUpdates() )
	{
		// Prevent multiple dialogs from being displayed
		KillUpdateCheckTimer();

		INT_PTR aAnswer = LocalizationUtility::LocalizedMessageBox(
			m_hWnd, myLocMan, KEYOF(IDS_O_MNTC_MSG_UPDYES), 0, MB_YESNO|MB_ICONQUESTION);

		if (aAnswer == IDYES)
			ShellExecute(m_hWnd, L"open", myUpdateCheckPackage.GetData().Url(), 0, 0, SW_SHOW);

		// Instead of simply enabling the update check timer, consult the preferences
		// as they might have been changed when the message box was active.
		BindToApplicationSettings();
	}

	return 0L;
}

//////////////////////////////////////////////////////////////////////////////////////////////

LRESULT RootWindow::MsgHandler_ShellHook(UINT theMsg, WPARAM theWParam, LPARAM theLParam, BOOL & theHandled)
{
#if _DEBUG

	DWORD aCurThread = GetCurrentThreadId();
	DWORD aMyThread = GetWindowThreadProcessId(m_hWnd, NULL);

	ATLASSERT(aCurThread == aMyThread);

#endif

	for (ShellHookProcIter aIt = myShellHooks.begin(); aIt != myShellHooks.end(); aIt++)
		(*aIt)->ProcessShellHook(theWParam, theLParam);

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////////

LRESULT RootWindow::MsgHandler_TaskbarCreated( UINT theMsg, WPARAM theWParam, LPARAM theLParam, BOOL & theHandled )
{
	SetForegroundWindow(m_hWnd);
	PostMessage(WM_NULL, 0, 0);
	Shell_NotifyIcon(NIM_ADD, &myData);

	return 0L;
}

//////////////////////////////////////////////////////////////////////////////////////////////

void RootWindow::BindToLocalization()
{
	LocalizationUtility::SetWindowDirection(m_hWnd, myLocMan);

	LocalizationUtility::LocPair aStringArr[] = 
	{
		LOCENTRY_S(IDP_MAIN_ENBL_FLDMNUS),
		LOCENTRY_S(IDP_MAIN_ENBL_APCHITMS),
		LOCENTRY_S(IDP_MAIN_ENBL_TTLBARMNUS),
		LOCENTRY_S(IDP_MAIN_OPTIONS),
		LOCENTRY_S(IDP_MAIN_EXIT),

		{ IDP_MAIN_HELP, KEYOF(IDS_GLBL_CMD_HELP) }
	};

	const int StringArrLength = sizeof aStringArr / sizeof aStringArr[0];

	LocalizationUtility::LocalizeMenu(myPopupMenu, myLocMan, aStringArr, StringArrLength);
}

//////////////////////////////////////////////////////////////////////////////////////////////

void RootWindow::BindToApplicationSettings()
{
	const ApplicationSettings * aS = Application::InstanceC().Prefs();

	if (myUpdateCheckTimerID != 0)
	{
		if ( !aS->GetEnablePeriodicUpdateChecks() )
			KillUpdateCheckTimer();
	}
	else
	{
		if ( aS->GetEnablePeriodicUpdateChecks() )
			SetUpdateCheckTimer();
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////

void RootWindow::ShowOptionsDlg()
{
	if (myShowingOptionsDlg)
		return;

	myShowingOptionsDlg = true;
	EnableMenuItem(myPopupMenu, IDP_MAIN_OPTIONS, MF_BYCOMMAND|MF_GRAYED);

	ComInstance<DlgOptionsPropPages> aOptions;
	aOptions.DoModal(NULL);

	EnableMenuItem(myPopupMenu, IDP_MAIN_OPTIONS, MF_BYCOMMAND|MF_ENABLED);
	myShowingOptionsDlg = false;
}

//////////////////////////////////////////////////////////////////////////////////////////////

void RootWindow::HandlePopupMenu()
{
	POINT aPt;
	GetCursorPos(&aPt);

	SetForegroundWindow(m_hWnd);


	int aRetCmd = TrackPopupMenu
		(
		myPopupMenu,
		TPM_NONOTIFY|TPM_RETURNCMD|TPM_RIGHTBUTTON,
		aPt.x,
		aPt.y,
		0,
		m_hWnd,
		NULL
		);

	::PostMessage(m_hWnd, WM_NULL, 0, 0);

	switch( aRetCmd )
	{
	case IDP_MAIN_ENBL_FLDMNUS:
		SafeToggleHotspot(GUID_Hotspot_FolderMenus);
		break;

	case IDP_MAIN_ENBL_APCHITMS:
		SafeToggleHotspot(GUID_Hotspot_ApproachItems);
		break;

	case IDP_MAIN_ENBL_TTLBARMNUS:
		SafeToggleHotspot(GUID_Hotspot_TitlebarMenus);
		break;

	case IDP_MAIN_HELP:
		{
			const HtmlHelpProvider * aHP = Application::InstanceC().GetHtmlHelpProvider();

			if ( aHP->IsHelpEnabled() )
				aHP->DisplayHelp(NULL);
		}
		break;

	case IDP_MAIN_OPTIONS:
		ShowOptionsDlg();
		break;

	case IDP_MAIN_EXIT:
		PostQuitMessage(0);
		break;

	default:
		break;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////

void RootWindow::CheckForUpdatesInBackground()
{
	if ( !UpdateCheckTask::ShouldPerformUpdateCheck() )
		return;

	myUpdateCheckPackage.HandleUpdates(this);
}

//////////////////////////////////////////////////////////////////////////////////////////////

void RootWindow::SetUpdateCheckTimer()
{
	Trace("RootWindow -- Setting update check timer\n");

	const static UINT MillisecondsInHour = 1000 * 3600;
	myUpdateCheckTimerID = SetTimer(TimerID_UpdateCheckBackground, MillisecondsInHour, NULL);
}

//////////////////////////////////////////////////////////////////////////////////////////////

void RootWindow::KillUpdateCheckTimer()
{
	Trace("RootWindow -- Killing update check timer\n");

	KillTimer(myUpdateCheckTimerID);
	myUpdateCheckTimerID = 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////

void RootWindow::Restart()
{
	if ( !Quit() )
		return;

	SecondaryProcess(NULL, false);
}

//////////////////////////////////////////////////////////////////////////////////////////////

void RootWindow::KeyRestore()
{
	HWND aMyHandle = FindSingleInstance();

	FilePathProvider aPrv( _Module.GetModuleInstance() );

	TCHAR aPath_RealKey[1000];
	int aSize_RealKey = 1000;

	HRESULT aRes = aPrv.GetPath(FilePathProvider::File_Key, aPath_RealKey, &aSize_RealKey);

	TCHAR aPath_TempKey[1000];
	int aSize_TempKey = 1000;

	aRes |= aPrv.GetPath(FilePathProvider::File_TempKey, aPath_TempKey, &aSize_TempKey);

	BOOL aCopiedSuccessfully = FALSE;

	if ( SUCCEEDED(aRes) )
		aCopiedSuccessfully = CopyFile(aPath_TempKey, aPath_RealKey, FALSE);

	::PostMessage(aMyHandle, WM_USER_KEYRESTORERESULT, (WPARAM) aCopiedSuccessfully, 0);
}

#include "stdafx.h"

#include "sdk_WindowNavigator.h"
#include "sdk_MemoryWriter.h"

#include "HotspotImpl_FolderIcon.h"
#include "MenuManager.h"
#include "Framework.h"
#include "ShellItemFactory.h"
#include "Application.h"
#include "IpcModule.h"

#include "UtilPidlFunc.h"
#include "UtilShellFunc.h"

//////////////////////////////////////////////////////////////////////////////////////////////

/*
Explorer hierarchy under Windows 7:
	CabinetWClass
		ShellTabWindowClass
			DUIViewWndClassName
				DirectUIHWND
					CtrlNotifySink
						SHELLDLL_DefView
							DirectUIHWND
*/

//////////////////////////////////////////////////////////////////////////////////////////////

extern const TCHAR FolderMenus_SpiedWindowClassName[] = _T("SHELLDLL_DefView");

//////////////////////////////////////////////////////////////////////////////////////////////

HotspotImpl_FolderIcon::HotspotImpl_FolderIcon() : myItem(0)
{
	Application & aApp = Application::Instance();

	myIpcModule = aApp.GetIpcModule();
	myRootWindow = aApp.GetRootWindowInstance();

	myTimerID = myRootWindow->GetAvailableTimerID();

	SetIconClickMode();
}

//////////////////////////////////////////////////////////////////////////////////////////////

HotspotImpl_FolderIcon::~HotspotImpl_FolderIcon()
{
	if (myEnabled)
		Stop();

	if (myItem != 0)
		delete myItem;
}

//////////////////////////////////////////////////////////////////////////////////////////////

void HotspotImpl_FolderIcon::OnToggle( bool theEnable )
{
	Hotspot::OnToggle(theEnable);

	if (theEnable)  Start();
	else            Stop();
}

//////////////////////////////////////////////////////////////////////////////////////////////

void HotspotImpl_FolderIcon::OnPreferencesEvent(PreferencesEvent theEvent, ApplicationSettings & thePrefs)
{
	myIpcModule->LLSetPreferences(thePrefs);
}

//////////////////////////////////////////////////////////////////////////////////////////////

void HotspotImpl_FolderIcon::ProcessIpcIo(const void * theMemory)
{
	if (MenuManager::HasNonFloatingMenus() )
		return;

	// Obtain the cursor position ass soon as possible
	RECT aRct;
	GetCursorPos( (POINT *) &aRct );
	aRct.right = aRct.left - 1;
	aRct.bottom = aRct.top - 1;

	const char * aBuf = (const char *)theMemory;

	// 1. Base invoke window
	HWND aBaseActivateWindow = *(const HWND *)aBuf;
	aBuf += sizeof HWND;

	// 2. Absolute PIDL of the item that was clicked on
	LPCITEMIDLIST aFullPidl = (LPCITEMIDLIST) aBuf;

	LPCITEMIDLIST aRelativePidl = 0;	// points to a part inside aFullPidl; needs not be released
	CComPtr<IShellFolder> aSF;
	HRESULT aRes = UtilShellFunc::SHBindToParentEx(aFullPidl, IID_IShellFolder, (void**)&aSF, &aRelativePidl);

	if ( SUCCEEDED(aRes) )
	{
		if (myItem != 0)
			delete myItem;

		LPITEMIDLIST aItemPidl = UtilPidlFunc::Copy<MemoryWriter_Crt>(aRelativePidl);
		myItem = Application::Instance().GetShellItemFactory()->CreateChild(aSF, aItemPidl);

		MenuManager::SetIpcShellViewWnd(aBaseActivateWindow);
		MenuManager::SetFGWindowMode(myDblClkOpensIcons ? 1 : 2 );
		MenuManager::ConstructByDataItem(myItem, 0, &aRct, true);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////

void HotspotImpl_FolderIcon::ProcessShellHook( WPARAM theWParam, LPARAM theLParam )
{
	if (theWParam == HSHELL_WINDOWCREATED)
	{
		Trace("HotspotImpl_FolderIcon::Message_Handler_ShellMsg. theWParam=0x%8x, theLParam=0x%8x.\n", theWParam, theLParam);
		ReEnumerate((HWND) theLParam);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////

LRESULT HotspotImpl_FolderIcon::MsgHandler_Timer( UINT theMsg, WPARAM theWParam, LPARAM theLParam, BOOL & theHandled )
{
	if (theWParam == myTimerID)
		ReEnumerate(0);

	else
		theHandled = FALSE;

	return 0L;
}

//////////////////////////////////////////////////////////////////////////////////////////////

LRESULT HotspotImpl_FolderIcon::MsgHandler_SettingChange( UINT theMsg, WPARAM theWParam, LPARAM theLParam, BOOL & theHandled )
{
	if (theWParam != 0 || theLParam == 0)
		return 0L;

	TCHAR * aString = (TCHAR *) theLParam;

#ifdef _DEBUG

	// Trace the setting change string

#  ifdef _UNICODE
	int aLen = lstrlen(aString);

	char * aTranslated = new char[aLen+1];

	ZeroMemory(aTranslated, (aLen + 1) * sizeof(char) );

	WideCharToMultiByte(CP_ACP, 0, aString, aLen, aTranslated, aLen, NULL, NULL);

	Trace("WM_SETTINGCHANGE. theString = %s.\n", aTranslated);

	delete [] aTranslated;
#  else
	Trace("WM_SETTINGCHANGE. theString = %s.\n", aString);
#  endif
#endif

	// Since we are only interested in the "click items as follows" parameter,
	// simply do nothing in all other cases.
	if ( lstrcmp(aString, _T("ShellState") ) == 0)
		UpdateIconClickMode();

	return 0L;
}

//////////////////////////////////////////////////////////////////////////////////////////////

void HotspotImpl_FolderIcon::Start()
{
	// 0. Application interaction
	Application & aApp = Application::Instance();
	const ApplicationSettings * aS = aApp.Prefs();

	myIpcModule->LLSetPreferences(*aS);

	aApp.AddPreferencesEventProcessor(this);

	// 1. Root window interaction
	myRootWindow->AddIpcProcessor(GUID_Hotspot_FolderMenus, this);
	myRootWindow->AddShellHookProcessor(this);
	myRootWindow->AddMessageMap(this);

	// 2. Spied threads
	ReEnumerate(0);

	// 3. Periodic re-enumeration timer
	::SetTimer(*myRootWindow, myTimerID, 5000, NULL);
}

//////////////////////////////////////////////////////////////////////////////////////////////

void HotspotImpl_FolderIcon::Stop()
{
	// 3. Periodic re-enumeration timer
	::KillTimer(*myRootWindow, myTimerID);

	// 2. Spied threads
	RemoveAllMonitoring();

	// 1. Root window interaction
	myRootWindow->RemoveMessageMap(this);
	myRootWindow->RemoveShellHookProcessor(this);
	myRootWindow->RemoveIpcProcessor(GUID_Hotspot_FolderMenus);

	// 0. Application interaction
	Application & aApp = Application::Instance();
	aApp.RemovePreferencesEventProcessor(this);
}

//////////////////////////////////////////////////////////////////////////////////////////////

bool HotspotImpl_FolderIcon::SetIconClickMode()
{
	SHELLFLAGSTATE aSt = {0};
	SHGetSettings( &aSt, SSF_DOUBLECLICKINWEBVIEW);

	bool aDblClkOpensIcons = (aSt.fDoubleClickInWebView != 0);
	if (aDblClkOpensIcons != myDblClkOpensIcons)
	{
		myDblClkOpensIcons = aDblClkOpensIcons;
		return true;
	}
	else
		return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////

void HotspotImpl_FolderIcon::UpdateIconClickMode()
{
	bool aChanged = SetIconClickMode();

	if ( aChanged && IsEnabled() )
	{
		RemoveAllMonitoring();
		ReEnumerate(0);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////

void HotspotImpl_FolderIcon::ReEnumerate(HWND theParentWnd)
{
	UnlimitedDepthWindowNavigator < ClassCondition<FolderMenus_SpiedWindowClassName> > aNav;

	ThreadMaskMap aMarkedThreads;
	NavigationContext aCtx(*this, aMarkedThreads, theParentWnd);

	aNav.iterate(0, aCtx);

	if (theParentWnd == 0)
	{
		// cleanup unused in case a global search is taking place

		for (int i = 0; i < mySpiedThreads.GetSize();)
		{
			DWORD aThreadID = mySpiedThreads.GetKeyAt(i);

			if (aMarkedThreads.FindKey(aThreadID) < 0)
				RemoveUnusedThreadMonitoring(aThreadID);
			else
				i++;
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////

void HotspotImpl_FolderIcon::EnsureWindowMonitoring( HWND theWnd, ThreadMaskMap & theMarkedThreads )
{
	DWORD aThreadId = GetWindowThreadProcessId(theWnd, NULL);

	theMarkedThreads.Add(aThreadId, true);

	int aIndex = mySpiedThreads.FindKey(aThreadId);

	if ( aIndex != -1 )
		return;

	// Only do something if the thread isn't already monitored.

	int aHookType = 0;
	HOOKPROC aHookProc = 0;

	if (myDblClkOpensIcons)
	{
		aHookType = WH_MOUSE;
		aHookProc = myIpcModule->GetHookAddress_FolderMenus_DoubleClickMode();
	}
	else
	{
		aHookType = WH_GETMESSAGE;
		aHookProc = myIpcModule->GetHookAddress_FolderMenus_SingleClickMode();
	}

	HHOOK aHook = SetWindowsHookEx(aHookType, aHookProc, myIpcModule->GetModuleInstance(), aThreadId);

	mySpiedThreads.Add(aThreadId, aHook);
}

//////////////////////////////////////////////////////////////////////////////////////////////

void HotspotImpl_FolderIcon::RemoveAllMonitoring()
{
	if (mySpiedThreads.GetSize() == 0)
		return;

	for (int i = 0; i != mySpiedThreads.GetSize(); i++)
	{
		HHOOK aHk = mySpiedThreads.GetValueAt(i);
		UnhookWindowsHookEx(aHk);
	}

	mySpiedThreads.RemoveAll();
}

//////////////////////////////////////////////////////////////////////////////////////////////

void HotspotImpl_FolderIcon::RemoveUnusedThreadMonitoring( DWORD theThreadId )
{
	int aIndex = mySpiedThreads.FindKey(theThreadId);

	HHOOK aHk = mySpiedThreads.GetValueAt(aIndex);
	UnhookWindowsHookEx(aHk);

	mySpiedThreads.RemoveAt(aIndex);
}

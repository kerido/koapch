//     MenuManager.cpp
//
//     The implementation of the UI MenuManager class. This class is a Singleton. It has
//     only static members. The main purposes of this class are:
//     1. maintaining the list of active top-level menus;
//     2. notifying the message loop of special events (i.e. the possible
//        thread exit moment);
//     3. implementing the window procedure (as a standard Windows callback function)
//        which internally resolves actual MenuWindow instances and further invokes
//        their virtual WndProc method
//     4. Activating child menus
//
//     (c) 2002-2005 Kirill Osipov
//
//////////////////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "sdk_MenuBuilder.h"
#include "sdk_Item.h"
#include "sdk_ComObject.h"
#include "sdk_WindowNavigator.h"

#include "Trace.h"
#include "MenuManager.h"
#include "Framework.h"
#include "Application.h"
#include "MenuWindow_Shell.h"
#include "DisplayItem.h"
#include "RootWindow.h"
#include "IpcModule.h"

#include "util_Localization.h"
#include "util_Window.h"

//////////////////////////////////////////////////////////////////////////////////////////////

WNDCLASS                            MenuManager::MenuWndClass = {0};
HWND                                MenuManager::ActivateBaseWnd = 0;
HWND                                MenuManager::HoverMenuWnd = 0;
Theme                             * MenuManager::DefaultTheme = 0;
DWORD                               MenuManager::OperationInProgress = 0;
int                                 MenuManager::FGSetMode = 0;
bool                                MenuManager::IsDraggingAndDropping = false;
MenuManager::ActiveMenusList        MenuManager::ActiveMenus;
IMenuManagerAgent                 * MenuManager::Agent = 0;
MenuManager::HelperWnd            * MenuManager::HelperWindow = 0;

//////////////////////////////////////////////////////////////////////////////////////////////

int CurMenuDepthLevel = 0;
HWND CurHookedWindow = 0;

//////////////////////////////////////////////////////////////////////////////////////////////

//TODO: test routines for CurMenuDepthLevel
void ChangeDepthLevel(bool theForward)
{
	const static int      ShiftBits_L  = 5;
	const static int      ShiftBits_R  = 32 - ShiftBits_L;

	const static unsigned Magic_Add_1 = 0x05AD13AE;
	const static unsigned Magic_Xor_1 = 0x64DAF14C;
	const static unsigned Magic_Xor_2 = 0x67C8A10D;

	//this is an obfuscated variant of changing depth level
	//the values that this func produces are those indexes
	//that are defined in the menu builder map

	if (theForward)
	{
		unsigned aTemp = (unsigned) CurMenuDepthLevel;
		aTemp += Magic_Add_1;
		aTemp ^= Magic_Xor_1;
		aTemp = (aTemp << ShiftBits_L) | (aTemp >> ShiftBits_R);		//same as  ROL 5
		aTemp ^= Magic_Xor_2;
		CurMenuDepthLevel = aTemp;
	}
	else
	{
		unsigned aTemp = (unsigned) CurMenuDepthLevel;
		aTemp ^= Magic_Xor_2;
		aTemp = (aTemp >> ShiftBits_L) | (aTemp << ShiftBits_R);    //same as  ROR 5
		aTemp ^= Magic_Xor_1;
		aTemp -= Magic_Add_1;
		CurMenuDepthLevel = aTemp;
	}

	Trace("Current depth level is: %x\n", CurMenuDepthLevel);
}

//////////////////////////////////////////////////////////////////////////////////////////////

class MenuManager::HelperWnd : public CMessageMap, public IIpcIoProcessor
{
private:
	RootWindow * myRootWindow;
	UINT myTimerID;
	HHOOK myMenuHook_Post;
	HHOOK myMenuHook_Get;
	DWORD myTick;


public:
	HelperWnd() :
			myRootWindow( Application::Instance().GetRootWindowInstance() ),
			myMenuHook_Post(0), myMenuHook_Get(0), myTick(0)
	{
		myTimerID = myRootWindow->GetAvailableTimerID();

		myRootWindow->AddMessageMap(this);
		myRootWindow->AddIpcProcessor(GUID_Util_MessageRedirect, this);
	}

	~HelperWnd()
	{
		myRootWindow->RemoveIpcProcessor(GUID_Util_MessageRedirect);
		myRootWindow->RemoveMessageMap(this);
	}


public:
	void StartMenuActivityMonitor()
	{
		Trace("MenuManager::HelperWnd -- StartMenuActivityMonitor\n");

		OSVERSIONINFO aVer;
		aVer.dwOSVersionInfoSize = sizeof OSVERSIONINFO;
		GetVersionEx(&aVer);

		if (aVer.dwMajorVersion >= 6 && aVer.dwMinorVersion >= 1)
			CurHookedWindow = GetIpcShellViewWnd();	//older OS-es don't support this type of monitoring, use the timer based
		else
			CurHookedWindow = NULL;

		if (CurHookedWindow != NULL)
		{
			UnlimitedDepthParentWindowNavigator< WindowStyleCondition<WS_CHILD, false> > aNav;

			HWND aWnd = aNav.find(CurHookedWindow);
			WindowUtility::SetForegroundWindow_ThreadInput(aWnd);

			DWORD aThreadID = GetWindowThreadProcessId(aWnd, NULL);

			IpcModule * aIpc = Application::InstanceC().GetIpcModule();
			HINSTANCE aHkInst = aIpc->GetModuleInstance();

			HOOKPROC aHk = aIpc->GetHookAddress_Menu_Post();
			myMenuHook_Post = SetWindowsHookEx(WH_GETMESSAGE, aHk, aHkInst, aThreadID);

			aHk = aIpc->GetHookAddress_Menu_Get();
			myMenuHook_Get = SetWindowsHookEx(WH_CALLWNDPROC, aHk, aHkInst, aThreadID);

			myTick = GetTickCount();
		}
		else
		{
			myRootWindow->SetTimer(myTimerID, 10);
		}
	}

	void StopMenuActivityMonitor()
	{
		if (myMenuHook_Post != 0)
			UnhookWindowsHookEx(myMenuHook_Post);

		if (myMenuHook_Get != 0)
			UnhookWindowsHookEx(myMenuHook_Get);

		myRootWindow->KillTimer(myTimerID);
	}


// WTL Messaging
	BEGIN_MSG_MAP(HelperWnd)
		MESSAGE_HANDLER(WM_TIMER,                                  MsgHandler_Timer)
		MESSAGE_HANDLER(RootWindow::WM_USER_DESTROYNONFLOATING,    MsgHandler_DestroyNonFloating)
		MESSAGE_RANGE_HANDLER(WM_KEYFIRST, WM_KEYLAST,             MsgHandler_Keyboard)
		MESSAGE_HANDLER(WM_MOUSEWHEEL,                             MsgHandler_MouseWheel)
	END_MSG_MAP()

protected:
	void ProcessIpcIo(const void * theMemory)
	{
		const MsgData * a = reinterpret_cast<const MsgData *>(theMemory);

		LRESULT aDummy;
		ProcessWindowMessage(*myRootWindow, a->Msg, a->Wparam, a->Lparam, aDummy, 0);
	}

// Message Handlers
private:
	LRESULT MsgHandler_Timer(UINT theMsg, WPARAM theWParam, LPARAM theLParam, BOOL & theHandled)
	{
		if (theWParam != myTimerID)
		{
			theHandled = FALSE;
			return 0L;
		}

		if ( ActiveMenus.empty() )
			return 0;

		if (IsDraggingAndDropping || OperationInProgress != 0)
			return 0;

		HWND aForeground = GetForegroundWindow();

		if ( !IsMenuWindow(aForeground) )
		{
#ifdef _DEBUG

			TCHAR aWndText[256] = {0};
			::GetWindowText(aForeground, aWndText, 256);

			HDC aDC = ::GetDC(NULL);

			::TextOut(aDC, 10, 10, aWndText, lstrlen(aWndText) );

			::DeleteDC(aDC);

#  ifdef _UNICODE
			char aMulti [256] = {0};

			WideCharToMultiByte(CP_ACP, 0, aWndText, -1, aMulti, 256, NULL, NULL);
			Trace("Foreground window is: %x -- %s\n", aForeground, aMulti);

#  else
			Trace("Foreground window is: %x -- %s\n", aForeground, aWndText);
#  endif	//_UNICODE


			Trace("MenuWindow instances: ");

			for (ActiveMenusIter aIt = ActiveMenus.begin(); aIt != ActiveMenus.end(); aIt++)
			{
				MenuWindow * aWnd = * aIt;

				do
				{
					Trace("%x; ", aWnd->GetHandle() );

					aWnd = aWnd->GetChild();
				} while (aWnd != 0);

			}
			Trace("\n");
#endif

			DestroyNonFloatingMenus();
		}
		return 0;
	}

	LRESULT MsgHandler_DestroyNonFloating(UINT theMsg, WPARAM theWParam, LPARAM theLParam, BOOL & theHandled)
	{
		Trace("MenuManager::HelperWnd --MsgHandler_DestroyNonFloating, WPARAM=%x.\n", theWParam);

		DWORD aTick = GetTickCount();

		if (aTick - myTick > 50)
		{
			EndMenu();
			DestroyNonFloatingMenus();
		}

		return 0L;
	}

	LRESULT MsgHandler_Keyboard(UINT theMsg, WPARAM theWParam, LPARAM theLParam, BOOL & theHandled)
	{
		HWND aMenu = GetTopmostMenu();
		if (aMenu != NULL)
			::PostMessage(aMenu, theMsg, theWParam, theLParam);

		return 0L;
	}

	LRESULT MsgHandler_MouseWheel( UINT theMsg, WPARAM theWParam, LPARAM theLParam, BOOL & theHandled )
	{
		if (HoverMenuWnd != NULL)
			::PostMessage(HoverMenuWnd, theMsg, theWParam, theLParam);

		return 0L;
	}
};

//////////////////////////////////////////////////////////////////////////////////////////////

class MenuManager::MenuManagerAgentImpl : public ComEntry2<IMenuManagerAgent, IMenuKeyboardNav>
{
protected:
	STDMETHODIMP GetMenuData(MenuData * theOutData)
	{
		theOutData->WndProc         = MenuWindowProc;
		theOutData->WindowClassName = MenuWndClass.lpszClassName;
		theOutData->ModuleInstance  = _Module.GetModuleInstance();

		return S_OK;
	}

	STDMETHODIMP CreateMenuWindow(MenuWindow * theObj, HWND * theOutHwnd)
	{
		return MenuManager::CreateMenuWindow(theObj, theOutHwnd);
	}

	STDMETHODIMP DestroyMenuChain(MenuWindow * theStartAt)
	{
		return MenuManager::DestroyMenuChain(theStartAt);
	}

	STDMETHODIMP GetKeyCode(int theNavKeyType, UINT * theOutCode)
	{
		if (theOutCode == 0)
			return E_POINTER;

		*theOutCode = GetNavKey(theNavKeyType);
		return S_OK;
	}
};

//////////////////////////////////////////////////////////////////////////////////////////////

void MenuManager::Init(Theme * theTheme)
{
	Assert ( theTheme != 0 );

	ActiveMenus.clear();

	DefaultTheme = theTheme;

	Agent = new ComInstance<MenuManagerAgentImpl>;

	//create a helper window to handle timer events
	HelperWindow = new HelperWnd;


	MenuWndClass.style          = DefaultTheme->GetStyle(STYLE_CLASS_MENU_DEFAULT);
	MenuWndClass.lpfnWndProc    = MenuWindowProc;
	MenuWndClass.hInstance      = _Module.GetModuleInstance();
	MenuWndClass.hCursor        = LoadCursor(NULL, IDC_ARROW);
	MenuWndClass.hbrBackground  = (HBRUSH) COLOR_WINDOW;
	MenuWndClass.lpszClassName  = _T("KOMenuClass");

	RegisterClass(&MenuWndClass);

	ActivateBaseWnd = 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////

void MenuManager::Dispose()
{
	DestroyNonFloatingMenus();

	UnregisterClass(MenuWndClass.lpszClassName, _Module.GetModuleInstance() );

	delete DefaultTheme;
	delete HelperWindow;
	Agent->Release();

	OleFlushClipboard();
}

//////////////////////////////////////////////////////////////////////////////////////////////

HRESULT MenuManager::CreateMenuWindow(MenuWindow * theObj, HWND * theOutHwnd)
{
	*theOutHwnd = CreateWindowEx
	(
		WS_EX_TOOLWINDOW | WS_EX_DLGMODALFRAME | WS_EX_NOACTIVATE,
		MenuWndClass.lpszClassName,
		_T(""),
		WS_POPUP | WS_BORDER,
		CW_USEDEFAULT, CW_USEDEFAULT,
		CW_USEDEFAULT, CW_USEDEFAULT,
		NULL,
		0,
		_Module.GetModuleInstance(),
		theObj
	);

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////////////////////////

HRESULT MenuManager::DestroyMenuChain(MenuWindow * theStartAt)
{
	MenuWindow * aParent = theStartAt->GetParent();

	if (aParent == 0)
		DestroyNonFloatingMenus();
	else
	{
		DestroyMenuChainRecursive(theStartAt);

		aParent->SetChild(0);

		if (GetIpcShellViewWnd() == NULL)
			BringMenuToForeground(aParent);
	}

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////////////////////////

LRESULT CALLBACK MenuManager::MenuWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	MenuWindow * aMW = 0;

	switch (msg)
	{
	case WM_NCCREATE:
		ChangeDepthLevel(true);
		SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG) ( (CREATESTRUCT*) lParam )->lpCreateParams );
		return TRUE;

	case WM_CREATE:
		LocalizationUtility::SetWindowDirection(hWnd, LocalizationManagerPtr() );
		break;

	case WM_MOUSEMOVE:
		HoverMenuWnd = hWnd;
		DefaultMouseTrack(hWnd, TME_LEAVE);
		break;

	case WM_MOUSELEAVE:
		HoverMenuWnd = 0;
		break;

	case WM_LBUTTONDBLCLK:
	case WM_LBUTTONUP:
	case WM_LBUTTONDOWN:
		if (OperationInProgress == 1)
			return DefWindowProc(hWnd, msg, wParam, lParam);
		else
			break;

	case WM_MOUSEWHEEL:
		aMW = Resolve(HoverMenuWnd);
		if (aMW) return aMW->WndProc(HoverMenuWnd, msg, wParam, lParam);
		else     return 0;

	case WM_MOUSEACTIVATE:
		return MA_NOACTIVATE;	// prevent parent window from appearing above its child

	case WM_DESTROY:
		ChangeDepthLevel(false);
		return OnDestroy(hWnd, wParam, lParam);

	case WM_NCPAINT:
		{
			aMW = Resolve(hWnd);

			Theme * aTheme = aMW->GetTheme();

			if (aTheme == 0)
				break;

			RECT aRect;
			GetWindowRect(hWnd, &aRect);

			HDC hdc = GetWindowDC (hWnd);

			HRESULT aRes = aMW->GetTheme()->DrawMenuNcArea(hdc, aRect);

			::ReleaseDC(hWnd, hdc);

			if ( SUCCEEDED(aRes) )
				return 0;

			break;
		}

	default:
		break;
	}

	aMW = Resolve(hWnd);

	//these lines were added for mouse hook. they were not here with the v1 hook
	if (aMW == 0)
		return DefWindowProc(hWnd, msg, wParam, lParam);

	return  aMW->WndProc(hWnd, msg, wParam, lParam);
}

//////////////////////////////////////////////////////////////////////////////////////////////
//     MenuManager::Resolve
//
//     TODO: add description
//////////////////////////////////////////////////////////////////////////////////////////////

INLINE MenuWindow * MenuManager::Resolve(HWND hWnd)
{
	LONG_PTR aPtr = GetWindowLongPtr(hWnd, GWLP_USERDATA);
	return (MenuWindow *)aPtr;
}

//////////////////////////////////////////////////////////////////////////////////////////////

MenuWindow * MenuManager::ConstructByDataItem(Item * theDataItem, MenuWindow * theParent,
                                              RECT * theAlignRect, bool theShow)
{
	if (theDataItem == 0)
		return 0;


	if (theParent == 0)
		DestroyNonFloatingMenus();

	UINT aExpCode = 0;

	IMenuBuilder * aMB = Framework::GetMenuBuilder(theDataItem, aExpCode);

	if (aMB == 0 || aExpCode == IMenuBuilder::NON_EXPANDABLE) return false;


	MenuBuilderData aData;
	aData.Agent = Agent;
	aData.Theme = DefaultTheme;

	//align the window to parent (considering the alignment rectangle)
	if (theAlignRect)
		aData.Rect = *theAlignRect;
	else
	{
		POINT aPt;
		GetCursorPos(&aPt);

		aData.Rect.left   = aPt.x;
		aData.Rect.top    = aPt.y;
		aData.Rect.right  = aPt.x - 1;   // indicate that only the first two members are valid
		aData.Rect.bottom = aPt.y - 1;   // by setting the last two to values less than the first
	}


	MenuWindow * aNewWindow = 0;
	HRESULT aRes = aMB->Build(theDataItem, &aData, &aNewWindow);

	bool aNeedToSetTimer = ActiveMenus.empty();

	if (theParent)	//maintain the parent-child relationship
	{
		theParent->SetChild(aNewWindow);
		aNewWindow->SetParent(theParent);
	}

	if (theShow)
		AdjustPosition(aNewWindow, &aData.Rect, POS_DEFAULT);

	return aNewWindow;
}

//////////////////////////////////////////////////////////////////////////////////////////////

void MenuManager::AdjustPosition(MenuWindow * theTarget, RECT * theConstraintRect, int theFlags)
{
	RECT aWindowRect;
	GetWindowRect(theTarget->GetHandle(), &aWindowRect);

	POINT aPt = { theConstraintRect->left, theConstraintRect->top };
	HMONITOR aMon = MonitorFromPoint(aPt, MONITOR_DEFAULTTONEAREST);

	MONITORINFO aInfo;
	aInfo.cbSize = sizeof MONITORINFO;
	GetMonitorInfo(aMon, &aInfo);

	int aMinX = aInfo.rcMonitor.left,    //was 0
	    aMinY = aInfo.rcMonitor.top,     //was 0
	    aMaxX = aInfo.rcMonitor.right,   //was GetSystemMetrics(SM_CXSCREEN),
	    aMaxY = aInfo.rcMonitor.bottom;  //was GetSystemMetrics(SM_CYSCREEN);

	int aConstraintX1, aConstraintX2, aConstraintY1, aConstraintY2;

	if ( theFlags == POS_DEFAULT)
	{
		if (theConstraintRect->right  >= theConstraintRect->left &&
				theConstraintRect->bottom >= theConstraintRect->top )
		{
			int aBorderX = GetSystemMetrics(SM_CXDLGFRAME),
			    aBorderY = GetSystemMetrics(SM_CYDLGFRAME);

			aConstraintX1 = theConstraintRect->right  - aBorderX;
			aConstraintX2 = theConstraintRect->left   + aBorderX;
			//the first is for right-alignment, the second for left

			aConstraintY1 = theConstraintRect->top    - aBorderY;
			aConstraintY2 = theConstraintRect->bottom + aBorderY;
			//the first is for top-alignment, the second for bottom
		}
		else
		{
			aConstraintX1 = aConstraintX2 = theConstraintRect->left;
			aConstraintY1 = aConstraintY2 = theConstraintRect->top;
		}
	}
	else if ( theFlags == POS_CENTER_IN_RECT)
	{
		// theConstraintRect stores the selection rect in window coordinates
		POINT aPt;
		GetCursorPos(&aPt);

		int aSelHalfWidth = (theConstraintRect->right - theConstraintRect->left) / 2;
		int aSelHeight = theConstraintRect->bottom - theConstraintRect->top;
		int aSelHalfHeight = aSelHeight / 2;

		aConstraintX1 = aPt.x - aSelHalfWidth;
		aConstraintX2 = aPt.x + aSelHalfWidth;

		aConstraintY1 = aPt.y - aSelHalfHeight;
		aConstraintY2 = aPt.y + aSelHalfHeight;

		// we know where the selection rect is in screen coords. Now we must know
		// where the window rect is in screen coords.

		aConstraintX1 -= theConstraintRect->left - aWindowRect.left;
		aConstraintX2 -= theConstraintRect->left - aWindowRect.left;

		aConstraintY1 -= theConstraintRect->top - aWindowRect.top;
		aConstraintY2 -= theConstraintRect->top - aWindowRect.top;

		if (aConstraintX1 < aMinX || aConstraintX2 > aMaxX)
			aConstraintX1 = aConstraintX2 = aPt.x;

		if (aConstraintY1 < aMinY || aConstraintY2 > aMaxY)
			aConstraintY1 = aConstraintY2 = aPt.y;
	}

	int aWidth  = aWindowRect.right  - aWindowRect.left,
	    aHeight = aWindowRect.bottom - aWindowRect.top;


	//adjust vertical positioning
	if      (aConstraintY1 + aHeight <= aMaxY) aWindowRect.top = aConstraintY1;
	else if (aConstraintY2 - aHeight >= aMinY) aWindowRect.top = aConstraintY2 - aHeight;
	else                                       aWindowRect.top = aMinY;


	//adjust horizontal positioning
	int aDirection = GetDirection( theTarget->GetParent() );

	if (aDirection > 0)
	{
		// This will also be the case when no parent menu exists.

		if (aConstraintX1 + aWidth <= aMaxX)
		{
			aWindowRect.left = aConstraintX1;
			theTarget->SetDirection(aDirection);
		}
		else
		{
			aWindowRect.left = aConstraintX2 - aWidth;
			theTarget->SetDirection(-aDirection);
		}
	} //end if display left to right
	else
	{
		if (aConstraintX2 - aWidth >= aMinX)
		{
			aWindowRect.left = aConstraintX2 - aWidth;
			theTarget->SetDirection(aDirection);
		}
		else
		{
			aWindowRect.left = aConstraintX1;
			theTarget->SetDirection(-aDirection);
		}
	}//end if display right to left

#if 0
	TCHAR aChar[2000];
	_stprintf(aChar, _T("aWindowRect.left=%d, aWindowRect.top=%d"), aWindowRect.left, aWindowRect.top);
	MessageBox(NULL, aChar, NULL, MB_OK);
#endif

	UINT aSwpFlags = SWP_NOSIZE|SWP_SHOWWINDOW;;

	if (CurHookedWindow != NULL)
		aSwpFlags |= SWP_NOACTIVATE;

	SetWindowPos(theTarget->GetHandle(), HWND_TOPMOST, aWindowRect.left, aWindowRect.top, 0, 0, aSwpFlags);


	/// new
	bool aNeedToSetTimer = ActiveMenus.empty();

	if (CurHookedWindow == NULL)
		BringMenuToForeground(theTarget);

	// otherwise, we will hook mouse input of the active view and
	// simulate a foreground window without flicker

	if (theTarget->GetParent() == 0) 	//top-level
		ActiveMenus.push_back(theTarget);

	PostMessage(theTarget->GetHandle(), WM_NULL, 0, 0);

	if (aNeedToSetTimer)
		HelperWindow->StartMenuActivityMonitor();
}

//////////////////////////////////////////////////////////////////////////////////////////////

void MenuManager::DefaultMouseTrack(HWND theWnd, DWORD theFlags)
{
	TRACKMOUSEEVENT aTME = {0};
	aTME.cbSize = sizeof(TRACKMOUSEEVENT);
	aTME.dwFlags = theFlags;
	aTME.dwHoverTime = HOVER_DEFAULT;
	aTME.hwndTrack = theWnd;

	TrackMouseEvent(&aTME);
}

//////////////////////////////////////////////////////////////////////////////////////////////

bool MenuManager::IsMenuWindow(HWND theWnd)
{
	for (ActiveMenusIter aIt = ActiveMenus.begin(); aIt != ActiveMenus.end(); aIt++)
	{
		MenuWindow * aWnd = * aIt;

		do
		{
			if (aWnd->GetHandle() == theWnd)
				return true;

			aWnd = aWnd->GetChild();

		}    while (aWnd != 0);

	}
	return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////

void MenuManager::DestroyNonFloatingMenus()
{
	for (ActiveMenusIter aIt = ActiveMenus.begin(); aIt != ActiveMenus.end();)
	{
		MenuWindow * aWnd = * aIt;

		MenuWindow * aTmp = aWnd;

		if (false)	//aWnd->IsFloating()
		{
			aTmp = aWnd->GetChild();
			aIt++;
		}
		else
			aIt = ActiveMenus.erase(aIt);

		if (aTmp != 0)
			DestroyMenuChainRecursive(aTmp);
	}

	if ( ActiveMenus.empty() )
		HelperWindow->StopMenuActivityMonitor();
}

//////////////////////////////////////////////////////////////////////////////////////////////

UINT MenuManager::GetNavKey( int theKeyType )
{
	bool aLtr = true;
	LocalizationUtility::IsDirectionLtr(LocalizationManagerPtr(), &aLtr);

	if (theKeyType == IMenuKeyboardNav::KEY_NEXT)
		return aLtr ? VK_RIGHT : VK_LEFT;

	else if (theKeyType == IMenuKeyboardNav::KEY_PREV)
		return aLtr ? VK_LEFT : VK_RIGHT;

	else
		return 0xFFFFFFFF;
}

//////////////////////////////////////////////////////////////////////////////////////////////

int MenuManager::GetDirection(const MenuWindow * theWnd)
{
	int aRet = 0;

	if (theWnd != 0)
		aRet = theWnd->GetDirection();

	if ( aRet != 0 ) return aRet;

	bool aLtr = true;
	LocalizationUtility::IsDirectionLtr(LocalizationManagerPtr(), &aLtr);

	return aLtr ? 1 : -1;
}

//////////////////////////////////////////////////////////////////////////////////////////////

LRESULT MenuManager::OnDestroy(HWND theWnd, WPARAM theWParam, LPARAM theLParam)
{
	//ReleaseTraceCode(MENUMANAGER_DESTROYWND);
	Trace("MenuManager::OnDestroy. HWND = %x\n", theWnd);

	MenuWindow * aWnd = Resolve(theWnd);
	return aWnd->WndProc(theWnd, WM_DESTROY, theWParam, theLParam);
}

//////////////////////////////////////////////////////////////////////////////////////////////

void MenuManager::BringMenuToForeground(MenuWindow * theMenu)
{
	if ( FGSetMode == 1 && !HasNonFloatingMenus() )
		WindowUtility::SetForegroundWindow_SimulatedMouseClick( theMenu->GetHandle() );

	else
		WindowUtility::SetForegroundWindow_ThreadInput( theMenu->GetHandle() );
}

//////////////////////////////////////////////////////////////////////////////////////////////

bool MenuManager::HasNonFloatingMenus()
{
	//for (ActiveMenusIter aIt = ActiveMenus.begin(); aIt != ActiveMenus.end(); aIt++)
	//	if (true)	//is floating
	//		return true;

	//return false;

	return !ActiveMenus.empty();
}

//////////////////////////////////////////////////////////////////////////////////////////////

HWND MenuManager::GetTopmostMenu()
{
	if ( ActiveMenus.empty() )
		return NULL;

	MenuWindow * aWnd = ActiveMenus.back();

	while (aWnd->GetChild() != 0)
		aWnd = aWnd->GetChild();

	return aWnd->GetHandle();
}

//////////////////////////////////////////////////////////////////////////////////////////////

void MenuManager::DestroyMenuChainRecursive(MenuWindow * theStartAt)
{
	ShowWindow(theStartAt->GetHandle(), SW_HIDE);

	MenuWindow * aChild = theStartAt->GetChild();

	if (aChild != 0)
	{
		DestroyMenuChainRecursive(aChild);
		theStartAt->SetChild(0);
	}

	delete theStartAt;
}
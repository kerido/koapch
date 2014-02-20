#include "Common.h"

#include "sdk_MemoryWriter.h"
#include "sdk_WindowNavigator.h"

#include "IpcGuids.h"
#include "IpcRootWindow.h"
#include "Main.h"
#include "UtilPidlFunc.h"
#include "Trace.h"
#include "FolderViewHitTest.h"

//////////////////////////////////////////////////////////////////////////////////////////////

extern LONG          gMustReact;
extern LONG          gTimeout;
extern Activator     gFnActivator;
extern GetCtxHandler gFnGetCtxMenuHandler;
extern HANDLE        gWaiterThread;

extern const TCHAR WndClass_SHELLDLL_DefView[]    = _T("SHELLDLL_DefView");
extern const TCHAR WndClass_CabinetWClass[]       = _T("CabinetWClass");
extern const TCHAR WndClass_ShellTabWindowClass[] = _T("ShellTabWindowClass");


//////////////////////////////////////////////////////////////////////////////////////////////

int gX = -1, gY = -1;

//////////////////////////////////////////////////////////////////////////////////////////////

class AsyncWriter : public MemoryWriter_NoCrt
{
private:
	HANDLE myHandle;
	void * myBuf, * myWritableBuf;

public:
	AsyncWriter(const GUID & theGuid, size_t theSize) : myHandle(0), myBuf(0), myWritableBuf(0)
	{
		myHandle = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, File_IpcExchange);

		if (myHandle != 0)
		{
			myBuf = MapViewOfFile(myHandle, FILE_MAP_ALL_ACCESS, 0, 0, sizeof GUID + theSize);
			myWritableBuf = AdvanceWrite(myBuf, &theGuid, sizeof GUID);
		}
	}

	~AsyncWriter()
	{
		if (myBuf != 0)
			UnmapViewOfFile(myBuf);

		if (myHandle != 0)
		{
			CloseHandle(myHandle);

			HANDLE aEvent = OpenEvent(EVENT_MODIFY_STATE, FALSE, Event_IpcSignal);
			SetEvent(aEvent);
			CloseHandle(aEvent);
		}
	}

	void * GetWriteBuffer() const
	{
		return myWritableBuf;
	}

	static void SerializeMessage(UINT theMsg, WPARAM theWParam, LPARAM theLParam)
	{
		AsyncWriter aWr(GUID_Util_MessageRedirect, sizeof MsgData);
		void * aBuf_Temp = aWr.GetWriteBuffer();

		if (aBuf_Temp != 0)
		{
			MsgData aD = { theMsg, theWParam, theLParam };
			aWr.AdvanceWrite(aBuf_Temp, &aD, sizeof MsgData);
		}
	}
};


//////////////////////////////////////////////////////////////////////////////////////////////
//     Common_GetCtxHandlerWndFromShellView_Win2kXp2003
//
//     <TODO>
//
//     ReturnValue:
//          HWND
//////////////////////////////////////////////////////////////////////////////////////////////

HWND Common_GetCtxHandlerWndFromShellView_Win2kXp2003(HWND theShellViewWnd)
{
	UnlimitedDepthParentWindowNavigator< ClassCondition<WndClass_CabinetWClass> > aNav;
	return aNav.find(theShellViewWnd);
}


//////////////////////////////////////////////////////////////////////////////////////////////
//     Common_GetCtxHandlerWndFromShellView_WinVista
//
//     <TODO>
//
//     ReturnValue:
//          HWND
//////////////////////////////////////////////////////////////////////////////////////////////


HWND Common_GetCtxHandlerWndFromShellView_WinVista(HWND theShellViewWnd)
{
	UnlimitedDepthParentWindowNavigator< ClassCondition<WndClass_ShellTabWindowClass> > aNav;
	return aNav.find(theShellViewWnd);
}

//////////////////////////////////////////////////////////////////////////////////////////////
//     FolderMenus_WaiterThreadProc
//
//     Thread routine for Folder Menus waiter thread.
//     This function checks the state of Folder Menus IPC, and, if the conditions are met,
//     writes data to shared memory and then raises an IPC event.
//
//     Parameters:
//          void * theParams. A pointer to BaseShellClickData class is passed from the thread
//              creator
//
//     ReturnValue:
//          DWORD - thread return code (meaningless for the program)
//////////////////////////////////////////////////////////////////////////////////////////////

DWORD __stdcall FolderMenus_WaiterThreadProc(void * theParams)
{
	BaseShellClickData * aDt = (BaseShellClickData *) theParams;

	if (gTimeout == 0)
	{
		Trace("The preferences are not set!\n");
		gTimeout = 300;
	}

	SleepEx(gTimeout, FALSE);		//do not put the thread in alertable state

	if (gMustReact != TRUE)
	{
		Trace("gMustReact is FALSE, returning from waiter thread\n");
		delete aDt;

		return 0;
	}

	if ( !aDt->Validate() )
	{
		delete aDt;

		return 0;
	}

	IPersistFolder2 * aPF = 0;
	HRESULT aRes = aDt->SF->QueryInterface(IID_IPersistFolder2, (void **) &aPF);

	if ( FAILED(aRes) || aPF == 0)
	{
		Trace("Unable to query IPersistFolder2\n");

		delete aDt;

		return 0;
	}


	LPITEMIDLIST aAbsolutePidl = 0;		//pidl that represents a folder
	aRes = aPF->GetCurFolder( &aAbsolutePidl );

	aPF->Release();

	if ( FAILED(aRes) || aAbsolutePidl == 0)
	{
		Trace("Unable to extract current folder's pidl via IPersistFolder2::GetCurFolder\n");

		delete aDt;

		return 0;
	}

	//pidl that represents the item which was clicked
	LPITEMIDLIST aNewPidl = UtilPidlFunc::PidlAppend<MemoryWriter_NoCrt>(aAbsolutePidl, aDt->Pidl);

	CoTaskMemFree(aAbsolutePidl);

	UINT aSize = UtilPidlFunc::PidlSize(aNewPidl);

	AsyncWriter aWr(GUID_Hotspot_FolderMenus, aSize + sizeof HWND);
	void * aBuf_Temp = aWr.GetWriteBuffer();

	int aRetVal = 1;
	if (aBuf_Temp != 0)
	{
		//1. Invocation window
		aBuf_Temp = aWr.AdvanceWrite(aBuf_Temp, &aDt->BaseInvokeWindow, sizeof HWND );

		//2. Absolute PIDL of the item that was clicked on
		aBuf_Temp = aWr.AdvanceWrite(aBuf_Temp,  aNewPidl,              aSize );

		aRetVal = 0;
	}

	CoTaskMemFree(aNewPidl);
	delete aDt;

	return aRetVal;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//     FolderMenus_CreateWaiterThread
//
//     <TODO>
//
//////////////////////////////////////////////////////////////////////////////////////////////

void FolderMenus_CreateWaiterThread(BaseShellClickData * theStr)
{
	Trace("Creating waiter thread...\n");

	DWORD aTemp = 0;					//stores thread exit code and ID afterwards (if the thread is created)
	bool aCanCreate = false;

	if (gWaiterThread == 0)
		aCanCreate = true;

	else
	{
		BOOL aRes = GetExitCodeThread(gWaiterThread, &aTemp);
		aCanCreate = (aRes != 0) && (aTemp != STILL_ACTIVE);
	}

	if (aCanCreate)
	{
		if (gWaiterThread != 0)	//need to close handle for the previous worker thread
			CloseHandle(gWaiterThread);

		gWaiterThread = CreateThread(
					NULL,
					0,
					FolderMenus_WaiterThreadProc,
					theStr,
					0,
					&aTemp);
	}

	else
	{
		delete theStr;
		FolderMenus_SetReactFlag(FALSE);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
//     FolderMenus_SetReactFlag
//
//     <TODO>
//
//////////////////////////////////////////////////////////////////////////////////////////////

void FolderMenus_SetReactFlag(LONG theReact, bool theCheck /*=true*/)
{
	//Trace("SpyHook -- SetReactFlag. theReact = %d, theCheck = %d\n", theReact, theCheck);

	if (!theCheck || (gMustReact & 0x4000) == 0)
		InterlockedExchange(&gMustReact, theReact);
}

//////////////////////////////////////////////////////////////////////////////////////////////

void FolderMenus_OnMouseMove(int theX, int theY, bool theSingleClick = false)
{
	//the first after gMustReact must be omitted
	if (gMustReact == TRUE)
	{
		if (gX - theX > 2 || gX - theX < -2 || gY - theY > 2 || gY - theY < -2)
		{
			gX = theX;
			gY = theY;

			Trace("  OnMouseMove. The cursor if more than 2px away from the previous.\n");
			FolderMenus_SetReactFlag(FALSE);
		}
	}


	// This check should work wne the user opens another window from Explorer
	// however it causes problems resulting with not appearing menus under
	// certain circumstances

	/*
	else if (theX == gX && theY == gY && theSingleClick)
		FolderMenus_SetReactFlag(TRUE);
	*/

	else
	{
		gX = theX;
		gY = theY;

		FolderMenus_SetReactFlag(FALSE);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////

void FolderMenus_ProcessIconView(HWND theIconView, int theX, int theY)
{
	// 1. Check that there is a shell view actually
	TCHAR aTempWndClassName[256];
	HWND aParent = GetParent(theIconView);

	while (aParent != NULL)
	{
		GetClassName(aParent, aTempWndClassName, 256);

		if ( lstrcmp(aTempWndClassName, _T("SHELLDLL_DefView") ) == 0)
			break;

		aParent = GetParent(aParent);
	}

	if (aParent == NULL)
		return;

	ShellFolderViewHitTest::HitTestFunc aFunc = ShellFolderViewHitTest::GetHitTestFunc(theIconView);

	if (aFunc == 0)
		return;

	int aItem = aFunc(theIconView, theX, theY);

	if (aItem == -1)
		return;

	if (gMustReact == FALSE)	//HACK
	{
		FolderMenus_SetReactFlag(TRUE);

		if (gFnActivator == NULL)
			return;

		BaseShellClickData * aStr = gFnActivator(aParent, theIconView, aItem);

		if (aStr != 0)
			FolderMenus_CreateWaiterThread(aStr);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////

void FolderMenus_DoProcessHookMouse(DWORD theMouseMsg, const MOUSEHOOKSTRUCT * theHookStruct)
{
	//Trace("DoProcessHook_Mouse. theMouseMsg = %x\n", theMouseMsg);

	if (theMouseMsg != WM_LBUTTONDOWN && theMouseMsg != WM_MOUSEMOVE)
	{
		FolderMenus_SetReactFlag(FALSE);
		return;
	}

	else if (theMouseMsg == WM_MOUSEMOVE)
	{
		int aX = theHookStruct->pt.x, aY = theHookStruct->pt.y;
		FolderMenus_OnMouseMove(aX, aY);
		return;
	}

	else
		FolderMenus_ProcessIconView(theHookStruct->hwnd, theHookStruct->pt.x, theHookStruct->pt.y);
}

//////////////////////////////////////////////////////////////////////////////////////////////

void FolderMenus_DoProcessHookGetMsg(const MSG * theMsg)
{
	Trace("Message %x. HWND = 0x%8x\n", theMsg->message);

	if (theMsg->message == WM_MOUSEMOVE)
	{
		int
			aX = theMsg->pt.x,
			aY = theMsg->pt.y;

		FolderMenus_OnMouseMove(aX, aY, true);
	}

	else if (theMsg->message == WM_LBUTTONDOWN)
		FolderMenus_SetReactFlag(FALSE, false);

	else if (theMsg->message == WM_KEYDOWN)
		FolderMenus_SetReactFlag(FALSE, false);

	else if (theMsg->message == WM_MOUSEHOVER)
	{
		if (gMustReact == FALSE)
			FolderMenus_ProcessIconView(theMsg->hwnd, theMsg->pt.x, theMsg->pt.y);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////////////
//     TitlebarMenus_ProcessTitleBarClick
//
//     This is a new function, yet undone.
//     The purpose is to handle CTRL+clicks on Explorer's title bar clicks and displaying
//     parent-to-child menus.
//////////////////////////////////////////////////////////////////////////////////////////////

void TitlebarMenus_ProcessTitleBarClick(HWND thePrimaryWindow, int theX, int theY)
{
	if (gFnActivator == 0)
		return;

	TCHAR aTempWndClassName[256];
	GetClassName(thePrimaryWindow, aTempWndClassName, 256);

	if ( lstrcmp(aTempWndClassName, _T("CabinetWClass") ) != 0)
		return;


	// NOTE: On Windows Vista there is multi-level hierarchy in order to get to the
	//       SHELLDLL_DefView Window. Need to find recursively

	UnlimitedDepthWindowNavigator < ClassCondition<WndClass_SHELLDLL_DefView> > aNav;
	HWND aShellDllDefView = aNav.find(thePrimaryWindow);

	if (aShellDllDefView == 0)
		return;

	IUnknown * aSF = 0;
	BaseShellClickData * aStr = gFnActivator(aShellDllDefView, NULL, -1);

	if ( aStr != 0 )
		aSF = aStr->SF;

	delete aStr;

	if (aSF == 0)
		return;
	
	IPersistFolder2 * aPF = 0;

	HRESULT aRes = aSF->QueryInterface(IID_IPersistFolder2, (void **) &aPF);

	if ( FAILED(aRes) || aPF == 0)
		return;

	LPITEMIDLIST aAbsolutePidl = 0;		//pidl that represents a folder
	aRes = aPF->GetCurFolder( &aAbsolutePidl );

	aPF->Release();

	if ( FAILED(aRes) || aAbsolutePidl == 0)
		return;

	UINT aSize = UtilPidlFunc::PidlSize(aAbsolutePidl);

	if (aSize == 0)
		return;

	AsyncWriter aWr(GUID_Hotspot_TitlebarMenus, aSize + sizeof HWND);

	void * aBuf_Temp = aWr.GetWriteBuffer();

	if (aBuf_Temp != 0)
	{
		HWND aContextMenuHandlerWnd = gFnGetCtxMenuHandler(aShellDllDefView);

		aBuf_Temp = aWr.AdvanceWrite(aBuf_Temp, &aContextMenuHandlerWnd, sizeof HWND);
		aBuf_Temp = aWr.AdvanceWrite(aBuf_Temp,  aAbsolutePidl,          aSize      );
	}

	CoTaskMemFree(aAbsolutePidl);
}

//////////////////////////////////////////////////////////////////////////////////////////////

void MenuActivity_DoProcess_Post(WPARAM wParam, LPARAM lParam)
{
	MSG * aS = reinterpret_cast<MSG *>(lParam);

	Trace("MSG=%x, HWND=%x, WPARAM=%x, LPARAM=%x\n", aS->message, aS->hwnd, aS->wParam, aS->lParam);

	if
				(
					aS->message == WM_LBUTTONDOWN ||
					aS->message == WM_RBUTTONDOWN ||
					aS->message == WM_MBUTTONDOWN ||
					aS->message == WM_XBUTTONDOWN ||
					aS->message == WM_NCLBUTTONDOWN ||
					aS->message == WM_NCRBUTTONDOWN ||
					aS->message == WM_NCMBUTTONDOWN ||
					aS->message == WM_NCXBUTTONDOWN
				)
	{
		AsyncWriter::SerializeMessage(IpcRootWindow::WM_USER_DESTROYNONFLOATING, aS->message, 0);
	}
	else if (aS->message >= WM_KEYFIRST && aS->message <= WM_KEYLAST || aS->message == WM_MOUSEWHEEL)
	{
		AsyncWriter::SerializeMessage(aS->message, aS->wParam, aS->lParam);
		aS->message = WM_NULL;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////

void MenuActivity_DoProcess_Get(WPARAM wParam, LPARAM lParam)
{
	CWPSTRUCT * aS = reinterpret_cast<CWPSTRUCT *>(lParam);
	UINT aM = aS->message;

	bool aShouldDestroyNonfloating = false;

	if (aM == WM_ACTIVATEAPP || aM == WM_NCACTIVATE)
		aShouldDestroyNonfloating = (aS->wParam == FALSE);
	else if (aM == WM_ACTIVATE)
		aShouldDestroyNonfloating = ( LOWORD(aS->wParam) == WA_INACTIVE);

	if (aShouldDestroyNonfloating)
		AsyncWriter::SerializeMessage(IpcRootWindow::WM_USER_DESTROYNONFLOATING, aS->message, 0);
}


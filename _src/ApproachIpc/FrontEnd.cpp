#include "Common.h"

#include "Main.h"
#include "InterProcessContextMenuInfo.h"
#include "sdk_MemoryWriter.h"

//////////////////////////////////////////////////////////////////////////////////////////////

extern LONG gTimeout;
extern HINSTANCE gInstance;
extern HHOOK gHook;

//////////////////////////////////////////////////////////////////////////////////////////////
//    Hook_Mouse
//
//    <TODO>
//    This function is exportable (as set in the Module Definition file)
//
//////////////////////////////////////////////////////////////////////////////////////////////

LRESULT CALLBACK Hook_Mouse(int theCode, WPARAM wParam, LPARAM lParam)
{
	switch (theCode)
	{
	case HC_ACTION:
		FolderMenus_DoProcessHookMouse( (DWORD) wParam, (const MOUSEHOOKSTRUCT *) lParam );
		break;

	default:
		break;
	}
	return CallNextHookEx(NULL, theCode, wParam, lParam);
}


//////////////////////////////////////////////////////////////////////////////////////////////
//    Hook_GetMsg                                                                           //
//                                                                                          //
//    Processes the WH_GETMESSAGE hook. This hook is installed if explorer features the     //
//    single-click-to-open-icons mode (set via Folder Options -> Click Items As Follows.    //
//    The primary goal of this function is to catch mouse clicks upon Explorer icons.       //
//                                                                                          //
//    This function is exportable (as set in the Module Definition file)                    //
//                                                                                          //
//////////////////////////////////////////////////////////////////////////////////////////////

LRESULT CALLBACK Hook_GetMsg(int theCode, WPARAM wParam, LPARAM lParam)
{
	switch (theCode) 
	{
	case HC_ACTION:
		FolderMenus_DoProcessHookGetMsg( (const MSG *) lParam );
		break;

	default:
		break;
	}
	return CallNextHookEx(NULL, theCode, wParam, lParam);
}


//////////////////////////////////////////////////////////////////////////////////////////////
//    SetGlobalPrefs                                                                        //
//                                                                                          //
//    Currently sets the FolderMenus timeout value to a Shared variable. The data is        //
//    obtained from the Application properties.                                             //
//                                                                                          //
//    This function is exportable (as set in the Module Definition file)                    //
//                                                                                          //
//////////////////////////////////////////////////////////////////////////////////////////////

void SetGlobalPrefs(const PrefsSerz & theSerz)
{
	InterlockedExchange(&gTimeout, (LONG) theSerz.GetTimeoutInit() );
}


//////////////////////////////////////////////////////////////////////////////////////////////
//    Hook_GetMsg_ContextMenu                                                               //
//                                                                                          //
//    <TODO>                                                                                //
//////////////////////////////////////////////////////////////////////////////////////////////

LRESULT CALLBACK Hook_GetMsg_ContextMenu(int theCode, WPARAM wParam, LPARAM lParam)
{
	UnhookWindowsHookEx(gHook);

	if (gHook != 0)
	{
		gHook = 0;
		InterProcessContextMenuInfo::ReadAndExecuteFromSharedMemory<MemoryWriter_NoCrt>();
	}

	return CallNextHookEx(NULL, theCode, wParam, lParam);
}


//////////////////////////////////////////////////////////////////////////////////////////////
//    ExecuteContextMenu                                                                    //
//                                                                                          //
//    Executes a Contextual Menu command in the address space of the process that owns the  //
//    host window. This is essential, for example, for the folders to open in the same      //
//    window.                                                                               //
//    This function does the following:                                                     //
//      1. Writes Context menu data to the shared memory space                              //
//      2. Installs a WH_GETMESSAGE thread hook                                             //
//      3. Posts a message to the thread to ensure the hook will be executed shortly        //
//                                                                                          //
//    This function is exportable (as set in the Module Definition file)                    //
//                                                                                          //
//////////////////////////////////////////////////////////////////////////////////////////////

void ExecuteContextMenu(DWORD theThreadID, InterProcessContextMenuInfo * theInfo)
{
	InterProcessContextMenuInfo::WriteToSharedMemory<MemoryWriter_NoCrt>(theInfo);
	gHook = SetWindowsHookEx(WH_GETMESSAGE, Hook_GetMsg_ContextMenu, gInstance, theThreadID);
	PostThreadMessage(theThreadID, WM_NULL, 0, 0);
}


//////////////////////////////////////////////////////////////////////////////////////////////
//    Hook_Title
//
//    <TODO>
//    This function is exportable (as set in the Module Definition file)
//
//////////////////////////////////////////////////////////////////////////////////////////////

LRESULT CALLBACK Hook_Title(int theCode, WPARAM wParam, LPARAM lParam)
{
	switch (theCode)
	{
	case HC_ACTION:
		{
			if (wParam != WM_NCLBUTTONDOWN) break;

			const MOUSEHOOKSTRUCT * aS = (const MOUSEHOOKSTRUCT *) lParam;

			if (aS->wHitTestCode != HTCAPTION) break;

			SHORT aKeyState = GetKeyState(VK_CONTROL);

			if ( (aKeyState & 0x8000) == 0 ) break;

			TitlebarMenus_ProcessTitleBarClick(aS->hwnd, aS->pt.x, aS->pt.y);
		}
		break;

	default:
		break;
	}
	return CallNextHookEx(NULL, theCode, wParam, lParam);
}

//////////////////////////////////////////////////////////////////////////////////////////////

LRESULT CALLBACK Hook_Menu_Post(int theCode, WPARAM wParam, LPARAM lParam)
{
	switch (theCode)
	{
	case HC_ACTION:
		MenuActivity_DoProcess_Post(wParam, lParam);
		break;

	default:
		break;
	}
	return CallNextHookEx(NULL, theCode, wParam, lParam);
}

//////////////////////////////////////////////////////////////////////////////////////////////

LRESULT CALLBACK Hook_Menu_Get(int theCode, WPARAM wParam, LPARAM lParam)
{
	switch (theCode)
	{
	case HC_ACTION:
		MenuActivity_DoProcess_Get(wParam, lParam);
		break;

	default:
		break;
	}
	return CallNextHookEx(NULL, theCode, wParam, lParam);
}
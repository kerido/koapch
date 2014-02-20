#include "Common.h"

#include <shlobj.h>

#include "Main.h"
#include "UtilPidlFunc.h"
#include "Trace.h"

//////////////////////////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG
#  define EntryPoint DllMain
#else
#  define EntryPoint DllMainCRTStartup
#endif //_DEBUG



//////////////////////////////////////////////////////////////////////////////////////////////
//                                       Global data                                        //
//////////////////////////////////////////////////////////////////////////////////////////////

//! Stores a pointer to the Activator function. Because different implementations exist
//! depending on the current OS version (other factors my come into play in the future)
//! this pointer is initialized during DLL loading phase.
Activator        gFnActivator = 0;

//! Stores a pointer to the Context Menu Handler function
GetCtxHandler    gFnGetCtxMenuHandler = 0;

//! TODO
HANDLE           gWaiterThread = 0;

//! TODO
HINSTANCE        gInstance = 0;


//////////////////////////////////////////////////////////////////////////////////////////////
//                                        Shared data                                       //
//                                                                                          //
#pragma data_seg( "FolderMenusShared" )                                                     //
LONG gMustReact = 0;                                                                        //
LONG gTimeout = 0;                                                                          //
HHOOK gHook = 0;                                                                            //
#pragma data_seg()                                                                          //
#pragma comment( linker, "/SECTION:FolderMenusShared,rws" )                                 //
//                                                                                          //
//////////////////////////////////////////////////////////////////////////////////////////////

BOOL OnAttach(HINSTANCE theInstDll)
{
	gInstance = theInstDll;

	FolderMenus_SetReactFlag(FALSE, false);

	OSVERSIONINFO aVer;
	aVer.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

	GetVersionEx(&aVer);
	

	//OSSPECIFIC
	if (aVer.dwMajorVersion == 5 && aVer.dwMinorVersion == 0)     //Win2000
	{
		gFnActivator         = Common_Activate_Win2K;          //does not support IFolderView
		gFnGetCtxMenuHandler = Common_GetCtxHandlerWndFromShellView_Win2kXp2003;
	}

	else if (aVer.dwMajorVersion == 5 && aVer.dwMinorVersion > 0)	//WinXp, Win2003, Win2003 R2, Win MCE
	{
		gFnActivator         = Common_Activate_WinXP2003VistaWin7;	//supports IFolderView
		gFnGetCtxMenuHandler = Common_GetCtxHandlerWndFromShellView_Win2kXp2003;
	}

	else                                                          //Vista and Windows 7, TODO: Windows Server 2008
	{
		gFnActivator         = Common_Activate_WinXP2003VistaWin7; //supports IFolderView
		gFnGetCtxMenuHandler = Common_GetCtxHandlerWndFromShellView_WinVista;
	}
	//end OSSPECIFIC

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////////

BOOL OnDetach()
{
	if (gWaiterThread == 0)
		return TRUE;

	DWORD aRes = WaitForSingleObject(gWaiterThread, 5000);

	if (aRes == WAIT_TIMEOUT)
		TerminateThread(gWaiterThread, 1);

	CloseHandle(gWaiterThread);

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////////

BOOL WINAPI EntryPoint(HINSTANCE theInstDll, DWORD theReason, void *)
{
	switch(theReason)
	{
	case DLL_PROCESS_ATTACH:
		return OnAttach(theInstDll);

	case DLL_PROCESS_DETACH:
		return OnDetach();
	}

	return TRUE;
}

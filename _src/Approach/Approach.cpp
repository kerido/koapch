#include "stdafx.h"

#include "resource.h"
#include "Trace.h"

#include "LogicCommon.h"
#include "HotspotImpl_NotifyIcon.h"
#include "HotspotImpl_FolderIcon.h"
#include "HotspotImpl_TitlebarMenus.h"
#include "HotspotNew.h"
#include "Application.h"
#include "Framework.h"
#include "RootWindow.h"

#include "util_Warning.h"

//////////////////////////////////////////////////////////////////////////////////////////////

AppModule _Module;

//////////////////////////////////////////////////////////////////////////////////////////////

HRESULT AppModule::Init(ATL::_ATL_OBJMAP_ENTRY * theObjMap, HINSTANCE theInstance,
												const GUID * theLibID/* = NULL*/)
{
	myApplication = new Application();

	// Load preferences
	if ( !myApplication->LoadPrefs() )
		if ( !myApplication->ResetPrefs() )
			return E_FAIL;

	myApplication->Initialize();


	// Load Plug-ins (important to do before toggling hotspots)

	TCHAR aModuleDir[MAX_PATH];
	int aSize = MAX_PATH;
	myApplication->GetFilePathProvider()->GetPath(FilePathProvider::Dir_Plugins, aModuleDir, &aSize);

	Framework::GetPluginManager()->LoadPlugins(aModuleDir, aSize);


	// Create Hotspots, the hotspot manager will release them
	HotspotManager * aHsMan = myApplication->GetHotspotManager();

	HotspotImpl_FolderIcon * aFI = new HotspotImpl_FolderIcon();
	aHsMan->Add(aFI);

	HotspotImpl_NotifyIcon * aNI = new ComInstance<HotspotImpl_NotifyIcon>();
	aHsMan->Add(aNI);

	HotspotImpl_TitlebarMenus * aTM = new HotspotImpl_TitlebarMenus();
	aHsMan->Add(aTM);

	RootWindow * aRootWnd = myApplication->GetRootWindowInstance();
	aRootWnd->WatchAfterHotspots();

	const ApplicationSettings * aPrefs = myApplication->Prefs();

	// Auto-toggle base upon the Preferences
	aHsMan->ToggleHotspot(GUID_Hotspot_FolderMenus,   aPrefs->GetAutoEnableFolderMenus() );
	aHsMan->ToggleHotspot(GUID_Hotspot_ApproachItems, aPrefs->GetAutoEnableKoApproachItems() );
	aHsMan->ToggleHotspot(GUID_Hotspot_TitlebarMenus, aPrefs->GetAutoEnableTitleBar() );


	//base class method
	HRESULT aRes = CAppModule::Init(theObjMap, theInstance, theLibID);

	// Do additional tasks with the Tray Icon
	aRootWnd->ProcessStartupOptions();

	return aRes;
}

//////////////////////////////////////////////////////////////////////////////////////////////

void AppModule::Term()
{
	myApplication->GetRootWindowInstance()->StopWatchingAfterHotspots();

	HotspotManager * aHsMan = myApplication->GetHotspotManager();
	aHsMan->DestroyFeatures();

	delete myApplication;

	CAppModule::Term();
}

//////////////////////////////////////////////////////////////////////////////////////////////

int AppModule::Run(LPTSTR /*lpstrCmdLine = NULL*/, int /*nCmdShow = SW_SHOWDEFAULT*/)
{
	CMessageLoop aLoop;
	AddMessageLoop(&aLoop);

#ifdef _ANTI_DEBUGGER_PROTECT
	ProtectionThread aThr;
#endif

	int aRet = aLoop.Run();

	RemoveMessageLoop();

	return aRet;
}


//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

extern "C"
int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE, LPTSTR lpstrCmdLine, int nCmdShow)
{
	HWND aRootWnd = RootWindow::FindSingleInstance();

	if (aRootWnd != 0)
	{
		RootWindow::NotifyAnotherInstance();
		return 0;
	}

	HRESULT hRes = ::CoInitializeEx(NULL, COINIT_APARTMENTTHREADED/*COINIT_MULTITHREADED*/);


	ATLASSERT(SUCCEEDED(hRes));

	// this resolves ATL window thunking problem when Microsoft Layer for Unicode (MSLU) is used
	::DefWindowProc(NULL, 0, 0, 0L);

	AtlInitCommonControls(ICC_BAR_CLASSES);	// add flags to support other controls

	hRes = _Module.Init(NULL, hInstance);

	int aRet = 1;

	if ( SUCCEEDED(hRes) )
		aRet = _Module.Run(lpstrCmdLine, nCmdShow);

	_Module.Term();
	::CoUninitialize();

	return aRet;
}


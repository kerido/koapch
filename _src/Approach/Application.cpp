#include "stdafx.h"

#include "sdk_ItemFactory.h"
#include "sdk_Item.h"

#include "Application.h"
#include "DlgOptionsPropPages.h"
#include "IpcModule.h"
#include "ShellItemEnumerator.h"
#include "ItemContextMenuHandlerDefault.h"
#include "MenusetImpl_Run.h"
#include "MenuSetImpl_RunningProcesses.h"
#include "StandardPropertyPages.h"
#include "ShellItemFactory.h"
#include "UxManager.h"
#include "RootWindow.h"
#include "ShellIconCache.h"
#include "MenuManager.h"
#include "ThemeDefault.h"
#include "PreferencesPersistence.h"
#include "MenuBuilder_Shell.h"
#include "ShellItemGenericIconManager.h"

//////////////////////////////////////////////////////////////////////////////////////////////


/*!

\mainpage KO Approach

\section intro Introduction

\section architecture Program Architecture

\section shareware Implementation of Shareware Limitations

\section plugins Creating Plugins

*/

//////////////////////////////////////////////////////////////////////////////////////////////

Application * Application::ourInstance = 0;

#ifdef _DEBUG
int Application::ourGeneration = -1;
#endif

//////////////////////////////////////////////////////////////////////////////////////////////

const Application & Application::InstanceC()
{
	ATLASSERT(ourInstance != 0);
	return *ourInstance;
}

//////////////////////////////////////////////////////////////////////////////////////////////

Application & Application::Instance()
{
	ATLASSERT(ourInstance != 0);
	return *ourInstance;
}

//////////////////////////////////////////////////////////////////////////////////////////////

Application::Application() : myStub(this), myFilePathPrv( _Module.GetModuleInstance() )
{
	myIpcModule               = new IpcModule;


#ifdef _DEBUG
	ourGeneration++;	//2
#endif

	myRootWindow              = new RootWindow;
	myUxManager               = new UxManager;
	myStandardPropPages       = new StandardPropPages;

#ifdef _DEBUG
	ourGeneration++; //3
#endif


	myIconCache               = new ShellIconCacheImplNew;
	myItemCtxMenuHandler      = new ItemContextMenuHandlerDefault;
	myHSManager               = new HotspotManager;
	myMenuBuilderDefault      = new MenuBuilder_Shell;
	myShellItemGenericIconMan = new ShellItemGenericIconManager;

#ifdef _DEBUG
	ourGeneration++; //4
#endif

	//TEMP: Add factories
	myItemFactories[OBJ_RunItem]   = new ComInstance<RunItemFactory>();
	myItemFactories[OBJ_ProcItem]  = new ComInstance<ItemFactory_Processes>();
	//end TEMP

	RegisterContextMenuExtension(&myShellExt, _Module.GetModuleInstance() );
}

//////////////////////////////////////////////////////////////////////////////////////////////

Application::~Application()
{
	//TODO: remove these
	MenuManager::Dispose();
	Framework::UnIninitialize();


	UnregisterContextMenuExtension(&myShellExt);

	//delete myItemFactories[OBJ_RunItem];

#ifdef _DEBUG
	ourGeneration--; //3
#endif

	delete myShellItemGenericIconMan;
	delete myMenuBuilderDefault;
	delete myHSManager;
	delete myItemCtxMenuHandler;
	delete myIconCache;

#ifdef _DEBUG
	ourGeneration--; //2
#endif

	delete myStandardPropPages;
	delete myUxManager;
	delete myRootWindow;

#ifdef _DEBUG
	ourGeneration--;	//1
#endif

	delete myIpcModule;

	ReleaseTraceCode(PROGRAM_QUIT);
}

//////////////////////////////////////////////////////////////////////////////////////////////

void Application::Initialize()
{
	// TODO: remove these two
	Framework::InitializeNew();
	MenuManager::Init( new ThemeDefault() );
}

//////////////////////////////////////////////////////////////////////////////////////////////

bool Application::LoadPrefs()
{
	ApplicationSettingsRegistrySerializer aSer;
	bool aRes = aSer.Load(&myPrefs);

	if (!aRes) return false;


#ifdef _USE_RELEASE_TRACE

	if ( myPrefs->GetUseReleaseLogging() )
		myLogger->InitReleaseTrace();

#endif	//_USE_RELEASE_TRACE


	ReleaseTraceCode(PROGRAM_LAUNCHED);

	ReportPrefEvent(IPreferencesEventProcessor::PREFEVENT_LOAD);
	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////

bool Application::SavePrefs()
{
	ApplicationSettingsRegistrySerializer aSer;
	return aSer.Save(&myPrefs);
}

//////////////////////////////////////////////////////////////////////////////////////////////

bool Application::SetPrefs(const ApplicationSettings & thePrefs)
{
	myPrefs->Set(thePrefs);

	if ( !SavePrefs() )
		return false;

	ReportPrefEvent(IPreferencesEventProcessor::PREFEVENT_CHANGE);
	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////

bool Application::ResetPrefs()
{
	myPrefs->SetDefault();

	if ( !SavePrefs() )
		return false;

	ReportPrefEvent(IPreferencesEventProcessor::PREFEVENT_RESET);
	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////

void Application::ReportPrefEvent( IPreferencesEventProcessor::PreferencesEvent theEvent )
{
	for (int i = 0; i < myPrefEventProcessors->GetSize(); i++)
	{
		IPreferencesEventProcessor * aIt = myPrefEventProcessors->operator [](i);
		aIt->OnPreferencesEvent(theEvent, &myPrefs);
	}

	UpdateShellItemFactory();
}

//////////////////////////////////////////////////////////////////////////////////////////////

void Application::RemovePreferencesEventProcessor( IPreferencesEventProcessor * theProcessor )
{
	int aIndex = myPrefEventProcessors->Find(theProcessor);

	if (aIndex != -1)
		myPrefEventProcessors->RemoveAt(aIndex);
}

//////////////////////////////////////////////////////////////////////////////////////////////

void Application::RegisterPropPageFactory( IPropPageFactory * theFactory )
{
	myPropPageFactories.Add(theFactory);
}

//////////////////////////////////////////////////////////////////////////////////////////////

void Application::UnregisterPropPageFactory( IPropPageFactory * theFactory )
{
	int aIndex = myPropPageFactories.Find(theFactory);

	if (aIndex != -1)
		myPropPageFactories.RemoveAt(aIndex);
}

//////////////////////////////////////////////////////////////////////////////////////////////

void Application::AddKoApproachItemsRoot( const TCHAR * theRoot )
{
	if ( myPrefs->AddKoApproachItemsRoot(theRoot) )
		ReportPrefEvent(IPreferencesEventProcessor::PREFEVENT_CHANGE);
}

//////////////////////////////////////////////////////////////////////////////////////////////

void Application::AddPreferencesEventProcessor( IPreferencesEventProcessor * theProcessor )
{
#ifdef _DEBUG
	ATLASSERT(ourGeneration >= 0);
#endif

	myPrefEventProcessors->Add(theProcessor);
}

//////////////////////////////////////////////////////////////////////////////////////////////

int Application::GetItemListCount()
{
	return (int) myItemLists.size();
}

//////////////////////////////////////////////////////////////////////////////////////////////

void Application::GetItemListData( int theIndex, ObjectData * theData )
{
	//TEMP
	ItemListIter aIt = myItemLists.begin();
	for (int i = 0; aIt != myItemLists.end() && i < theIndex; aIt++, i++);

	aIt->second->GetTypeGuid(&theData->ObjectID);
	theData->Module = 0;	//TODO
}

//////////////////////////////////////////////////////////////////////////////////////////////

IItemList * Application::GetItemList(REFGUID theListID)
{
	ItemListIter aIt = myItemLists.find(theListID);

	if (aIt == myItemLists.end() )
		return 0;

	else
		return aIt->second;
}

//////////////////////////////////////////////////////////////////////////////////////////////

void Application::RegisterItemList( REFGUID theListID, IItemList * theList, HMODULE theModule )
{
	myItemLists[theListID] = theList;
}

//////////////////////////////////////////////////////////////////////////////////////////////

void Application::UnregisterItemList( REFGUID theListID, IItemList * theList )
{
	myItemLists.erase(theListID);
}

//////////////////////////////////////////////////////////////////////////////////////////////

int Application::GetItemFactoryCount()
{
	return (int) myItemFactories.size();
}

//////////////////////////////////////////////////////////////////////////////////////////////

void Application::GetItemFactoryData( int theIndex, ObjectData * theData )
{
	//TEMP
	ItemFactoryIter aIt = myItemFactories.begin();
	for (int i = 0; aIt != myItemFactories.end() && i < theIndex; aIt++, i++);

	aIt->second->GetTypeGuid(&theData->ObjectID);
	theData->Module = 0;	//TODO
}

//////////////////////////////////////////////////////////////////////////////////////////////

IItemFactory * Application::GetItemFactory(REFGUID theListID)
{
	ItemFactoryIter aIt = myItemFactories.find(theListID);

	if (aIt == myItemFactories.end() )
		return 0;

	else
		return aIt->second;
}

//////////////////////////////////////////////////////////////////////////////////////////////

void Application::RegisterItemFactory( REFGUID theFactID, IItemFactory * theFact, HMODULE theModule )
{
	myItemFactories[theFactID] = theFact;
}

//////////////////////////////////////////////////////////////////////////////////////////////

void Application::RegisterContextMenuExtension(IContextMenuExtension * theExt, HMODULE theModule)
{
	bool aMyInstance = ( theModule ==  (HMODULE) _Module.GetModuleInstance() );
	ContextMenuExtensions & aStore = myContextMenuExtensions;


	// if aMyInstance is TRUE, the extension must be registered prior to any
	// other extension with Module Instance not equal to m_hInst

	if ( aMyInstance )
	{
		bool aWasInserted = false;

		ContextMenuExtensionData aTemp(theExt, theModule);

		for ( int i = 0; i != aStore.GetSize(); i++)
		{
			if ( aWasInserted )
			{
				ContextMenuExtensionData aTempTemp = aStore[i];
				aStore[i] = aTemp;
				aTemp = aTempTemp;
			}
			else if ( aStore[i].Module != _Module.GetModuleInstance() )
			{
				ContextMenuExtensionData aTempTemp = aStore[i];
				aStore[i] = aTemp;
				aTemp = aTempTemp;

				aWasInserted = true;
			}
		}

		aStore.Add(aTemp);
	}
	else
		myContextMenuExtensions.Add( ContextMenuExtensionData(theExt, theModule) );
}

//////////////////////////////////////////////////////////////////////////////////////////////

void Application::UnregisterContextMenuExtension(IContextMenuExtension * theExt)
{
	int aIndex = -1;

	for ( int i = 0; i != myContextMenuExtensions.GetSize(); i++ )
		if ( myContextMenuExtensions[i].Object == theExt)
		{ aIndex = i; break; }

		if (aIndex != -1)
			myContextMenuExtensions.RemoveAt(aIndex);
}

//////////////////////////////////////////////////////////////////////////////////////////////

void Application::ReleaseContextMenuExtensions( HMODULE theModule )
{
	for ( int i = 0; i != myContextMenuExtensions.GetSize(); )
	{
		if ( myContextMenuExtensions[i].Module == theModule )
		{
			myContextMenuExtensions[i].Object->Release();
			myContextMenuExtensions.RemoveAt(i);
		}
		else
			i++;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////

IpcModule * Application::GetIpcModule() const
{
#ifdef _DEBUG
	ATLASSERT(ourGeneration >= 2);
#endif

	return myIpcModule;
}

//////////////////////////////////////////////////////////////////////////////////////////////

IItemContextMenuHandler * Application::GetItemContextMenuHandler() const
{
#ifdef _DEBUG
	ATLASSERT(ourGeneration >= 4);
#endif

	return myItemCtxMenuHandler;
}

//////////////////////////////////////////////////////////////////////////////////////////////

IIconCache * Application::GetIconCache()
{
#ifdef _DEBUG
	ATLASSERT(ourGeneration >= 4);
#endif

	return myIconCache;
}

//////////////////////////////////////////////////////////////////////////////////////////////

IShellItemFactory * Application::GetShellItemFactory()
{
	return mySFF.GetObject();
}

//////////////////////////////////////////////////////////////////////////////////////////////

IMenuBuilder * Application::GetMenuBuilder( Item * theTarget, UINT & theOutExpansionCode )
{
	UINT aTemp = 0;
	HRESULT aRes = myMenuBuilderDefault->CanBuild(theTarget, &aTemp);

	if ( SUCCEEDED(aRes) && aTemp != 0 )
	{ theOutExpansionCode = aTemp; return myMenuBuilderDefault; }

	// query the menu builders that might be initialized from external sources, i.e. plugins
	for (MenuBuilderIter aIt = myMenuBuilders.begin(); aIt != myMenuBuilders.end(); aIt++)
	{
		aRes = aIt->Object->CanBuild(theTarget, &aTemp);

		if ( SUCCEEDED(aRes) && aTemp != 0 )
		{ theOutExpansionCode = aTemp; return aIt->Object; }
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////

void Application::RegisterMenuBuilder(IMenuBuilder * theBuilder, HMODULE theModule)
{
	myMenuBuilders.push_back( PluginMenuBuilderData(theBuilder, theModule) );
}

//////////////////////////////////////////////////////////////////////////////////////////////

void Application::UnregisterMenuBuilder( IMenuBuilder * theBuilder )
{
	for (MenuBuilderIter aIt = myMenuBuilders.begin(); aIt != myMenuBuilders.end(); )
	{
		if ( aIt->Object == theBuilder )
			aIt = myMenuBuilders.erase(aIt);
		else
			aIt++;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////

void Application::ReleaseMenuBuilders( HMODULE theModule )
{
	for (MenuBuilderIter aIt = myMenuBuilders.begin(); aIt != myMenuBuilders.end(); )
	{
		if ( aIt->Module == theModule )
		{
			aIt->Object->Release();
			aIt = myMenuBuilders.erase(aIt);
		}
		else
			aIt++;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////

UxManager * Application::GetUxManager() const
{
#ifdef _DEBUG
	ATLASSERT(ourGeneration >= 3);
#endif

	return myUxManager;
}

//////////////////////////////////////////////////////////////////////////////////////////////

HWND Application::GetRootWindow() const
{
#ifdef _DEBUG
	ATLASSERT(ourGeneration >= 3);
#endif

	return myRootWindow->m_hWnd;
}

//////////////////////////////////////////////////////////////////////////////////////////////

RootWindow * Application::GetRootWindowInstance()
{
#ifdef _DEBUG
	ATLASSERT(ourGeneration >= 3);
#endif

	return myRootWindow;
}

//////////////////////////////////////////////////////////////////////////////////////////////

const RootWindow * Application::GetRootWindowInstance() const
{
#ifdef _DEBUG
	ATLASSERT(ourGeneration >= 3);
#endif
	return myRootWindow;
}

//////////////////////////////////////////////////////////////////////////////////////////////

HotspotManager * Application::GetHotspotManager()
{
#ifdef _DEBUG
	ATLASSERT(ourGeneration >= 4);
#endif

	return myHSManager;
}

//////////////////////////////////////////////////////////////////////////////////////////////

const ShellItemGenericIconManager * Application::GetShellItemGenericIconManager() const
{
#ifdef _DEBUG
	ATLASSERT(ourGeneration >= 4);
#endif

	return myShellItemGenericIconMan;
}

//////////////////////////////////////////////////////////////////////////////////////////////

ShellItemGenericIconManager * Application::GetShellItemGenericIconManager()
{
#ifdef _DEBUG
	ATLASSERT(ourGeneration >= 4);
#endif

	return myShellItemGenericIconMan;
}

//////////////////////////////////////////////////////////////////////////////////////////////

void Application::RegisterLocalizationEventProcessor(ILocalizationEventProcessor * theProcessor)
{
	myProcessors.push_back(theProcessor);
}

//////////////////////////////////////////////////////////////////////////////////////////////

void Application::UnregisterLocalizationEventProcessor(ILocalizationEventProcessor * theProcessor)
{
	LocalizationEventProcessorIter aIt = std::find(
		myProcessors.begin(), myProcessors.end(), theProcessor);

	if ( aIt != myProcessors.end() )
		myProcessors.erase(aIt);
}

//////////////////////////////////////////////////////////////////////////////////////////////

void Application::RaiseLocalizationChanged( ILocalization * theLoc, const TCHAR * theLocale, int theSize_Locale )
{
	for
	(
		LocalizationEventProcessorIter aLocEventIter = myProcessors.begin();
		aLocEventIter != myProcessors.end();
		aLocEventIter++
	)
		(*aLocEventIter)->OnLocalizationChanged(theLoc, theLocale, theSize_Locale);
}

//////////////////////////////////////////////////////////////////////////////////////////////

void Application::UpdateShellItemFactory()
{
	OSVERSIONINFO aVer;
	aVer.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&aVer);

	int aTp = Unknown;


	//OSSPECIFIC
	if (aVer.dwMajorVersion == 5 && aVer.dwMinorVersion == 0)          // 2000
		aTp = ZipsAsFolders;
	else                                                               // every other, including Vista
		aTp = myPrefs->GetTreatZipArchivesAsFolders() ? ZipsAsFolders : ZipsAsFiles;

	if ( myPrefs->GetTreatFolderLinksAsFolders() )
		aTp |= FolderShortcutsAreFolders;
	else
		aTp |= FolderShortcutsAreFiles;


	if ( aTp == mySFF.GetType() )
		return;

	IShellItemFactory * aSFF = mySFF.GetObject();

	if (aSFF != 0)
		delete aSFF;

	if (aTp == ZipsAsFolders)
		aSFF = new ShellItemFactoryImpl_ZipOK( (aTp&FolderShortcutsAreFiles) == 0 );
	else
		aSFF = new ShellItemFactoryImpl_ZipNo( (aTp&FolderShortcutsAreFiles) == 0 );

	Trace("   + Created Shell item factory: %x\n", aSFF);
	mySFF.Set(aTp, aSFF);
}

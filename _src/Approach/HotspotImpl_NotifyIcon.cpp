#include "stdafx.h"

#include "sdk_MemoryWriter.h"
#include "sdk_ItemProvider.h"
#include "sdk_MenuWindow.h"
#include "sdk_ItemFactory.h"

#include "ShellItemFactory.h"
#include "ShellItemEnumerator.h"
#include "HotspotImpl_NotifyIcon.h"
#include "MenuManager.h"
#include "MenuWindow_Shell.h"
#include "Application.h"
#include "RootWindow.h"
#include "MenuSet.h"
#include "ExceptItem.h"
#include "DynamicDisplayItem.h"
#include "DisplayItem_Separator.h"
#include "Trace.h"

#include "UtilShellFunc.h"
#include "util_Preferences.h"


//////////////////////////////////////////////////////////////////////////////////////////////
//                                 MenuWindow_ApproachItems Class                           //
//////////////////////////////////////////////////////////////////////////////////////////////

class MenuWindow_ApproachItems : public MenuWindow_DisplayItemList
{
	// Constructors, destructor
public:
	MenuWindow_ApproachItems(IMenuManagerAgent * theAgent) : MenuWindow_DisplayItemList(theAgent)
	{
		Trace("MenuWindow_ApproachItems::MenuWindow_ApproachItems\n");
	}

	~MenuWindow_ApproachItems()
	{
		Trace("MenuWindow_ApproachItems::~MenuWindow_ApproachItems\n");
	}

	// Methods
public:
	void PopulateAndMeasure(const MenuSet * theSet)
	{
		Application & aApp = Application::Instance();

		myMainItems.OnPopulateStarted();

		ExceptItemType aTp = EMPTY;

		for (int i = 0; i < theSet->GetEntryCount(); i++)
		{
			const MenuSetEntry & aEntry = theSet->GetEntry(i);

			if (aEntry.GetType() == MenuSetEntry::ITEM)
			{
				IItemFactory * aFact = aApp.GetItemFactory( aEntry.GetObjectID() );

				if (aFact == 0)
					continue;

				Item * aItem = 0;
				HRESULT aRes = aFact->CreateItem(aEntry.GetParam(), aEntry.GetParamSize(), &aItem);

				if (FAILED(aRes) || aItem == 0)
					continue;

				DynamicDisplayItem * aDI = new DynamicDisplayItem(aItem);
				myMainItems.AddItem(aDI);
				aTp = SUCCESS;
			}
			/*else if (aEntry.GetType() == MenuSetEntry::DISPITEM)
			{
				IDisplayItemFactory * aFact = aApp.GetDisplayItemFactory( aEntry.GetObjectID() );

				if (aFact == 0)
					continue;

				DisplayItem * aDI = 0;
				HRESULT aRes = aFact->CreateDisplayItem(aEntry.GetParam(), aEntry.GetParamSize(), &aDI);

				if (FAILED(aRes) || aItem == 0)
					continue;

				myMainItems.AddItem(aDI);
				aTp = SUCCESS;
			}*/
			else if (aEntry.GetType() == MenuSetEntry::LIST)
			{
				IItemList * aList = aApp.GetItemList( aEntry.GetObjectID() );

				if (aList == 0)
					continue;

				ULONG aOptions = PreferencesUtility::GetEnumItemsOptions();
				IEnumItems * aEnum = 0;

				HRESULT aRes = aList->EnumItems(aEntry.GetParam(), aEntry.GetParamSize(), aOptions, &aEnum);

				if (FAILED(aRes) || aEnum == 0)
					continue;

				while (true)
				{
					ULONG aNumItems = 0UL;
					Item * aItem = 0;

					HRESULT aRes = aEnum->Next(1, &aItem, &aNumItems);

					if (aRes != S_OK || aItem == 0 || aNumItems < 1UL)
						break;

					DisplayItem * aDI = new DynamicDisplayItem(aItem);
					myMainItems.AddItem(aDI);
					aTp = SUCCESS;

					//TODO: encapsulate shell notifications
				}

				aEnum->Release();
			}
			else if (aEntry.GetType() == MenuSetEntry::SEPARATOR)
			{
				DisplayItem * aDI = new DisplayItem_Separator();
				myMainItems.AddItem(aDI);
				aTp = SUCCESS;
			}
		}

		if (aTp != SUCCESS)
		{
			DisplayItem * aDI = new ExceptItem(aTp);
			myMainItems.AddItem(aDI);
		}

		myMainItems.OnPopulateFinished();

		InitialLayout();
		OnPostPopulate();
	}
};


//////////////////////////////////////////////////////////////////////////////////////////////
//                     HotspotImpl_NotifyIcon::ContextMenuExecutor Class                    //
//////////////////////////////////////////////////////////////////////////////////////////////

class HotspotImpl_NotifyIcon::ContextMenuExecutor : public ComEntry1<IContextMenuExecutor>
{
// Fields
private:
	UINT myMinID;

	IShellFolder * mySF;
	LPITEMIDLIST myPidl;

	IShellFolder * myFullSF;
	LPITEMIDLIST myFullPidl;

	HotspotImpl_NotifyIcon * myFeature;


// Constructors, destructor
public:
	ContextMenuExecutor(IApproachShellItem * theItem, HotspotImpl_NotifyIcon * theFeature) : myFeature(theFeature)
	{
		theItem->GetShellItemData(IApproachShellItem::PRIMARY_RELATIVE, &mySF, &myPidl);
		theItem->GetShellItemData(IApproachShellItem::PRIMARY_FULL, &myFullSF, &myFullPidl);
	}

	~ContextMenuExecutor()
	{
		mySF->Release();
		CoTaskMemFree(myPidl);

		myFullSF->Release();
		CoTaskMemFree(myFullPidl);
	}


// IContextMenuExecutor Members
public:
	STDMETHODIMP PopulateMenu(HMENU theOut, UINT theMinID, UINT theMaxID, ULONG theFlags)
	{
		myMinID = theMinID;

		LocalizationManagerPtr aMan;

		TCHAR aString[1000];
		int aSize = sizeof aString / sizeof TCHAR;

		HRESULT aRes = aMan->GetStringSafe( KEYOF(IDP_CXMN_TOAPPROACHITEMS), aString, &aSize);

		if (FAILED(aRes) )
			return aRes;

		AppendMenu(theOut, MF_STRING, myMinID, aString );

		return MAKE_HRESULT(0, 0, 1);
	}

	STDMETHODIMP InvokeCommand(UINT theID, HWND theOwner, ULONG theExt)
	{
		if ( theID == myMinID )
		{
			CreateShortcut();
			return S_OK;
		}
		else
			return E_INVALIDARG;
	}

	STDMETHODIMP HandleMessage(UINT theMsg, WPARAM theWParam, LPARAM theLParam, LRESULT * theRes)
	{
		*theRes = 0;
		return S_OK;
	}


// Implementation Details
private:
	void CreateShortcut()
	{
		IShellLink * aSL = 0;

		TCHAR aDirectory[MAX_PATH];
		TCHAR aPath[MAX_PATH];
		TCHAR aDisplayName[MAX_PATH];

		//EnsureDirectoryExists(myDestPath);

		GUID aGuid = CLSID_ShellLink;
		HRESULT aRes = SHCoCreateInstance(NULL, &aGuid, NULL, IID_IShellLink, (void **) &aSL);

		if ( SUCCEEDED(aRes) && aSL != 0)
		{
			aRes = aSL->SetIDList(myFullPidl);

			UtilShellFunc::GetDirectoryName(mySF, myPidl, aDirectory, MAX_PATH);

			aRes = aSL->SetWorkingDirectory(aDirectory);

			SHORT aKeyState = GetKeyState(VK_CONTROL);

			//if CTRL key is pressed, add as _Target	
			if ( (aKeyState & 0x8000) == 0 )
				UtilShellFunc::GetDisplayName(mySF, myPidl, SHGDN_FOREDITING, aDisplayName, MAX_PATH);
			else
				lstrcpy(aDisplayName, _T("_Target" ) );



			lstrcpy(aPath, myFeature->GetApproachItemsPath(false) );
			lstrcat(aPath, _T("\\") );
			lstrcat(aPath, aDisplayName);
			lstrcat(aPath, _T(".lnk") );

			IPersistFile * aPF = 0;
			aRes = aSL->QueryInterface(IID_IPersistFile, (void **) &aPF);

			if ( SUCCEEDED(aRes) && aPF != 0)
			{
				myFeature->EnsureDirectoryExists();

				aRes = aPF->Save( aPath, FALSE );
				aRes = aPF->Release();
			}

			aSL->Release();
		}
	}
};

//////////////////////////////////////////////////////////////////////////////////////////////
//                               HotspotImpl_NotifyIcon Members                             //
//////////////////////////////////////////////////////////////////////////////////////////////

HotspotImpl_NotifyIcon::HotspotImpl_NotifyIcon() :
	myItem(0), myApproachItems(0), myDesktopSF(0), myApproachItemsPidl(0),
	myChangeNotifyID_Main(0), myHasTargetItem(false)
{
	Application & aApp = Application::Instance();
	myRootWindow = aApp.GetRootWindowInstance();

	myTimerID_ProcessApproachItemsChange = myRootWindow->GetAvailableTimerID();

	// initialize myFolderPath
	SecureZeroMemory(myFolderPath, sizeof(TCHAR) * MAX_PATH);

	SHGetSpecialFolderPath(NULL, myFolderPath, CSIDL_APPDATA, FALSE);
	lstrcat(myFolderPath, _T("\\KO Approach Items") );

	aApp.RegisterItemList(GUID_Hotspot_ApproachItems, this, NULL);
}

//////////////////////////////////////////////////////////////////////////////////////////////

HotspotImpl_NotifyIcon::~HotspotImpl_NotifyIcon()
{
	Application::Instance().UnregisterItemList(GUID_Hotspot_ApproachItems, this);

	Disable();

	if ( myItem != 0)
		delete myItem;

}

//////////////////////////////////////////////////////////////////////////////////////////////

void HotspotImpl_NotifyIcon::Enable()
{
	EnsureDirectoryExists();


	HRESULT aRes = SHGetDesktopFolder(&myDesktopSF);

	if ( FAILED(aRes) )
	{
		Trace("Unable to query the Desktop folder\n");
		return;
	}

	aRes = myDesktopSF->ParseDisplayName(NULL, NULL, myFolderPath, NULL, &myApproachItemsPidl, NULL);

	if ( FAILED(aRes) )
	{
		Trace("Unable to query the Approach Items folder\n");
		return;
	}

	aRes = myDesktopSF->BindToObject(myApproachItemsPidl, NULL, IID_IShellFolder, (void**) &myApproachItems);

	if ( FAILED(aRes) )
	{
		Trace("Unable to query the Approach Items folder\n");
		return;
	}


	// Monitor Approach Items folder
	if (myApproachItemsPidl != 0)
		myChangeNotifyID_Main = UtilShellNotify::ChangeNotifyRegister
		(
			myApproachItemsPidl,
			*myRootWindow,
			WM_USER_SHELLNOTIFY_APPROACHITEMS,
			true
		);


	UpdateEffectiveItem();


	Application::Instance().RegisterContextMenuExtension(this, _Module.GetModuleInstance() );
	myRootWindow->AddMessageMap(this);
}

//////////////////////////////////////////////////////////////////////////////////////////////

void HotspotImpl_NotifyIcon::Disable()
{
	myRootWindow->RemoveMessageMap(this);
	Application::Instance().UnregisterContextMenuExtension(this);

	if (myChangeNotifyID_Main != 0)
	{
		SHChangeNotifyDeregister(myChangeNotifyID_Main);
		myChangeNotifyID_Main = 0;
	}


	//release Shell interfaces

	if (myApproachItems != 0)
	{
		myApproachItems->Release();
		myApproachItems = 0;
	}

	if (myDesktopSF != 0)
	{
		myDesktopSF->Release();
		myDesktopSF = 0;
	}

	if (myApproachItemsPidl != 0)
	{
		CoTaskMemFree(myApproachItemsPidl);
		myApproachItemsPidl = 0;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////

void HotspotImpl_NotifyIcon::OnToggle(bool theEnable)
{
	Hotspot::OnToggle(theEnable);

	if (myEnabled)
		Enable();
	else
		Disable();
}

//////////////////////////////////////////////////////////////////////////////////////////////

void HotspotImpl_NotifyIcon::EnsureDirectoryExists()
{
	HANDLE aDir = CreateFile
	(
		myFolderPath,
		GENERIC_READ,
		FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,
		NULL,
		OPEN_EXISTING,
		FILE_FLAG_BACKUP_SEMANTICS,
		NULL
	);

	if (aDir != INVALID_HANDLE_VALUE)
		CloseHandle(aDir);

	else
	{
		CreateDirectory(myFolderPath, NULL);
	}

	Application::Instance().AddKoApproachItemsRoot(myFolderPath);
}

//////////////////////////////////////////////////////////////////////////////////////////////

void HotspotImpl_NotifyIcon::ShowItems()
{
	EnsureDirectoryExists();
	ShellExecute(NULL, L"open", myFolderPath, NULL, NULL, SW_SHOW);
}

//////////////////////////////////////////////////////////////////////////////////////////////

void HotspotImpl_NotifyIcon::ShowMenu()
{
	MenuWindow_ApproachItems * aWnd = new MenuWindow_ApproachItems( MenuManager::GetAgent() );
	aWnd->SetTheme( MenuManager::GetTheme() );

	const MenuSet * aSet = Application::InstanceC().Prefs()->GetApproachItemsSet();
	aWnd->PopulateAndMeasure(aSet);


	RECT aRct;
	myRootWindow->GetCursorPositionForMenu((POINT *) &aRct);
	aRct.right  = 0;
	aRct.bottom = 0;


	MenuManager::SetFGWindowMode(0);
	MenuManager::SetIpcShellViewWnd(NULL);
	MenuManager::AdjustPosition(aWnd, &aRct, MenuManager::POS_DEFAULT);
}

//////////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP HotspotImpl_NotifyIcon::QueryExecutor(Item * theItem, ULONG, ULONG, IContextMenuExecutor ** theOut)
{
	if (theItem == 0)
		return E_INVALIDARG;

	else if (theOut == 0)
		return E_POINTER;

	CComQIPtr<IApproachShellItem, &IID_IApproachShellItem> aItem(theItem);

	if (aItem != 0)
	{
		*theOut = new ComInstance<ContextMenuExecutor>(aItem, this);
		return S_OK;
	}

	return E_FAIL;
}

//////////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP HotspotImpl_NotifyIcon::EnumItems( const BYTE * theData, int theDataSize, ULONG theOptions, IEnumItems ** theOutEnum )
{
	if (myItem == 0)
		return E_FAIL;

	CComQIPtr<IItemProvider> aIP( static_cast<Item*>(myItem) );

	if (aIP == 0)
		return E_FAIL;

	return aIP->EnumItems(theOptions, theOutEnum);
}

//////////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP HotspotImpl_NotifyIcon::GetNumItems( int * theOut )
{
	if (theOut == 0)
		return E_POINTER;

	*theOut = 1;
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP HotspotImpl_NotifyIcon::GetItemData( int theIndex, MenuSetItemData * theOut )
{
	if (theIndex != 0)
		return E_INVALIDARG;

	HRESULT aRes = GetItemName(theOut, theOut->Name, theOut->NameSize);

	theOut->Type = MenuSetEntry::LIST;
	theOut->ObjectID = GUID_Hotspot_ApproachItems;
	theOut->Param = 0;
	theOut->ParamSize = 0;
	theOut->IconIndex = -1;

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP HotspotImpl_NotifyIcon::GetItemName( const MenuSetItemData * theItem, TCHAR * theOut, int theBufSize )
{
	if (theOut == 0)
		return E_POINTER;

	if (theItem->Size != sizeof MenuSetItemData)
		return E_INVALIDARG;

	if (theBufSize < 18)
		return E_OUTOFMEMORY;

	lstrcpyn(theOut, _T("KO Approach Items"), 18);
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP HotspotImpl_NotifyIcon::GetTypeGuid( LPGUID theOutGuid )
{
	if ( theOutGuid == 0)
		return E_POINTER;

	GetGuid(*theOutGuid);
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////////////////////////

void HotspotImpl_NotifyIcon::UpdateEffectiveItem()
{
	if (myItem != 0)
		delete myItem;


	EnsureDirectoryExists();

	//VISTA-ready code
	CComPtr<IEnumIDList> aLst;
	myApproachItems->EnumObjects(NULL, SHCONTF_FOLDERS|SHCONTF_NONFOLDERS, &aLst);


	LPITEMIDLIST aItemPidl = 0; //released only if the is no _Target.lnk. Otherwise it is used for item creation.
	ULONG aNumFetched = 0;
	myHasTargetItem = false;

	HRESULT aFetchStatus = aLst->Next(1, &aItemPidl, &aNumFetched);

	while ( SUCCEEDED(aFetchStatus) && (aNumFetched == 1) )
	{
		TCHAR aDispName[MAX_PATH];
		UtilShellFunc::GetDisplayName(myApproachItems, aItemPidl, SHGDN_FOREDITING, aDispName, MAX_PATH);

		if ( lstrcmp(aDispName, _T("_Target") ) == 0 )
		{
			ULONG aAttributes = SFGAO_LINK;
			HRESULT aRes = myApproachItems->GetAttributesOf(1, (LPCITEMIDLIST *)&aItemPidl, &aAttributes);

			if ( SUCCEEDED(aRes) && (aAttributes & SFGAO_LINK) != 0 )
			{ myHasTargetItem = true; break; }
		}

		CoTaskMemFree(aItemPidl);
		aFetchStatus = aLst->Next(1, &aItemPidl, &aNumFetched);
	}

	if (myHasTargetItem)
	{
		IShellItemFactory * aFact = Application::Instance().GetShellItemFactory();

		ATLASSERT(aFact != 0);

		ShellItem * aItem = aFact->CreateChild(myApproachItems, aItemPidl);

		if (aItem != 0)
		{
			myItem = aItem;
			return;
		}
	}

	myItem = new ShellFolder( UtilPidlFunc::Copy<MemoryWriter_Crt>(myApproachItemsPidl) );
}

//////////////////////////////////////////////////////////////////////////////////////////////

LRESULT HotspotImpl_NotifyIcon::MsgHandler_Timer( UINT theMsg, WPARAM theWParam, LPARAM theLParam, BOOL & theHandled )
{
	if (theWParam == myTimerID_ProcessApproachItemsChange)
	{
		KillTimer(*myRootWindow, myTimerID_ProcessApproachItemsChange);
		UpdateEffectiveItem();
	}
	else
		theHandled = FALSE;

	return 0L;
}

//////////////////////////////////////////////////////////////////////////////////////////////

LRESULT HotspotImpl_NotifyIcon::MsgHandler_ShNotifApchItems( UINT theMsg, WPARAM theWParam, LPARAM theLParam, BOOL & theHandled )
{
	DWORD aCode = (DWORD) theLParam;
	const UtilShellNotify::SHNOTIFYSTRUCT * aShn = (const UtilShellNotify::SHNOTIFYSTRUCT *) theWParam;

#ifdef _DEBUG

	char aBuf[2000];
	UtilShellNotify::OutputChangeNotifyEvent(aCode, NULL, aBuf);
	Trace("HotspotImpl_NotifyIcon::WM_SHELLNOTIFY_APPROACHITEMS. Flags=%s\n", aBuf);

#endif	//_DEBUG

	if  ( UtilShellNotify::CommonCanIgnoreEventByCode(aCode) )
		return 0;


	//actual processing

	bool aNeedToDisable = false;

	if ( (aCode & SHCNE_RMDIR)        != 0 ||
		(aCode & SHCNE_RENAMEFOLDER) != 0   )
	{
		//check if the user has removed KO Approach Items directory. If so,
		//disable the hotspot, otherwise, just update the window

		LPCITEMIDLIST aPidl1 = (LPCITEMIDLIST) aShn->Item1;

		TCHAR aDisp1 [2000], aDisp2[2000];

		const SHGDNF Flags = SHGDN_NORMAL|SHGDN_FORADDRESSBAR|SHGDN_FORPARSING;

		UtilShellFunc::GetDisplayName(myDesktopSF, aPidl1,              Flags, aDisp1, 2000);
		UtilShellFunc::GetDisplayName(myDesktopSF, myApproachItemsPidl, Flags, aDisp2, 2000);

		aNeedToDisable = (lstrcmpi(aDisp1, aDisp2) == 0 );
	}

	if (aNeedToDisable)
		myRootWindow->SafeToggleHotspot(GUID_Hotspot_ApproachItems);

	else
	{
		SetTimer(*myRootWindow, myTimerID_ProcessApproachItemsChange, 200, NULL);
	}
	//end actual processing

	return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////

LRESULT HotspotImpl_NotifyIcon::MsgHandler_TrayNotify( UINT theMsg, WPARAM theWParam, LPARAM theLParam, BOOL & theHandled )
{
	ShowMenu();
	return 0L;
}
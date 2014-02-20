#include "stdafx.h"

#include "sdk_MemoryWriter.h"
#include "sdk_WindowNavigator.h"
#include "sdk_ComObject.h"

#include "HotspotImpl_TitlebarMenus.h"
#include "Application.h"
#include "UtilShellFunc.h"
#include "MenuManager.h"
#include "ShellFolder.h"
#include "IpcModule.h"
#include "MenuWindow_Shell.h"

//////////////////////////////////////////////////////////////////////////////////////////////

EXTERN_C const TCHAR WndClass_SHELLDLL_DefView[] = _T("CabinetWClass");

#define SUPPORT_CURFOLDER_AS_ITEM

//////////////////////////////////////////////////////////////////////////////////////////////

class ChildToParentEnumerator :
	public Item,
	public ComEntry2<IItemProvider, IEnumItems>
{
private:

#ifdef SUPPORT_CURFOLDER_AS_ITEM

	struct ShellFolderWithPidl
	{
		IShellFolder * SF;
		LPITEMIDLIST   Pidl;

		ShellFolderWithPidl(IShellFolder * theSF = 0, LPITEMIDLIST thePidl = 0)
			: SF(theSF), Pidl(thePidl)
		{ if (SF != 0) SF->AddRef(); }

		ShellFolderWithPidl(const ShellFolderWithPidl & theOrig)
			: SF(theOrig.SF), Pidl(theOrig.Pidl)
		{ if (SF != 0) SF->AddRef(); }

		~ShellFolderWithPidl()
		{ if (SF != 0) SF->Release(); }
	};

	typedef std::vector<ShellFolderWithPidl> ItemVect;
	typedef ItemVect::size_type              Index;

#else

	typedef std::vector<ShellFolder *> ItemVect;
	typedef ItemVect::size_type        Index;

#endif

private:
	LPITEMIDLIST myEnumerationPidl;
	bool myParentToChild;
	bool myCurFolderAsItem;

	ItemVect myItems;
	Index myCur;


public:
	ChildToParentEnumerator(LPCITEMIDLIST theFullPidl, bool theChildToParent, bool theCurFolderAsItem)
		: myParentToChild(!theChildToParent), myCurFolderAsItem(theCurFolderAsItem)
	{
		UINT aSize = UtilPidlFunc::PidlSize(theFullPidl);
		myEnumerationPidl = (LPITEMIDLIST) CoTaskMemAlloc(aSize);
		CopyMemory(myEnumerationPidl, (void *)theFullPidl, aSize);

		FillItemVector();
		ResetCurrent();
	}

	~ChildToParentEnumerator()
	{
		if (myEnumerationPidl != 0)
			CoTaskMemFree(myEnumerationPidl);
	}

public:
	int GetNumItems() const
	{
		return (int) myItems.size();
	}


// Item Members
protected:
	STDMETHODIMP GetDisplayName(bool theFullName, TCHAR * theOutName, int * theSize)
	{
		return E_NOTIMPL;
	}


// IItemProvider Members
protected:
	STDMETHODIMP EnumItems(ULONG theOptions, IEnumItems ** theOutEnum)
	{
		if (theOutEnum == 0)
			return E_POINTER;

		IEnumItems * aEI = static_cast<IEnumItems *>(this);

		*theOutEnum = aEI;
		aEI->AddRef();

		return S_OK;
	}


// IEnumItems Members
protected:
	STDMETHODIMP Next(ULONG theNumItems, Item ** theOutEnum, ULONG * theNumExtracted)
	{
		if ( myCur >= myItems.size() )	//this is either because of overflow (reverse), or because of end-of-range (forward)
		{
			*theNumExtracted = 0;
			return S_FALSE;
		}
		else
		{
#ifdef SUPPORT_CURFOLDER_AS_ITEM

			const ShellFolderWithPidl & aItem = myItems[myCur];

			if (myCurFolderAsItem && myCur == myItems.size() - 1)
				*theOutEnum = new ShellItem(aItem.Pidl, aItem.SF);

			else
				*theOutEnum = new ShellFolder(aItem.Pidl, aItem.SF);

#else
			*theOutEnum = myItems[myCur];
#endif // SUPPORT
			*theNumExtracted = 1;
			AdvanceCurrent(1);

			return S_OK;
		}
	}

	STDMETHODIMP Skip(ULONG theNumItems)
	{
		AdvanceCurrent(theNumItems);
		return S_OK;
	}

	STDMETHODIMP Reset()
	{
		ResetCurrent();
		return S_OK;
	}

	STDMETHODIMP Clone(IEnumItems ** theOut)
		{ return E_NOTIMPL; }


// Implementation Details
private:
	void FillItemVector()
	{
		// 1. Desktop

#ifdef SUPPORT_CURFOLDER_AS_ITEM

		ShellFolderWithPidl aDesktop;

		SHGetDesktopFolder(&aDesktop.SF);
		SHGetSpecialFolderLocation(NULL, CSIDL_DESKTOP, &aDesktop.Pidl);

		myItems.push_back( aDesktop );

#else

		myItems.push_back( new ShellFolder(CSIDL_DESKTOP) );

#endif


		// 2. Others
		if (myEnumerationPidl->mkid.cb == 0)
			return;

		LPCITEMIDLIST aTempPidl = myEnumerationPidl;
		IShellFolder * aTempSF = 0;
		SHGetDesktopFolder(&aTempSF);

		while(aTempPidl != 0)
		{
			// the PIDL needs not be destroyed here since ShellFolder will destroy it
			LPITEMIDLIST aLocalSinglePidl = UtilPidlFunc::Copy<MemoryWriter_Crt>(aTempPidl, aTempPidl->mkid.cb);

#ifdef SUPPORT_CURFOLDER_AS_ITEM
			myItems.push_back( ShellFolderWithPidl(aTempSF, aLocalSinglePidl) );
#else
			myItems.push_back( new ShellFolder(aLocalSinglePidl, aTempSF) );
#endif

			// Advance the Shell Folder to the next PIDL
			IShellFolder * aSF = 0;
			HRESULT aRes = aTempSF->BindToObject(aLocalSinglePidl, NULL, IID_IShellFolder, (void **) &aSF);

			if ( FAILED(aRes) || aSF == 0)
				break;

			aTempSF->Release();
			aTempSF = aSF;

			// Advance the PIDL
			aTempPidl = UtilPidlFunc::GetNextItemID(aTempPidl);
		}

		aTempSF->Release();
	}

	void ResetCurrent()
	{
		if (myParentToChild)
			myCur = 0;
		else
			myCur = myItems.size() - 1;
	}

	void AdvanceCurrent(Index theDelta)
	{
		if (myParentToChild)
			myCur += theDelta;
		else
			myCur -= theDelta;
	}
};

//////////////////////////////////////////////////////////////////////////////////////////////

HotspotImpl_TitlebarMenus::HotspotImpl_TitlebarMenus()
{
	Application & aApp = Application::Instance();

	myIpcModule = aApp.GetIpcModule();
	myRootWindow = aApp.GetRootWindowInstance();

	myTimerID_Infreq = myRootWindow->GetAvailableTimerID();
	myTimerID_Shell  = myRootWindow->GetAvailableTimerID();
}

//////////////////////////////////////////////////////////////////////////////////////////////

HotspotImpl_TitlebarMenus::~HotspotImpl_TitlebarMenus()
{
	if (myEnabled)
		Stop();
}

//////////////////////////////////////////////////////////////////////////////////////////////

bool HotspotImpl_TitlebarMenus::operator() (HWND theIter_Wnd)
{
	DWORD aThreadID = GetWindowThreadProcessId(theIter_Wnd, NULL);

	HHOOK aHook = SetWindowsHookEx
	(
		WH_MOUSE,
		myIpcModule->GetHookAddress_TitlebarMenus(),
		myIpcModule->GetModuleInstance(),
		aThreadID
	);

	mySpiedThreads.Add(aThreadID, aHook);

	return true;	//continue search
}

//////////////////////////////////////////////////////////////////////////////////////////////

void HotspotImpl_TitlebarMenus::OnToggle(bool theEnable)
{
	Hotspot::OnToggle(theEnable);

	if (theEnable)  Start();
	else            Stop();
}

//////////////////////////////////////////////////////////////////////////////////////////////

void HotspotImpl_TitlebarMenus::GetGuid( GUID & theOut ) const
{
	theOut = GUID_Hotspot_TitlebarMenus;
}

//////////////////////////////////////////////////////////////////////////////////////////////

void HotspotImpl_TitlebarMenus::ProcessIpcIo(const void * theMemory)
{
	const char * aBuf = (const char *) theMemory;

	HWND aBaseActivateWindow = *(const HWND *)aBuf;
	aBuf += sizeof HWND;

	// Obtain an absolute PIDL of the folder
	LPCITEMIDLIST aFullPidl = (LPCITEMIDLIST)aBuf;

	ApplicationSettings::DataType aFlags = Application::InstanceC().Prefs()->GetTitlebarMenusFlags();
	bool aChildToParent     = (aFlags & ApplicationSettings::TBM_ORDER_CHILD_TO_PARENT) != 0;
	bool aCurFolderAsItem   = (aFlags & ApplicationSettings::TBM_CURFOLDER_AS_ITEM) != 0;
	bool aCurFolderAutoSel  = aCurFolderAsItem  && ( (aFlags & ApplicationSettings::TBM_CURFOLDER_AUTO_SELECT) != 0 );

	ComInstance<ChildToParentEnumerator> aEn(aFullPidl, aChildToParent, aCurFolderAsItem);

	MenuManager::SetIpcShellViewWnd(aBaseActivateWindow);
	MenuManager::SetFGWindowMode(1);

	if (!aCurFolderAutoSel)
		MenuManager::ConstructByDataItem(&aEn);

	else
	{
		int aNumItems = aEn.GetNumItems();

		MenuWindow * aWnd = MenuManager::ConstructByDataItem(&aEn, 0, 0, false);
		MenuWindow_DisplayItemList * aListWnd = reinterpret_cast<MenuWindow_DisplayItemList *>(aWnd);

		aListWnd->SelectItem(aChildToParent ? 0 : aNumItems-1);

		RECT aRect = { 0 };
		GetCursorPos( (POINT *) &aRect );

		//TEMP: more intelligent menu positioning. Need to uncomment it as soon as menu wouldn't disappear every once in a while.
		//aCurFolderAutoSel = aListWnd->GetSelectionRect(aRect);

		MenuManager::AdjustPosition(aWnd, &aRect, MenuManager::POS_DEFAULT);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////

void HotspotImpl_TitlebarMenus::ProcessShellHook(WPARAM theWParam, LPARAM theLParam)
{
	ATLTRACE( _T("HotspotImpl_TitlebarMenus::ProcessShellHook. theWParam=%x\n"), theWParam);

	if (theWParam == HSHELL_WINDOWCREATED || theWParam == HSHELL_WINDOWDESTROYED)
		SetTimer(*myRootWindow, myTimerID_Shell, 50, NULL);
}

//////////////////////////////////////////////////////////////////////////////////////////////

LRESULT HotspotImpl_TitlebarMenus::MsgHandler_Timer(UINT theMsg, WPARAM theWParam, LPARAM theLParam, BOOL & theHandled)
{
	if (theWParam == myTimerID_Infreq)
	{
		RemoveAllMonitoring();
		ReEnumerate();

		// keep the timer
	}
	else if (theWParam == myTimerID_Shell)
	{
		RemoveAllMonitoring();
		ReEnumerate();

		KillTimer(*myRootWindow, myTimerID_Shell);
	}
	else
		theHandled = FALSE;

	return 0L;
}

//////////////////////////////////////////////////////////////////////////////////////////////

void HotspotImpl_TitlebarMenus::Start()
{
	// 1. Root window interaction
	myRootWindow->AddIpcProcessor(GUID_Hotspot_TitlebarMenus, this);
	myRootWindow->AddShellHookProcessor(this);
	myRootWindow->AddMessageMap(this);

	// 2. Spied threads
	ReEnumerate();

	// 3. Periodic re-enumeration timer
	SetTimer(*myRootWindow, myTimerID_Infreq, 5000, NULL);
}

//////////////////////////////////////////////////////////////////////////////////////////////

void HotspotImpl_TitlebarMenus::Stop()
{
	// 3. Periodic re-enumeration timer
	KillTimer(*myRootWindow, myTimerID_Infreq);
	KillTimer(*myRootWindow, myTimerID_Shell);

	// 2. Spied threads
	RemoveAllMonitoring();

	// 1. Root window interaction
	myRootWindow->RemoveMessageMap(this);
	myRootWindow->RemoveShellHookProcessor(this);
	myRootWindow->RemoveIpcProcessor(GUID_Hotspot_TitlebarMenus);
}

//////////////////////////////////////////////////////////////////////////////////////////////

void HotspotImpl_TitlebarMenus::ReEnumerate()
{
	MaxDepthWindowNavigator < ClassCondition<WndClass_SHELLDLL_DefView>, 1 > aNav;
	aNav.iterate(0, *this);
}

//////////////////////////////////////////////////////////////////////////////////////////////

void HotspotImpl_TitlebarMenus::RemoveAllMonitoring()
{
	for (int i = 0; i != mySpiedThreads.GetSize(); i++ )
	{
		HHOOK aHk = mySpiedThreads.GetValueAt(i);
		UnhookWindowsHookEx(aHk);
	}

	mySpiedThreads.RemoveAll();
}
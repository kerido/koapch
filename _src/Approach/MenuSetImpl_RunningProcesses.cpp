#include "stdafx.h"

#include <psapi.h>

#include "sdk_ComObject.h"

#include "MenusetImpl_RunningProcesses.h"
#include "ShellItemGenericIconManager.h"
#include "Trace.h"
#include "Application.h"
#include "IpcGuids.h"

#include "UtilShellFunc.h"
#include "util_Localization.h"

#include "resource.h"
#include "auto_LocResource.h"

class IProcessNameRetriever
{
public:
	virtual int GetProcessName(HANDLE theHProcess, TCHAR * theOut, int theOutSize) = 0;

public:
	static IProcessNameRetriever * Instance()
	{
		static IProcessNameRetriever * ourInst = InitInstance();
		return ourInst;
	}

private:
	static IProcessNameRetriever * InitInstance();
};


class ProcessNameRetriever_BeforeVista : public IProcessNameRetriever
{
protected:
	int GetProcessName(HANDLE theHProcess, TCHAR * theOut, int theOutSize)
	{
		HMODULE aFirstModule = 0;
		DWORD aBytesRequired = 0;

		BOOL aRes = EnumProcessModules(theHProcess, &aFirstModule, sizeof(HMODULE), &aBytesRequired);

		if (!aRes)
			return 0;


		int aRet = (int)GetModuleFileNameEx(theHProcess, aFirstModule, theOut, theOutSize);

		if (aRet == 0)
			return aRet;

		const static UINT LengthOfUglyPrefix =  4; // length of string \??\ 
		const static UINT LengthOfSystemRoot = 12; // length of string \systemroot\ 


		/////////////////////////////////////////////////////////////
		// Replace paths containing \??\ 

		if (aRet > LengthOfUglyPrefix)
		{
			TCHAR aTemp = theOut[LengthOfUglyPrefix];
			theOut[LengthOfUglyPrefix] = 0;

			int aComparison = lstrcmpi( theOut, _T("\\??\\") );

			theOut[LengthOfUglyPrefix] = aTemp;

			if (aComparison == 0)
			{
				TCHAR aBuf[1024];

				lstrcpyn(aBuf, theOut+LengthOfUglyPrefix, 1024);
				lstrcpyn(theOut, aBuf, theOutSize);

				return lstrlen(theOut);
			}
		}


		/////////////////////////////////////////////////////////////
		// Replace paths containing \systemroot\ 

		if (aRet > LengthOfSystemRoot)
		{
			TCHAR aTemp = theOut[LengthOfSystemRoot];
			theOut[LengthOfSystemRoot] = 0;

			int aComparison = lstrcmpi( theOut, _T("\\systemroot\\") );

			theOut[LengthOfSystemRoot] = aTemp;

			if (aComparison == 0)
			{
				TCHAR aBuf[1024];
				UINT aLength = GetWindowsDirectory(aBuf, 1024);

				UINT aStartAt = LengthOfSystemRoot;

				if (aBuf[aLength - 1] != _T('\\') )
					--aStartAt;

				lstrcat(aBuf, theOut+aStartAt);

				lstrcpyn(theOut, aBuf, theOutSize);

				return lstrlen(theOut);
			}
		}

		return aRet;
	}
};


class ProcessNameRetriever_AfterVista : public IProcessNameRetriever
{
private:
	typedef BOOL  (WINAPI * fnQueryFullProcessImageName)(HANDLE, DWORD, LPTSTR, PDWORD);

	fnQueryFullProcessImageName myFn;

public:
	ProcessNameRetriever_AfterVista()
	{
		HMODULE aMod = LoadLibrary( _T("kernel32.dll") );

#ifdef _UNICODE
		myFn = (fnQueryFullProcessImageName) GetProcAddress(aMod, "QueryFullProcessImageNameW");
#else
		myFn = (fnQueryFullProcessImageName) GetProcAddress(aMod, "QueryFullProcessImageNameA");
#endif // _UNICODE
	}

protected:
	int GetProcessName(HANDLE theHProcess, TCHAR * theOut, int theOutSize)
	{
		DWORD aSize = theOutSize;
		BOOL aRes = myFn(theHProcess, 0, theOut, &aSize);

		if (aRes == 0)
			return 0;
		else
			return aSize;
	}
};

IProcessNameRetriever * IProcessNameRetriever::InitInstance()
{
	OSVERSIONINFO aVer;
	aVer.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&aVer);

	if (aVer.dwMajorVersion >= 6)	// windows vista, windows 7 or server 2008
		return new ProcessNameRetriever_AfterVista();
	else
		return new ProcessNameRetriever_BeforeVista();
};

//////////////////////////////////////////////////////////////////////////////////////////////

class ProcessItemComparer
{
public:
	bool operator() (const LogicalItem_Processes_Process * theX, const LogicalItem_Processes_Process * theY) const
	{
		int aRet = lstrcmpi( theX->GetShortName(), theY->GetShortName() );
		return aRet < 0;
	}
};

//////////////////////////////////////////////////////////////////////////////////////////////

class ProcItemsEnumerator : public ComEntry1<IEnumItems>
{
private:
	typedef std::vector<LogicalItem_Processes_Process *> ItemVect;
	typedef ItemVect::iterator                           ItemIter;
	typedef ItemVect::const_iterator                     ItemIterC;
	typedef ULONG                                        ItemIndex;

private:
	ULONG      myOptions;
	ItemVect   myFldrs;
	ItemVect   myFiles;
	ItemVect * myEffective;
	ItemIndex  myCurIndex;

public:
	ProcItemsEnumerator(ULONG theOptions) : myOptions(theOptions), myCurIndex(), myEffective(0)
	{
		if ( (myOptions & (ENUMOPTIONS_FOLDERS_ON_TOP|ENUMOPTIONS_FOLDERS_ON_BTM)) != 0)
			FillLists<true>();
		else
			FillLists<false>();

		if ( (myOptions & ENUMOPTIONS_SORT_ITEMS) != 0)
			SortLists();

		MergeLists( (myOptions & ENUMOPTIONS_FOLDERS_ON_BTM) == 0);
	}


// IEnumItems Members
protected:
	STDMETHODIMP Next(ULONG theNumItems, Item ** theOutItem, ULONG * theNumExtracted)
	{
		if (theOutItem == 0 || theNumExtracted == 0)
			return E_POINTER;

		if (theNumItems == 0)
			return E_INVALIDARG;

		ItemVect & aItms = *myEffective;

		ItemIndex aMax = __min(theNumItems, (ItemIndex) aItms.size()-myCurIndex);
		ULONG aSkipped = 0UL;

		for (ItemIndex i = 0; i < aMax; i++, myCurIndex++)
			theOutItem[i] = aItms[myCurIndex];

		*theNumExtracted = aMax;
		return aMax == 0 ? S_FALSE : S_OK;
	}

	STDMETHODIMP Skip(ULONG theNumItems)
	{
		myCurIndex++;
		return S_OK;
	}

	STDMETHODIMP Reset()
	{
		myCurIndex = ItemIndex();
		return S_OK;
	}

	STDMETHODIMP Clone(IEnumItems ** /*theOut*/)
	{
		return E_NOTIMPL;
	}

// Implementation Details
private:
	template<bool tGroupFolders>
	void FillLists()
	{
		DWORD aProcIDs[1024];
		DWORD aArraySize = 0;
		EnumProcesses(aProcIDs, 1024, &aArraySize);	// aArraySize will contain the number of BYTES, not elements

		aArraySize /= sizeof DWORD;


		for (DWORD i = 0; i < aArraySize; i++)
		{
			DWORD aProcID = aProcIDs[i];

			HANDLE aHPrc = OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_VM_READ, TRUE, aProcID);

			if (aHPrc == NULL)
			{
				Trace("ProcItemsEnumerator -- Unable to open process %d.\n", aProcID);
				continue;
			}

			LogicalItem_Processes_Process * aItem = new LogicalItem_Processes_Process(aProcID, aHPrc);

			if (tGroupFolders)
			{
				if ( aItem->HasOneFlag(LogicalItem_Processes_Child::CanQuit|LogicalItem_Processes_Child::NameValid) )
					myFldrs.push_back(aItem);
				else
					myFiles.push_back(aItem);
			}
			else
				myFldrs.push_back(aItem);

			CloseHandle(aHPrc);
		}
	}

	void SortLists()
	{
		ProcessItemComparer aC;

		if ( !myFldrs.empty() )
			std::sort(myFldrs.begin(), myFldrs.end(), aC);

		if ( !myFiles.empty() )
			std::sort(myFiles.begin(), myFiles.end(), aC);
	}

	void MergeLists(bool theFoldersOnTop)
	{
		if (theFoldersOnTop)
		{
			myFldrs.reserve( myFldrs.size() + myFiles.size() );

			for (ItemIterC aIt = myFiles.begin(); aIt != myFiles.end(); aIt++)
				myFldrs.push_back(*aIt);

			myFiles.clear();
			myEffective = &myFldrs;
		}
		else
		{
			myFiles.reserve( myFldrs.size() + myFiles.size() );

			for (ItemIterC aIt = myFldrs.begin(); aIt != myFldrs.end(); aIt++)
				myFiles.push_back(*aIt);

			myFldrs.clear();
			myEffective = &myFiles;
		}
	}
};


//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

class TaskItemsEnumerator : public ComEntry1<IEnumItems>
{
private:
	const static ULONG NumTaskItems = 2;
	typedef LogicalItem_Processes_Task::ItemType TaskItemType;
	typedef LogicalItem_Processes_Child::ProcessItemFlags CheckFlag;

private:
	ULONG myCurIndex;
	const LogicalItem_Processes_Child * myProcessItem;

public:
	TaskItemsEnumerator(const LogicalItem_Processes_Child * theProcessItem)
		: myCurIndex(0UL), myProcessItem(theProcessItem) { }


// IEnumItems Members
protected:
	STDMETHODIMP Next(ULONG theNumItems, Item ** theOutItem, ULONG * theNumExtracted)
	{
		if (theOutItem == 0 || theNumExtracted == 0)
			return E_POINTER;

		if (theNumItems == 0)
			return E_INVALIDARG;

		ULONG aMax = __min(theNumItems, NumTaskItems-myCurIndex);

		for (ULONG i = 0; i < aMax; ++myCurIndex)
		{
			TaskItemType aTp = (myCurIndex == 0) ? LogicalItem_Processes_Task::Reveal     : LogicalItem_Processes_Task::Quit;
			CheckFlag aFl    = (myCurIndex == 0) ? LogicalItem_Processes_Child::NameValid : LogicalItem_Processes_Child::CanQuit;

			if (myProcessItem->HasOneFlag(aFl))
				theOutItem[i++] = new ComInstance<LogicalItem_Processes_Task>(myProcessItem, aTp);
			else
				aMax--;
		}

		*theNumExtracted = aMax;
		return aMax == 0 ? S_FALSE : S_OK;
	}

	STDMETHODIMP Skip(ULONG theNumItems)
	{
		return E_NOTIMPL;
	}

	STDMETHODIMP Reset()
	{
		return E_NOTIMPL;
	}

	STDMETHODIMP Clone(IEnumItems ** theOut)
	{
		return E_NOTIMPL;
	}
};

//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

class ProcessImageRevealer
{
public:
	static HRESULT RevealProcessImage(const LogicalItem_Processes_Child * theItem, HWND theParent)
	{
		if ( theItem->HasOneFlag(LogicalItem_Processes_Child::NameValid) )
		{
			TCHAR aArgs[1024];
			_stprintf(aArgs, _T("/e,/select,\"%s\""), theItem->GetFullName() );

			int aRes = (int)ShellExecute(theParent, _T("open"), _T("explorer.exe"), aArgs, NULL, SW_SHOWNORMAL);
			return UtilShellFunc::ShellExecuteRetValToHResult(aRes);
		}
		else
			return E_FAIL;
	}
};

//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

LogicalItem_Processes_Root::LogicalItem_Processes_Root()
{
	ShellItemGenericIconManager * aMan = Application::Instance().GetShellItemGenericIconManager();

	struct { const TCHAR * Name; int Index; REFGUID Guid; } aIcons[] = 
	{
		{ _T("shell32.dll"),  71, OBJ_ProcItem   },
		{ _T("shell32.dll"),  22, OBJ_RevealItem },
		{ _T("shell32.dll"), 131, OBJ_QuitItem   },
	};

	OSVERSIONINFO aVer;
	aVer.dwOSVersionInfoSize = sizeof OSVERSIONINFO;
	GetVersionEx(&aVer);

	if (aVer.dwMajorVersion == 5 && aVer.dwMinorVersion == 0)	//windows 2000
		aIcons[2].Index = 105;	// this is not the X (as in delete), but the stop sign; close enough

	const static int NumIcons = sizeof aIcons / sizeof aIcons[0];

	for (int i = 0; i < NumIcons; ++i)
	{
		ItemIconData aDt;
		HRESULT aRes = aMan->GetWellKnownIconData(aIcons[i].Guid, &aDt);

		if ( FAILED(aRes) && aIcons[i].Index > 0)
		{
			HICON aIcn = 0;
			UINT aNumExtracted = ExtractIconEx(aIcons[i].Name, aIcons[i].Index, NULL, &aIcn, 1);

			if (aNumExtracted > 0 && aIcn != 0)
			{
					aRes = aMan->EnsureWellKnownIcon(aIcons[i].Guid, aIcn);
					ATLASSERT(aRes == S_OK);
					DestroyIcon(aIcn);
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP LogicalItem_Processes_Root::GetDisplayName( bool theFullName, TCHAR * theOutName, int * theSize )
{
	LocalizationManagerPtr aM;
	return aM->GetStringSafe( KEYOF(IDS_APCHITEMS_ITEMPROC), theOutName, theSize);
}

//////////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP LogicalItem_Processes_Root::EnumItems(ULONG theOptions, IEnumItems ** theOut)
{
	*theOut = new ComInstance<ProcItemsEnumerator>(theOptions);
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP LogicalItem_Processes_Root::RetrieveIconData(IconExtractParams * theParams, void **)
{
	const ShellItemGenericIconManager * aMan = Application::InstanceC().GetShellItemGenericIconManager();
	HRESULT aRes = aMan->GetWellKnownIconData(OBJ_ProcItem, theParams);

	if ( FAILED(aRes) )
		return aRes;
	else
		return MAKE_HRESULT(0, 0, CHANGE_ICON);
}

//////////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP LogicalItem_Processes_Root::DoLaunch(HWND theParent, ULONG theOptions)
{
	if (theOptions == SHOW_CONTEXT_MENU)
		return E_NOTIMPL;

	int aRetVal = (int) ShellExecute(NULL, _T("open"), _T("taskmgr.exe"), 0, 0, SW_SHOW);
	return UtilShellFunc::ShellExecuteRetValToHResult(aRetVal);
}

//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

LogicalItem_Processes_Child::LogicalItem_Processes_Child(DWORD theProcessID, HANDLE theHProcess)
	: myProcessID(theProcessID), myActualLength(0), myProcessShortName(myProcessName), myFlags(0)
{
	IProcessNameRetriever * aN = IProcessNameRetriever::Instance();
	myActualLength = aN->GetProcessName(theHProcess, myProcessName, ProcessNameSize);

	if (myActualLength == 0)
	{
		LocalizationManagerPtr aP;
		myActualLength = ProcessNameSize;
		aP->GetStringSafe( KEYOF(IDS_APCHITEMS_ITEMPROC_SYS), myProcessName, &myActualLength);
	}
	else
	{
		TCHAR * aChar = _tcsrchr( myProcessName, _T('\\') );

		if (aChar != 0)
			myProcessShortName = ++aChar; // because the pointer is pointing to the backslash, advance by one character

		myFlags |= NameValid;
	}


	HANDLE aH = GetCurrentProcess();
	BOOL aRes = DuplicateHandle(aH, theHProcess, aH, &myHProcess, PROCESS_TERMINATE|PROCESS_SET_INFORMATION, FALSE, 0);
	if (aRes)     myFlags |= CanQuit|CanEdit;
	else
	{
		aRes = DuplicateHandle(aH, theHProcess, aH, &myHProcess, PROCESS_TERMINATE, FALSE, 0);
		if (aRes)   myFlags |= CanQuit;
		else
		{
			aRes = DuplicateHandle(aH, theHProcess, aH, &myHProcess, PROCESS_SET_INFORMATION, FALSE, 0);
			if (aRes)  myFlags |= CanEdit;
			else       myHProcess = NULL;
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////

LogicalItem_Processes_Child::~LogicalItem_Processes_Child()
{
	if (myHProcess != NULL)
		CloseHandle(myHProcess);
}

//////////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP LogicalItem_Processes_Child::GetDisplayName( bool /*theFullName*/, TCHAR * theOutName, int * theSize )
{
	if (theOutName == 0 || theSize == 0)
		return E_POINTER;

	int aBufSz = *theSize;

	DWORD aLen = myActualLength - (DWORD) (myProcessShortName - myProcessName);

	* theSize = aLen;

	if ( (DWORD) aBufSz < aLen + 1)
		return E_OUTOFMEMORY;

	lstrcpyn(theOutName, myProcessShortName, aLen + 1);

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP LogicalItem_Processes_Child::EnumItems(ULONG theOptions, IEnumItems ** theOut)
{
	if ( !HasOneFlag(CanQuit|NameValid) )
		return E_ACCESSDENIED;

	else if (theOut == NULL)
		return E_POINTER;

	*theOut = new ComInstance<TaskItemsEnumerator>(this);
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP LogicalItem_Processes_Child::RetrieveIconData(IconExtractParams * theParams, void ** theOutAsyncArg)
{
	HRESULT aRes;

	if (myProcessID != 0)
	{
		CComPtr<IShellFolder> aSF;
		aRes = SHGetDesktopFolder(&aSF);

		if ( SUCCEEDED(aRes) )
		{
			LPITEMIDLIST aPidl = 0;
			aRes = aSF->ParseDisplayName(NULL, NULL, myProcessName, NULL, &aPidl, NULL);

			myFlags |= NameChecked;

			if (SUCCEEDED(aRes) && aPidl != 0)
			{
				aSF.Release();	//it will be used for the immediate parent folder

				LPCITEMIDLIST aSingleLevelPidl = 0;
				aRes = UtilShellFunc::SHBindToParentEx(aPidl, IID_IShellFolder, (void **) &aSF, &aSingleLevelPidl);

				if (SUCCEEDED(aRes) && aSingleLevelPidl != 0)
					aRes = UtilShellFunc::GetIconNew(aSF, aSingleLevelPidl, theParams, theOutAsyncArg);

				CoTaskMemFree(aPidl);

				return aRes;
			}
			else
			{
				myFlags &= ~NameValid;
			}
		}
	}

	const ShellItemGenericIconManager * aM = Application::InstanceC().GetShellItemGenericIconManager();
	aRes = aM->GetIconLocation(ShellItemGenericIconManager::GENERIC_ICON_APPLICATION, theParams);

	if ( SUCCEEDED(aRes) )
		return MAKE_HRESULT(0, 0, CHANGE_ICON);
	else
		return aRes;
}

//////////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP LogicalItem_Processes_Child::RetrieveIconDataAsync(IconExtractParams * theParams, void * theAsyncArg)
{
	return UtilShellFunc::GetIconAsync(theParams, theAsyncArg);
}

//////////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP LogicalItem_Processes_Child::ReleaseAsyncArg(void * theAsyncArg)
{
	return UtilShellFunc::ReleaseAsyncArg(theAsyncArg);
}

//////////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP LogicalItem_Processes_Child::DoLaunch(HWND theParent, ULONG theOptions)
{
	if (theOptions == SHOW_CONTEXT_MENU)
		return E_NOTIMPL;
	else
		return ProcessImageRevealer::RevealProcessImage(this, theParent);
}

//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP LogicalItem_Processes_Task::GetDisplayName(bool theFullName, TCHAR * theOutName, int * theSize)
{
	LocalizationManagerPtr aL;

	switch (myType)
	{
	case Reveal:
		return aL->GetStringSafe( KEYOF(IDS_APCHITEMS_ITEMPROC_REVEAL), theOutName, theSize);

	case Quit:
		return aL->GetStringSafe( KEYOF(IDS_APCHITEMS_ITEMPROC_QUIT), theOutName, theSize);

	default:
		return E_INVALIDARG;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP LogicalItem_Processes_Task::DoLaunch(HWND theParent, ULONG/* theOptions*/)
{
	switch (myType)
	{
	case Reveal:
		return ProcessImageRevealer::RevealProcessImage(myProcessItem, theParent);

	case Quit:
		TerminateProcess(myProcessItem->GetProcessHandle(), 0xFFFFFFFF);
		return S_OK;

	default:
		return E_INVALIDARG;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP LogicalItem_Processes_Task::RetrieveIconData(IconExtractParams * theParams, void **)
{
	const ShellItemGenericIconManager * aMan = Application::InstanceC().GetShellItemGenericIconManager();
	HRESULT aRes;

	switch (myType)
	{
	case Reveal:
		aRes = aMan->GetWellKnownIconData(OBJ_RevealItem, theParams);
		break;

	case Quit:
		aRes = aMan->GetWellKnownIconData(OBJ_QuitItem, theParams);
		break;

	default:
		aRes = E_INVALIDARG;
		break;
	}

	if ( FAILED(aRes) )
		return aRes;
	else
		return MAKE_HRESULT(0, 0, CHANGE_ICON);
}

//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP ItemFactory_Processes::CreateItem( const BYTE * theData, int theDataSize, Item ** theOutItem )
{
	*theOutItem = new ComInstance<LogicalItem_Processes_Root>();
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP ItemFactory_Processes::GetNumItems( int * theOut )
{
	if (theOut == 0)
		return E_POINTER;

	*theOut = 1;
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP ItemFactory_Processes::GetItemData( int theIndex, MenuSetItemData * theOut )
{
	if (theIndex != 0)
		return E_INVALIDARG;

	HRESULT aRes = GetItemName(theOut, theOut->Name, theOut->NameSize);

	if ( FAILED(aRes) )
		return aRes;

	theOut->Type = MenuSetItemData::ITEM;
	theOut->ObjectID = OBJ_ProcItem;
	theOut->Param = 0;
	theOut->ParamSize = 0;
	theOut->IconIndex = -1;

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP ItemFactory_Processes::GetItemName( const MenuSetItemData * theItem, TCHAR * theOut, int theBufSize )
{
	if (theOut == 0)
		return E_POINTER;

	if (theItem->Size != sizeof MenuSetItemData)
		return E_INVALIDARG;

	int aSize = theBufSize;

	LocalizationManagerPtr aM;
	return aM->GetStringSafe( KEYOF(IDS_APCHITEMS_ITEMPROC), theOut, &aSize);
}

//////////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP ItemFactory_Processes::GetTypeGuid( GUID * theOut )
{
	if (theOut == 0)
		return E_POINTER;

	*theOut = OBJ_ProcItem;
	return S_OK;
}

#include "stdafx.h"

#include "sdk_ComObject.h"

#include "MenusetImpl_Run.h"
#include "Application.h"
#include "Trace.h"
#include "IpcGuids.h"
#include "ShellItemGenericIconManager.h"

#include "util_Localization.h"

#include "resource.h"
#include "auto_LocResource.h"

//////////////////////////////////////////////////////////////////////////////////////////////

//! Represents an object
class ShellRunExecutor
{
	const static size_t StringSize = 2048;
	const static DWORD SHPPFW_MEDIACHECKONLY_MY = 0x00000010;

private:
	SHELLEXECUTEINFO mySEE;
	TCHAR myFile[StringSize];
	TCHAR myDir[StringSize];
	TCHAR myArgs[StringSize];
	int myDriveNumber;


public:
	ShellRunExecutor() : myDriveNumber(-100)
	{
		mySEE.cbSize = sizeof SHELLEXECUTEINFO;
		mySEE.fMask = SEE_MASK_NOASYNC|SEE_MASK_DOENVSUBST;
		mySEE.lpVerb = 0;
		mySEE.lpFile = myFile;
		mySEE.lpParameters = 0;	// may be set during processing
		mySEE.lpDirectory = 0;	// may be set during processing
		mySEE.nShow = SW_SHOWNORMAL;

		myFile[0] = myArgs[0] = myDir[0] = 0;	//just to monitor when the strings get set
	}



public:
	//! Executes a command from a Shell MRU list.
	//! 
	//! \param theWndParent
	//!     The Approach Menu containing one or more LogicaiItem_Run_Child objects.
	//! 
	//! \param theCommand
	//!     The command string to be executed.
	//! 
	//! \param theCommandSize
	//!     The size of the command string in charachters, without the null terminator.
	//! 
	//! \return
	//!     S_OK if the command was executed successfully; otherwise, a COM error.
	HRESULT Execute(HWND theWndParent, const TCHAR * theCommand, size_t theCommandSize)
	{
		TCHAR aCmd[StringSize];
		lstrcpyn(aCmd, theCommand, (int) theCommandSize+1);

		PathRemoveBlanks(aCmd);

		ExpandEnvironmentStrings(aCmd, myFile, StringSize);

		// If the path is pointing to a valid file, do nothing.
		// Otherwise, separate arguments from file name.

		BOOL aRet = PathFileExists(myFile);

		if (!aRet)
		{
			TCHAR * aArgsStart = PathGetArgs(myFile);

			if (*aArgsStart != 0)	//not the end of the string
			{
				lstrcpy(myArgs, aArgsStart);
				mySEE.lpParameters = myArgs;

				*aArgsStart = 0;	//null-terminate the file

				PathUnquoteSpaces(myArgs);
				PathRemoveBlanks(myArgs);
			}
		}

		if ( PathIsUNC(myFile) )
			myDriveNumber = -2;	//special code
		else if ( PathIsURL(myFile) )
			myDriveNumber = -3;	//special code
		else
			myDriveNumber = PathGetDriveNumber(myFile);

		if (myDriveNumber != -3)	//url
		{
			HRESULT aRes = SHPathPrepareForWrite(theWndParent, 0, myFile, SHPPFW_IGNOREFILENAME);

			if (FAILED(aRes))
				return aRes;
		}


		// Determine path type / drive number

		PathUnquoteSpaces(myFile);
		PathRemoveBlanks(myFile);


		// set working directory
		if (myDriveNumber != -1 && myDriveNumber != -3)	// either a UNC or known drive
		{
			TCHAR * aFile = PathFindFileName(myFile);

			if (aFile != myFile)
			{
				int aDiff = (int)(aFile - myFile);

				lstrcpyn(myDir, myFile, aDiff);
				mySEE.lpDirectory = myDir;
			}
		}
		else if (myDriveNumber == -1)
		{
			GetWindowsDirectory(myDir, StringSize);
			mySEE.lpDirectory = myDir;
		}


		//execute
		aRet = ShellExecuteEx(&mySEE);

		if (aRet == TRUE)
			return S_OK;
		else
			return E_FAIL;
	}

private:
};

//////////////////////////////////////////////////////////////////////////////////////////////

//! Represents an object that enumerates entries of the Shell Run MRU list.
//! Upon each iteration step, creates one object of type LogicalItem_Run_Child.
class LogicalItem_Run_Root::RunItemsEnumerator : public IEnumItems
{
private:
	typedef std::vector<std::wstring>   StringList;
	typedef StringList::iterator        StringIter;
	typedef StringList::const_iterator  StringIterC;

	const static int MAX_KEY_LENGTH =   255;
	const static int MAX_VALUE_NAME = 16383;
	const static int NUM_ITEMS      =    26;


private:
	ULONG myRefs;                         //!< Stores the number of references to the current object.
	const LogicalItem_Run_Root & myOwner; //!< Stores the object's owner, where all data actually resides.
	int myCurItemPosition;                //!< Advances during enumeration.

	StringList myStrings;                 //!< Stores names of child items.
	TCHAR myOrder[NUM_ITEMS + 1];         //!< Stores the sequence in which strings from #myStrings will be enumerated.
	int myOrderLength;                    //!< Stores the length of #myOrder.


// Constructors, Destructor
public:
	RunItemsEnumerator(const LogicalItem_Run_Root & theOwner)
		: myRefs(1UL), myCurItemPosition (0), myOwner(theOwner), myOrderLength(0)
	{
		myStrings.reserve(NUM_ITEMS);

		myOrder[0] = 0;

		LoadItems();
	}


// IEnumItems Members
protected:
	STDMETHODIMP Next(ULONG theNumItems, Item ** theOutItem, ULONG * theNumExtracted)
	{
		if (myCurItemPosition < myOrderLength)
		{
			size_t aIndex = myOrder[myCurItemPosition++] - 'a';

			while (aIndex + 1 > myStrings.size())
			{
				if (myCurItemPosition >= myOrderLength)
				{
					*theNumExtracted = 0;
					return S_FALSE;
				}

				aIndex = myOrder[myCurItemPosition++] - 'a';
			}

			*theOutItem = new ComInstance<LogicalItem_Run_Child>( myStrings[aIndex] );
			*theNumExtracted = 1;
			return S_OK;
		}
		else
		{
			*theNumExtracted = 0;
			return S_FALSE;
		}
	}

	STDMETHODIMP Skip(ULONG theNumItems)
	{
		myCurItemPosition += theNumItems;
		return S_OK;
	}

	STDMETHODIMP Reset()
	{
		myCurItemPosition = 0;
		return S_OK;
	}

	STDMETHODIMP Clone(IEnumItems ** theOut)
	{
		*theOut = new RunItemsEnumerator(myOwner);
		return S_OK;
	}


// IUnknown Members
protected:
	ULONG STDMETHODCALLTYPE AddRef()
	{
		return ++myRefs;
	}

	ULONG STDMETHODCALLTYPE Release()
	{
		ULONG aNumRefs = --myRefs;

		if (aNumRefs == 0UL)
			delete this;

		return aNumRefs;
	}

	STDMETHODIMP QueryInterface(REFIID theIID,  void ** theOut)
	{
		if (theIID == IID_IUnknown)
			*theOut = static_cast<IUnknown *>(this);

		else if (theIID == IID_IEnumItems)
			*theOut = static_cast<IEnumItems *>(this);

		else
			return E_NOINTERFACE;

		AddRef();
		return S_OK;
	}


// Implementation Details
private:

	//! Opens the registry and populates #myStrings with RunMRU values.
	void LoadItems()
	{
		HKEY aKey;

		LONG aRes = RegOpenKeyEx
		(
			HKEY_CURRENT_USER,
			_T("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\RunMRU"),
			0,
			KEY_READ|KEY_ENUMERATE_SUB_KEYS,
			&aKey
		);

		if (aRes != ERROR_SUCCESS)
			return;

		DWORD    aNumValues           = 0;
		DWORD    aMaxValueSize        = 0;
		DWORD    aMaxValueDataSize    = 0;

		// Get the class name and the value count
		aRes = RegQueryInfoKey
		(
			aKey,                     // key handle
			NULL,                     // buffer for class name
			NULL,                     // size of class string
			NULL,                     // reserved
			NULL,                     // number of subkeys
			NULL,                     // longest subkey size
			NULL,                     // longest class string
			&aNumValues,              // number of values for this key
			&aMaxValueSize,           // longest value name
			&aMaxValueDataSize,       // longest value data
			NULL,                     // security descriptor
			NULL                      // last write time
		);


		if (aNumValues != 0)
			EnumerateRunEntries(aKey, aNumValues, aMaxValueDataSize);


		RegCloseKey(aKey);
	}

	//! Called from LoadItems
	void EnumerateRunEntries(HKEY aKey, DWORD aNumValues, DWORD aMaxValueDataSize)
	{
		TCHAR aValue[MAX_VALUE_NAME];

		BYTE * aData = new BYTE[aMaxValueDataSize];

		for (DWORD i = 0; i < aNumValues; i++) 
		{
			DWORD aValueSize = MAX_VALUE_NAME;
			DWORD aMaxDataSize = aMaxValueDataSize;
			DWORD aType = 0;

			LONG aRes = RegEnumValue
			(
				aKey,
				i,
				aValue,
				&aValueSize,
				NULL,
				&aType,
				aData,
				&aMaxDataSize
			);

			if (aRes != ERROR_SUCCESS || aType != REG_SZ)
				continue;

			TCHAR * aDataAsString = reinterpret_cast<TCHAR *>(aData);

#ifdef _DEBUG

			// Trace the content of the Registry value

#  ifdef _UNICODE

			//need to convert to ASCII before tracing
			char * aTemp_Data = new char[aMaxValueDataSize];
			char aTemp_Val[MAX_VALUE_NAME];

			WideCharToMultiByte(CP_ACP, 0, aDataAsString, -1, aTemp_Data, aMaxValueDataSize, NULL, NULL);
			WideCharToMultiByte(CP_ACP, 0, aValue,        -1, aTemp_Val,  MAX_VALUE_NAME,    NULL, NULL);

			Trace
			(
				"(%d) %s = %s\n",
				i+1,
				aTemp_Val,
				aTemp_Data
			);

			delete [] aTemp_Data;
#  else

			Trace
			(
				"(%d) %s = %s\n",
				i+1,
				aValue,
				aDataAsString
			);

#  endif	//_UNICODE
#endif	//_DEBUG

			if ( lstrcmp(aValue, _T("MRUList") ) == 0 )
			{
				lstrcpy(myOrder, aDataAsString);
				myOrderLength = lstrlen(myOrder);
			}
			else
			{
				myStrings.push_back( aDataAsString );
			}
		} // for (DWORD i = 0; i < aNumValues; i++)

		delete [] aData;
	}
};

//////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////

LogicalItem_Run_Root::LogicalItem_Run_Root() : myRefs(1UL)
{
	ShellItemGenericIconManager * aMan = Application::Instance().GetShellItemGenericIconManager();

	ItemIconData aDt;
	HRESULT aRes = aMan->GetWellKnownIconData(OBJ_RunItem, &aDt);

	if ( FAILED(aRes) )
	{
		IconManager & aIM = Application::Instance().GetIconManager();

		HICON aIcn = aIM.LoadIconDpiAware( _Module.GetResourceInstance(), MAKEINTRESOURCE(IDI_RUNRECENT), true );

		if (aIcn != 0)
		{
			aRes = aMan->EnsureWellKnownIcon(OBJ_RunItem, aIcn);
			ATLASSERT(aRes == S_OK);
			DestroyIcon(aIcn);
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP LogicalItem_Run_Root::GetDisplayName(bool theFullName, TCHAR * theOutName, int * theSize)
{
	LocalizationManagerPtr aM;
	return aM->GetStringSafe( KEYOF(IDS_APCHITEMS_ITEMRUN), theOutName, theSize);
}

//////////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP LogicalItem_Run_Root::EnumItems(ULONG theOptions, IEnumItems ** theOut)
{
	*theOut = new RunItemsEnumerator(*this);
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP LogicalItem_Run_Root::DoLaunch(HWND theParent, ULONG theOptions)
{
	CComPtr<IShellDispatch> aDisp;
	HRESULT aRes = aDisp.CoCreateInstance(CLSID_Shell);

	if (FAILED(aRes))
		return aRes;

	return aDisp->FileRun();
}

//////////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP LogicalItem_Run_Root::RetrieveIconData(IconExtractParams * theParams, void ** theOutAsyncArg)
{
	const ShellItemGenericIconManager * aMan = Application::InstanceC().GetShellItemGenericIconManager();

	HRESULT aRes = aMan->GetWellKnownIconData(OBJ_RunItem, theParams);

	if ( FAILED(aRes) )
		return aRes;
	else
		return MAKE_HRESULT(0, 0, CHANGE_ICON);
}

//////////////////////////////////////////////////////////////////////////////////////////////

ULONG STDMETHODCALLTYPE LogicalItem_Run_Root::AddRef()
{
	return ++myRefs;
}

//////////////////////////////////////////////////////////////////////////////////////////////

ULONG STDMETHODCALLTYPE LogicalItem_Run_Root::Release()
{
	ULONG aNumRefs = --myRefs;

	if (aNumRefs == 0UL)
		delete this;

	return aNumRefs;
}

//////////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP LogicalItem_Run_Root::QueryInterface(REFIID theIID, void ** theOut)
{
	if (theIID == IID_IItemProvider)
		*theOut = static_cast<IItemProvider *>(this);

	else if (theIID == IID_ILaunchable)
		*theOut = static_cast<ILaunchable *>(this);

	else if (theIID == IID_IIconAcceptor)
		*theOut = static_cast<IIconAcceptor *>(this);

	else if (theIID == IID_IUnknown)
		*theOut = static_cast<IUnknown *>( static_cast<IItemProvider *>(this) );

	else
		return E_NOINTERFACE;

	AddRef();
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

LogicalItem_Run_Child::LogicalItem_Run_Child(const std::wstring & theString)
{
	size_t aIndex = theString.rfind( _T("\\1") );

	if ( aIndex != theString.npos )
		myString = theString.substr(0, aIndex);
	else
		myString = theString;
}

//////////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP LogicalItem_Run_Child::DoLaunch(HWND theMenuParent, ULONG theOptions)
{
	if (theOptions == SHOW_CONTEXT_MENU)
		return E_NOTIMPL; //TODO: delete item

	ComInstance<ShellRunExecutor> aRun;
	return aRun.Execute( theMenuParent, myString.c_str(), myString.size() );
}

//////////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP LogicalItem_Run_Child::GetDisplayName(bool theFullName, TCHAR * theOutName, int * theSize)
{
	if (theOutName == 0 || theSize == 0)
		return E_POINTER;

	int aSz = (int) myString.size();
	int aBufSz = *theSize;

	* theSize = aSz;

	if ( aBufSz < aSz + 1)
		return E_OUTOFMEMORY;

	lstrcpyn(theOutName, myString.c_str(), aSz + 1);

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP LogicalItem_Run_Child::RetrieveIconData(IconExtractParams * theParams, void **)
{
	const ShellItemGenericIconManager * aMan = Application::InstanceC().GetShellItemGenericIconManager();

	HRESULT aRes = aMan->GetWellKnownIconData(OBJ_RunItem, theParams);

	if ( FAILED(aRes) )
		return aRes;
	else
		return MAKE_HRESULT(0, 0, CHANGE_ICON);
}

//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP RunItemFactory::CreateItem(const BYTE * theData, int theDataSize, Item ** theOutItem)
{
	*theOutItem = new LogicalItem_Run_Root();
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP RunItemFactory::GetNumItems( int * theOut )
{
	if (theOut == 0)
		return E_POINTER;

	*theOut = 1;
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP RunItemFactory::GetItemData(int theIndex, MenuSetItemData * theOut)
{
	if (theIndex != 0)
		return E_INVALIDARG;

	HRESULT aRes = GetItemName(theOut, theOut->Name, theOut->NameSize);

	if ( FAILED(aRes) )
		return aRes;

	theOut->Type = MenuSetItemData::ITEM;
	theOut->ObjectID = OBJ_RunItem;
	theOut->Param = 0;
	theOut->ParamSize = 0;
	theOut->IconIndex = -1;

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP RunItemFactory::GetItemName( const MenuSetItemData * theItem, TCHAR * theOut, int theBufSize )
{
	if (theOut == 0)
		return E_POINTER;

	if (theItem->Size != sizeof MenuSetItemData)
		return E_INVALIDARG;

	if (theBufSize < 4)
		return E_OUTOFMEMORY;

	lstrcpyn(theOut, _T("Run"), 4);
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP RunItemFactory::GetTypeGuid(GUID * theOut)
{
	if (theOut == 0)
		return E_POINTER;

	*theOut = OBJ_RunItem;
	return S_OK;
}

/*!

\page Run_Algorithm   Run Algorithm (reversed engineered)

The dialog proc of the run dialog is 0x7ca40121 on Windows XP SP3
which is part of shell32.dll. The procedure is exportable, the name is: TODO




*/
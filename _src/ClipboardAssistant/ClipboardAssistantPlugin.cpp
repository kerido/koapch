#include "Common.h"
#include <shlwapi.h>

#include "sdk_ComObject.h"
#include "sdk_Item.h"
#include "sdk_BinaryStream.h"

#include "ClipboardAssistantPlugin.h"
#include "Trace.h"

//////////////////////////////////////////////////////////////////////////////////////////////

// {0078193C-42C1-48c7-9FFA-3148BCC323D2}
static const GUID gGuid  = { 0x78193c, 0x42c1, 0x48c7, { 0x9f, 0xfa, 0x31, 0x48, 0xbc, 0xc3, 0x23, 0xd2 } };

//////////////////////////////////////////////////////////////////////////////////////////////

void * operator new(size_t theSize) throw()
	{ return HeapAlloc(GetProcessHeap(), 0, theSize); }

void operator delete(void * thePtr) throw()
	{ HeapFree(GetProcessHeap(), 0, thePtr); }

//////////////////////////////////////////////////////////////////////////////////////////////

class DECLSPEC_NOVTABLE ClipboardAssistantExecutor : public ComEntry1<IContextMenuExecutor>
{
private:
	IShellFolder * mySF;
	LPITEMIDLIST myPidl;

	UINT myMinID;
	UINT myID_FullPath, myID_TargetPath, myID_DirListing;

	ClipboardAssistantPlugin * myHost;

public:
	ClipboardAssistantExecutor(ClipboardAssistantPlugin * theHost, IShellFolder * theSF, LPITEMIDLIST thePidl) :
			myHost(theHost), mySF(theSF), myPidl(thePidl),
			myID_FullPath(0), myID_TargetPath(0), myID_DirListing(0)
	{
		ULONG aAttr = SFGAO_LINK|SFGAO_FILESYSANCESTOR;
		HRESULT aRes = mySF->GetAttributesOf(1, (LPCITEMIDLIST *) &myPidl, &aAttr);

		//Initially set command IDs to auxiliary values
		//so that they serve as flags for further item insertion

		myID_FullPath   = -1;
		myID_TargetPath = (aAttr & SFGAO_LINK)             != 0 ? -1 : 0;
		myID_DirListing = (aAttr & SFGAO_FILESYSANCESTOR ) != 0 ? -1 : 0;
	}

	~ClipboardAssistantExecutor()
	{
		mySF->Release();
		CoTaskMemFree(myPidl);
	}


// IContextMenuExecutor
protected:
	STDMETHODIMP PopulateMenu(HMENU theOutMenu, UINT theMinID, UINT theMaxID, ULONG theFlags)
	{
		int aNumMenuItems = 1;

		myID_FullPath = myMinID = theMinID;
		AppendMenu(theOutMenu, MF_STRING, myID_FullPath, _T("Copy full path") );

		if (myID_TargetPath == -1)
		{
			myID_TargetPath = theMinID + aNumMenuItems++;
			AppendMenu(theOutMenu, MF_STRING, myID_TargetPath, _T("Copy target path") );
		}

		if (myID_DirListing == -1)
		{
			myID_DirListing = theMinID + aNumMenuItems++;
			AppendMenu(theOutMenu, MF_STRING, myID_DirListing, _T("Copy directory listing") );
		}

		return MAKE_HRESULT(0, 0, aNumMenuItems);
	}

	STDMETHODIMP InvokeCommand(UINT theID, HWND theOwner, ULONG theExt)
	{
		if ( theID == myID_FullPath )
			return HandleCopyFullPath(theExt);

		else if ( theID == myID_TargetPath )
			return HandleCopyTargetPath(theExt);

		else if ( theID == myID_DirListing )
			return HandleCopyDirListing(theExt);

		else
			return E_INVALIDARG;
	}

	STDMETHODIMP HandleMessage(UINT theMsg, WPARAM theWParam, LPARAM theLParam, LRESULT * theRes)
		{ return E_NOTIMPL; }


protected:
	HRESULT HandleCopyFullPath(ULONG theExt)
	{
		BOOL aRes = OpenClipboard( myHost->GetHandlerWindow() );

		if (aRes == 0)
			return E_FAIL;

		aRes = EmptyClipboard();

		if (aRes == 0)
			return E_FAIL;

		TCHAR aDispName[1000];
		HRESULT aRet = GetDisplayName(aDispName, 1000);

		if ( SUCCEEDED(aRet) )
			CopyStringToClipboard(aDispName);

		CloseClipboard();

		return S_OK;
	}

	HRESULT HandleCopyTargetPath(ULONG theExt)
	{
		BOOL aRes = OpenClipboard( myHost->GetHandlerWindow() );

		if (aRes == 0)
			return E_FAIL;

		aRes = EmptyClipboard();

		HRESULT aRetVal;

		if (aRes != 0)
		{
			TCHAR aDispName[1000];

			IShellLink * aSL = 0;

			aRetVal = mySF->GetUIObjectOf(NULL,
				1,
				(LPCITEMIDLIST *) &myPidl,
				IID_IShellLink,
				NULL,
				(void **) &aSL);

			if ( SUCCEEDED(aRetVal) && aSL != 0)
			{
				aRetVal = aSL->GetPath(aDispName, sizeof aDispName / sizeof TCHAR - 1, NULL, NULL);

				if (SUCCEEDED (aRetVal) )
					CopyStringToClipboard(aDispName);

				aSL->Release();
			}
		}
		CloseClipboard();
		return aRetVal;
	}

	HRESULT HandleCopyDirListing(ULONG theExt)
	{
		MemoryStream<TCHAR, NoCrtMemMgr<TCHAR> > aStream;


		TCHAR aDispName[MAX_PATH];

		GetDisplayName(aDispName, MAX_PATH);
		lstrcat(aDispName, _T("\\*") );

		WIN32_FIND_DATA aWFD;
		HANDLE aFnd = FindFirstFile(aDispName, &aWFD);

		if (aFnd == INVALID_HANDLE_VALUE)
			return E_FAIL;

		while ( FindNextFile(aFnd, &aWFD) != FALSE )
		{
			size_t aFileNameLen = lstrlen(aWFD.cFileName);
			aStream.Write(aWFD.cFileName, 0, aFileNameLen);

			aStream.Write( _T("\r\n"), 0, 2);
		}

		aStream.Write( _T("\0"), 0, 1);

		FindClose(aFnd);

		BOOL aRes = OpenClipboard( myHost->GetHandlerWindow() );

		if (aRes == 0)
			return E_FAIL;

		aRes = EmptyClipboard();

		CopyStringToClipboard( aStream.GetBuffer() );

		CloseClipboard();
		return S_OK;
	}

	void CopyStringToClipboard(const TCHAR * theString)
	{
		size_t aSize = lstrlen(theString) + 1;	//we copy null also

		HANDLE aHndl = myHost->EnsureMemoryHandle(aSize * sizeof(wchar_t) );	//new size

		if (aHndl != 0)
		{
			TCHAR * aRaw = reinterpret_cast<TCHAR *>( GlobalLock(aHndl) );

			if (aRaw != 0)
			{
				NoCrtMemMgr<TCHAR>::Copy(aRaw, theString, aSize);
				GlobalUnlock(aHndl);

				SetClipboardData(CF_UNICODETEXT, aHndl);
			}
			else
				GlobalUnlock(aHndl);
		}
	}

	HRESULT GetDisplayName(TCHAR * theOut, UINT theNumChars)
	{
		STRRET aName;
		HRESULT aRet = mySF->GetDisplayNameOf(myPidl, SHGDN_NORMAL|SHGDN_FORADDRESSBAR|SHGDN_FORPARSING, &aName);

		if ( SUCCEEDED(aRet) )
			aRet = StrRetToBuf(&aName, myPidl, theOut, theNumChars);

		return aRet;
	}
};

//////////////////////////////////////////////////////////////////////////////////////////////

ClipboardAssistantPlugin::ClipboardAssistantPlugin( HINSTANCE theDllInstance )
	: myInstance(theDllInstance), myMemory(0), myCurSize(0)
{
	InitWindow();
}

//////////////////////////////////////////////////////////////////////////////////////////////

ClipboardAssistantPlugin::~ClipboardAssistantPlugin()
{
	DestroyWindow(myWindow);
	UnregisterClass(myClass.lpszClassName, myInstance);

	GlobalFree(myMemory);
}

//////////////////////////////////////////////////////////////////////////////////////////////

HGLOBAL ClipboardAssistantPlugin::EnsureMemoryHandle( size_t theNewSize )
{
	return myMemory = GlobalAlloc(GMEM_MOVEABLE|GMEM_ZEROINIT, myCurSize = theNewSize);
}

//////////////////////////////////////////////////////////////////////////////////////////////

HWND ClipboardAssistantPlugin::GetHandlerWindow() const
	{ return myWindow; }

//////////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP ClipboardAssistantPlugin::OnLoad(int theFrameworkVersionCode)
	{ return S_OK; }

//////////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP ClipboardAssistantPlugin::OnUnload()
	{ return S_OK; }

//////////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP ClipboardAssistantPlugin::QueryExecutor(Item * theItem, ULONG, ULONG, IContextMenuExecutor ** theOut)
{
	IApproachShellItem * aItem = 0;
	HRESULT aRes = theItem->QueryInterface(IID_IApproachShellItem, (void**) &aItem);

	if (FAILED(aRes) || aItem == 0)
		return aRes;


	IShellFolder * aSF = 0;
	LPITEMIDLIST aPidl = 0;

	aRes = aItem->GetShellItemData(IApproachShellItem::PRIMARY_RELATIVE, &aSF, &aPidl);

	aItem->Release();

	if ( SUCCEEDED(aRes) && aSF != 0 && aPidl != 0)
	{
		LPCITEMIDLIST aConstPidl = aPidl;
		ULONG aAttribs = SFGAO_FILESYSTEM;

		aRes = aSF->GetAttributesOf(1, &aConstPidl, &aAttribs);

		if ( SUCCEEDED(aRes) && (aAttribs & SFGAO_FILESYSTEM) != 0 )
			*theOut = new ComInstance<ClipboardAssistantExecutor>(this, aSF, aPidl);

		else
		{
			aSF->Release();
			CoTaskMemFree(aPidl);
		}
	}

	return aRes;
}

//////////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP ClipboardAssistantPlugin::CreateInstance( IUnknown * theOuter, REFIID theIID, void ** theOut )
{
	if (theIID != IID_IContextMenuExtension)
		return E_NOINTERFACE;

	IContextMenuExtension * aCME = this;
	aCME->AddRef();
	*theOut = aCME;

	return S_OK;

}

//////////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP ClipboardAssistantPlugin::LockServer( BOOL theLock )
{
	return E_NOTIMPL;
}

//////////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP ClipboardAssistantPlugin::GetTypeGuid(LPGUID theOutGuid)
{
	if (theOutGuid == 0)
		return E_POINTER;

	* theOutGuid = gGuid;
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////////////////////////

void ClipboardAssistantPlugin::InitWindow()
{
	//register class
	SecureZeroMemory(&myClass, sizeof(WNDCLASS) );

	myClass.lpfnWndProc    = WndProc;
	myClass.lpszClassName  = _T("ClipboardAssistandWnd");
	myClass.hInstance      = myInstance;

	RegisterClass(&myClass);

	//create a handler window
	myWindow = CreateWindow
		(
		myClass.lpszClassName,
		_T(""),
		0, 
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		NULL,
		0,
		myInstance,
		NULL);
}

//////////////////////////////////////////////////////////////////////////////////////////////

LRESULT __stdcall ClipboardAssistantPlugin::WndProc( HWND theWnd, UINT theMsg, WPARAM theWParam, LPARAM theLParam )
{
	return DefWindowProc(theWnd, theMsg, theWParam, theLParam);
}

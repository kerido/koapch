#include "stdafx.h"

#include "sdk_ContextMenuExtension.h"
#include "sdk_ComObject.h"
#include "sdk_Item.h"

#include "ShellItemContextMenuExtension.h"
#include "Application.h"
#include "IpcModule.h"
#include "InterProcessContextMenuInfo.h"
#include "MenuManager.h"
#include "UtilShellFunc.h"
#include "Prefs.h"

//////////////////////////////////////////////////////////////////////////////////////////////

class ShellItemContextMenuExecutor : public ComEntry1<IContextMenuExecutor>
{
private:
	IApproachShellItem *  myItem;  //!< Pointer to the item which will be used for obtaining the Shell data.
	IContextMenu  * myCM;          //!< Handles menu core operations, i.e. construction.
	IContextMenu2 * myCM2;         //!< Handles the WM_DRAWITEM, WM_MEASUREITEM, and WM_INITMENUPOPUP messages.
	IContextMenu3 * myCM3;         //!< Handles the WM_MENUCHAR message.

	HWND myActivateWnd;            //!< Handle to the window that originated invocation of a KO Approach menu.
	                               //!  \remarks This window may not belong to the KO Approach process.


public:
	ShellItemContextMenuExecutor (IApproachShellItem * theItem)
		: myItem(theItem), myCM(0), myCM2(0), myCM3(0)
	{
		myActivateWnd = MenuManager::GetIpcShellViewWnd();

		CComPtr<IShellFolder> aSF;
		LPITEMIDLIST aPidl;

		HRESULT aRes = myItem->GetShellItemData(IApproachShellItem::PRIMARY_RELATIVE, &aSF, &aPidl);

		if ( FAILED(aRes) || aSF == 0 || aPidl == 0)
			return;

		LPCITEMIDLIST aTemplPidl = aPidl;
		aRes = aSF->GetUIObjectOf(NULL, 1, &aTemplPidl, IID_IContextMenu, NULL, (void**)&myCM);

		if ( SUCCEEDED (aRes) && myCM != 0 )
		{
			aRes = myCM->QueryInterface(IID_IContextMenu2, (void**) &myCM2);
			aRes = myCM->QueryInterface(IID_IContextMenu3, (void**) &myCM3);
		}

		CoTaskMemFree(aPidl);
	}

	~ShellItemContextMenuExecutor()
	{
		if (myCM)  myCM ->Release();
		if (myCM2) myCM2->Release();
		if (myCM3) myCM3->Release();
	}


// IContextMenuExecutor Members
protected:

	STDMETHODIMP PopulateMenu(HMENU theOutMenu, UINT theMinID, UINT theMaxID, ULONG theFlags)
	{
		UINT aFlags = CMF_NORMAL;

		if ( (theFlags & DEFAULT_ONLY) != 0)
			aFlags = CMF_DEFAULTONLY;

		return myCM->QueryContextMenu (theOutMenu, 0, theMinID, theMaxID, aFlags);
	}

	STDMETHODIMP InvokeCommand(UINT theID, HWND theOwner, ULONG theExt)
	{
		CComPtr<IShellFolder> aSF;
		LPITEMIDLIST aPidl = 0;

		HRESULT aRes = myItem->GetShellItemData(IApproachShellItem::SECONDARY_RELATIVE, &aSF, &aPidl);

		char aVerbA[MAX_PATH];
		wchar_t aVerbW[MAX_PATH];

		aVerbA[0] = 0;
		aVerbW[0] = 0;

		const ApplicationSettings * aSt = Application::InstanceC().Prefs();

		bool aUseShellBrowser = myActivateWnd != 0;

		if (aUseShellBrowser)
		{
			//query the verbs
			aRes = myCM->GetCommandString(theID-1, GCS_VERBW, NULL, (char *) aVerbW, MAX_PATH);

			if ( SUCCEEDED(aRes) )
				aUseShellBrowser = (lstrcmpW(aVerbW, L"open") == 0);


			aRes  = myCM->GetCommandString(theID-1, GCS_VERBA, NULL, aVerbA, MAX_PATH);

			if ( SUCCEEDED(aRes) )
				aUseShellBrowser |= (lstrcmpA(aVerbA, "open") == 0);


			// When called as a result of a double click, the IContextMenu implementation returns
			// "open". However in the verb to open in the same window varies between "open" and
			// "explore", depending on the presence of the "folder navigation pane" which does not
			// matter at all

			if (aUseShellBrowser)
			{
				ULONG aAttribs = SFGAO_FOLDER;

				LPCITEMIDLIST aTempPidl = aPidl;
				aRes = aSF->GetAttributesOf(1, &aTempPidl, &aAttribs);

				aUseShellBrowser = SUCCEEDED(aRes) ? (aAttribs & SFGAO_FOLDER) != 0 : false;

				if (aUseShellBrowser)
					aUseShellBrowser = (aSt->GetActualBrowseFoldersMode() == PrefsSerz::BROWSE_EXISTINGWINDOW);
			}
		}

		// Invert aUseShellBrowser if the CTRL key is pressed
		if ( aSt->GetBrowseForFoldersMode() == PrefsSerz::BROWSE_USESYSTEM)
			if ( (GetKeyState(VK_CONTROL) & 0x8000) != 0)
				aUseShellBrowser = !aUseShellBrowser;


		char aDirA[MAX_PATH];
		wchar_t aDirW[MAX_PATH];

		aDirA[0] = 0;
		aDirW[0] = 0;

		GetDirectoryAnsi(aSF, aPidl, aDirA, MAX_PATH);        // query the directory
		GetDirectoryWide(aSF, aPidl, aDirW, MAX_PATH);        // in two formats


		//OSSPECIFIC
		if (aUseShellBrowser)
		{
			DWORD aThread = GetSafeWindowThreadId(myActivateWnd);

			InterProcessContextMenuInfo aDt(aSF, aPidl, myActivateWnd);

			aUseShellBrowser = aDt.ConstructAbsoluteFolderPidl();

			if (aUseShellBrowser)
			{
				IpcModule * aIpcMod = Application::Instance().GetIpcModule();
				aIpcMod->LLExecuteContextMenu(aThread, &aDt);
				
				aRes = S_OK;
			}
		}
		// end OSSPECIFIC


		if (!aUseShellBrowser)
		{
			CMINVOKECOMMANDINFOEX aCII;
			SecureZeroMemory(&aCII, sizeof CMINVOKECOMMANDINFOEX);

			aCII.cbSize       = sizeof CMINVOKECOMMANDINFOEX;
			aCII.nShow        = SW_SHOWNORMAL;
			aCII.fMask        = CMIC_MASK_ASYNCOK|CMIC_MASK_UNICODE;
			aCII.lpVerb       = MAKEINTRESOURCEA(theID-1);
			aCII.hwnd         = theOwner;
			aCII.lpDirectory  = aDirA;
			aCII.lpVerbW      = MAKEINTRESOURCEW(theID-1);
			aCII.lpDirectoryW = aDirW;

			aRes = myCM->InvokeCommand ( (CMINVOKECOMMANDINFO *) &aCII );

			if ( SUCCEEDED(aRes) )
				aRes = S_FALSE;
		}

		CoTaskMemFree(aPidl);
		return aRes;
	}

	STDMETHODIMP HandleMessage(UINT theMsg, WPARAM theWParam, LPARAM theLParam, LRESULT * theRes)
	{
		HRESULT aRes = E_FAIL;

		switch(theMsg)
		{
		case WM_INITMENUPOPUP:
		case WM_DRAWITEM:
		case WM_MEASUREITEM:
			if (myCM2 != 0)
			{
				aRes = myCM2->HandleMenuMsg (theMsg, theWParam, theLParam);

				if (SUCCEEDED(aRes))
					*theRes = 0;
			}
			break;


		case WM_MENUCHAR:
			if (myCM3 != 0)
				aRes = myCM3->HandleMenuMsg2 (WM_MENUCHAR, theWParam, theLParam, theRes);

			break;


		default:
			break;

		}

		return aRes;
	}


// Implementation Details
private:

	static void GetDirectoryAnsi(IShellFolder * theSF, LPCITEMIDLIST thePidl, char * theOut, int theLength)
	{
#ifdef _UNICODE
		TCHAR aDirectory[MAX_PATH];
		UtilShellFunc::GetDirectoryName(theSF, thePidl, aDirectory, MAX_PATH);

		WideCharToMultiByte(CP_ACP, 0, aDirectory, -1, theOut, theLength, NULL, NULL);
#else
		UtilShellFunc::GetDirectoryName(theSF, thePidl, theOut, theLength);
#endif //_UNICODE
	}

	static void GetDirectoryWide(IShellFolder * theSF, LPCITEMIDLIST thePidl, wchar_t * theOut, int theLength)
	{
#ifdef _UNICODE
		UtilShellFunc::GetDirectoryName(theSF, thePidl, theOut, theLength);
#else
		TCHAR aDirectory[MAX_PATH];
		UtilShellFunc::GetDirectoryName(theSF, thePidl, theOut, MAX_PATH);

		MultiByteToWideChar(CP_ACP, 0, aDirectory, -1, theOut, theLength);
#endif //_UNICODE
	}

	static DWORD GetSafeWindowThreadId( HWND theWnd )
	{
		DWORD aThread = GetWindowThreadProcessId( theWnd, NULL );

		if (aThread != 0)
			return aThread;

		//unable to identify the thread for theWnd, bind to the owner of the Start menu
		HWND aWnd = FindWindowEx(NULL, NULL, _T("Shell_TrayWnd"), NULL);
		return (aWnd != 0) ? GetWindowThreadProcessId(aWnd, NULL) : 0;
	}
};

//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP ShellItemContextMenuExtension::QueryExecutor(Item * theItem, ULONG theExt1, ULONG theExt2, IContextMenuExecutor ** theOut)
{
	CComQIPtr<IApproachShellItem> aItem = theItem;

	if (aItem == 0)
		return E_FAIL;

	*theOut = new ComInstance<ShellItemContextMenuExecutor>(aItem);
	return S_OK;
}
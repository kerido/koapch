#pragma once

#include "LogicCommon.h"
#include "Localization_Global.h"
#include "Framework.h"

//////////////////////////////////////////////////////////////////////////////////////////////


// TODO: this class is not used explicitly, only as a base class for ContextMenuHandler_IPC
class ContextMenuHandler : public IShellContextMenuHandler
{
protected:
	IContextMenu * myCM;
	IContextMenu2 * myCM2;
	IContextMenu3 * myCM3;

	WNDCLASS myClass;
	HWND myHwnd;
	HWND myActivateWnd;	//added 2006-03-21

	const char * myDirName;

	const static UINT MinID = 1, MaxID = 0x7FFF;
	const static UINT CancelID = MaxID + 1;


	struct ContextMenuExecutorData 
	{
		IContextMenuExecutor * Executor;
		int MinID;
		int MaxID;
	};

	typedef std::list<ContextMenuExecutorData> ContextMenuExecutorList;
	ContextMenuExecutorList::iterator          ContextMenuExecutorIter;
	ContextMenuExecutorList::const_iterator    ContextMenuExecutorIterC;


protected: enum FlagsEx { CHAIN_TO_BASE = 1 };

public:
	ContextMenuHandler() : myCM(0), myCM2(0), myCM3(0), myHwnd(0), myActivateWnd(0)
	{
		SecureZeroMemory(&myClass, sizeof(WNDCLASS) );
		InitWindow();
	}


	virtual ~ContextMenuHandler()
	{
		DestroyWindow(myHwnd);
		UnregisterClass(myClass.lpszClassName, myClass.hInstance);
	}


public:
	void InitWindow()
	{
		//register class
		SecureZeroMemory(&myClass, sizeof(WNDCLASS) );

		myClass.lpfnWndProc    = WndProc;
		myClass.lpszClassName  = _T("KOContextMenuHandlerStub");
		myClass.hInstance      = gThisModule;

		RegisterClass(&myClass);

		//create a handler window
		myHwnd = CreateWindow
			(
			myClass.lpszClassName,
			_T(""),
			0, 
			CW_USEDEFAULT, CW_USEDEFAULT,
			CW_USEDEFAULT, CW_USEDEFAULT,
			NULL,
			0,
			gThisModule,
			this);
	}

private:

	static LRESULT __stdcall WndProc(HWND theWnd, UINT theMsg, WPARAM theWParam, LPARAM theLParam)
	{
		ContextMenuHandler * aH = 0;
		switch (theMsg)
		{
		case WM_NCCREATE:
			aH = (ContextMenuHandler *) ( (CREATESTRUCT*) theLParam )->lpCreateParams;
#pragma warning(disable:4244)
			SetWindowLongPtr(theWnd, GWL_USERDATA, (LONG_PTR) aH);
#pragma warning(default:4244)
			return TRUE;

		default:
			break;
		}

#pragma warning(disable:4312)
		aH = (ContextMenuHandler *)GetWindowLongPtr(theWnd, GWL_USERDATA);
#pragma warning(default:4312)

		if (aH == 0)
			return DefWindowProc(theWnd, theMsg, theWParam, theLParam);
		else
			return aH->WndProc(theMsg, theWParam, theLParam);
	}

	LRESULT WndProc(UINT theMsg, WPARAM theWParam, LPARAM theLParam)
	{
		Trace("ContextMenuHandler::WndProc. theMsg = %x\n", theMsg);

		switch (theMsg)
		{
		case WM_MENUCHAR:
			if (myCM3)
			{
				LRESULT aRes = 0;
				myCM3->HandleMenuMsg2 (theMsg, theWParam, theLParam, &aRes);
				return (aRes);
			}
			else
				break;

		case WM_DRAWITEM:
		case WM_MEASUREITEM:
			return OnMenuMessage(theMsg, theWParam, theLParam);

		case WM_INITMENUPOPUP:
			SendMessage(myActivateWnd, theMsg, theWParam, theLParam);
			return OnMenuMessage(theMsg, theWParam, theLParam);
			
		case WM_MENUSELECT:
			return SendMessage(myActivateWnd, theMsg, theWParam, theLParam);

		default:
			break;
		}

		return DefWindowProc(myHwnd, theMsg, theWParam, theLParam);
	}

	LRESULT OnMenuMessage(UINT theMsg, WPARAM theWParam, LPARAM theLParam)
	{
		if (myCM2 != NULL)
			myCM2->HandleMenuMsg (theMsg, theWParam, theLParam);

		return (theMsg == WM_INITMENUPOPUP ? 0 : TRUE);
	}

public:

	//TODO: Change to DoHandle(Item * theItem, ...)
	virtual bool DoHandle(IShellFolder * theSF, LPCITEMIDLIST thePidl, HWND theActivateWnd, HWND theParentWnd, bool theDefaultOnly)
	{
		if ( QueryInterfaces(theSF, thePidl) )
		{
			myActivateWnd = theParentWnd;

			UINT aFlags = 0, aFlagsEx = 0;
			char aVerb[MAX_PATH];

			int aCmdID = GetCommandID(theDefaultOnly, aVerb, &aFlags, &aFlagsEx);

			if (aCmdID >= MinID && aCmdID <= MaxID)
				InvokeCommand(theSF, thePidl, theActivateWnd, aCmdID, aVerb, aFlags, aFlagsEx);

			//else need to check if some of the extensions might be processed

			ReleaseInterfaces();
			myActivateWnd = 0;
			return true;
		}
		else
			return false;
	}

	virtual bool DoHandle(Item * theItem, HWND theActivateWnd, HWND theParentWnd, bool theDefaultOnly)
	{
		IApproachShellItem * aItem = QUERY_ApproachShellItem::Request(theItem);


		if (aItem != 0)
		{
			IShellFolder * aSF = 0;
			LPITEMIDLIST aPidl = 0;

			bool aRet = aItem->GetShellItemData(IApproachShellItem::PRIMARY_RELATIVE, &aSF, &aPidl);
			QUERY_ApproachShellItem::Release(theItem, aItem);

			if ( aRet )
				aRet = DoHandle(aSF, aPidl, theActivateWnd, theParentWnd, theDefaultOnly);

			CoTaskMemFree(aPidl);
			aSF->Release();

			return aRet;
		}
		else
			return false;
	}

protected:
	virtual void InvokeCommand(IShellFolder * theSF, LPCITEMIDLIST thePidl,
		HWND theActivateWnd, int theCmdID, const char * theVerb,
		UINT theFlags, UINT theFlagsEx)
	{
		char aDir_Ansi[MAX_PATH] = { 0 };
		GetDirectoryAnsi(theSF, thePidl, aDir_Ansi, MAX_PATH);

		CMINVOKECOMMANDINFO aCII = {0};
		aCII.cbSize      = sizeof(CMINVOKECOMMANDINFO);
		aCII.nShow       = SW_SHOWNORMAL;
		aCII.fMask       = CMIC_MASK_ASYNCOK;

		aCII.lpVerb      = MAKEINTRESOURCEA(theCmdID-1);//theVerb;
		aCII.hwnd        = theActivateWnd;
		aCII.lpDirectory = aDir_Ansi;

		myCM->InvokeCommand (&aCII);  //invoke command
	}




protected:
	int GetCommandID(bool theDefaultOnly, char * theOutVerb, UINT * theFlags, UINT * theFlagsEx)
	{
		HMENU aMenu = CreatePopupMenu();

		int aCmdID = -1;
		UINT aFlags = 0;

		if (!theDefaultOnly)
		{
			HRESULT aRes = myCM->QueryContextMenu (aMenu, 0, MinID, MaxID, aFlags = CMF_NORMAL);


			int aMaxActual = HRESULT_CODE(aRes);

			if ( SUCCEEDED(aRes) )
			{
				TCHAR aCancelString[1000];
				ILocalizationManager * aMan = Framework::GetLocalizationManager();

				int aLen = 1000;
				HRESULT aRes = aMan->GetStringSafe( "IDS_MENU_CANCEL", 15, aCancelString, &aLen);

				if ( SUCCEEDED(aRes) )
				{
					lstrcat(aCancelString, _T("\tEsc") );
					AppendMenu(aMenu, MF_STRING, CancelID, aCancelString);
				}

				POINT aPt;
				GetCursorPos (&aPt);

				aCmdID = TrackPopupMenu (aMenu, TPM_LEFTALIGN|TPM_RETURNCMD|TPM_RIGHTBUTTON,
														aPt.x, aPt.y, 0, myHwnd, NULL);
			}
		}
		else
		{
			HRESULT aRes = myCM->QueryContextMenu (aMenu, 0, MinID, MaxID, aFlags = CMF_DEFAULTONLY);

			if ( SUCCEEDED(aRes) )
			 aCmdID = GetMenuDefaultItem(aMenu, FALSE, 0);
		}


		//query the verb

		HRESULT aRes = myCM->GetCommandString(aCmdID-1, GCS_VERBA, NULL, theOutVerb, MAX_PATH);

		if ( FAILED(aRes) )
			lstrcpyA(theOutVerb, MAKEINTRESOURCEA(aCmdID-1) );


		if (theFlags != 0)
				*theFlags = aFlags;

		if (theFlagsEx != 0)
		{
			MENUITEMINFO aInfo = {0};
			aInfo.cbSize = sizeof(MENUITEMINFO);
			aInfo.fMask = MIIM_SUBMENU;

			if ( GetMenuItemInfo(aMenu, (UINT) aCmdID-MinID, FALSE, &aInfo) != 0)
				if (aInfo.hSubMenu != NULL)
					*theFlagsEx = CHAIN_TO_BASE;
		}

		DestroyMenu (aMenu);
		return aCmdID;
	}

	bool QueryInterfaces(IShellFolder * theSF, LPCITEMIDLIST thePidl)
	{
		HRESULT aRes = theSF->GetUIObjectOf(NULL, 1, &thePidl, IID_IContextMenu, NULL, (void**)&myCM);

		if ( FAILED (aRes) || myCM == 0 )
			{ myCM = 0; return false; }

		myCM->QueryInterface(IID_IContextMenu2, (void**) &myCM2);
		myCM->QueryInterface(IID_IContextMenu3, (void**) &myCM3);

		return true;
	}

	void ReleaseInterfaces()
	{
		if (myCM)  myCM ->Release();
		if (myCM2) myCM2->Release();
		if (myCM3) myCM3->Release();

		myCM  = 0;
		myCM2 = 0;
		myCM3 = 0;
	}


protected:
	static void GetDirectoryAnsi(IShellFolder * theSF, LPCITEMIDLIST thePidl, char * theOut, int theLength)
	{
#ifdef _UNICODE
		TCHAR aDirectory[MAX_PATH] = { 0 };
		UtilShellFunc::GetDirectoryName(theSF, thePidl, aDirectory, MAX_PATH);

		WideCharToMultiByte(CP_ACP, 0, aDirectory, -1, theOut, theLength, NULL, NULL);
#else
		UtilShellFunc::GetDirectoryName(theSF, thePidl, theOut, theLength);
#endif //_UNICODE
	}

};


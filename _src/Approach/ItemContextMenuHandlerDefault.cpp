#include "stdafx.h"

#include "sdk_Item.h"

#include "Application.h"
#include "ItemContextMenuHandlerDefault.h"
#include "RootWindow.h"

#include "resource.h"
#include "auto_LocResource.h"

//////////////////////////////////////////////////////////////////////////////////////////////

//! Represents the context of a shortcut menu operation.
class ItemContextMenuHandlerDefault::ExecutionContext : public CMessageMap
{
private:
	//! Stores a pointer to the context menu executor and the range of
	//! menu item indexes within which the executor applies.
	struct ContextMenuExecutorData
	{
		IContextMenuExecutor * Executor;  //!< Pointer to the executor.
		UINT MinID;                       //!< The first (minimum) item index in the range.
		UINT MaxID;                       //!< The first (minimum) item index beyond the range.
	};

// Type definitions
private:
	typedef std::list<ContextMenuExecutorData>      ContextMenuExecutorList;
	typedef ContextMenuExecutorList::iterator       ContextMenuExecutorIter;
	typedef ContextMenuExecutorList::const_iterator ContextMenuExecutorIterC;


// Constants
private:
	const static UINT MinID    = 1;
	const static UINT CancelID = 0x7FFF;


// Fields
private:
	Item * myTargetItem;              //!< The item on which a context menu action is performed.
	RootWindow * myRootWindow;        //!< Pointer to the window whose procedure receives menu messages.
	ContextMenuExecutorList myExecs;  //!< List of executors which are currently in use.

	HWND myInitiatorWnd;              //!< Handle to the window that initiated a context menu operation.
	                                  //! \remarks Typically, it is an instance of a MenuWindow-derived class.

public:
	ExecutionContext(Item * theItem, HWND theInitiatorWnd) :
		myTargetItem(theItem),
		myInitiatorWnd(theInitiatorWnd),
		myRootWindow( Application::Instance().GetRootWindowInstance() )
	{
		myRootWindow->AddMessageMap(this);
	}

	~ExecutionContext()
	{
		for ( ContextMenuExecutorIter aIt = myExecs.begin(); aIt != myExecs.end(); aIt++)
			aIt->Executor->Release();

		myRootWindow->RemoveMessageMap(this);
	}


//WTL Messaging
	BEGIN_MSG_MAP(ItemContextMenuHandlerDefault::ExecutionContext)
		MESSAGE_HANDLER(WM_INITMENUPOPUP, MsgHandler_InitMenuPopup)
		MESSAGE_HANDLER(WM_MEASUREITEM,   MsgHandler_MeasureItem)
		MESSAGE_HANDLER(WM_DRAWITEM,      MsgHandler_DrawItem)
		MESSAGE_HANDLER(WM_MENUCHAR,      MsgHandler_MenuChar)
		MESSAGE_HANDLER(WM_MENUSELECT,    MsgHandler_MenuSelect)
	END_MSG_MAP()


public:
	//! Performs actual context menu operation invocation.
	//! 
	//! \param [in] theDefaultOnly
	//!     True if the operation should return the default command from
	//!     a context menu without displaying the menu itself; otherwise, false.
	//! 
	//! \return
	//!   - S_OK if the command was executed successfully.
	//!   - S_FALSE if the menu operation was cancelled.
	//!   - Otherwise, a COM error value.
	HRESULT DoHandle(bool theDefaultOnly)
	{
		UINT aCmdID = GetCommandID(theDefaultOnly);
		Trace("Command ID=%d\n", aCmdID);

		if (aCmdID < CancelID)
		{
			IContextMenuExecutor * aEx = FindExecutorByMenuID(aCmdID);

			if (aEx != 0)
				return aEx->InvokeCommand(aCmdID, myInitiatorWnd, 0);
			else
				return E_INVALIDARG;
		}

		else if (aCmdID == CancelID)	//cancel
			return S_FALSE;

		else
			return E_FAIL;
	}


//Message Handlers
private:

	//! Called when an owner-drawn menu item needs to be painted.
	//! 
	//! \param [in] theMsg
	//!     Always equals WM_DRAWITEM.
	//! 
	//! \param [in] theWParam
	//!     Always equals zero.
	//! 
	//! \param [in] theLParam
	//!     Pointer to the DRAWITEMSTRUCT structure describing the item that needs to be painted.
	//! 
	//! \param [in, out] theHandled
	//!     By default, TRUE. Upon completion can be set to FALSE to indicate that the message was
	//!     not handled and needs to be chained to the default message handler.
	//! 
	//! \return
	//!     TRUE, if the message has been processed successfully; otherwise, FALSE.
	LRESULT MsgHandler_DrawItem(UINT theMsg, WPARAM theWParam, LPARAM theLParam, BOOL & theHandled)
	{
		DRAWITEMSTRUCT * aMIS = (DRAWITEMSTRUCT *) theLParam;
		IContextMenuExecutor * aEx = FindExecutorByMenuID(aMIS->itemID);

		LRESULT aRet = 0;

		if (aEx != 0)
			aEx->HandleMessage(theMsg, theWParam, theLParam, &aRet);

		else
			theHandled = FALSE;

		return aRet;
	}


	//! Called when an owner-drawn menu item needs to be measured.
	//! 
	//! \param [in] theMsg
	//!     Always equals WM_MEASUREITEM.
	//! 
	//! \param [in] theWParam
	//!     Always equals zero.
	//! 
	//! \param [in] theLParam
	//!     Pointer to the MEASUREITEMSTRUCT structure describing the item that needs to be measured.
	//! 
	//! \param [in, out] theHandled
	//!     By default, TRUE. Upon completion can be set to FALSE to indicate that the message was
	//!     not handled and needs to be chained to the default message handler.
	//! 
	//! \return
	//!     TRUE, if the message has been processed successfully; otherwise, FALSE.
	LRESULT MsgHandler_MeasureItem(UINT theMsg, WPARAM theWParam, LPARAM theLParam, BOOL & theHandled)
	{
		MEASUREITEMSTRUCT * aMIS = (MEASUREITEMSTRUCT *) theLParam;
		IContextMenuExecutor * aEx = FindExecutorByMenuID(aMIS->itemID);

		LRESULT aRet = FALSE;

		if (aEx != 0)
			aEx->HandleMessage(theMsg, theWParam, theLParam, &aRet);

		else
			theHandled = FALSE;

		return aRet;
	}


	//! Called when a drop-down menu or submenu is about to become active.
	//! This allows an application to modify the menu before it is displayed,
	//! without changing the entire menu.
	//!
	//! \param [in] theMsg
	//!     Always equals WM_INITMENUPOPUP.
	//! 
	//! \param [in] theWParam
	//!     Handle to the drop-down menu or submenu.
	//! 
	//! \param [in] theLParam
	//!     The low-order word specifies the zero-based relative position of the
	//!     menu item that opens the drop-down menu or submenu.
	//!     The high-order word indicates whether the drop-down menu is the window menu.
	//!     If the menu is the window menu, this parameter is TRUE; otherwise, it is FALSE.
	//! 
	//! \param [in, out] theHandled
	//!     By default, TRUE. Upon completion can be set to FALSE to indicate that the message was
	//!     not handled and needs to be chained to the default message handler.
	//! 
	//! \return
	//!     Returns zero if the application processes this message; otherwise nonzero. 
	LRESULT MsgHandler_InitMenuPopup(UINT theMsg, WPARAM theWParam, LPARAM theLParam, BOOL & theHandled)
	{
		UINT aID = LOWORD(theLParam);

		IContextMenuExecutor * aEx = FindExecutorByMenuID(aID);

		LRESULT aRet = 0;

		if (aEx != 0)
			aEx->HandleMessage(theMsg, theWParam, theLParam, &aRet);

		else
			theHandled = FALSE;

		return aRet;
	}

	LRESULT MsgHandler_MenuChar(UINT theMsg, WPARAM theWParam, LPARAM theLParam, BOOL & theHandled)
	{
		//TODO: call executor

		theHandled = FALSE;
		return 0L;
	}

	LRESULT MsgHandler_MenuSelect(UINT theMsg, WPARAM theWParam, LPARAM theLParam, BOOL & theHandled)
	{
		//TODO: determine why this is necessary

		theHandled = TRUE;
		return 0L;
	}


// Implementation Details
private:

	//! Returns the ID of the menu command that needs to be executed.
	//! 
	//! \param [in] theDefaultOnly
	//!     If this parameter is true, no shortcut menu needs to be displayed,
	//!     but only the ID of the default menu command needs to be retrieved.
	//!     If this parameter is false, a shortcut menu is displayed and the method
	//!     will return the ID of the command that the user chooses.
	//! 
	//! \return
	//!   - The ID of the menu command that will be executed on a menu.
	//!   - The ID of the Cancel command instructing the menu to disappear.
	//!   - -1 in case of an error.
	//! 
	//! \remarks
	//!     Depending on the \a theDefaultOnly parameter, displays a shortcut menu
	//!     or simply returns the ID of the default menu command.
	int GetCommandID(bool theDefaultOnly)
	{
		Application & aApp = Application::Instance();
		int aNumCtxMenuExt = aApp.GetNumContextMenuExtensions();

		int aCmdID = -1;

		if (aNumCtxMenuExt == 0)
		{
			Trace("There are no context menu extensions. No action will be taken");
			return aCmdID;
		}

		HMENU aMenu = CreatePopupMenu();


		int aNumCtxMenuExtToProcess = theDefaultOnly ? 1 : aNumCtxMenuExt;
		ULONG aFlags = theDefaultOnly ? IContextMenuExecutor::DEFAULT_ONLY : IContextMenuExecutor::NORMAL;
		UINT aMin = MinID;  // initially set
		bool aSeparatorAdded = false;

		//HACK
		for (int i = 0; i < aNumCtxMenuExtToProcess; i++)
		{
			const ContextMenuExtensionData & aDt = aApp.GetContextMenuExtension(i);

			IContextMenuExecutor * aEx = 0;
			HRESULT aRes = aDt.Object->QueryExecutor(myTargetItem, 0L, 0L, &aEx);

			if (SUCCEEDED(aRes) && aEx != 0)
			{
				if ( !aSeparatorAdded )
				{
					AppendMenu(aMenu, MF_SEPARATOR, NULL, NULL);
					aSeparatorAdded = true;
				}

				aRes = aEx->PopulateMenu(aMenu, aMin, CancelID, aFlags);

				if ( SUCCEEDED(aRes) && HRESULT_CODE(aRes) > 0)
				{
					ContextMenuExecutorData  aDt;
					aDt.Executor = aEx;
					aDt.MinID = aMin;
					aDt.MaxID = aMin + HRESULT_CODE(aRes);

					myExecs.push_back(aDt);

					aMin = aDt.MaxID + 1;
				}
			}
		}
		//end HACK


		if (theDefaultOnly) 
			aCmdID = GetMenuDefaultItem(aMenu, FALSE, 0);

		else
		{
			// Add Cancel menu
			TCHAR aCancelString[1000];
			int aLen = 1000;

			ILocalizationManager * aMan = aApp.GetLocalizationManager();
			HRESULT aRes = aMan->GetStringSafe( KEYOF(IDS_GLBL_CMD_CANCEL), aCancelString, &aLen);

			if ( SUCCEEDED(aRes) )
			{
				lstrcat(aCancelString, _T("\tEsc") );
				AppendMenu(aMenu, MF_STRING, CancelID, aCancelString);
			}

			// Display the resulting menu
			POINT aPt;
			GetCursorPos (&aPt);

			aCmdID = TrackPopupMenu
			(
				aMenu,
				TPM_LEFTALIGN|TPM_RETURNCMD|TPM_RIGHTBUTTON,
				aPt.x,
				aPt.y,
				0,
				*myRootWindow, //theParentWnd,
				NULL
			);
		}

		DestroyMenu (aMenu);
		return aCmdID;
	}


	//! Obtains a context menu executor from a list of abailable objects
	//! based on the index of the menu item.
	//! 
	//! \param [in] theID
	//!     The index of the menu item for which the executor is being requested.
	//! 
	//! \return
	//!     A valid pointer if an object is found; otherwise, a null pointer.
	//! 
	//! \sa
	//!   - ContextMenuExecutorData
	IContextMenuExecutor * FindExecutorByMenuID(UINT theID)
	{
		if ( theID >= CancelID)
			return 0;

		for ( ContextMenuExecutorIter aIt = myExecs.begin(); aIt != myExecs.end(); aIt++)
			if (theID >= aIt->MinID && theID < aIt->MaxID)
				return aIt->Executor;

		return 0;
	}
};

//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

ItemContextMenuHandlerDefault::ItemContextMenuHandlerDefault() : myCurContext(0)
{ }

//////////////////////////////////////////////////////////////////////////////////////////////

HRESULT ItemContextMenuHandlerDefault::DoHandle(Item * theItem, HWND theParentWnd, bool theDefaultOnly)
{
	ExecutionContext aCtx(theItem, theParentWnd);

	myCurContext = &aCtx;  // Set the pointer to the context so that messages could be forwarded to it

	HRESULT aRes = aCtx.DoHandle(theDefaultOnly);

	myCurContext = 0;

	return aRes;
}
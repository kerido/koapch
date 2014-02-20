////////////////////////////////////////////////////////////////////////////////////////////////
//! 
//! \file
//!
//! The implementation of the UI MenuWindow_DisplayItemList and MenuWindow_Shell classes.
//! 
//! (c) 2002-2009 Kirill Osipov
//! 
////////////////////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "sdk_ItemProvider.h"
#include "sdk_MenuBuilder.h"
#include "sdk_Item.h"

#include "Application.h"
#include "MenuManager.h"
#include "DynamicDisplayItem.h"
#include "MenuWindow_Shell.h"
#include "UtilShellNotify.h"
#include "DisplayItemEventHandler.h"
#include "ShellIconBackgroundExtraction.h"

#include "util_Preferences.h"

////////////////////////////////////////////////////////////////////////////////////////////////

class AllEventHandlers
{
private:
	ClickItemHandler_Dumb     myDumb;
	ClickItemHandler_Activate myActivate;
	ClickItemHandler_Expand   myExpand;

public:
	const IDispayItemEventHandler * GetDisplayItemEventHandler(UINT theMouseMsg, DisplayItem * theItem) const
	{
		UINT aExpCode = theItem->GetExpansionCode();
		const ApplicationSettings * aSt = Application::InstanceC().Prefs();

		if (theMouseMsg == WM_LBUTTONUP)
		{
			if (aExpCode == IMenuBuilder::NON_EXPANDABLE && aSt->GetActivateMode() != PrefsSerz::ACTIVATE_DOUBLE_CLICK_ALL )
				return &myActivate;

			else
				return &myDumb;
		}

		else if (theMouseMsg == WM_LBUTTONDOWN)
		{
			if ( aExpCode != IMenuBuilder::NON_EXPANDABLE )
			{
				if ( aSt->GetActivateMode() == PrefsSerz::ACTIVATE_SINGLE_CLICK_ALL )
					return &myActivate;

				else
					return &myExpand;
			}
			else
				return &myDumb;
		}

		else if (theMouseMsg == WM_LBUTTONDBLCLK)
		{
			if ( aExpCode != IMenuBuilder::NON_EXPANDABLE  )
			{
				if ( aSt->GetActivateMode() != PrefsSerz::ACTIVATE_SINGLE_CLICK_ALL )
					return &myActivate;

				else
					return &myDumb;
			}

			else if ( aSt->GetActivateMode() == PrefsSerz::ACTIVATE_DOUBLE_CLICK_ALL )
				return &myActivate;

			else
				return &myDumb;
		}

		else return &myDumb;
	}

	const IDispayItemEventHandler * GetActivateHandler() const
	{ return &myActivate; }

	const IDispayItemEventHandler * GetExpandHandler() const
	{ return &myExpand; }
};

////////////////////////////////////////////////////////////////////////////////////////////////

static AllEventHandlers ourHandlers;	//TODO: remove as it uses static constrictor

////////////////////////////////////////////////////////////////////////////////////////////////

MenuWindow_DisplayItemList::MenuWindow_DisplayItemList(IMenuManagerAgent * theAgent) :
	MenuWindow(theAgent), myTop(true), myBottom(false),
	myPrevX(-1), myPrevY(-1), myDragDropThX(-1), myDragDropThY(-1),
	myTempDC(0), myAvailableTimerID(TimerID_FirstAvailable), myFlags( GetDefaultFlags() )
{
	ReleaseTraceCode(MENUWINDOW_CONSTRUCTOR);

	myMerged[0] = &myTop;
	myMerged[1] = &myMainItems;
	myMerged[2] = &myBottom;

	int aW = GetSystemMetrics(SM_CXSMICON);
	int aH = GetSystemMetrics(SM_CYSMICON);
	myIconList = ImageList_Create(aW, aH, ILC_COLOR32|ILC_MASK, 1, 1);
}

////////////////////////////////////////////////////////////////////////////////////////////////

MenuWindow_DisplayItemList::~MenuWindow_DisplayItemList()
{
	ReleaseTraceCode(MENUWINDOW_DESTRUCTOR);
	Trace("MenuWindow::~MenuWindow. HWND = %x\n", GetHandle() );

	SetFlag(FLAGS_ISDELETING, true);

	myCurSel.Set(0, 0);

	BackgroundIconExtractionManager & aBG = BackgroundIconExtractionManager::Instance();
	aBG.DeleteItems( GetHandle() );

	ReleaseTempDC();

	ImageList_Destroy(myIconList);
}

//////////////////////////////////////////////////////////////////////////////////////////////

int MenuWindow_DisplayItemList::GetDefaultFlags()
{
	int aRetVal = FLAGS_USEICONCACHE|FLAGS_ISTRACKINGMOUSELEAVE;

	const Application & aInst = Application::InstanceC();
	const ApplicationSettings * aSt = aInst.Prefs();

	if ( aSt->GetShowNumOfScrollItems() )
		aRetVal |= FLAGS_SHOWNUMBEROFSCROLLITEMS;

	return aRetVal;
}

//////////////////////////////////////////////////////////////////////////////////////////////

void MenuWindow_DisplayItemList::PopulateAndMeasure(IItemProvider * theProvider)
{
	myMainItems.OnPopulateStarted();

	ExceptItemType aTp = EMPTY;

	ULONG aFlags = PreferencesUtility::GetEnumItemsOptions();
	IEnumItems * aEI = 0;
	
	HRESULT aRes = theProvider->EnumItems(aFlags, &aEI);

	if ( SUCCEEDED(aRes) && aEI != 0)
	{
		Item * aItem = 0;
		ULONG aNumFetched = 0;        //shows how many items have been enumerated on each step

		while (true)
		{
			aRes = aEI->Next(1, &aItem, &aNumFetched);

			if (aItem == 0 || aNumFetched == 0 || FAILED(aRes))
				break;

			aTp = SUCCESS;
			myMainItems.AddItem( new DynamicDisplayItem(aItem) );
		}

		aEI->Release();
	}

	if ( FAILED(aRes) )
		aTp = UNAVAILABLE;

	if (aTp != SUCCESS)
		myMainItems.AddItem( new ExceptItem(aTp) );


	myMainItems.OnPopulateFinished();

	InitialLayout();

	OnPostPopulate();
}

//////////////////////////////////////////////////////////////////////////////////////////////

void MenuWindow_DisplayItemList::MsgHandler_Paint()
{
	Trace("MenuWindow::Draw\n");

	PAINTSTRUCT aPS;
	HDC aDC = BeginPaint(GetHandle(), &aPS);	//all drawing ops are inside BeginPaint/EndPaint

	for (int i = 0; i < 3; i++)
	{
		IDisplayItemHost * aIt = myMerged[i];

		const RECT & aViewRect = aIt->GetVisibleRect();

		if (aViewRect.left == 0 && aViewRect.right == 0 &&
		    aViewRect.top == 0 && aViewRect.bottom == 0 )
			continue;

		ItemDataIter aWr = aIt->CreateIterator();

		for ( aWr->SetRelativePosition(IItemDataIterator::Visible_Min);
		      aWr->IsAtRelativePosition(IItemDataIterator::Visible_Max) <= 0;
		      aWr->Next() )
		{
			ULONG aStateMask = ( myCurSel.GetCurrentDisplayItem() == aWr->GetCurrent().GetItem() ) ?
				DisplayItem::ITEMSTATE_SELECTED : DisplayItem::ITEMSTATE_NORMAL;

			DrawItem(aWr->GetCurrent(), aDC, aStateMask, DisplayItem::ITEMCONTENT_ALL);
		}
	}

	EndPaint(GetHandle(), &aPS);

	SetFlag(FLAGS_ALREADYPAINTED, true);
}

//////////////////////////////////////////////////////////////////////////////////////////////

void MenuWindow_DisplayItemList::MsgHandler_MouseMove(int theX, int theY)
{
	if (myPrevX == -1 && myPrevY == -1)
		myPrevX = theX, myPrevY = theY;

	else if ( myPrevX != theX || myPrevY != theY )
	{
		POINT aPt = { theX, theY };

		UpdateSelection(aPt, CHANGE_SELECTION_DEFAULT);

		myPrevX = theX, myPrevY = theY;
	}

	//drag-and-drop support
	if ( myDragDropThX != -1 && myDragDropThY != -1)
	{
		if ( (myDragDropThX - theX) * (myDragDropThX - theX) + (myDragDropThY - theY) * (myDragDropThY - theY) > 9)
		{
			if ( DisplayItem * aDI = myCurSel.GetCurrentDisplayItem() )
				if ( Item * aLI = aDI->GetLogicalItem() )
				{
					CComQIPtr<IApproachShellItem, &IID_IApproachShellItem> aItem(aLI);

					if (aItem != 0)
					{
						CComPtr<IShellFolder> aSF;
						LPITEMIDLIST aPidl = 0;

						HRESULT aHR = aItem->GetShellItemData(IApproachShellItem::PRIMARY_RELATIVE, &aSF, &aPidl);

						if ( SUCCEEDED(aHR) )
						{
							LPCITEMIDLIST aConstPidl = aPidl;

							CComPtr<IDataObject> aDO;
							aHR = aSF->GetUIObjectOf(NULL, 1, &aConstPidl, IID_IDataObject, 0, (void **) &aDO);

							if ( SUCCEEDED(aHR) && aDO != 0 )
							{
								DWORD aOut = 0;

								MenuManager::SetDragDropMode(true);

								aHR = DoDragDrop
								(
									aDO,
									this,
									DROPEFFECT_COPY|DROPEFFECT_MOVE|DROPEFFECT_LINK|DROPEFFECT_SCROLL,
									&aOut
								);

								MenuManager::SetDragDropMode(false);
							}

							CoTaskMemFree(aPidl);
						}
					}
				}
			myDragDropThX = myDragDropThY = -1;
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////

void MenuWindow_DisplayItemList::MsgHandler_LeftButton(UINT theMsg, int theX, int theY)
{
	Trace("MenuWindow_Shell::MsgHandler_LeftButton. theMsg = %x\n", theMsg);


	if (theMsg == WM_LBUTTONDOWN)
	{
		myDragDropThX = theX;
		myDragDropThY = theY;

		POINT aPt = {myDragDropThX, myDragDropThY};

		// Force the window to update the current selection. This is useful
		// because for some reason the menu might not be tracking mouse
		// movements. If this is the case, updating selection here
		// will improve the visual appeal
		UpdateSelection(aPt, CHANGE_SELECTION_DONT_NOTIFY);
	}
	else
	{
		myDragDropThX = -1;
		myDragDropThY = -1;
	}

	if ( myCurSel.Position != 0 )
	{
		DisplayItem * aDI = myCurSel.Position->GetCurrent().GetItem();

		if (aDI != 0)
		{
			const IDispayItemEventHandler * aCIH = ourHandlers.GetDisplayItemEventHandler(theMsg, aDI);
			ProcessClickItemHandler(aCIH);
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////

void MenuWindow_DisplayItemList::MsgHandler_ContextMenu(int theX, int theY)
{
	if ( theX != -1 && theY != -1 )
	{
		POINT aPt = {theX, theY};
		MapWindowPoints(NULL, GetHandle(), &aPt, 1);

		// Force the window to update the current selection. This is useful
		// because for some reason the menu might not be tracking mouse
		// movements. If this is the case, updating selection here
		// will improve the visual appeal
		UpdateSelection(aPt, CHANGE_SELECTION_DONT_NOTIFY);
	}

	Item * aLogItem = myCurSel.GetCurrentLogicalItem();

	SetFlag(FLAGS_ISTRACKINGMOUSELEAVE, false);
	ClickItemHandler_Activate::ProcessLogicalItem(this, aLogItem, false);
	SetFlag(FLAGS_ISTRACKINGMOUSELEAVE, true);
}

//////////////////////////////////////////////////////////////////////////////////////////////

void MenuWindow_DisplayItemList::MsgHandler_KeyDown(UINT theVKeyCode)
{
	UINT
		aPrev = MenuManager::GetNavKey(IMenuKeyboardNav::KEY_PREV),
		aNext = MenuManager::GetNavKey(IMenuKeyboardNav::KEY_NEXT);

	if (theVKeyCode == VK_ESCAPE || theVKeyCode == aPrev)
	{
		MenuManager::DestroyMenuChain(this);
	}
	else if (theVKeyCode == aNext)
	{
		ProcessClickItemHandler( ourHandlers.GetExpandHandler() );
	}
	else if (theVKeyCode == VK_RETURN)
	{
		ProcessClickItemHandler( ourHandlers.GetActivateHandler() );
	}
	else if (theVKeyCode == VK_UP)
	{
		IItemDataIterator * aIter = FindNeighborSelectable(
			&IItemDataIterator::Prev,
			IItemDataIterator::EntireContents_Max,
			IItemDataIterator::EntireContents_Min);

		if (aIter != 0)
			EnsureSelectionVisible(aIter, CHANGE_SELECTION_DONT_NOTIFY);
	}
	else if (theVKeyCode == VK_DOWN)
	{
		IItemDataIterator * aIter = FindNeighborSelectable(
			&IItemDataIterator::Next,
			IItemDataIterator::EntireContents_Min,
			IItemDataIterator::EntireContents_Max);

		if (aIter != 0)
			EnsureSelectionVisible(aIter, CHANGE_SELECTION_DONT_NOTIFY);
	}
	else if (theVKeyCode == VK_PRIOR)
	{
		if ( myMainItems.GetOffset() > myMainItems.GetMinOffset() )
		{
			int aNumItems = Application::InstanceC().Prefs()->GetNumScrollItemsPage();

			int aOffset = myMainItems.GetNewOffsetAfterScroll(-aNumItems);
			LayoutViews(aOffset);
			InvalidateRect(myHandle, NULL, FALSE);
		}
	}
	else if (theVKeyCode == VK_NEXT)
	{
		if ( myMainItems.GetOffset() < myMainItems.GetMaxOffset() )
		{
			int aNumItems = Application::InstanceC().Prefs()->GetNumScrollItemsPage();

			int aOffset = myMainItems.GetNewOffsetAfterScroll(aNumItems);
			LayoutViews(aOffset);
			InvalidateRect(myHandle, NULL, FALSE);
		}
	}
	else if (theVKeyCode == VK_HOME)
	{
		if (myMainItems.GetOffset() > myMainItems.GetMinOffset() )
		{
			LayoutViews ( myMainItems.GetMinOffset() );
			InvalidateRect(myHandle, NULL, FALSE);
		}
	}
	else if (theVKeyCode == VK_END)
	{
		if (myMainItems.GetOffset() < myMainItems.GetMaxOffset() )
		{
			LayoutViews( myMainItems.GetMaxOffset() );
			InvalidateRect(myHandle, NULL, FALSE);
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////

void MenuWindow_DisplayItemList::MsgHandler_MouseLeave()
{
	//Step 1. Delete all timers

	KillTimer(TimerID_ConstructChild);

	if ( !HasFlag(FLAGS_ISTRACKINGMOUSELEAVE) )
		return;

	else if (myChild == 0)
		ClearSelection();

	else
		RevertSelection();
}

//////////////////////////////////////////////////////////////////////////////////////////////

void MenuWindow_DisplayItemList::MsgHandler_MouseWheel(int theDelta)
{
	Trace("MenuWindow_DisplayItemList -- MsgHandler_MouseWheel. theDelta=%d\n", theDelta);

	if (GetChild() != 0)
		MenuManager::DestroyMenuChain( GetChild() );

	int aNumItemsToScroll = Application::InstanceC().Prefs()->GetNumScrollItemsWheel();


	if (theDelta < 0)
	{
		if ( myMainItems.GetOffset() < myMainItems.GetMaxOffset() )
			Scroll(aNumItemsToScroll, 1);
	}
	else
	{
		if ( myMainItems.GetOffset() > myMainItems.GetMinOffset() )
			Scroll(-aNumItemsToScroll, 1);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////

void MenuWindow_DisplayItemList::MsgHandler_Timer(int theID)
{
	TimerIter aIt = myTimerStorage.find(theID);

	if ( aIt != myTimerStorage.end() )
		aIt->second.Target->OnTimer(theID, 0, aIt->second.Arg);
}

//////////////////////////////////////////////////////////////////////////////////////////////

void MenuWindow_DisplayItemList::MsgHandler_Char(TCHAR theChar)
{
	mySearchBuf += theChar;
	int aLen = (int) mySearchBuf.size();

	bool aFound = false;

	ItemDataIter aWr = myMainItems.CreateIterator();

	for ( aWr->SetRelativePosition(IItemDataIterator::EntireContents_Min);
	      aWr->IsAtRelativePosition(IItemDataIterator::EntireContents_Max) <= 0;
	      aWr->Next() )
	{
		DisplayItem * aDI = aWr->GetCurrent().GetItem();

		IDisplayItemGetName * aDiGn = 0;
		HRESULT aRes = aDI->QueryInterface(IID_IDisplayItemGetName, (void **) &aDiGn);

		if ( FAILED (aRes) )
			continue;

		TCHAR aDisplayName[1024];
		int aSize = 1024;

		aRes = aDiGn->GetDisplayString(aDisplayName, &aSize);
		aDiGn->Release();

		if ( FAILED (aRes) || aSize == 0 ) continue;

		if (aSize < aLen)   continue;

		int aEquals = CompareString(LOCALE_INVARIANT, NORM_IGNORECASE, aDisplayName, aLen, mySearchBuf.c_str(), aLen);

		if (aEquals == CSTR_EQUAL) { aFound = true; break; }
	}

	if (aFound)
		EnsureSelectionVisible(aWr, CHANGE_SELECTION_DONT_NOTIFY);

	SetTimer(TimerID_SearchBufCleanup, 500, (ITimerTarget *) this, 0);
}

//////////////////////////////////////////////////////////////////////////////////////////////

void MenuWindow_DisplayItemList::MsgHandler_FetchIcon(int theItemID, HRESULT theFetchResult)
{
	ItemDataIter aWr = myMainItems.CreateIterator();

	bool aRet = aWr->SetAbsolutePosition(theItemID);

	if (aRet)
		OnIconFetched( &aWr->GetCurrent(), theFetchResult );
}

//////////////////////////////////////////////////////////////////////////////////////////////

void MenuWindow_DisplayItemList::DrawItem( const ItemData & theItemData, HDC theDC,
                                           ULONG theStateMask, ULONG theContentMask )
{
	RECT aItemWndRect;
	theItemData.GetWindowRect(aItemWndRect);

	const RECT & aViewVisibleRect = theItemData.GetHost()->GetVisibleRect();

	RECT aVisibleRect;
	::IntersectRect(&aVisibleRect, &aItemWndRect, &aViewVisibleRect);

	if ( IsRectEmpty(aVisibleRect) )
		return;

	ItemDrawData aDt (theDC, aItemWndRect, theStateMask, theContentMask);


	IntersectClipRect(theDC, aVisibleRect.left, aVisibleRect.top, aVisibleRect.right, aVisibleRect.bottom);

	DisplayItem * aItem = theItemData.GetItem();
	aItem->Draw(&aDt, myTheme);

	SelectClipRgn(theDC, NULL);
}

//////////////////////////////////////////////////////////////////////////////////////////////

void MenuWindow_DisplayItemList::OnIconFetched(const ItemData * theItemData, HRESULT theIconFetchResult)
{
	int aIconUpdateCode = HRESULT_CODE(theIconFetchResult);

	bool aIconUpdated = (aIconUpdateCode & IIconAcceptor::CHANGE_ALL) != 0;

	if (aIconUpdated && ::IsWindowVisible(myHandle) && HasFlag(FLAGS_ALREADYPAINTED) )
	{
		ULONG aStateMask = myCurSel.GetCurrentDisplayItem() == theItemData->GetItem() ?
			DisplayItem::ITEMSTATE_SELECTED : DisplayItem::ITEMSTATE_NORMAL;

		DrawItem(*theItemData, EnsureTempDC(), aStateMask, DisplayItem::ITEMCONTENT_ICON);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////

void MenuWindow_DisplayItemList::EnsureSelectionVisible(IItemDataIterator * thePosition, int theFlags)
{
	//TODO: support for non-standard scroll item positioning

	RECT aWndRect;
	thePosition->GetCurrent().GetWindowRect(aWndRect);

	const RECT & aViewVisibleRect = myMainItems.GetVisibleRect();

	if(aWndRect.bottom > aViewVisibleRect.bottom)
	{
		int aOffset = myMainItems.GetOffset();
		aOffset += aWndRect.bottom - aViewVisibleRect.bottom;

		LayoutViews(aOffset);
		ChangeSelectionNoRedraw(&myMainItems, thePosition);

		::InvalidateRect(myHandle, NULL, FALSE);
	}

	else if(aWndRect.top < aViewVisibleRect.top)
	{
		int aOffset = myMainItems.GetOffset();
		aOffset -= aViewVisibleRect.top - aWndRect.top;

		LayoutViews(aOffset);
		ChangeSelectionNoRedraw(&myMainItems, thePosition);

		::InvalidateRect(myHandle, NULL, FALSE);
	}
	else
		ChangeSelection(&myMainItems, thePosition, theFlags);
}

//////////////////////////////////////////////////////////////////////////////////////////////

void MenuWindow_DisplayItemList::ChangeSelection(IDisplayItemHost * theHost, IItemDataIterator * thePosition, int theFlags)
{
	DisplayItem * aOld = myCurSel.GetPreviousItem();
	DisplayItem * aNew = thePosition->GetCurrent().GetItem();

	if (aOld != aNew)
	{
		HDC aTempDC = GetDC( myHandle );

		HRESULT aRes = S_OK;
		CComQIPtr<ISelectable> aSel;

		if(aOld != 0 && myCurSel.Host != 0)
		{
			aSel = aOld;

			if (aSel != 0)
				aRes = aSel->OnSelect(ISelectable::DESELECTED);

			bool aNeedToRedraw = SUCCEEDED(aRes) && (HRESULT_CODE(aRes) & ISelectable::REDRAW) != 0;

			if (aNeedToRedraw)
				DrawItem(*myCurSel.Previous, aTempDC, DisplayItem::ITEMSTATE_NORMAL, DisplayItem::ITEMCONTENT_ALL);
		}

		if (aNew != 0 && theHost != 0)
		{
			aSel = aNew;

			if (aSel != 0)
				aRes = aSel->OnSelect(ISelectable::SELECTED);

			bool aNeedToRedraw = SUCCEEDED(aRes) && (HRESULT_CODE(aRes) & ISelectable::REDRAW) != 0;

			if (aNeedToRedraw)
				DrawItem(thePosition->GetCurrent(), aTempDC, DisplayItem::ITEMSTATE_SELECTED, DisplayItem::ITEMCONTENT_ALL);
		}

		ReleaseDC(myHandle, aTempDC);

		myCurSel.Set(theHost, thePosition);

		if ( (theFlags & CHANGE_SELECTION_DONT_NOTIFY) == 0 )
			OnSelectionChanged();
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////

void MenuWindow_DisplayItemList::ChangeSelectionNoRedraw(IDisplayItemHost * theHost, IItemDataIterator * thePosition)
{
	DisplayItem * aOld = myCurSel.GetPreviousItem();
	DisplayItem * aNew = thePosition->GetCurrent().GetItem();

	if (aOld != aNew)
	{
		CComQIPtr<ISelectable> aSel = aOld;

		if(aSel != 0)
			aSel->OnSelect(ISelectable::DESELECTED);

		aSel = aNew;

		if (aSel != 0)
			aSel->OnSelect(ISelectable::SELECTED);

		myCurSel.Set(theHost, thePosition);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////

void MenuWindow_DisplayItemList::ClearSelection()
{
	if (myCurSel.Previous != 0 && myCurSel.Host != 0)
	{
		CComQIPtr<ISelectable> aSel = myCurSel.Previous->GetItem();

		if (aSel != 0)
		{
			HRESULT aRes = aSel->OnSelect(ISelectable::DESELECTED);

			bool aNeedToRedraw = SUCCEEDED(aRes) && (HRESULT_CODE(aRes) & ISelectable::REDRAW) != 0;

			if (aNeedToRedraw)
			{
				HDC aTempDC = EnsureTempDC();
				DrawItem(*myCurSel.Previous, aTempDC, DisplayItem::ITEMSTATE_NORMAL, DisplayItem::ITEMCONTENT_ALL);
			}
		}
	}

	myCurSel.Set(0, 0);
	OnSelectionChanged();
}

//////////////////////////////////////////////////////////////////////////////////////////////

bool MenuWindow_DisplayItemList::UpdateSelection(const POINT & thePt, int theFlags)
{
	IDisplayItemHost * aCurActiveView = 0;

	for(int i = 0; i < 3; i++)
	{
		IDisplayItemHost * aIt = myMerged[i];
		const RECT & aRct = aIt->GetVisibleRect();

		if ( ::PtInRect(&aRct, thePt) == TRUE )
		{
			aCurActiveView = aIt;
			break;
		}
	}

	if (aCurActiveView == 0)
	{
		ClearSelection();  // no view was found
	}
	else
	{
		ItemDataIter aWr = aCurActiveView->CreateIterator();

		for ( aWr->SetRelativePosition(IItemDataIterator::Visible_Min);
			aWr->IsAtRelativePosition(IItemDataIterator::Visible_Max) <= 0;
			aWr->Next() )
		{
			const ItemData & aData = aWr->GetCurrent();

			RECT aWndRect;
			aData.GetWindowRect(aWndRect);

			if ( ::PtInRect(&aWndRect, thePt) == TRUE )
			{
				ChangeSelection(aCurActiveView, aWr, theFlags);
				break;
			}
		}
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////

void MenuWindow_DisplayItemList::RevertSelection()
{
	IDisplayItemHost * aHost = myCurActiveItem->GetHost();
	IItemDataIterator * aIter = 0;

	if (aHost != myCurSel.Host)
		aIter = aHost->CreateIterator();
	else
		aIter = myCurSel.Position;

	const ItemData * aNew = myCurActiveItem;
	aIter->SetAbsolutePosition(*aNew);

	EnsureSelectionVisible(aIter, CHANGE_SELECTION_DONT_NOTIFY);
}

//////////////////////////////////////////////////////////////////////////////////////////////

IItemDataIterator * MenuWindow_DisplayItemList::FindNeighborSelectable(TravFunc theTraverse, TravBoundary theInitial, TravBoundary theCheck)
{
	IItemDataIterator * aIter = 0;
	bool aNeedToTraverse = true;

	if (myCurSel.Host == &myMainItems)
		aIter = myCurSel.Position;

	if (aIter == 0)
	{
		aIter = myMainItems.CreateIterator();
		aIter->SetRelativePosition(theInitial);
		aNeedToTraverse = false;
	}

	const DisplayItem * aInit = aIter->GetCurrent().GetItem();

	bool aFound = false;
	bool aMadeALoop = false;

	do
	{
		if( aIter->IsAtRelativePosition(theCheck) == 0 )
		{
			aMadeALoop = true;
			aIter->SetRelativePosition(theInitial);
		}
		else if (aNeedToTraverse)
			(aIter->*theTraverse) ();

		aNeedToTraverse = true;

		const ItemData & aData = aIter->GetCurrent();
		DisplayItem * aItem = aData.GetItem();

		if (aMadeALoop && aItem == aInit)
			break;
		
		CComQIPtr<ISelectable> aSel = aItem;

		if (aSel != 0)
			aFound = true;
	}
	while(aFound == false);

	if (aFound)
		return aIter;
	else
		return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////

void MenuWindow_DisplayItemList::OnSelectionChanged()
{
	Item * aItem = myCurSel.GetCurrentLogicalItem();

	if (aItem)
		SetTimer(TimerID_ConstructChild, Application::InstanceC().Prefs()->GetTimeoutSec(), this, 0);

	else
		KillTimer(TimerID_ConstructChild);
}

//////////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP MenuWindow_DisplayItemList::Scroll(int theOffset, int theFlags)
{
	int aNewOffset = myMainItems.GetNewOffsetAfterScroll(theOffset);
	LayoutViews(aNewOffset);
	InvalidateRect(myHandle, NULL, FALSE);

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////////////////////////

void MenuWindow_DisplayItemList::SetTheme(Theme * theTheme)
{
	MenuWindow::SetTheme(theTheme);

	for (int i = 0; i < 3; i++)
		myMerged[i]->Initialize(this);
}

//////////////////////////////////////////////////////////////////////////////////////////////

void MenuWindow_DisplayItemList::SetChild(MenuWindow * theChild)
{
	if (theChild == 0)
		myCurActiveItem = 0;

	MenuWindow::SetChild(theChild);
}

//////////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP MenuWindow_DisplayItemList::GetAvailableTimerID(int * theOutTimerID)
{
	if (theOutTimerID == 0)
		return E_POINTER;

	*theOutTimerID = myAvailableTimerID++;
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP MenuWindow_DisplayItemList::SetTimer(int theID, int theTimeout, ITimerTarget * theTarget, void * theArg)
{
	myTimerStorage[theID] = TimerHandlerData(theTarget, theArg);
	::SetTimer( GetHandle(), (UINT_PTR) theID, (UINT) theTimeout, 0);

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP MenuWindow_DisplayItemList::KillTimer(int theID)
{
	if ( !HasFlag(FLAGS_ISDELETING) )
		myTimerStorage.erase(theID);

	::KillTimer( GetHandle(), (UINT_PTR) theID );

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP MenuWindow_DisplayItemList::OnTimer(int theID, int, void *)
{
	if (theID == TimerID_SearchBufCleanup)
	{
		mySearchBuf.clear();
		KillTimer(theID);
	}
	else if (theID == TimerID_ConstructChild)
	{
		KillTimer(TimerID_ConstructChild);
		ProcessClickItemHandler( ourHandlers.GetExpandHandler() );
	}

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////////////////////////

LRESULT MenuWindow_DisplayItemList::WndProc(HWND theWnd, UINT theMsg, WPARAM wParam, LPARAM lParam)
{
	switch (theMsg)
	{
	case WM_MOUSEMOVE:
		MsgHandler_MouseMove( GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) );
		break;

	case WM_PAINT:
		MsgHandler_Paint();
		break;

	case WM_LBUTTONDBLCLK:
	case WM_LBUTTONUP:
	case WM_LBUTTONDOWN:
		MsgHandler_LeftButton(theMsg, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) );
		return TRUE;

	case WM_KEYDOWN:
		MsgHandler_KeyDown((UINT) wParam);
		break;

	case WM_CHAR:
		MsgHandler_Char((TCHAR) wParam);
		break;

	case WM_CONTEXTMENU:
		MsgHandler_ContextMenu(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) );
		break;

	case WM_TIMER:
		MsgHandler_Timer( (int) wParam );
		break;

	case WM_MOUSELEAVE:
		MsgHandler_MouseLeave();
		break;

	case WM_MOUSEWHEEL:
		MsgHandler_MouseWheel( (int) GET_WHEEL_DELTA_WPARAM(wParam) );
		break;

	case WM_USER_FETCHICON:
		MsgHandler_FetchIcon( (int) wParam, (HRESULT) lParam );
		break;

	default:
		return DefWindowProc(GetHandle(), theMsg, wParam, lParam);
	}

	return 0L;
}

//////////////////////////////////////////////////////////////////////////////////////////////

void MenuWindow_DisplayItemList::OnPostPopulate()
{
	RECT aWndRect = myClientRect;  // convert from client rect back to window rect

	DWORD aStyle = GetWindowLong(GetHandle(), GWL_STYLE);
	DWORD aExStyle = GetWindowLong(GetHandle(), GWL_EXSTYLE);
	AdjustWindowRectEx( &aWndRect, aStyle, FALSE, aExStyle);

	int
		aWidth = aWndRect.right - aWndRect.left,
		aHeight = aWndRect.bottom - aWndRect.top;

	SetWindowPos( GetHandle(), NULL, 0, 0, aWidth, aHeight, SWP_NOZORDER|SWP_NOMOVE|SWP_NOREDRAW|SWP_NOACTIVATE);



	// try to extract fast icons

	ItemDataIter aWr = myMainItems.CreateIterator();

	PerWindowIconExtraction * aIconExtr = 0;

	for (aWr->SetRelativePosition(IItemDataIterator::EntireContents_Min);
		   aWr->IsAtRelativePosition(IItemDataIterator::EntireContents_Max) <= 0;
		   aWr->Next() )
	{
		const ItemData & aItemData = aWr->GetCurrent();

		DisplayItem * aDI = aItemData.GetItem();

		CComQIPtr<IIconAcceptor> aAcc(aDI);

		if (aAcc == 0)
			continue;

		IconExtractParams aPars;
		aPars.ImageList = myIconList;
		aPars.Flags = HasFlag(FLAGS_USEICONCACHE) ? EXTRACT_DEFAULT|EXTRACT_PREFERFAST : CACHE_IGNORE|EXTRACT_PREFERFAST;

		void * aAsyncArg = 0;
		HRESULT aRes = aAcc->RetrieveIconData(&aPars, &aAsyncArg);

		if (aRes == E_PENDING)
		{
			if (aIconExtr == 0)
				aIconExtr = BackgroundIconExtractionManager::Instance().EnsurePerWindowData(GetHandle(), WM_USER_FETCHICON);

			aIconExtr->QueueIconExtraction(aItemData.GetID(), aDI, aPars, aAsyncArg);
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP MenuWindow_DisplayItemList::GiveFeedback( DWORD theEffect )
{
	return DRAGDROP_S_USEDEFAULTCURSORS;
}

//////////////////////////////////////////////////////////////////////////////////////////////

HRESULT __stdcall MenuWindow_DisplayItemList::QueryContinueDrag(BOOL theEsc, DWORD theKeyState)
{
	if ( theEsc == TRUE)
		return DRAGDROP_S_CANCEL;

	else if ( (theKeyState & MK_LBUTTON) == 0)
		return DRAGDROP_S_DROP;

	else
		return S_OK;
}

//////////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP MenuWindow_DisplayItemList::RegisterProcessor(IMessageProcessor * theProcessor)
{
	myMsgProcs.push_back(theProcessor);
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP MenuWindow_DisplayItemList::UnregisterProcessor(IMessageProcessor * theProcessor)
{
	HRESULT aRes = S_FALSE;
	for (MessageProcessorIter aIt = myMsgProcs.begin(); aIt != myMsgProcs.end();)
	{
		IMessageProcessor * aPrc = *aIt;

		if (aPrc == theProcessor)
		{
			aRes = S_OK;
			aIt = myMsgProcs.erase(aIt);
		}
		else
			aIt++;
	}

	return aRes;
}

//////////////////////////////////////////////////////////////////////////////////////////////

void MenuWindow_DisplayItemList::InitialLayout()
{
	// TODO: multiple monitor support

	const RECT & aEntire = myMainItems.GetEntireRect();
	myClientRect = aEntire;

	int aOwnerWidth = aEntire.right - aEntire.left;
	int aOwnerHeight = aEntire.bottom - aEntire.top;
	int aOwnerOrigin = 0;

	myTop.SetOwnerWidth(aOwnerWidth);
	myBottom.SetOwnerWidth(aOwnerWidth);

	int aMaxHeight = GetMaxWindowHeight();

	if ( aEntire.bottom - aEntire.top > aMaxHeight)
	{
		// need to adjust everything to accommodate scrolling arrows
		aOwnerHeight = 0;

		const RECT & aRectT = myTop.GetEntireRect();
		int aTopHeight = aRectT.bottom - aRectT.top;

		const RECT & aRectB = myBottom.GetEntireRect();
		int aBottomHeight = aRectB.bottom - aRectB.top;

		RECT aFit = aEntire;	// the rectangle (in the coordinates of the list view) that can fit
		int aScrollItemsHeight = aBottomHeight; //leave room for bottom item

		const ApplicationSettings * aPrefs = Application::InstanceC().Prefs();
		ApplicationSettings::ScrollItemFlags aLayoutScenario = aPrefs->GetScrollItemPositioning();

		if (aLayoutScenario != ApplicationSettings::SIF_POS_RESPECTIVE)
		{
			aScrollItemsHeight += aTopHeight;           // leave room for top item also
			aOwnerHeight += aTopHeight + aBottomHeight; // items don't ever disappear

			if (aLayoutScenario == ApplicationSettings::SIF_POS_TOP)
				aOwnerOrigin += aBottomHeight + aTopHeight;
		}

		aFit.bottom = aMaxHeight - aScrollItemsHeight;

		myMainItems.FitClippingRectangle(aFit);

		myClientRect.bottom = aFit.bottom - aFit.top + aScrollItemsHeight;

		aOwnerHeight = myClientRect.bottom - myClientRect.top - aOwnerHeight;
	}

	myMainItems.SetOwnerHeight(aOwnerHeight);
	myMainItems.SetOrigin(aOwnerOrigin);

	LayoutViews( myMainItems.GetMinOffset() );
}

//////////////////////////////////////////////////////////////////////////////////////////////

void MenuWindow_DisplayItemList::LayoutViews(int theMainItemsNewOffset)
{
	myMainItems.OnLayoutChangeStarted();

	int aOffset = theMainItemsNewOffset;

	aOffset = __min(myMainItems.GetMaxOffset(), aOffset);
	aOffset = __max(myMainItems.GetMinOffset(), aOffset);

	myMainItems.SetOffset( aOffset );

	RECT aMainRect = myClientRect;                 // aMainRect is in window coordinates

	const ApplicationSettings * aPrefs = Application::InstanceC().Prefs();
	ApplicationSettings::ScrollItemFlags aLayoutScenario = aPrefs->GetScrollItemPositioning();

	if (aLayoutScenario == ApplicationSettings::SIF_POS_RESPECTIVE)
	{
		if ( aOffset > myMainItems.GetMinOffset() )
		{
			RECT aRect = myTop.GetEntireRect();        // aRect is now in view's coordinates
			int aHeight = aRect.bottom - aRect.top;

			aRect.top = myClientRect.top;
			aRect.bottom = myClientRect.top + aHeight; // aRect is now in window coordinates

			myTop.SetVisibleRect(aRect);               // Set the view's visible area (in window coords)

			aMainRect.top += aHeight;
		}
		else
			HideView(&myTop);

		if ( aOffset < myMainItems.GetMaxOffset() )
		{
			RECT aRect = myBottom.GetEntireRect();     // aRect is now in view's coordinates
			int aHeight = aRect.bottom - aRect.top;

			aRect.top = myClientRect.bottom - aHeight; // aRect is now in window coordinates
			aRect.bottom = myClientRect.bottom;

			myBottom.SetVisibleRect(aRect);            // Set the view's visible area (in window coords)
			aMainRect.bottom -= aHeight;
		}
		else
			HideView(&myBottom);
	}
	else
	{
		int aOrigin = 0;

		if ( aOffset > myMainItems.GetMinOffset() || aOffset < myMainItems.GetMaxOffset() )
		{
			RECT aRectT = myTop.GetEntireRect();
			int aHeightT = aRectT.bottom - aRectT.top;

			RECT aRectB = myBottom.GetEntireRect();
			int aHeightB = aRectB.bottom - aRectB.top;

			if ( aLayoutScenario == ApplicationSettings::SIF_POS_TOP)
			{
				aOrigin += aHeightB + aHeightT;	//reserve space at top for both scroll items

				aRectT.top = myClientRect.top;
				aRectT.bottom = aRectT.top + aHeightT;

				aRectB.top = aRectT.bottom;
				aRectB.bottom = aRectB.top + aHeightB;

				aMainRect.top += aOrigin;
			}
			else
			{
				aRectB.bottom = myClientRect.bottom;
				aRectB.top = aRectB.bottom - aHeightB;

				aRectT.bottom = aRectB.top;
				aRectT.top = aRectT.bottom - aHeightT;

				aMainRect.bottom -= aHeightB + aHeightT;
			}

			myTop.SetVisibleRect(aRectT);
			myTop.SetEnabled( aOffset > myMainItems.GetMinOffset() );

			myBottom.SetVisibleRect(aRectB);
			myBottom.SetEnabled( aOffset < myMainItems.GetMaxOffset() );
		}
		else
		{
			HideView(&myTop);
			HideView(&myBottom);
		}
	}


	myMainItems.SetVisibleRect(aMainRect);

	myMainItems.OnLayoutChangeFinished();


	if ( HasFlag(FLAGS_SHOWNUMBEROFSCROLLITEMS) )
	{
		if ( IsViewVisible(&myTop) )
			myTop.SetNumRemainingItems( myMainItems.GetMinVisibleIndex() );

		if ( IsViewVisible(&myBottom) )
			myBottom.SetNumRemainingItems( myMainItems.GetItemCount() - myMainItems.GetMaxVisibleIndex() - 1);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////

void MenuWindow_DisplayItemList::HideView(IDisplayItemHost * theView)
{
	RECT aRect;
	SecureZeroMemory(&aRect, sizeof RECT);

	theView->SetVisibleRect(aRect);

	//update current selection

	if (myCurSel.Host == theView)
		ClearSelection();
}

//////////////////////////////////////////////////////////////////////////////////////////////

void MenuWindow_DisplayItemList::ProcessClickItemHandler(const IDispayItemEventHandler * theHandler)
{
	if (myCurSel.Position != 0)
		theHandler->DoProcess(this, myCurSel.Position->GetCurrent() );
}

//////////////////////////////////////////////////////////////////////////////////////////////

int MenuWindow_DisplayItemList::GetMaxWindowHeight()
{
	int aCY      = GetSystemMetrics(SM_CYSCREEN),
		aBorderY = myTheme->GetMetric(HEIGHTWIDTH_MENUBORDER);

	return aCY - 2*aBorderY;
}

//////////////////////////////////////////////////////////////////////////////////////////////

bool MenuWindow_DisplayItemList::IsChildMenuPresentFromItem(const ItemData & theItemData) const
{
	DisplayItem * aCurActive = 0;
	if (myCurActiveItem != 0)
		aCurActive = myCurActiveItem->GetItem();

	if (aCurActive == 0)
		return false;
	else
		return aCurActive == theItemData.GetItem();
}

//////////////////////////////////////////////////////////////////////////////////////////////

void MenuWindow_DisplayItemList::OnChildMenuConstructedFromItem( const ItemData & theItemData )
{
	myCurActiveItem = theItemData.Clone();
}

//////////////////////////////////////////////////////////////////////////////////////////////

void MenuWindow_DisplayItemList::SelectItem( int theIndex )
{
	ItemDataIter aWr = myMainItems.CreateIterator();

	int i = 0;
	for (aWr->SetRelativePosition(IItemDataIterator::EntireContents_Min);
	     aWr->IsAtRelativePosition(IItemDataIterator::EntireContents_Max) <= 0 && i < theIndex;
	     aWr->Next() ) i++;

	int aFlags = CHANGE_SELECTION_DONT_NOTIFY;

	if ( IsWindowVisible(myHandle) )
		aFlags |= CHANGE_SELECTION_DONT_REDRAW;

	ChangeSelection(&myMainItems, aWr,aFlags);
}

//////////////////////////////////////////////////////////////////////////////////////////////

bool MenuWindow_DisplayItemList::GetSelectionRect( RECT & theOutRect )
{
	if (myCurSel.Position == 0)
		return false;

	myCurSel.Position->GetCurrent().GetWindowRect(theOutRect);
	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////

ULONG STDMETHODCALLTYPE MenuWindow_DisplayItemList::AddRef()
{
	return MenuWindow::AddRef();
}

//////////////////////////////////////////////////////////////////////////////////////////////

ULONG STDMETHODCALLTYPE MenuWindow_DisplayItemList::Release()
{
	return MenuWindow::Release();
}

//////////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP MenuWindow_DisplayItemList::QueryInterface(REFIID theIID, void ** theOut)
{
	HRESULT aRes = InternalQueryInterface(theIID, this, theOut);

	if ( FAILED(aRes) )
		return aRes;

	AddRef();
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//                                 MenuWindow_Shell class
//////////////////////////////////////////////////////////////////////////////////////////////

MenuWindow_Shell::MenuWindow_Shell(IMenuManagerAgent * theAgent)
	: MenuWindow_DisplayItemList(theAgent), myRestorableItem(0), myChangeNotifyID(0L)
{
	ZeroMemory(&myRestorableRect, sizeof(RECT) );
}

//////////////////////////////////////////////////////////////////////////////////////////////

MenuWindow_Shell::~MenuWindow_Shell()
{
	SetFlag(FLAGS_ISDELETING, true);

	if (myChangeNotifyID != 0)
		SHChangeNotifyDeregister(myChangeNotifyID);
}

//////////////////////////////////////////////////////////////////////////////////////////////

void MenuWindow_Shell::SetRestorableData(Item * theItem, const RECT & theRect)
{
	CComQIPtr<IApproachShellItem, &IID_IApproachShellItem> aItem(theItem);

	if (aItem != 0)
	{
		LPITEMIDLIST aFullPidl = 0;
		IShellFolder * aSF = 0;       //not needed

		HRESULT aRes = aItem->GetShellItemData(IApproachShellItem::SECONDARY_FULL, &aSF, &aFullPidl);

		if ( SUCCEEDED(aRes) )
		{
			if (aSF != 0)
				aSF->Release();

			if (aFullPidl != 0)
			{
				myRestorableItem = theItem;
				myRestorableRect = theRect;

				myChangeNotifyID = UtilShellNotify::ChangeNotifyRegister(aFullPidl, GetHandle(), WM_USER_CHANGENOTIFY, false);

				CoTaskMemFree(aFullPidl);
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////

void MenuWindow_Shell::MsgHandler_ChangeNotify(DWORD theCode, UtilShellNotify::SHNOTIFYSTRUCT * theNotify)
{
#ifdef _DEBUG
	char aBuf[2000];

	UtilShellNotify::OutputChangeNotifyEvent(theCode, theNotify, aBuf);
	Trace("MenuWindow_Shell::MsgHandler_ChangeNotify. this=0x%x, hWnd=0x%x. Codes=%s\n", this, GetHandle(), aBuf);

#endif	//# _DEBUG

	if ( UtilShellNotify::CommonCanIgnoreEventByCode(theCode) )
		return;

	// Because we may be processing other messages (such as WM_CONTEXTMENU)
	// and because this menu may rebuild by first destroying itself,
	// defer handling of this message by posting another message.
	PostMessage(GetHandle(), WM_USER_CHANGENOTIFY_HANDLE, 0, 0);
}

//////////////////////////////////////////////////////////////////////////////////////////////

void MenuWindow_Shell::MsgHandler_ChangeNotify_Handle()
{
	if ( HasFlag(FLAGS_ISDELETING) )
		return;

	Item * aRestoreItem = myRestorableItem;

	if (!aRestoreItem) return;

	RECT aRestoreRect = myRestorableRect;

	MenuWindow * aParent = GetParent();

	MenuManager::DestroyMenuChain(this);

	//it is unsafe to query 'this' after destruction
	MenuManager::ConstructByDataItem(aRestoreItem, aParent, &aRestoreRect);
}

//////////////////////////////////////////////////////////////////////////////////////////////

LRESULT MenuWindow_Shell::WndProc(HWND theWnd, UINT theMsg, WPARAM wParam, LPARAM lParam)
{
	switch (theMsg)
	{
	case WM_USER_CHANGENOTIFY:	//custom message, based upon WM_USER
		MsgHandler_ChangeNotify( (DWORD) lParam, (UtilShellNotify::SHNOTIFYSTRUCT *) wParam );
		break;

	case WM_USER_CHANGENOTIFY_HANDLE:	//custom message, based upon WM_USER
		MsgHandler_ChangeNotify_Handle();
		break;

	default:
		return MenuWindow_DisplayItemList::WndProc(theWnd, theMsg, wParam, lParam);
	}
	return 0;
}

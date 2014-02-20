//////////////////////////////////////////////////////////////////////////////////////////////

#pragma once



/*
MenuWindow.h : the draft of the interface of UI MenuWindow class
This class represents a window similar to a menu shown in Approach.
The window itself is responsible for performing actions only in the window...
well, sounds ugly. Say, it can only redraw itself, populate and so on.
A WindowManager tells a window whether to perform such an action or not.
Neither window knows when to display a child window or when to terminate.
*/

/*! \page MenuWindow_Scrolling Scrolling scenario (the simplest possible)

We've got a special interface, ISelectable, which can be dynamically supported by multiple
DisplayItem classes. The MenuWindow instance queries this interface in two cases:

1. The user's mouse cursor moves over a DisplayItem
2. The user presses one of the navigation keys (arrow keys, for example)

Scroll items also implement the ISelectable interface. In case the user hovers the item
the OnSelect is called. Inside this method's the item creates a timer, say, with the
TIMER_SCROLL id, which is fired every certain period of time until the OnDeselect method
is called. The latter would destroy that time.

Because we store a vector of DisplayItem instances we could simply store indexes of the
first and the last items that are diplayed. The TIMER_SCROLL procedure updates those indexes
dependently on the scroll direction and then redraws the main contents (typically, excluding
the scroll items and other content not subject to scrolling)

What we should be aware of is that the query scenario based on keyboard events is different
from the mouse-based one. Typically scroll items aren't anyhow affected by keystroke events.
The MenuWindow must simply query the interface from its dynamic content skipping the scroll
items.

This third parameter might be used for distinguishing the scenario which we target while
querying the logic: either KB- or mouse-based. However this might not be very essential for
scroll items are already treated alternatively from the rest of the MenuWindow content. If so
there is no need in using this third parameter because the window itself can do the filtering
job.
*/


/*! \page MenuWindow_PopulateMeasure Populate-and-Measure scenario

There is a special method for doing this. We must pay attention item width and the number of
items in the MenuWindow.

The item width can be determined by summing the length of the display text, the icon width,
gap sizes, and the auxiliary region sizes. There is a maximum width a MenuWindow can have.
If we reach this maximum width there is no need to calculate the width. In this case we
preserve resources by not performing lengthy GDI operations.

The number of items is important because there is a certain number of items, by reaching which
we need to switch the window into scrolling mode. Then we must also determine the scrolling
dimensions so that there was space left for scroll items.

*/

//////////////////////////////////////////////////////////////////////////////////////////////

#include "sdk_MenuWindow.h"

#include "typedefs.h"
#include "LogicCommon.h"
#include "ScrollItem.h"
#include "UtilShellNotify.h"
#include "Trace.h"
#include "DisplayItemView.h"

//////////////////////////////////////////////////////////////////////////////////////////////

class Theme;
class DisplayItem;
class Item;
interface IItemProvider;
class IDispayItemEventHandler;

//////////////////////////////////////////////////////////////////////////////////////////////

//! Represents an Approach Menu consisting of a series of DisplayItem instances.
class MenuWindow_DisplayItemList :
	public MenuWindow,
	public ComEntry5<ITimerHandler, ITimerTarget, IScrollable, IDropSource, IMessageProcessorStorage>
{
// Constants
protected:
	static const int MaxSearchBufLength       = 5;

// Timer ID's
protected:
	static const int TimerID_SearchBufCleanup = 1;      //!< Fired when the keyboard search buffer needs to be emptied 
	static const int TimerID_ConstructChild   = 2;      //!< Fired when the child window needs to be built (after selection
	                                                    //!  has been changed via mouse move)
	static const int TimerID_FirstAvailable   = 256;    //!< 255 timers is reserved for MenuWindow's internal needs,
	                                                    //!  the rest goes to ITimerHandler-s

	static const UINT WM_USER_FETCHICON = WM_USER + 1;
	static const UINT WM_USER_FIRSTAVAILABLE = WM_USER_FETCHICON + 1;

private:
	typedef std::map<int, TimerHandlerData> TimerStorage;
	typedef TimerStorage::iterator          TimerIter;
	typedef TimerStorage::const_iterator    TimerIterC;

	typedef std::list<IMessageProcessor *>       MessageProcessorList;
	typedef MessageProcessorList::iterator       MessageProcessorIter;
	typedef MessageProcessorList::const_iterator MessageProcessorIterC;

	typedef IItemDataIterator::TraverseFunc            TravFunc;
	typedef IItemDataIterator::IteratorBoundary        TravBoundary;


protected:

	//! Specifies the way current selection needs to be changed.
	enum ChangeSelectionFlags
	{
		//! Do regular processing
		CHANGE_SELECTION_DEFAULT = 0,

		//! Informs the menu window that it should not redraw
		//! the items whose state has changed
		CHANGE_SELECTION_DONT_REDRAW = 1,

		//! Informs the menu window that it should not perform any post-change
		//! actions (i.e. notify the menu manager that the selection has changed).
		//! Consequently, with this flag set, no TimerID_ConstructChild will be
		//! fired to further build the child menu
		CHANGE_SELECTION_DONT_NOTIFY = 2
	};

	enum WindowFlags
	{
		FLAGS_ISDELETING              =  1,  //!< Set to true in the destructor body in order to speed-up interface pointer release.
		FLAGS_ISTRACKINGMOUSELEAVE    =  2,  //!< Set to true when context menu is displayed in order to retain current selection.
		                                     //!  When true, the current selection will not be cleared even when the cursor leaves the client area.
		FLAGS_USEICONCACHE            =  4,  //!< Specifies if the icon cache should be used for querying icons.
		FLAGS_ALREADYPAINTED          =  8,  //!< True when the window has been fully painted at least once.
		FLAGS_ISFLOATING              = 16,  //!< Currently not used.
		FLAGS_SHOWNUMBEROFSCROLLITEMS = 32,  //!< When set, instructs scroll items to display the number of items left to scroll
	};

	struct CurrentSelectionData
	{
		ItemDataIter       Position;
		IDisplayItemHost * Host;
		ItemDataWrapper    Previous;

		CurrentSelectionData() : Host(0), Previous(0) { }
		~CurrentSelectionData() { }

		DisplayItem * GetPreviousItem()
		{
			if (Previous == 0)
				return 0;
			else
				return Previous->GetItem();
		}

		DisplayItem * GetCurrentDisplayItem() const
		{
			if (Position == 0)
				return 0;
			else
				return Position->GetCurrent().GetItem();
		}

		Item * GetCurrentLogicalItem() const
		{
			DisplayItem * aDI = GetCurrentDisplayItem();

			if (aDI == 0)
				return 0;
			else
				return aDI->GetLogicalItem();
		}

		void Set(IDisplayItemHost * theHost, IItemDataIterator * thePosition)
		{
			Host = theHost;
			Position = thePosition;

			if (thePosition != 0) Previous = thePosition->GetCurrent().Clone();
			else                  Previous = 0;
		}
	};

//data members
protected:
	ItemListView myMainItems;
	ScrollItemView myTop, myBottom;
	IDisplayItemHost * myMerged[3];

	CurrentSelectionData myCurSel;

	TimerStorage myTimerStorage;

	String mySearchBuf;                //!< Stores a short string that the user types when searching for an item

	int
		myPrevX,                         //!< TODO
		myPrevY,                         //!< TODO
		myDragDropThX,                   //!< TODO
		myDragDropThY,                   //!< TODO
		myAvailableTimerID,              //!< TODO
		myFlags;                         //!< TODO

	HIMAGELIST myIconList;             //!< The image list that contains icons for items

	HDC myTempDC;                      //!< The Device Context used for immediate item drawing. Such
	                                   //!  drawing happens when the item's icon is successfully fetched.

	MessageProcessorList myMsgProcs;   //!< TODO

	RECT myClientRect;                 //!< the window's client rect (cached from the call to GetClientRect)
	ItemDataWrapper myCurActiveItem;   //!< the item for which a child menu is already constructed


// Constructors, destructor
public:
	MenuWindow_DisplayItemList(IMenuManagerAgent * theAgent);
	virtual ~MenuWindow_DisplayItemList();


// Main methods
public:
	bool HasFlag(int theFlag) const        { return (myFlags & theFlag) == theFlag; }
	void SetFlag(int theFlag, bool theSet) { if (theSet) myFlags |= theFlag; else myFlags &= ~theFlag; }

	static int GetDefaultFlags();

	//! Populates the list of items and measures the required dimensions of the window
	//! \param [in] theProvider     The item provider from which Item instances will be requested.
	void PopulateAndMeasure(IItemProvider * theProvider);

	//! Sets the visible area of a specified view to an empty rectangle
	//! and ensures that no item from the specified view is selected.
	//! 
	//! \param [in] theView     The view to be hidden.
	void HideView(IDisplayItemHost * theView);

	//! Returns the maximum height, in pixels, of the window's client area.
	int GetMaxWindowHeight();

	//! TODO
	bool IsChildMenuPresentFromItem(const ItemData & theItemData) const;

	//! TODO
	void OnChildMenuConstructedFromItem(const ItemData & theItemData);

	//! TODO
	void SelectItem(int theIndex);

	//! Retrieves selection rectangle in window coordinates.
	//! \return  True if the rectangle was successfully retrieved; otherwise, false.
	bool GetSelectionRect(RECT & theOutRect);


protected:
	HDC EnsureTempDC()
	{
		if (myTempDC == 0)
			myTempDC = GetDC( GetHandle() );

		return myTempDC;
	}

	void ReleaseTempDC()
	{
		if (myTempDC == 0)
			return;
	}



// MenuWindow overrides
public:
	virtual void SetTheme(Theme * theTheme);
	virtual void SetChild(MenuWindow * theChild);
	virtual LRESULT WndProc(HWND theWnd, UINT theMsg, WPARAM wParam, LPARAM lParam);


// Overridables
protected:

	//! TODO
	virtual void OnPostPopulate();

	//! Sets the dimensions of the views hosted by the current menu object
	//! according to the dimensions obtained during population.
	virtual void InitialLayout();


// ITimerHandler members
protected:
	STDMETHODIMP GetAvailableTimerID(int * theOutTimerID);
	STDMETHODIMP SetTimer(int theID, int theTimeout, ITimerTarget * theTarget, void * theArg);
	STDMETHODIMP KillTimer(int theID);


// ITimerTarget members
protected:
	STDMETHODIMP OnTimer(int theID, int theElapsed, void * theArg);


// IScrollable members
protected:
	STDMETHODIMP Scroll(int theDeltaItems, int theFlags);


// IDropTarget Members
protected:
	STDMETHODIMP GiveFeedback(DWORD theEffect);

	STDMETHODIMP QueryContinueDrag(BOOL theEsc, DWORD theKeyState);


// IMessageProcessorStorage Members
public:
	STDMETHODIMP RegisterProcessor(IMessageProcessor * theProcessor);

	STDMETHODIMP UnregisterProcessor(IMessageProcessor * theProcessor);


// IUnknown Members
public:
	ULONG STDMETHODCALLTYPE AddRef();

	ULONG STDMETHODCALLTYPE Release();

	STDMETHODIMP QueryInterface(REFIID theIID, void ** theOut);


// Win32 Message Handlers
private:
	void MsgHandler_Paint();
	void MsgHandler_MouseMove(int theX, int theY);

	//! Handles left button up, down, and double click.
	void MsgHandler_LeftButton(UINT theMsg, int theX, int theY);
	void MsgHandler_ContextMenu(int theX, int theY);
	void MsgHandler_KeyDown(UINT theVKeyCode);
	void MsgHandler_Char(TCHAR theChar);
	void MsgHandler_MouseLeave();
	void MsgHandler_MouseWheel(int theDelta);
	void MsgHandler_Timer(int theID);
	void MsgHandler_FetchIcon(int theItemID, HRESULT theFetchResult);


// Implementation Details
private:

	/// <summary>
	///    TODO
	/// </summary>
	//TODO: move LayoutViews to layout algorithm
	void LayoutViews(int theMainItemsNewOffset);

	static bool IsViewVisible(const IDisplayItemHost * theHost)
	{
		const RECT & aViewRect = theHost->GetVisibleRect();
		return !IsRectEmpty(aViewRect);
	}

	static bool IsRectEmpty(const RECT & aViewRect)
	{
		return 
			aViewRect.right  <= aViewRect.left &&
			aViewRect.bottom <= aViewRect.top;
	}


	//! Called when a DisplayItem needs to be painted.
	//! 
	//! \param [in] theItemData
	//!     The item and its bounding rectangle.
	//! 
	//! \param [in] theDC
	//!     The device context which performs all drawing operations.
	//! 
	//! \param [in] theStateMask
	//!     One of the values from the ItemState enumeration.
	//! 
	//! \param [in] theContentMask
	//!     One or more values from the ItemContent enumeration.
	//! 
	//! \remarks
	//!     Called in the following situations:
	//!  -# In DrawView (while handling WM_PAINT message).
	//!  -# From ChangeSelectionWithRedraw.
	//!  -# When drawing scroll items (while handling WM_PAINT message).
	//!  -# When the item's icon is fetched.
	void DrawItem(const ItemData & theItemData, HDC theDC, ULONG theStateMask, ULONG theContentMask);


	void EnsureSelectionVisible(IItemDataIterator * thePosion, int theFlags);

	/// <summary>
	///    Sets the current selection to the item that is below the specified point.
	///    Optionally, can fire the child window creation timer.
	/// </summary>
	/// <param name="thePt">TODO</param>
	/// <param name="theFlags">TODO</param>
	bool UpdateSelection(const POINT & thePt, int theFlags);

	/// <summary>TODO</summary>
	/// <param name="theHost">TODO</param>
	/// <param name="thePosition">TODO</param>
	/// <param name="theFlags">TODO</param>
	void ChangeSelection(IDisplayItemHost * theHost, IItemDataIterator * thePosition, int theFlags);

	void ChangeSelectionNoRedraw(IDisplayItemHost * theHost, IItemDataIterator * thePosition);

	void ClearSelection();

	/// <summary>
	///    Restores selection to the item from which the child menu was constructed
	///    This happens when the mouse cursor leaves the client area and a child menu is present.
	///    Restoring selection improves visual appeal of the menus as the selection highlight
	///    gets consistent with the child menu's data.
	/// </summary>
	void RevertSelection();

	/// <summary>
	///    Called after the selection was changed via a mouse movement. Sets the
	///    child menu creating timer (TimerID_ConstructChild)
	/// </summary>
	void OnSelectionChanged();


	void OnIconFetched(const ItemData *, HRESULT theIconFetchResult);

	IItemDataIterator * FindNeighborSelectable(TravFunc theTraverse, TravBoundary theInitial, TravBoundary theCheck);

	void ProcessClickItemHandler(const IDispayItemEventHandler * theHandler);
};

//////////////////////////////////////////////////////////////////////////////////////////////

class MenuWindow_Shell : public MenuWindow_DisplayItemList
{
private:
	//! Posted to the Window Procedure by the Shell upon receiving a notification event.
	static const UINT WM_USER_CHANGENOTIFY         = WM_USER_FIRSTAVAILABLE + 1;
	static const UINT WM_USER_CHANGENOTIFY_HANDLE  = WM_USER_FIRSTAVAILABLE + 2;

protected:
	Item *         myRestorableItem;
	RECT           myRestorableRect;
	unsigned long  myChangeNotifyID;


// Constructors, Destructor
public:
	MenuWindow_Shell(IMenuManagerAgent * theAgent);
	virtual ~MenuWindow_Shell();


// Interface
public:
	void SetRestorableData(Item * theItem, const RECT & theRect);


// MenuWindow_DisplayItemList Members
protected:
	virtual LRESULT WndProc(HWND theWnd, UINT theMsg, WPARAM wParam, LPARAM lParam);


// Message Handlers
private:
	void MsgHandler_ChangeNotify(DWORD theCode, UtilShellNotify::SHNOTIFYSTRUCT * theNotify);
	void MsgHandler_ChangeNotify_Handle();
};

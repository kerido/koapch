#pragma once

#include "sdk_IconExtractor.h"

#include "ExceptItem.h"	// automatically includes "DisplayItem.h"
#include "ScrollItem.h"

//////////////////////////////////////////////////////////////////////////////////////////////

template <typename T>
class IReusableTwoWayIterator
{
public:
	enum IteratorBoundary { Visible_Min, Visible_Max, EntireContents_Min, EntireContents_Max };
	typedef void (IReusableTwoWayIterator<T>::*TraverseFunc) ();

public:
	virtual int  IsAtRelativePosition(IteratorBoundary theBoundary) const = 0;
	virtual void SetRelativePosition(IteratorBoundary theBoundary) = 0;
	virtual void SetAbsolutePosition(const ItemData & theData) = 0;
	virtual bool SetAbsolutePosition(int theID) = 0;

	virtual void Next() = 0;
	virtual void Prev() = 0;

	virtual const T & GetCurrent() const = 0;

public:
	virtual ULONG Release() = 0;
	virtual ULONG AddRef() = 0;
};

typedef IReusableTwoWayIterator<ItemData> IItemDataIterator;

//////////////////////////////////////////////////////////////////////////////////////////////

template <typename T>
class IteratorWrapper
{
	IReusableTwoWayIterator<T> * myIter;

public:
	IteratorWrapper(IReusableTwoWayIterator<T> * theIter = 0) : myIter(theIter)
		{}

	IteratorWrapper(const IteratorWrapper<T> & theOriginal) : myIter(theOriginal.myIter)
		{ myIter->AddRef(); }

	~IteratorWrapper() { SafeRelease(); }


//operators
public:
	IteratorWrapper<T> & operator = (IReusableTwoWayIterator<T> * theIter)
	{
		if (theIter != 0)
			theIter->AddRef();

		SafeRelease();
		myIter = theIter;

		return *this;
	}
	
	IteratorWrapper<T> & operator = (const IteratorWrapper<T> & theOriginal)
	{
		return *this = theOriginal.myIter;
	}

	IReusableTwoWayIterator<T> * operator ->() const
		{ return myIter; }

	bool operator == (IReusableTwoWayIterator<T> * theIter) const
	{
		return myIter == theIter;
	}

	bool operator != (IReusableTwoWayIterator<T> * theIter) const
	{
		return myIter != theIter;
	}

	operator IReusableTwoWayIterator<T> * ()
	{
		return myIter;
	}

private:
	void SafeRelease()
		{ if (myIter != 0) myIter->Release(); myIter = 0; }
};

typedef IteratorWrapper<ItemData> ItemDataIter;

//////////////////////////////////////////////////////////////////////////////////////////////

//! Represents an object that contains one ore more (display) items.
class IDisplayItemHost
{
public:
	//! Initializes the view.
	virtual void Initialize(MenuWindow * theWnd) = 0;

	//! Returns the view's visible rectangle (in window coordinates).
	virtual const RECT & GetVisibleRect() const = 0;

	//! Sets the view's visible rectangle (in window coordinates).
	virtual void SetVisibleRect(const RECT & theRect) = 0;

	//! Returns the view's bounding rectangle. Because the value is in view
	//! coordinates, the top and left are typically zero.
	virtual const RECT & GetEntireRect() const = 0;

	//! Creates an instance of the iterator allowing for element traversal.
	virtual IItemDataIterator * CreateIterator() = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////////

class ItemListView : public IDisplayItemHost
{
// Nested classes
private:
	struct DisplayItemDataBase
	{
		DisplayItem *   Item;

		DisplayItemDataBase(DisplayItem * theItem) : Item(theItem) { }

		operator DisplayItem * () throw()     { return Item; }
		DisplayItem * operator  -> () throw() { return Item; }
	};


	typedef std::vector <DisplayItemDataBase>    DisplayItemList;
	typedef DisplayItemList::iterator            DisplayItemIter;
	typedef DisplayItemList::const_iterator      DisplayItemIterC;
	typedef DisplayItemList::size_type           DisplayItemSize;

	//! The 'DisplayItemSize' member is the item's index in the DisplayItemList
	//! vector. The 'int' member is the height itself
	typedef std::map<DisplayItemSize, int>   CustomHeightMap;
	typedef CustomHeightMap::iterator        CustomHeightIter;
	typedef CustomHeightMap::const_iterator  CustomHeightIterC;

	class ItemDataDynamic : public ItemData
	{
	private:
		ItemListView & myView;

		int myY, myHeight;
		DisplayItemSize  myPos;

	public:
		ItemDataDynamic(ItemListView & theView, DisplayItemSize thePos = 0, int theY = 0, int theHeight = 0)
			: myView(theView), myPos(thePos), myY(theY), myHeight(theHeight)
			{ }

	public:
		int  GetY() const     { return myY; }
		void SetY(int theVal) { myY = theVal; }

		int  GetHeight() const     { return myHeight; }
		void SetHeight(int theVal) { myHeight = theVal; }

		DisplayItemSize GetPosition() const      { return myPos; }
		void SetPosition(DisplayItemSize theVal) { myPos = theVal; }

		ItemDataDynamic & operator = (const ItemDataDynamic & theSrc)
		{
			myY      = theSrc.myY;
			myHeight = theSrc.myHeight;
			myPos    = theSrc.myPos;

			return *this;
		}

	// ItemData Members
	protected:
		virtual DisplayItem * GetItem() const
			{ return myView.myVisualContents[myPos]; }

		virtual IDisplayItemHost * GetHost() const
			{ return &myView; }

		virtual void GetViewRect(RECT & theOut) const
		{
			theOut.left   = myView.myEntireRect.left;
			theOut.right  = myView.myEntireRect.right;
			theOut.top    = myY;
			theOut.bottom = myY + myHeight;
		}

		virtual void GetWindowRect(RECT & theOut) const
		{
			GetViewRect(theOut);

			int aOffset = myView.myOriginY - myView.myOffsetY;

			theOut.top += aOffset;
			theOut.bottom += aOffset;
		}

		virtual ItemData * Clone() const
		{
			return new ItemDataDynamic(myView, myPos, myY, myHeight);
		}

		virtual int GetID() const
		{
			return (int) myPos;
		}
	};

	class ViewIterator : public IItemDataIterator
	{
		ItemListView & myView;
		ItemDataDynamic myCurrent;
		CustomHeightIter myNextCHItem;
		ULONG myNumRefs;

	public:
		ViewIterator(ItemListView & theView) : myView(theView), myCurrent(theView), myNumRefs(1UL)
			{}

	public:
		virtual int IsAtRelativePosition(IteratorBoundary theBoundary) const
		{
			DisplayItemSize aPos = GetPosByBoundary(theBoundary);
			DisplayItemSize aCurPos = myCurrent.GetPosition();

			return (int) (aCurPos-aPos);
		}

		virtual void SetRelativePosition(IteratorBoundary theBoundary)
		{
			DisplayItemSize aPos = GetPosByBoundary(theBoundary);
			UpdateByAbsolutePosition(aPos);
		}

		virtual void SetAbsolutePosition(const ItemData & theData)
		{
			const ItemDataDynamic * aData = reinterpret_cast<const ItemDataDynamic *>( &theData );

			if (aData != 0)
			{
				DisplayItemSize aPos = aData->GetPosition();
				UpdateNextCustomHeightIter(aPos);

				myCurrent = *aData;
			}
		}

		virtual bool SetAbsolutePosition(int theID)
		{
			DisplayItemSize aPos = theID;
			if (aPos >= myView.myVisualContents.size() )
				return false;

			UpdateByAbsolutePosition(aPos);
			return true;
		}

		virtual void Next()
		{
			DisplayItemSize aPos = myCurrent.GetPosition();

			int aY = myCurrent.GetY();
			int aHeight = GetCurItemHeight();

			aY += aHeight;

			myCurrent.SetPosition(++aPos);
			myCurrent.SetY(aY);

			if ( myNextCHItem != myView.myCustomHeights.end() )
				if ( myNextCHItem->first < aPos )
					myNextCHItem++;

			myCurrent.SetHeight( GetCurItemHeight() );
		}

		virtual void Prev()
		{
			int aY = myCurrent.GetY();

			DisplayItemSize aPos = myCurrent.GetPosition();
			myCurrent.SetPosition(--aPos);

			if ( myNextCHItem != myView.myCustomHeights.begin() )
			{
				CustomHeightIter aTemp = myNextCHItem;
				aTemp--;

				if (aTemp->first >= aPos)
					myNextCHItem = aTemp;
			}

			int aHeightPrev = GetCurItemHeight();

			myCurrent.SetHeight(aHeightPrev);

			aY -= aHeightPrev;
			myCurrent.SetY(aY);
		}

		const ItemData & GetCurrent() const
			{ return myCurrent; }

		virtual ULONG AddRef()
			{ return ++myNumRefs; }

		virtual ULONG Release()
		{
			ULONG aRet = --myNumRefs;

			if (aRet == 0)
				delete this;

			return aRet;
		}

	protected:
		void GetItemMeasure(DisplayItemSize thePos, int & theOffsetY, int & theHeight) const
		{
			DisplayItemSize aCurPos = 0;

			int aCurItemHeight = 0;
			int aTotalHeight = 0;

			for (CustomHeightIterC aIt = myView.myCustomHeights.begin(); aIt != myView.myCustomHeights.end(); aIt++)
			{
				if (aIt->first > thePos)
					break;

				aCurItemHeight = myView.myDefaultHeight;
				DisplayItemSize aNumItems = aIt->first - aCurPos;
				aTotalHeight += (int)aNumItems * aCurItemHeight;

				aCurItemHeight = aIt->second;
				aTotalHeight += aCurItemHeight;

				aCurPos = aIt->first + 1;
			}

			if (aCurPos <= thePos)
			{
				aCurItemHeight = myView.myDefaultHeight;
				DisplayItemSize aNumItems = thePos - aCurPos + 1;
				aTotalHeight += (int)aNumItems * aCurItemHeight;
			}

			theOffsetY = aTotalHeight - aCurItemHeight;
			theHeight = aCurItemHeight;
		}

		int GetCurItemHeight() const
		{
			if (myNextCHItem != myView.myCustomHeights.end() )
				if (myNextCHItem->first == myCurrent.GetPosition() )
					return myNextCHItem->second;

			return myView.myDefaultHeight;
		}

		DisplayItemSize GetPosByBoundary(IteratorBoundary theBoundary) const
		{
			switch (theBoundary)
			{
			case EntireContents_Min: return 0;
			case EntireContents_Max: return myView.myVisualContents.size() - 1;
			case Visible_Min:        return myView.myMinVisible;
			case Visible_Max:        return myView.myMaxVisible;
			default: __assume(0);
			}
		}

		void UpdateByAbsolutePosition(DisplayItemSize thePos)
		{
			UpdateNextCustomHeightIter(thePos);

			int aY, aHeight;
			GetItemMeasure(thePos, aY, aHeight);

			myCurrent.SetPosition(thePos);
			myCurrent.SetHeight(aHeight);
			myCurrent.SetY(aY);
		}

		void UpdateNextCustomHeightIter(DisplayItemSize thePos)
		{
			for( myNextCHItem  = myView.myCustomHeights.begin();
			     myNextCHItem != myView.myCustomHeights.end(); myNextCHItem++)
				if (myNextCHItem->first >= thePos)
					break;
		}
	};

protected:
	static bool RectsEqual (const RECT & theRect1, const RECT & theRect2)
	{
		return theRect1.left   == theRect2.left  &&
		       theRect1.top    == theRect2.top   &&
		       theRect1.right  == theRect2.right &&
		       theRect1.bottom == theRect2.bottom;
	}

//Member variables
private:
	RECT myVisibleRect;
	RECT myEntireRect;

	DisplayItemList myVisualContents;  //!< the list of view items
	CustomHeightMap myCustomHeights;   //!< items whose height is different from the default height

	int myOffsetY;                     //!< Stores the visual offset to the window origin.
	int myOffsetYtoMin;                //!< Stores the visual offset to the minimum visible item (in view coords).
	int myOffsetSmallMin;              //!< Stores the visual offset from the top of the first visible item.
	int myOffsetSmallMax;              //!< Stores the visual offset to the bottom of the last visible item.
	int myDefaultHeight;               //!< Stores the default item height obtained from the theme.
	int myOwnerHeight;                 //!< Stores the client height of the MenuWindow that owns the view.
	int myOriginY;                     //!< Stores the Y-coordinate (in window coordinates) of the view's origin. The X-coordinate is always zero.

	DisplayItemSize
		myMinVisible,                    //!< Stores the minimum index of an item that is at least partially visible at the top.
		myMaxVisible;                    //!< Stores the maximum index of an item that is at least partially visible at the bottom.

	int myPrevOffsetY;                 //!< Stores the previous value of myOffsetY. If the two are equal, the visible boundaries may not be re-calculated.
	RECT myPrevVisibleRect;            //!< Stores the previous value of myVisibleRect.  If the two are equal, the visible boundaries may not be re-calculated.

	MenuWindow * myOwner;              //!< Stores the owner of the view

public:
	ItemListView()
		: myMinVisible(0), myMaxVisible(0), myOffsetY(0), myOffsetYtoMin(0),
	    myOffsetSmallMin(0), myOffsetSmallMax(0), myPrevOffsetY(-1), myOriginY(0)
	{
		SecureZeroMemory(&myVisibleRect, sizeof RECT);
		SecureZeroMemory(&myEntireRect, sizeof RECT);
	}

	~ItemListView()
	{
		try
		{
			for(DisplayItemIter aIt = myVisualContents.begin(); aIt != myVisualContents.end(); aIt++)
				delete *aIt;
		}
		catch (...)
		{
		}
	}

public:
	int GetOwnerHeight() const       { return myOwnerHeight; }
	void SetOwnerHeight (int theVal) { myOwnerHeight = theVal; }

	int GetOffset() const            { return myOffsetY; }
	void SetOffset (int theVal)      { myOffsetY = theVal; }

	int GetMinOffset() const         { return 0; }
	int GetMaxOffset() const         { return myEntireRect.bottom - myEntireRect.top - myOwnerHeight; }

	int GetItemCount() const         { return (int) myVisualContents.size(); }
	int GetMinVisibleIndex() const   { return (int) myMinVisible; }
	int GetMaxVisibleIndex() const   { return (int) myMaxVisible; }

	int GetOrigin() const            { return myOriginY; }
	void SetOrigin(int theVal)       { myOriginY = theVal; }


// IDisplayItemHost members
public:
	virtual void Initialize(MenuWindow * theWnd)
	{
		myOwner = theWnd;

		myDefaultHeight = myOwner->GetTheme()->GetMetric(HEIGHT_ITEM);
	}

	virtual const RECT & GetVisibleRect() const       { return myVisibleRect; }
	virtual void SetVisibleRect(const RECT & theVal)  { myVisibleRect = theVal; }

	virtual const RECT & GetEntireRect() const        { return myEntireRect; }

	virtual IItemDataIterator * CreateIterator()
		{ return new ViewIterator(*this); }


public:
	void OnPopulateStarted() { }

	void OnPopulateFinished()
	{
		// Measure items and populate the myCustomHeights map when items have non-standard measure
		HDC aTempDC = ::GetDC( myOwner->GetHandle() );

		int aMaxWidth = myOwner->GetTheme()->GetMetric(WIDTH_MAXWINDOW);

		for (DisplayItemIter aIt = myVisualContents.begin(); aIt != myVisualContents.end(); aIt++)
		{
			ItemMeasure aMeasure(ItemMeasure::HEIGHT);

			if (aMaxWidth >= myEntireRect.right - myEntireRect.left)
				aMeasure.Mask |= ItemMeasure::WIDTH;

			(*aIt)->Measure( aTempDC, aMeasure, myOwner->GetTheme() );

			if (aMeasure.Width != 0)
			{
				if (aMeasure.Width > myEntireRect.right)
				{
					if (aMeasure.Width < aMaxWidth)
						myEntireRect.right = aMeasure.Width;
					else
						myEntireRect.right = aMaxWidth;
				}
			}

			if (aMeasure.Height != 0 && aMeasure.Height != myDefaultHeight)
			{
				DisplayItemSize aIndex = aIt - myVisualContents.begin();
				myCustomHeights[aIndex] = aMeasure.Height;

				myEntireRect.bottom += aMeasure.Height;
			}
			else
				myEntireRect.bottom +=  myDefaultHeight;
		}

		ReleaseDC( myOwner->GetHandle(), aTempDC);
		DeleteDC(aTempDC);
	}

	void AddItem(DisplayItem * theItem)
	{
		theItem->OnEvent(DisplayItem::EVENT_INIT, myOwner);

		myVisualContents.push_back(theItem);
		theItem->OnEvent(DisplayItem::EVENT_LOAD, myOwner);
	}

	void FitClippingRectangle(RECT & theOut) const
	{
		int aMaxHeight = theOut.bottom - theOut.top;
		int aHeight = 0;

		DisplayItemSize i = 0;

		for (CustomHeightIterC aIt = myCustomHeights.begin(); aIt != myCustomHeights.end(); aIt++)
		{
			for (; i < aIt->first; i++)
			{
				int aDelta = myDefaultHeight;

				if (aHeight + aDelta > aMaxHeight)
					{ theOut.bottom = aHeight; return; }

				aHeight += aDelta;
			}

			int aDelta = aIt->second;

			if (aHeight + aDelta > aMaxHeight)
				{ theOut.bottom = aHeight; return; }

			i = aIt->first + 1;
		}

		for (; i < myVisualContents.size(); i++)
		{
			int aDelta = myDefaultHeight;

			if (aHeight + aDelta > aMaxHeight)
				{ theOut.bottom = aHeight; return; }

			aHeight += aDelta;
		}
	}

	void OnLayoutChangeStarted()
	{
		myPrevOffsetY = myOffsetY;
		myPrevVisibleRect = myVisibleRect;
	}

	void OnLayoutChangeFinished()
	{
		if (myPrevOffsetY == myOffsetY && RectsEqual(myPrevVisibleRect, myVisibleRect) )
			return;

		bool aMinValid = false;
		myMaxVisible = myVisualContents.size();

		int aHeight = 0;
		int aVisibleTop = myVisibleRect.top + myOffsetY - myOriginY;         // The Y (in view coords) that is visible at top
		int aVisibleBottom = myVisibleRect.bottom + myOffsetY - myOriginY;   // The Y (in view coords) that is visible at bottom

		DisplayItemSize i = 0;
		for (CustomHeightIterC aIt = myCustomHeights.begin(); aIt != myCustomHeights.end(); aIt++)
		{
			for (; i < aIt->first; i++)
			{
				if (!aMinValid)
				{
					if (aHeight + myDefaultHeight > aVisibleTop)
					{
						myMinVisible = i;

						myOffsetYtoMin = aHeight - myDefaultHeight;
						myOffsetSmallMin = aVisibleTop - aHeight;

						aMinValid = true;
					}
				}

				if (aMinValid)
				{
					if (aHeight + myDefaultHeight >= aVisibleBottom)
					{
						myMaxVisible = i;

						myOffsetSmallMax = aHeight + myDefaultHeight - aVisibleBottom;
						return;
					}
				}
				
				aHeight += myDefaultHeight;
			}

			if (!aMinValid)
			{
				if (aHeight + aIt->second > aVisibleTop)
				{
					myMinVisible = aIt->first;

					myOffsetYtoMin = aHeight - myDefaultHeight;
					myOffsetSmallMin = aVisibleTop - aHeight;

					aMinValid = true;
				}
			}
			if (aMinValid)
			{
				if (aHeight + aIt->second >= aVisibleBottom)
				{
					myMaxVisible = aIt->first;

					myOffsetSmallMax = aHeight + myDefaultHeight - aVisibleBottom;
					return;
				}
			}
			
			aHeight += aIt->second;
			i = aIt->first + 1;
		}

		for (; i < myMaxVisible; i++)
		{
			if (!aMinValid)
			{
				if (aHeight + myDefaultHeight > aVisibleTop)
				{
					myMinVisible = i;

					myOffsetYtoMin = aHeight - myDefaultHeight;
					myOffsetSmallMin = aVisibleTop - aHeight;

					aMinValid = true;
				}
			}
			if (aMinValid)
			{
				if (aHeight + myDefaultHeight >= aVisibleBottom)
				{
					myMaxVisible = i;

					myOffsetSmallMax = aHeight + myDefaultHeight - aVisibleBottom;
					return;
				}
			}
			
			aHeight += myDefaultHeight;
		}
	}

	//! Returns a new offset after scrolling a specified number of items up or down.
	//! 
	//! \param [in] theDeltaItems   The number of items to scroll. A Positive number
	//!                             denotes forward (downward) scrolling. A Negative number
	//!                             denotes backward (upward) scrolling.
	//! 
	//! \return                     An integer which contains the future absolute view offset (in pixels).
	int GetNewOffsetAfterScroll(int theDeltaItems) const
	{
		// the item towards the scrolling direction must be skipped in whole
		int aOffset = myOffsetY;

		if (theDeltaItems > 0)
		{
			if (myMaxVisible == myVisualContents.size())
				aOffset = 0;

			else
			{
				if (myOffsetSmallMax > 0)
					aOffset += myOffsetSmallMax;

				DisplayItemSize aMax = __min( myMaxVisible + theDeltaItems, myVisualContents.size() );

				for (DisplayItemSize i = myMaxVisible; i < aMax; i++)
				{
					CustomHeightIterC aIt = myCustomHeights.find(i);

					if (aIt == myCustomHeights.end() )
						aOffset += myDefaultHeight;
					else
						aOffset += aIt->second;
				}
			}
		}
		else
		{
			if (myMinVisible == 0)
				aOffset = GetMaxOffset();

			else
			{
				if (myOffsetSmallMin > 0)
					aOffset -= myOffsetSmallMin;

				// because we are dealing with unsigned ints, it is slightly more difficult
				// to compare values than in the positive delta (see above)
				DisplayItemSize aDeltaPos = -theDeltaItems;

				DisplayItemSize aMin = 0;
				if (aDeltaPos < myMinVisible)
					aMin = myMinVisible - aDeltaPos;

				for (DisplayItemSize i = myMinVisible; i > aMin; i--)
				{
					CustomHeightIterC aIt = myCustomHeights.find(i-1);

					if ( aIt == myCustomHeights.end() ) aOffset -= myDefaultHeight;
					else                                aOffset -= aIt->second;
				}
			}
		}

		aOffset = __min(GetMaxOffset(), aOffset);
		aOffset = __max(GetMinOffset() , aOffset);
		return aOffset;
	}

};

//////////////////////////////////////////////////////////////////////////////////////////////

class ScrollItemView : public IDisplayItemHost
{
	class ItemDataStatic : public ItemData
	{
	protected:
		DisplayItem * myItem;
		ScrollItemView * myView;

	public:
		ItemDataStatic(ScrollItemView * theView, DisplayItem * theItem)
			: myItem(theItem), myView(theView) { }

	public:
		virtual DisplayItem * GetItem() const
			{ return myItem; }

		virtual IDisplayItemHost * GetHost() const
			{ return myView; }

		virtual void GetViewRect(RECT & theOut) const
			{ theOut = myView->myVisibleRect; }

		virtual void GetWindowRect(RECT & theOut) const
			{ theOut = myView->myVisibleRect; }

		virtual ItemData * Clone() const
			{ return new ItemDataStatic(myView, myItem); }

		virtual int GetID() const
			{ return -1; }
	};

	class ScrollItemIterator : public IItemDataIterator
	{
		ItemDataStatic myItemData;
		int myCur;
		ULONG myNumRefs;

	public:
		ScrollItemIterator(ScrollItemView * theView, DisplayItem * theItem)
			: myItemData(theView, theItem), myNumRefs(1UL) {}

	public:
		virtual int IsAtRelativePosition(IteratorBoundary theBoundary) const
			{ return myCur > 0 ? 1 : myCur < 0 ? -1 : 0; }

		virtual void SetRelativePosition(IteratorBoundary theBoundary)
			{ myCur = 0; }

		virtual void SetAbsolutePosition(const ItemData & theData)
			{ myCur = 0; }

		virtual bool SetAbsolutePosition(int theID)
			{ myCur = 0; return true; }

		virtual void Next() { myCur++; }
		virtual void Prev() { myCur--; }

		virtual const ItemData & GetCurrent() const
			{ return myItemData; }

		virtual ULONG AddRef() { return ++myNumRefs; }

		virtual ULONG Release()
		{
			ULONG aRet = --myNumRefs;

			if (aRet == 0)
				delete this;

			return aRet;
		}
	};

	ScrollItemConcr myItem;
	RECT myVisibleRect;
	RECT myEntireRect;
	MenuWindow * myOwner;

public:
	ScrollItemView(bool theTop) : myItem(theTop)
	{
		SecureZeroMemory(&myVisibleRect, sizeof RECT);
		SecureZeroMemory(&myEntireRect, sizeof RECT);
	}

	~ScrollItemView()
	{
		myItem.OnEvent(DisplayItem::EVENT_UNLOAD, myOwner);
	}

public:
	void SetOwnerWidth(int theWidth)
	{
		myVisibleRect.top = myEntireRect.top = 0;
		myVisibleRect.left = myEntireRect.left = 0;
		myVisibleRect.right = myEntireRect.right = theWidth;

		//myVisibleRect.bottom has already been set in Initialize
	}

	void SetNumRemainingItems(int theNumItems)
		{ myItem.SetNumRemainingItems(theNumItems); }

	void SetEnabled(bool theVal)
		{ myItem.SetEnabled(theVal); }

// IDisplayItemHost Members
public:
	virtual void Initialize(MenuWindow * theWnd)
	{
		myOwner = theWnd;

		myItem.OnEvent(DisplayItem::EVENT_INIT, myOwner);
		myItem.OnEvent(DisplayItem::EVENT_LOAD, myOwner);

		myEntireRect.bottom = myOwner->GetTheme()->GetMetric(HEIGHT_ITEM);
	}

	//! Returns the view's visible rectangle (in window coordinates).
	virtual const RECT & GetVisibleRect() const      { return myVisibleRect; }

	//! Sets the view's visible rectangle (in window coordinates).
	virtual void SetVisibleRect(const RECT & theVal) { myVisibleRect = theVal; }

	virtual const RECT & GetEntireRect() const       { return myEntireRect; }

	virtual IItemDataIterator * CreateIterator()
		{ return new ScrollItemIterator(this, &myItem); }
};

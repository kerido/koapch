#pragma once

#include "sdk_ComObject.h"
#include "sdk_Selectable.h"
#include "sdk_Scrollable.h"
#include "sdk_Timer.h"

#include "DisplayItem.h"

//////////////////////////////////////////////////////////////////////////////////////////////


class ScrollItem :
	public DisplayItem,
	public ITimerTarget,
	public ComEntry1<ISelectable>
{
private:
	const static DWORD DrawItemTextDefaultFlags = DT_EDITCONTROL|DT_NOPREFIX|DT_RIGHT|DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS;

private:
	bool myUp;                         //!< TODO
	int myTimerID;                     //!< TODO
	const int myIncrement;             //!< TODO
	IScrollable * myHostWnd;           //!< TODO
	ITimerHandler * myTimerHandler;    //!< TODO
	int myNumRemainingItems;           //!< TODO
	bool myEnabled;                    //!< TODO


public:
	ScrollItem(bool theUp);


public:
	void SetNumRemainingItems(int theNumItems)
		{ myNumRemainingItems = theNumItems; }

	void SetEnabled(bool theVal);


// DisplayItem members
protected:
	void Measure (HDC theDC, ItemMeasure & theMeasure, const Theme * theTheme)
		{}

	void Draw(const ItemDrawData * theData, const class Theme * theTheme);

	class Item * GetLogicalItem() const
		{ return 0; }

public:
	void OnEvent(EventType theEvent, class MenuWindow * theHost);


// ITimerTarget members
protected:
	STDMETHODIMP OnTimer(int theID, int theElapsed, void * theArg);


// ISelectable members
protected:
	STDMETHODIMP OnSelect(ULONG theFlags);


// Implementation Details
private:
	void Draw(HDC theDC, bool theSelected, const RECT & theContainer, const Theme * theTheme);
};


class ScrollItemConcr : public ComInstance<ScrollItem>
{
public:
	ScrollItemConcr(bool theUp) : ComInstance<ScrollItem>(theUp) { }
};
#pragma once

#include "sdk_ComObject.h"
#include "sdk_DisplayItem.h"

//////////////////////////////////////////////////////////////////////////////////////////////

class DisplayItem_SeparatorImpl : public DisplayItem, public ComEntryDiscoverable<IUnknown>
{
// DisplayItem methods
protected:
	void Measure (HDC theDC, ItemMeasure & theMeasure, const Theme * theTheme)
		{ theMeasure.Height = theTheme->GetMetric(HEIGHT_ITEM_SEPARATOR); }

	void Draw(const ItemDrawData * theData, const Theme * theTheme)
		{ theTheme->DrawSeparatorItem(theData->DC, theData->Rect); }

	Item * GetLogicalItem() const
		{ return 0; }

	void OnEvent(EventType theEvent, class MenuWindow * theHost)
		{ }
};

//////////////////////////////////////////////////////////////////////////////////////////////

typedef ComInstance<DisplayItem_SeparatorImpl> DisplayItem_Separator;

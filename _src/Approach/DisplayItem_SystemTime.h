#pragma once

#include "sdk_DisplayItem.h"

//////////////////////////////////////////////////////////////////////////////////////////////

class DisplayItem_SystemTime : public DisplayItem, public ITimerTarget
{
	RECT myRect;
	ITimerHandler * myHandler;
	int myTimerID;

public:
	DisplayItem_SystemTime() : myHandler(0), myTimerID(0)
	{
		SecureZeroMemory(&myRect, sizeof RECT);
	}

// DisplayItem methods
public:
	virtual void Measure(HDC theDC, ItemMeasure & theMeasure, const Theme * theTheme)
	{
		theMeasure.Width = 200;
		theMeasure.Height = 50;
	}

	virtual void Draw(const ItemDrawData * theData, const Theme * theTheme)
	{
		myRect = theData->Rect;

		SYSTEMTIME aTime;
		GetSystemTime(&aTime);

		TCHAR aDisplay[48];
		aDisplay[0] = 0;

		_stprintf(aDisplay, "%d:%d:%d", (int) aTime.wHour, (int)aTime.wMinute, (int)aTime.wSecond);


		//Step 1. Prepare fonts, colors, brushes, etc.
		HBRUSH aBgBrush = theTheme->GetBrush(BG_ITEM_NORMAL);

		//Step 2. Fill the background
		FillRect(theData->DC, &theData->Rect, aBgBrush);
	}

	virtual Item * GetLogicalItem() const  { return 0; }

	virtual void OnEvent(EventType theEvent, MenuWindow * theHost)
	{
		switch (theEvent)
		{
			case EVENT_INIT:
				myHost = theHost;
				myHandler = QUERY_TimerHandler::Request(theHost);
				myTimerID = myHandler->GetAvailableTimerID()
				break;

			case EVENT_LOAD:
				myHandler->SetTimer(myTimerID, 1000, this);
				break;

			case EVENT_UNLOAD:
				myHandler = QUERY_TimerHandler::Release(theHost, myHandler);
				break;
		}
	}

public:
	STDMETHODIMP OnTimer(int /*theID*/, int /*theElapsed*/, void * /*theArg*/)
	{
		::InvalidateRect(myHost->GetHandle(), &myRect, TRUE);
		return S_OK;
	}
};

#include "stdafx.h"

#include "sdk_Theme.h"

#include "UtilGraphics.h"
#include "ScrollItem.h"
#include "MenuWindow_Shell.h"
#include "Application.h"

//////////////////////////////////////////////////////////////////////////////////////////////

ScrollItem::ScrollItem(bool theUp) :
	myUp(theUp),
	myTimerID(0),
	myHostWnd(0),
	myTimerHandler(0),
	myIncrement(theUp ? -1 : 1),
	myNumRemainingItems(0),
	myEnabled(true)
{ }

	//////////////////////////////////////////////////////////////////////////////////////////////

void ScrollItem::SetEnabled(bool theVal)
{
	myEnabled = theVal;

	if (!theVal)
		myTimerHandler->KillTimer(myTimerID);
}

//////////////////////////////////////////////////////////////////////////////////////////////

void ScrollItem::Draw(const ItemDrawData * theData, const class Theme * theTheme)
{
	return Draw(
		theData->DC,   (theData->State & ITEMSTATE_SELECTED) != 0,
		theData->Rect, theTheme);
}

//////////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP ScrollItem::OnTimer(int theID, int, void *)
{
	myHostWnd->Scroll(myIncrement, IScrollable::SCROLLFLAGS_NONE);
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP ScrollItem::OnSelect(ULONG theFlags)
{
	if (theFlags & SELECTED && myEnabled)
	{
		int aTimeout = (int) Application::InstanceC().Prefs()->GetTimeoutScroll();

		myTimerHandler->SetTimer(myTimerID, aTimeout, this, 0);
		myHostWnd->Scroll(myIncrement, IScrollable::SCROLLFLAGS_NONE);	//initial
	}
	else
		myTimerHandler->KillTimer(myTimerID);

	return MAKE_HRESULT(0, 0, REDRAW);
}

//////////////////////////////////////////////////////////////////////////////////////////////

void ScrollItem::OnEvent(EventType theEvent, MenuWindow * theHost)
{
	if (theEvent == EVENT_LOAD)
	{
		theHost->QueryInterface( __uuidof(ITimerHandler), (void **) &myTimerHandler);
		theHost->QueryInterface( __uuidof(IScrollable),   (void **) &myHostWnd);

		ATLASSERT(myTimerHandler != 0);
		ATLASSERT(myHostWnd      != 0);

		myTimerHandler->GetAvailableTimerID(&myTimerID);
	}
	else if (theEvent == EVENT_UNLOAD)
	{
		myTimerHandler->KillTimer(myTimerID);

		myHostWnd->Release();
		myTimerHandler->Release();
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////

void ScrollItem::Draw(HDC theDC, bool theSelected, const RECT & theContainer, const Theme * theTheme)
{
	BitmapInfo aInfo;

	if (theSelected)
		theTheme->GetBitmap(myUp? UARROW_S : DARROW_S, &aInfo);
	else
		theTheme->GetBitmap(myUp? UARROW_N : DARROW_N, &aInfo);

	theTheme->DrawItemBackground(theDC, theSelected, theContainer);

	HDC aMemoryDC = theTheme->GetMemoryDC(0);

	HGDIOBJ aPrevObj = SelectObject(aMemoryDC, aInfo.myBmpHandle);

	int aTopCrn = theContainer.top +
		(theContainer.bottom - theContainer.top - aInfo.myHeight) / 2;

	int aLeftCrn = theContainer.left +
		(theContainer.right - theContainer.left - aInfo.myWidth) / 2;

	if (myEnabled)
		UtilGraphics::BitBltSimple(theDC, aMemoryDC, aLeftCrn, aTopCrn, aInfo);
	else
		UtilGraphics::BitBltSemiTransparent(theDC, aMemoryDC, aLeftCrn, aTopCrn, aInfo, 64);

	SelectObject(aMemoryDC, aPrevObj);

	if (myNumRemainingItems != 0)
	{
		TCHAR aBuf[64];
		_stprintf(aBuf, _T("(%d)"), myNumRemainingItems);

		RECT aRct = theContainer;
		aRct.right -= theTheme->GetMetric(WIDTH_INDENT_RIGHT_EXPANDABLE);
		theTheme->DrawItemText(theDC, theSelected, aRct, aBuf, lstrlen(aBuf), DrawItemTextDefaultFlags);
	}
}

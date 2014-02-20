#include "stdafx.h"

#include "sdk_Theme.h"
#include "sdk_MenuWindow.h"
#include "sdk_MenuBuilder.h"
#include "sdk_CriticalSection.h"
#include "sdk_Item.h"

#include "Framework.h"
#include "DynamicDisplayItem.h"
#include "UtilGraphics.h"
#include "Application.h"


static CriticalSection gDisplayItemCS;

//////////////////////////////////////////////////////////////////////////////////////////////

DynamicDisplayItem::DynamicDisplayItem(Item * theItem) :
	myItem(theItem), myMB(0)
{ }

//////////////////////////////////////////////////////////////////////////////////////////////

DynamicDisplayItem::~DynamicDisplayItem()
{
	myItem->Release();
}

//////////////////////////////////////////////////////////////////////////////////////////////

void DynamicDisplayItem::DrawFull(HDC theDC, bool theSelected, const RECT & theContainer, const Theme * theTheme)
{
	//Step 1. Draw item background
	DrawBackground(theDC, theSelected, theContainer, theTheme);

	//Step 2. Draw icon
	DrawIcon(theDC, theSelected, theContainer, theTheme);

	//Step 3. Draw item name
	DrawText(theDC, theSelected, theContainer, theTheme);

	//Step 4. Draw submenu arrow (if necessary)
	if (myExpansionCode != IMenuBuilder::NON_EXPANDABLE)
		DrawArrow(theDC, theSelected, theContainer, theTheme);
}

//////////////////////////////////////////////////////////////////////////////////////////////

void DynamicDisplayItem::DrawIconOnly(HDC theDC, bool theSelected, const RECT & theContainer, const Theme * theTheme)
{
	// Step 1. Clip the output so that no text is touched.
	int aWidth = theTheme->GetMetric(WIDTH_ICONSITE) + theTheme->GetMetric(WIDTH_INDENT_LEFT);
	IntersectClipRect(theDC, theContainer.left, theContainer.top, aWidth, theContainer.bottom);

	// Step 2. Redraw a part of the background.
	DrawBackground(theDC, theSelected, theContainer, theTheme);

	// Step 3. Draw icon
	DrawIcon(theDC, theSelected, theContainer, theTheme);

	// Step 4. Remove clipping
	SelectClipRgn(theDC, NULL);
}

//////////////////////////////////////////////////////////////////////////////////////////////

void DynamicDisplayItem::Draw(const ItemDrawData * theData, const class Theme * theTheme)
{
	bool aSelected = (theData->State & ITEMSTATE_SELECTED) != 0;

	if ( theData->ContentMask == ITEMCONTENT_ALL )
		DrawFull(theData->DC, aSelected, theData->Rect, theTheme);

	else if (theData->ContentMask == ITEMCONTENT_ICON )
		DrawIconOnly(theData->DC, aSelected, theData->Rect, theTheme);
}

//////////////////////////////////////////////////////////////////////////////////////////////

void DynamicDisplayItem::Measure(HDC theDC, ItemMeasure & theMeasure, const Theme * theTheme)
{
	HFONT aFont = theTheme->GetFont(ITEM_DEFAULT_N);
	HGDIOBJ aPrev = SelectObject(theDC, aFont);

	if ( (theMeasure.Mask & ItemMeasure::WIDTH) != 0)
	{
		TCHAR aName[1000];
		int aSize = 1000;

		HRESULT aRet = myItem->GetDisplayName(false, aName, &aSize);

		SIZE aSz;
		BOOL aRes = GetTextExtentPoint32(theDC, aName, aSize, &aSz);

		if ( aRes != 0)	//success
		{
			ReleaseTraceCode(DYNAMIC_DISP_ITEM_GETTEXTEXTENT_SUCCESS);
		}
		else
		{
			ReleaseTraceCode(DYNAMIC_DISP_ITEM_GETTEXTEXTENT_FAILED);

			ReleaseTraceCode(COMMON_DUMP_GETLASTERROR_BEGIN);

			// output last error
			DWORD aErr = GetLastError();
			ReleaseTraceBytes(&aErr, sizeof DWORD);
			//end output last error

			ReleaseTraceCode(COMMON_DUMP_GETLASTERROR_END);
		}

		theMeasure.Width = aSz.cx +
			theTheme->GetMetric(WIDTH_ICONSITE         ) +
			theTheme->GetMetric(WIDTH_OFFSETTOTEXT_LEFT) +
			theTheme->GetMetric(WIDTH_INDENT_LEFT      );

		if ( myExpansionCode != 0)
			theMeasure.Width += 
				theTheme->GetMetric(WIDTH_RIGHTARROWSITE         ) +
				theTheme->GetMetric(WIDTH_INDENT_RIGHT_EXPANDABLE) +
				theTheme->GetMetric(WIDTH_OFFSETTOTEXT_RIGHT     );
		else
			theMeasure.Width += theTheme->GetMetric(WIDTH_INDENT_RIGHT_NORMAL);
	}

	if (theMeasure.Mask & ItemMeasure::HEIGHT)
		theMeasure.Height = 0;	//use default

	SelectObject(theDC, aPrev);
}

//////////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP DynamicDisplayItem::OnSelect(ULONG theFlags)
{
	return MAKE_HRESULT(0, 0, REDRAW);
}

//////////////////////////////////////////////////////////////////////////////////////////////

void DynamicDisplayItem::OnEvent(EventType theEvent, MenuWindow * theHost)
{
	if (theEvent == EVENT_INIT)
		myMB = Framework::GetMenuBuilder(myItem, myExpansionCode);
}

//////////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP DynamicDisplayItem::RetrieveIconData(IconExtractParams * theParams, void ** theOutAsyncArg)
{
	HRESULT aRes = E_FAIL;

	CComQIPtr<IIconAcceptor> aExtr(myItem);

	if (aExtr == 0)
		return aRes;

	aRes = aExtr->RetrieveIconData(theParams, theOutAsyncArg);

	if ( FAILED(aRes) && aRes != E_PENDING )
		return aRes;
	else if ( aRes == E_PENDING)
		UpdateIconData(CHANGE_ALL, theParams);
	else
		aRes = UpdateIconData( HRESULT_CODE(aRes), theParams);

	return aRes;
}

//////////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP DynamicDisplayItem::RetrieveIconDataAsync( IconExtractParams * theParams, void * theAsyncArg )
{
	HRESULT aRes = E_FAIL;

	CComQIPtr<IIconAcceptor> aExtr(myItem);

	if (aExtr == 0)
		return aRes;

	aRes = aExtr->RetrieveIconDataAsync(theParams, theAsyncArg);

	if ( FAILED(aRes) )
		return aRes;

	gDisplayItemCS.Enter();
	aRes = UpdateIconData(HRESULT_CODE(aRes), theParams);
	gDisplayItemCS.Leave();

	return aRes;
}

//////////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP DynamicDisplayItem::ReleaseAsyncArg(void * theAsyncArg)
{
	CComQIPtr<IIconAcceptor> aExtr(myItem);

	if (aExtr == 0)
		return E_FAIL;

	return aExtr->ReleaseAsyncArg(theAsyncArg);
}

//////////////////////////////////////////////////////////////////////////////////////////////

void DynamicDisplayItem::DrawBackground(HDC theDC, bool theSelected, const RECT & theContainer, const Theme * theTheme)
{
	theTheme->DrawItemBackground(theDC, theSelected, theContainer);
}

//////////////////////////////////////////////////////////////////////////////////////////////

void DynamicDisplayItem::DrawText(HDC theDC, bool theSelected, const RECT & theContainer, const Theme * theTheme)
{
	// Prepare fonts, colors, brushes, etc.

	TCHAR aName[1000];
	int aSize = 1000;

	HRESULT aRes = myItem->GetDisplayName(false, aName, &aSize);

	ATLASSERT( SUCCEEDED(aRes) );

	RECT aRect = theContainer;

	//TODO: move to theme
	aRect.left += theTheme->GetMetric(WIDTH_ICONSITE) +
		theTheme->GetMetric(WIDTH_OFFSETTOTEXT_LEFT)+
		theTheme->GetMetric(WIDTH_INDENT_LEFT);

	if (myExpansionCode != IMenuBuilder::NON_EXPANDABLE )
		aRect.right -=
			theTheme->GetMetric(WIDTH_INDENT_RIGHT_EXPANDABLE) +
			theTheme->GetMetric(WIDTH_RIGHTARROWSITE) +
			theTheme->GetMetric(WIDTH_OFFSETTOTEXT_RIGHT);
	else
		aRect.right -=
			theTheme->GetMetric(WIDTH_INDENT_RIGHT_NORMAL);


	theTheme->DrawItemText(theDC, theSelected, aRect, aName, aSize, DrawItemTextDefaultFlags);
}

//////////////////////////////////////////////////////////////////////////////////////////////

void DynamicDisplayItem::DrawIcon(HDC theDC, bool theSelected, const RECT & theContainer, const Theme * theTheme)
{
	gDisplayItemCS.Enter();

	int aIconSite = theTheme->GetMetric(WIDTH_ICONSITE);
	int aLeftIndent = theTheme->GetMetric(WIDTH_INDENT_LEFT);

	RECT aRect;
	aRect.left   = theContainer.left + aLeftIndent;
	aRect.top    = theContainer.top  + (theContainer.bottom - theContainer.top - aIconSite) / 2;
	aRect.right  = aRect.left + aIconSite;
	aRect.bottom = aRect.top + aIconSite;


	DWORD aPrevLayout = GetLayout(theDC);
	DWORD aNewLayout = aPrevLayout | LAYOUT_BITMAPORIENTATIONPRESERVED;
	SetLayout(theDC, aNewLayout);

	int aDrawItemFlags = theSelected ? DIIF_NORMAL : DIIF_SELECTED;
	if (myIconData.IsIconDimmed)
		aDrawItemFlags |= DIIF_DIMMED;

	theTheme->DrawItemIcon(theDC, aDrawItemFlags, aRect, myIconData.ImageList, myIconData.IconIndex);

	if (myIconData.OverlayIndex != -1)
	{
		HIMAGELIST aSmall, aLarge;
		BOOL aRet = Shell_GetImageLists(&aLarge, &aSmall);

		if (aRet && aSmall != 0)
			ImageList_Draw(aSmall, myIconData.OverlayIndex, theDC, aRect.left, aRect.top, ILD_TRANSPARENT);
	}

	SetLayout(theDC, aPrevLayout);

	gDisplayItemCS.Leave();
}

//////////////////////////////////////////////////////////////////////////////////////////////

void DynamicDisplayItem::DrawArrow( HDC theDC, bool theSelected, const RECT & theContainer, const Theme * theTheme )
{
	BitmapInfo aInfo;
	HRESULT aRes = myMB->GetArrowBitmap(myExpansionCode, theSelected, theTheme, &aInfo);

	aRes = theTheme->DrawItemBitmap(theDC, theSelected, theContainer, aInfo);
}

//////////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP DynamicDisplayItem::GetDisplayString( TCHAR * theOutBuf, int * theSize )
{
	if (myItem == 0)
		return E_FAIL;
	else
		return myItem->GetDisplayName(false, theOutBuf, theSize);
}

//////////////////////////////////////////////////////////////////////////////////////////////

ULONG STDMETHODCALLTYPE DynamicDisplayItem::AddRef()
{
	return myNumRefs.AddRef();
}

//////////////////////////////////////////////////////////////////////////////////////////////

ULONG STDMETHODCALLTYPE DynamicDisplayItem::Release()
{
	return myNumRefs.Release(this);
}

//////////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP DynamicDisplayItem::QueryInterface( REFIID theIID, void ** theOut )
{
	HRESULT aRes = InternalQueryInterface(theIID, this, theOut);

	if ( FAILED(aRes) )
		return aRes;

	AddRef();
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////////////////////////

HRESULT DynamicDisplayItem::UpdateIconData(int theCode, const ItemIconData * theData)
{
	HRESULT aRes = MAKE_HRESULT(0, 0, 0);

	if (theCode & CHANGE_ICON)
	{
		if (myIconData.IconIndex != theData->IconIndex)
		{
			myIconData.IconIndex = theData->IconIndex;
			aRes |= CHANGE_ICON;
		}
		if (myIconData.ImageList != theData->ImageList)
		{
			myIconData.ImageList = theData->ImageList;
			aRes |= CHANGE_ICON;
		}
		if (myIconData.IsIconDimmed != theData->IsIconDimmed)
		{
			myIconData.IsIconDimmed = theData->IsIconDimmed;
			aRes |= CHANGE_ICON;
		}
	}

	if (theCode & CHANGE_OVERLAY)
	{
		if (myIconData.OverlayIndex != theData->OverlayIndex)
		{
			myIconData.OverlayIndex = theData->OverlayIndex;
			aRes |= CHANGE_OVERLAY;
		}
	}

	return aRes;
}

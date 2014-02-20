#include "stdafx.h"

#include "sdk_Theme.h"

#include "ExceptItem.h"
#include "util_Localization.h"

#include "resource.h"
#include "auto_LocResource.h"

//////////////////////////////////////////////////////////////////////////////////////////////

ExceptItem::ExceptItem(ExceptItemType theType)
	: myType(theType), myNumRefs(1UL)
{ }

//////////////////////////////////////////////////////////////////////////////////////////////

void ExceptItem::Measure(HDC theDC, ItemMeasure & theMeasure, const Theme * theTheme)
{
	HFONT aFont = theTheme->GetFont(ITEM_DEFAULT_N);
	HGDIOBJ aPrev = SelectObject(theDC, aFont);

	if (theMeasure.Mask & ItemMeasure::WIDTH)
	{
		TCHAR aString [1000];
		int aSize = 1000;

		HRESULT aRes = GetText(aString, &aSize);

		SIZE aSz = {0};
		GetTextExtentPoint32(theDC, aString, aSize, &aSz);

		theMeasure.Width = aSz.cx + 
			theTheme->GetMetric(WIDTH_INDENT_LEFT) +
			theTheme->GetMetric(WIDTH_INDENT_RIGHT_NORMAL) +
			theTheme->GetMetric(WIDTH_ICONSITE);
	}

	if (theMeasure.Mask & ItemMeasure::HEIGHT)
		theMeasure.Height = 0;	//use default

	SelectObject(theDC, aPrev);
}

//////////////////////////////////////////////////////////////////////////////////////////////

void ExceptItem::Draw(HDC theDC, bool theSelected, const RECT & theContainer, const Theme * theTheme)
{
	HFONT aFont = theTheme->GetFont(ITEM_DEFAULT_N);
	HGDIOBJ aPrev = SelectObject(theDC, aFont);

	int aPrevMode = SetBkMode(theDC, TRANSPARENT);
	COLORREF aPrevColor = SetTextColor(theDC, GetSysColor(COLOR_HIGHLIGHTTEXT) );

	TCHAR aString [1000];
	int aSize = 1000;

	HRESULT aRes = GetText(aString, &aSize);

	RECT aRect = theContainer;

	const UINT DTFlags = DT_NOPREFIX|DT_CENTER|DT_VCENTER|DT_SINGLELINE;

	::DrawText(theDC, aString, aSize, &aRect, DTFlags);

	OffsetRect(&aRect, -1, -1);
	SetTextColor(theDC, GetSysColor(COLOR_BTNSHADOW) );

	::DrawText(theDC, aString, aSize, &aRect, DTFlags);


	SetBkMode(theDC, aPrevMode);
	SetTextColor(theDC, aPrevColor );
	SelectObject(theDC, aPrev);
}

//////////////////////////////////////////////////////////////////////////////////////////////

void ExceptItem::Draw(const ItemDrawData * theData, const class Theme * theTheme)
{
	return Draw(
		theData->DC,   (theData->State & ITEMSTATE_SELECTED) != 0,
		theData->Rect, theTheme);
}

//////////////////////////////////////////////////////////////////////////////////////////////

ULONG STDMETHODCALLTYPE ExceptItem::AddRef()
{
	return ++myNumRefs;
}

//////////////////////////////////////////////////////////////////////////////////////////////

ULONG STDMETHODCALLTYPE ExceptItem::Release()
{
	ULONG aNumRefs = --myNumRefs;

	if (aNumRefs == 0UL)
		delete this;

	return aNumRefs;
}

//////////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP ExceptItem::QueryInterface( REFIID theIID, void ** theOut )
{
	if (theIID == IID_ISelectable)
		*theOut = static_cast<ISelectable *>(this);

	else if (theIID == IID_IUnknown)
		*theOut = static_cast<IUnknown *>( static_cast<DisplayItem *>(this) );

	else
		return E_NOINTERFACE;

	AddRef();
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////////////////////////

HRESULT ExceptItem::GetText(TCHAR * theOutBuf, int * theBufSize)
{
	//ATLASSERT(theOutBuf != NULL);
	//ATLASSERT(theBufSize != NULL);

	LocIdKey aStringID = 0;

	switch (myType)
	{
	case EMPTY:
		aStringID = KEYOF(IDS_EXCEPT_EMPTY);
		break;

	case UNAVAILABLE:
		aStringID = KEYOF(IDS_EXCEPT_UNAVAILABLE);
		break;

	case GENERIC:
		aStringID = KEYOF(IDS_EXCEPT_GENERIC);
		break;

	default:
		return E_FAIL;
	}

	LocalizationManagerPtr aMan;
	return aMan->GetStringSafe(aStringID, theOutBuf, theBufSize);
}

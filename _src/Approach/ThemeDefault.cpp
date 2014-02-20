#include "stdafx.h"

#include "ThemeDefault.h"
#include "Application.h"
#include "UtilGraphics.h"
#include "UxManager.h"
#include "RootWindow.h"
#include "resource.h"

//////////////////////////////////////////////////////////////////////////////////////////////

class ThemeDefault_NoUxTheming : public Theme
{
protected:
	enum
	{
		Arrow_W = 4,
		Arrow_H = 7,

		VArrow_W = 9,
		VArrow_H = 5
	};

	const static COLORREF Color_Border_XP = RGB(172,168,153);
	const static COLORREF Color_Padding_XP = RGB(255, 255, 255);

	const static UINT DefaultClassStyle = /*CS_HREDRAW|CS_VREDRAW|CS_OWNDC|*/CS_DBLCLKS;



protected:
	HBITMAP myArrowN;         //!< TODO
	HBITMAP myArrowS;         //!< TODO

	HBITMAP myUArrowN;        //!< TODO
	HBITMAP myUArrowS;        //!< TODO

	HBITMAP myDArrowN;        //!< TODO
	HBITMAP myDArrowS;        //!< TODO

	HBITMAP myArrowN_T;       //!< TODO
	HBITMAP myArrowS_T;       //!< TODO

	HBITMAP myUArrowN_T;      //!< TODO
	HBITMAP myUArrowS_T;      //!< TODO

	HBITMAP myDArrowN_T;      //!< TODO
	HBITMAP myDArrowS_T;      //!< TODO

	HDC     myMemoryDC;       //!< TODO

	HFONT   myFont;           //!< TODO

	int     myIconWidth;      //!< Stores the width of the area where item icon is displayed.
	int     myIconHeight;     //!< Stores the height of the area where item icon is displayed.
	int     myItemHeight;     //!< Stores the default item height (unless not explicitly overriden by item instances).
	int     myMaxMenuWidth;   //!< TODO
	UINT    myClassStyle;     //!< TODO

	UxManager * myUxMan;


public:
	ThemeDefault_NoUxTheming(UxManager * theUxMan) : myUxMan(theUxMan), myFont(0)
	{
		InitializeMain();
		InitializeMenuStyle();
	}

	~ThemeDefault_NoUxTheming()
	{
		DeleteObject(myArrowN);
		DeleteObject(myArrowS);

		DeleteObject(myDArrowN);
		DeleteObject(myDArrowS);

		DeleteObject(myUArrowN);
		DeleteObject(myUArrowS);

		DeleteObject(myFont);

		DeleteDC(myMemoryDC);
	}


	int GetMetric(int theID) const
	{
		switch (theID)
		{
		case HEIGHT_ITEM:
			return myItemHeight;

		case WIDTH_ICONSITE:
			return myIconWidth;

		case WIDTH_MAXWINDOW:
			return myMaxMenuWidth;

		case WIDTH_OFFSETTOTEXT_LEFT:
			return 4;

		case WIDTH_OFFSETTOTEXT_RIGHT:
			return 13;

		case WIDTH_RIGHTARROWSITE:
			return 9;

		case WIDTH_INDENT_LEFT:
			return 1;

		case WIDTH_INDENT_RIGHT_EXPANDABLE:
			return 4;

		case WIDTH_INDENT_RIGHT_NORMAL:
			return 10;

		case HEIGHTWIDTH_MENUBORDER:
			return GetSystemMetrics(SM_CYDLGFRAME);

		case HEIGHT_ITEM_SEPARATOR:
			return 7;

		default:
			__assume(0);
			return -1;
		}
	}

	HFONT GetFont(int) const
		{ return myFont; }

	HDC GetMemoryDC(int theID, int theReserved = 0) const
		{ return myMemoryDC; }

	HRESULT GetBitmap(int theID, BitmapInfo * theOutInfo) const
	{
		if (theOutInfo == 0)
			return E_POINTER;

		switch(theID)
		{
		case ARROW_N:
			theOutInfo->myWidth  = Arrow_W;
			theOutInfo->myHeight = Arrow_H;
			theOutInfo->myTransparentColor = RGB(0, 255, 0);
			theOutInfo->myBmpHandle = myArrowN_T;
			return S_OK;

		case ARROW_S:
			theOutInfo->myWidth  = Arrow_W;
			theOutInfo->myHeight = Arrow_H;
			theOutInfo->myTransparentColor = RGB(0, 255, 0);
			theOutInfo->myBmpHandle = myArrowS_T;
			return S_OK;

		case UARROW_S:
			theOutInfo->myWidth  = VArrow_W;
			theOutInfo->myHeight = VArrow_H;
			theOutInfo->myTransparentColor = RGB(0, 255, 0);
			theOutInfo->myBmpHandle = myUArrowS_T;
			return S_OK;

		case UARROW_N:
			theOutInfo->myWidth = VArrow_W;
			theOutInfo->myHeight = VArrow_H;
			theOutInfo->myTransparentColor = RGB(0, 255, 0);
			theOutInfo->myBmpHandle = myUArrowN_T;
			return S_OK;

		case DARROW_S:
			theOutInfo->myWidth  = VArrow_W;
			theOutInfo->myHeight = VArrow_H;
			theOutInfo->myTransparentColor = RGB(0, 255, 0);
			theOutInfo->myBmpHandle = myDArrowS_T;
			return S_OK;

		case DARROW_N:
			theOutInfo->myWidth  = VArrow_W;
			theOutInfo->myHeight = VArrow_H;
			theOutInfo->myTransparentColor = RGB(0, 255, 0);
			theOutInfo->myBmpHandle = myDArrowN_T;
			return S_OK;

		default:
			return E_INVALIDARG;
		}
	}

	COLORREF GetColor(int theID) const
	{
		switch (theID)
		{
		case TEXT_ITEM_NORMAL:
			return (COLORREF)GetSysColor(COLOR_MENUTEXT);

		case TEXT_ITEM_SELECTED:
			return (COLORREF)GetSysColor(COLOR_HIGHLIGHTTEXT);

		case NC_BORDER:
			return Color_Border_XP;

		case NC_PADDING:
			return Color_Padding_XP;

		default:
			return 0;
		}
	}

	UINT GetStyle(int theID) const
	{
		switch(theID)
		{
		case STYLE_CLASS_MENU_DEFAULT:
			return myClassStyle;

		default:
			return 0;
		}
	}

	HBRUSH GetBrush(int theID) const
	{
		switch(theID)
		{
		case BG_ITEM_NORMAL:
			return GetSysColorBrush(COLOR_MENU);

		case BG_ITEM_SELECTED:
			return GetSysColorBrush(COLOR_HIGHLIGHT);

		default:
			return 0;
		}
	}

	HRESULT DrawMenuNcArea(HDC hdc, const RECT & theContainer) const
	{
		if ( !myUxMan->IsAppThemed() )
			return E_NOTIMPL;

		Trace("DrawMenuNcArea\n");

		long aWidth =  theContainer.right - theContainer.left - 1;
		long aHeight= theContainer.bottom - theContainer.top - 1;

		HPEN aPen = CreatePen(PS_SOLID, 1, GetColor(NC_BORDER) );
		HGDIOBJ aPrev = ::SelectObject(hdc, aPen);

		// 1. Border
		::MoveToEx(hdc, 0, 0, NULL);
		::LineTo(hdc, aWidth, 0);
		::LineTo(hdc, aWidth, aHeight);
		::LineTo(hdc, 0, aHeight);
		::LineTo(hdc, 0, 0);


		// 2. Padding
		RECT aRect;
		HBRUSH aBr = CreateSolidBrush( GetColor(NC_PADDING) );

		aRect.left = 1;
		aRect.top = 1;
		aRect.right = aWidth;
		aRect.bottom = aRect.top + 2;
		::FillRect(hdc, &aRect, aBr);

		aRect.top = aHeight - 2;
		aRect.bottom = aRect.top + 2;
		::FillRect(hdc, &aRect, aBr);


		aRect.left = 1;
		aRect.top = 3;
		aRect.right = aRect.left + 2;
		aRect.bottom = aHeight - 2;
		::FillRect(hdc, &aRect, aBr);

		aRect.left = aWidth - 2;
		aRect.right = aRect.left + 2;
		::FillRect(hdc, &aRect, aBr);


		// 3. Cleanup
		::SelectObject(hdc, aPrev);
		::DeleteObject(aPen);
		::DeleteObject(aBr);

		return S_OK;
	}

	HRESULT DrawItemBackground(HDC theDC, bool theSelected, const RECT & theContainer) const
	{
		HBRUSH aBgBrush = theSelected ? GetBrush(BG_ITEM_SELECTED) : GetBrush(BG_ITEM_NORMAL);
		FillRect(theDC, &theContainer, aBgBrush);

		return S_OK;
	}

	HRESULT DrawItemText(HDC theDC, bool theSelected, const RECT & theContainer, LPWSTR theText, int theSize, DWORD theFlags) const
	{
		RECT aRect = theContainer;

		int aPrevMode = SetBkMode(theDC, TRANSPARENT);
		COLORREF aPrevClr;
		HGDIOBJ aPrevObj;

		if (theSelected)
		{
			aPrevClr = SetTextColor(theDC, GetColor(TEXT_ITEM_SELECTED) );
			aPrevObj = SelectObject(theDC, GetFont(ITEM_DEFAULT_S) );
		}
		else
		{
			aPrevClr = SetTextColor(theDC, GetColor(TEXT_ITEM_NORMAL) );
			aPrevObj = SelectObject(theDC, GetFont(ITEM_DEFAULT_N) );
		}

		::DrawText
		(
			theDC,
			theText,
			theSize,
			&aRect,
			theFlags
		);

		SetTextColor(theDC, aPrevClr);
		SetBkMode(theDC, aPrevMode);
		SelectObject(theDC, aPrevObj);

		return S_OK;
	}

	HRESULT DrawItemBitmap(HDC theDC, bool theSelected, const RECT & theContainer, const BitmapInfo & theBmpInfo) const
	{
		BitmapInfo aNew = theBmpInfo;

		if (aNew.myBmpHandle == 0)
			GetBitmap(theSelected ? ARROW_S : ARROW_N, &aNew);

		//use the arrow bitmap is MenuBuilder did not provide us a bitmap to draw

		RECT aRect;
		aRect.left = theContainer.right -
			GetMetric(WIDTH_INDENT_RIGHT_EXPANDABLE) -
			aNew.myWidth;

		aRect.top = theContainer.top +
			(theContainer.bottom - theContainer.top - aNew.myHeight) / 2;

		aRect.right = aRect.left + aNew.myWidth;

		aRect.bottom = aRect.top + aNew.myHeight;

		HGDIOBJ aPrev = SelectObject(myMemoryDC, aNew.myBmpHandle);

		UtilGraphics::BitBltSimple(theDC, myMemoryDC, aRect.left, aRect.top, aNew);

		SelectObject(myMemoryDC, aPrev);
	
		return S_OK;
	}

	HRESULT DrawItemIcon(HDC theDC, int theFlags, const RECT & theContainer, HIMAGELIST theImgList, int theIconIndex) const
	{
		COLORREF aFg = CLR_NONE;
		UINT aStyle = ILD_NORMAL;

		if ( (theFlags & DIIF_DIMMED) != 0 )
		{
			aFg = RGB(255,255,255);
			aStyle = ILD_BLEND50;
		}

		ImageList_DrawEx(theImgList, theIconIndex, theDC,
			theContainer.left, theContainer.top,
			theContainer.right - theContainer.left, theContainer.bottom - theContainer.top,
			CLR_NONE, aFg, aStyle);

		return S_OK;
	}

	HRESULT DrawSeparatorItem(HDC theDC, const RECT & theContainer) const
	{
		RECT aRct = theContainer;

		HBRUSH aBgBrush = GetBrush(BG_ITEM_NORMAL);
		FillRect(theDC, &aRct, aBgBrush);

		aRct.bottom    = (aRct.top+aRct.bottom) / 2 + 2;

		DrawEdge(theDC, &aRct, EDGE_ETCHED, BF_BOTTOM);

		return S_OK;
	}

protected:
	void InitializeMain()
	{
		myArrowN = LoadBitmap( _Module.GetResourceInstance(), MAKEINTRESOURCE(IDB_ARROW_N) );
		myArrowS = LoadBitmap( _Module.GetResourceInstance(), MAKEINTRESOURCE(IDB_ARROW_S) );

		myDArrowN = LoadBitmap( _Module.GetResourceInstance(), MAKEINTRESOURCE(IDB_DARROW_N) );
		myDArrowS = LoadBitmap( _Module.GetResourceInstance(), MAKEINTRESOURCE(IDB_DARROW_S) );

		myUArrowN = LoadBitmap( _Module.GetResourceInstance(), MAKEINTRESOURCE(IDB_UARROW_N) );
		myUArrowS = LoadBitmap( _Module.GetResourceInstance(), MAKEINTRESOURCE(IDB_UARROW_S) );

		//target arrow bitmaps

		COLORREF aClrNormal =  GetSysColor(COLOR_MENU);
		DWORD aGrayNormal = ColorToGrayscale(aClrNormal);

		COLORREF aClrHighlight = GetSysColor(COLOR_HIGHLIGHT);
		DWORD aGrayHighlight = ColorToGrayscale(aClrHighlight);

		if (aGrayHighlight > 128)
		{
			myArrowS_T  = myArrowN;
			myUArrowS_T = myUArrowN;
			myDArrowS_T = myDArrowN;
		}
		else
		{
			myArrowS_T  = myArrowS;
			myUArrowS_T = myUArrowS;
			myDArrowS_T = myDArrowS;
		}


		if (aGrayNormal > 128)
		{
			myArrowN_T  = myArrowN;
			myUArrowN_T = myUArrowN;
			myDArrowN_T = myDArrowN;
		}
		else
		{
			myArrowN_T  = myArrowS;
			myUArrowN_T = myUArrowS;
			myDArrowN_T = myDArrowS;
		}

		HDC aMonitorDC = GetDC(NULL);	//default monitor
		myMemoryDC = CreateCompatibleDC(aMonitorDC);
		ReleaseDC(NULL, aMonitorDC);

		NONCLIENTMETRICS aNCM = {0};

		aNCM.cbSize = sizeof(NONCLIENTMETRICS);

		BOOL aRes = SystemParametersInfo(
			SPI_GETNONCLIENTMETRICS,
			sizeof(NONCLIENTMETRICS),
			(void *) &aNCM,
			0);

		if (aRes != 0)
			myFont = CreateFontIndirect(&aNCM.lfMenuFont);

		if (myFont == 0)
			myFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);


		myIconWidth = GetSystemMetrics(SM_CXSMICON);
		myIconHeight = GetSystemMetrics(SM_CYSMICON);

		myItemHeight = myIconWidth + 2;

		if (aNCM.iMenuHeight > myItemHeight)
			myItemHeight = aNCM.iMenuHeight;

		//TODO
		const PrefsSerz * aSt = Application::InstanceC().Prefs();

		if (aSt->myMaxMenuWidth < 0)	//in percent
		{
			int aScreenWidth = GetSystemMetrics(SM_CXSCREEN);
			myMaxMenuWidth = -aSt->myMaxMenuWidth * aScreenWidth / 100;
		}
		else
			myMaxMenuWidth = (int) aSt->myMaxMenuWidth;
	}

	void InitializeMenuStyle()
	{
		BOOL aValue = FALSE;
		BOOL aRes = SystemParametersInfo(SPI_GETDROPSHADOW, 0, &aValue, 0);

		if (aValue == TRUE && aRes != FALSE)
			myClassStyle = DefaultClassStyle|0x20000;
		else
			myClassStyle = DefaultClassStyle;
	}

	static int ColorToGrayscale(COLORREF theColor)
	{
		return ( 
			GetRValue(theColor) *  77 +
			GetGValue(theColor) * 151 +
			GetBValue(theColor) *  28 ) / 256;
	}
};

//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

//! Represents a theme that is in use on Windows Vista and Windows 7

//TODO: respond to theme change, fallback to ThemeDefault if theming got disabled
class ThemeDefault_Aero : public ThemeDefault_NoUxTheming
{
// Constants
	const static COLORREF Color_Border_Aero = RGB(151,151,151);
	const static COLORREF Color_Padding_Aero = RGB(245, 245, 245);

// Fields
private:
	HTHEME myThemeData;

	int myMenuElementSpacingWidth;
	int myGutterWidth;
	int mySeparatorHeight;
	int myRightMargin;
	SIZE mySubmenuArrowSize;


public:
	ThemeDefault_Aero(UxManager * theUxMan, HTHEME theThemeData) :
		ThemeDefault_NoUxTheming(theUxMan),
		myThemeData(theThemeData), myMenuElementSpacingWidth(0), myGutterWidth(0), myRightMargin(0)
	{
		Trace("ThemeDefault_Aero::ThemeDefault_Aero\n");

		// Override Metrics
		HRESULT aRes = S_OK;
		SIZE aTempSz;
		MARGINS aTempMg;
		int aTempInt;

		// Item Height (without margins)
		aRes = myUxMan->GetThemePartSize(myThemeData, NULL, MENU_POPUPCHECK, MPI_NORMAL, NULL, TS_TRUE, &aTempSz);

		if ( SUCCEEDED(aRes) )
		{
			myItemHeight = aTempSz.cy;
			myItemHeight = GetSystemMetrics(SM_CYSMICON);
		}


		// Item Height (add margins)

		aRes = myUxMan->GetThemeMargins(myThemeData, NULL, MENU_POPUPCHECK, 0, TMT_CONTENTMARGINS, NULL, &aTempMg);

		if ( SUCCEEDED(aRes) )
			myItemHeight += aTempMg.cyBottomHeight + aTempMg.cyTopHeight;


		// Element Spacing Width Offset
		aRes = myUxMan->GetThemePartSize(myThemeData, NULL, MENU_POPUPBORDERS, 0, NULL, TS_TRUE, &aTempSz);

		if ( SUCCEEDED(aRes) )
			myMenuElementSpacingWidth = aTempSz.cx;


		// Gutter Width
		aRes = myUxMan->GetThemePartSize(myThemeData, NULL, MENU_POPUPGUTTER, 0, NULL, TS_TRUE, &aTempSz);

		if ( SUCCEEDED(aRes) )
			myGutterWidth = aTempSz.cx;


		// Submenu Arrow
		aRes = myUxMan->GetThemePartSize(myThemeData, NULL, MENU_POPUPSUBMENU, MPI_NORMAL, NULL, TS_TRUE, &mySubmenuArrowSize);

		if (FAILED(aRes))
			mySubmenuArrowSize.cx = 40; //TEMP


		// Separator Height
		aRes = myUxMan->GetThemePartSize(myThemeData, NULL, MENU_POPUPSEPARATOR, MPI_NORMAL, NULL, TS_TRUE, &aTempSz);

		if ( SUCCEEDED(aRes) )
			mySeparatorHeight = aTempSz.cy;


		// Right Margin
		aRes = myUxMan->GetThemeInt(myThemeData, MENU_POPUPITEM, 0, TMT_BORDERSIZE, &aTempInt);

		if ( SUCCEEDED(aRes) )
			myRightMargin = aTempInt;
		else
			myRightMargin = ThemeDefault_NoUxTheming::GetMetric(WIDTH_INDENT_RIGHT_NORMAL);
	}

	~ThemeDefault_Aero()
	{
		myUxMan->CloseThemeData(myThemeData);
	}

public:
	int GetMetric(int theID) const
	{
		switch (theID)
		{
		case WIDTH_OFFSETTOTEXT_LEFT:
			return 2 * myMenuElementSpacingWidth + myGutterWidth;

		case WIDTH_INDENT_LEFT:
			return myMenuElementSpacingWidth;

		case HEIGHT_ITEM_SEPARATOR:
			return mySeparatorHeight;

		case WIDTH_INDENT_RIGHT_EXPANDABLE:
			return myRightMargin / 2;

		case WIDTH_INDENT_RIGHT_NORMAL:
			return myRightMargin;

		default:
			return ThemeDefault_NoUxTheming::GetMetric(theID);
		}
	}

	COLORREF GetColor(int theID) const
	{
		switch(theID)
		{
		case NC_BORDER:
			return Color_Border_Aero;

		case NC_PADDING:
			return Color_Padding_Aero;

		default:
			return ThemeDefault_NoUxTheming::GetColor(theID);
		}
	}


protected:
	HRESULT DrawItemBackground( HDC theDC, bool theSelected, const RECT & theContainer ) const
	{
		int aGutterWidth = 0;
		HRESULT aRes = DrawCommonBackground(theDC, theContainer, aGutterWidth);

		int aState = theSelected ? MPI_HOT : MPI_NORMAL;
		aRes = myUxMan->DrawThemeBackground(myThemeData, theDC, MENU_POPUPITEM, aState, &theContainer, NULL);

		return aRes;
	}

	HRESULT DrawItemText(HDC theDC, bool theSelected, const RECT & theContainer, LPWSTR theText, int theSize, DWORD theFlags) const
	{
		HGDIOBJ aPrevObj = SelectObject(theDC, myFont);

		int aState = theSelected ? MPI_HOT : MPI_NORMAL;
		HRESULT aRes = myUxMan->DrawThemeText(myThemeData, theDC, MENU_POPUPITEM, aState, theText, theSize, theFlags, 0, &theContainer);

		SelectObject(theDC, aPrevObj);

		return aRes;
	}

	HRESULT DrawItemBitmap( HDC theDC, bool theSelected, const RECT & theContainer, const BitmapInfo & theBmpInfo ) const
	{
		if (theBmpInfo.myBmpHandle != 0)
			return ThemeDefault_NoUxTheming::DrawItemBitmap(theDC, theSelected, theContainer, theBmpInfo);

		// UxTheme draws the arrow center-aligned

		RECT aRect = theContainer;
		aRect.left = aRect.right - mySubmenuArrowSize.cx - GetMetric(WIDTH_INDENT_RIGHT_EXPANDABLE);

		return myUxMan->DrawThemeBackground(myThemeData, theDC, MENU_POPUPSUBMENU, MPI_NORMAL, &aRect, NULL);
	}

	HRESULT DrawSeparatorItem(HDC theDC, const RECT & theContainer) const
	{
		int aGutterWidth = 0;
		DrawCommonBackground(theDC, theContainer, aGutterWidth);

		RECT aRect = theContainer;
		aRect.left = aGutterWidth;

		return myUxMan->DrawThemeBackground(myThemeData, theDC, MENU_POPUPSEPARATOR, MPI_NORMAL, &aRect, NULL);
	}

	private:
		HRESULT DrawCommonBackground(HDC theDC, const RECT & theContainer, int & theGutterWidth) const
		{
			// this is supposed to be a fully opaque bitmap
			HRESULT aRes = myUxMan->DrawThemeBackground(myThemeData, theDC, MENU_POPUPBACKGROUND, 0, &theContainer, NULL);

			theGutterWidth =
				GetMetric(WIDTH_INDENT_LEFT) +
				GetMetric(WIDTH_ICONSITE) +
				myMenuElementSpacingWidth +
				myGutterWidth; //TODO: can be optimized

			RECT aGutterRect = theContainer;
			aGutterRect.right = theGutterWidth;

			aRes = myUxMan->DrawThemeBackground(myThemeData, theDC, MENU_POPUPGUTTER, 0, &aGutterRect, NULL);

			return aRes;
		}
};

//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

ThemeDefault::ThemeDefault() : myImpl(0), myUxMan( Application::Instance().GetUxManager() )
{
	Reinitialize();

	Application & aApp = Application::Instance();
	RootWindow * aWnd = aApp.GetRootWindowInstance();

	myRootHWnd = *aWnd;
	myTimerID = aWnd->GetAvailableTimerID();

	// 1. Preferences events
	aApp.AddPreferencesEventProcessor(this);

	// 2. Child message map
	aWnd->AddMessageMap(this);
}

//////////////////////////////////////////////////////////////////////////////////////////////

ThemeDefault::~ThemeDefault()
{
	Application & aApp = Application::Instance();
	RootWindow * aWnd = aApp.GetRootWindowInstance();

	// 2. Child message map
	aWnd->RemoveMessageMap(this);

	// 1. Preferences events
	aApp.RemovePreferencesEventProcessor(this);

	delete myImpl;
}

//////////////////////////////////////////////////////////////////////////////////////////////

int ThemeDefault::GetMetric(int theID) const
{
	return myImpl->GetMetric(theID);
}

//////////////////////////////////////////////////////////////////////////////////////////////

HFONT ThemeDefault::GetFont(int theID) const
{
	return myImpl->GetFont(theID);
}

//////////////////////////////////////////////////////////////////////////////////////////////

HDC ThemeDefault::GetMemoryDC(int theID, int theReserved/* = 0*/) const
{
	return myImpl->GetMemoryDC(theID, theReserved);
}

//////////////////////////////////////////////////////////////////////////////////////////////

HRESULT ThemeDefault::GetBitmap(int theID, BitmapInfo * theOutInfo) const
{
	return myImpl->GetBitmap(theID, theOutInfo);
}

//////////////////////////////////////////////////////////////////////////////////////////////

COLORREF ThemeDefault::GetColor(int theID) const
{
	return myImpl->GetColor(theID);
}

//////////////////////////////////////////////////////////////////////////////////////////////

UINT ThemeDefault::GetStyle(int theID) const
{
	return myImpl->GetStyle(theID);
}

//////////////////////////////////////////////////////////////////////////////////////////////

HBRUSH ThemeDefault::GetBrush(int theID) const
{
	return myImpl->GetBrush(theID);
}

//////////////////////////////////////////////////////////////////////////////////////////////

HRESULT ThemeDefault::DrawMenuNcArea(HDC theDC, const RECT & theContainer) const
{
	return myImpl->DrawMenuNcArea(theDC, theContainer);
}

//////////////////////////////////////////////////////////////////////////////////////////////

HRESULT ThemeDefault::DrawItemBackground(HDC theDC, bool theSelected, const RECT & theContainer) const
{
	return myImpl->DrawItemBackground(theDC, theSelected, theContainer);
}

//////////////////////////////////////////////////////////////////////////////////////////////

HRESULT ThemeDefault::DrawItemText(HDC theDC, bool theSelected, const RECT & theContainer, LPWSTR theText, int theSize, DWORD theFlags) const
{
	return myImpl->DrawItemText(theDC, theSelected, theContainer, theText, theSize, theFlags);
}

//////////////////////////////////////////////////////////////////////////////////////////////

HRESULT ThemeDefault::DrawItemBitmap(HDC theDC, bool theSelected, const RECT & theContainer, const BitmapInfo & theBmpInfo) const
{
	return myImpl->DrawItemBitmap(theDC, theSelected, theContainer, theBmpInfo);
}

//////////////////////////////////////////////////////////////////////////////////////////////

HRESULT ThemeDefault::DrawItemIcon(HDC theDC, int theFlags, const RECT & theContainer, HIMAGELIST theImgList, int theIconIndex ) const
{
	return myImpl->DrawItemIcon(theDC, theFlags, theContainer, theImgList, theIconIndex);
}

//////////////////////////////////////////////////////////////////////////////////////////////

HRESULT ThemeDefault::DrawSeparatorItem( HDC theDC, const RECT & theContainer ) const
{
	return myImpl->DrawSeparatorItem(theDC, theContainer);
}

//////////////////////////////////////////////////////////////////////////////////////////////

void ThemeDefault::OnPreferencesEvent( PreferencesEvent, class ApplicationSettings & )
{
	Reinitialize();
}

//////////////////////////////////////////////////////////////////////////////////////////////

void ThemeDefault::Reinitialize()
{
	if (myImpl != 0)
		delete myImpl;

	if ( myUxMan->IsAppThemed() )
	{
		HTHEME aTh = myUxMan->OpenThemeData(0, L"Menu");

		if (aTh != 0)
		{
			myImpl = new ThemeDefault_Aero(myUxMan, aTh);
			return;
		}
	}

	myImpl = new ThemeDefault_NoUxTheming(myUxMan);
}

//////////////////////////////////////////////////////////////////////////////////////////////

LRESULT ThemeDefault::MsgHandler_Timer( UINT theMsg, WPARAM theWParam, LPARAM theLParam, BOOL & theHandled )
{
	if (theWParam == myTimerID)
	{
		myUxMan->ReInitialize();
		Reinitialize();
	}
	else
		theHandled = FALSE;

	return 0L;
}

//////////////////////////////////////////////////////////////////////////////////////////////

LRESULT ThemeDefault::MsgHandler_SettingChange(UINT theMsg, WPARAM theWParam, LPARAM theLParam, BOOL & theHandled)
{
	if (theLParam == 0)
		return 0L;

	TCHAR * aString = (TCHAR *) theLParam;

	bool aAffectingParameter =
		( lstrcmp(aString, _T("WindowMetrics") ) == 0 ) ||
		( lstrcmp(aString, _T("VisualEffects") ) == 0 ) ||
		theWParam == SPI_SETDROPSHADOW;

	if (aAffectingParameter)
	{
		KillTimer(myRootHWnd, myTimerID);

		myUxMan->ReInitialize();
		Reinitialize();
	}

	return 0L;
}

//////////////////////////////////////////////////////////////////////////////////////////////

LRESULT ThemeDefault::MsgHandler_ThemeChanged(UINT theMsg, WPARAM theWParam, LPARAM theLParam, BOOL & theHandled)
{
	Trace("ThemeDefault::WM_THEMECHANGED.\n");

	SetTimer(myRootHWnd, myTimerID, 500, NULL);

	return 0L;
}

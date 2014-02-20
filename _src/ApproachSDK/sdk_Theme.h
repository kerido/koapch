#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////


struct BitmapInfo
{
	int     myWidth;
	int     myHeight;
	UINT    myTransparentColor;
	HBITMAP myBmpHandle;

	BitmapInfo() : myWidth(0), myHeight(0), myTransparentColor(0xFFFFFFFF), myBmpHandle(0) { }

	bool IsTransparent() const { return myTransparentColor != 0xFFFFFFFF; }
};

//////////////////////////////////////////////////////////////////////////////////////////////

//! Contains identifiers of theme metrics.
enum MetricID
{
	WIDTH_ICONSITE,                   //!< Specifies the width of the left area containing the item's icon.

	WIDTH_OFFSETTOTEXT_LEFT,          //!< Specifies the distance from the right edge of the icon site
	                                  //!  to the left origin of item text.

	WIDTH_OFFSETTOTEXT_RIGHT,         //!< TODO

	WIDTH_RIGHTARROWSITE,             //!< Specifies the width of the right area containing the item's submenu arrow.

	WIDTH_INDENT_LEFT,                //!< Specifies the distance from the menu's zero client coordinate,
	                                  //!  to the left edge of the item's icon area.

	WIDTH_INDENT_RIGHT_NORMAL,        //!< TODO

	WIDTH_INDENT_RIGHT_EXPANDABLE,    //!< TODO

	WIDTH_MAXWINDOW,                  //!< Specifies maximum menu width.

	HEIGHT_ITEM,                      //!< Specifies default item height.

	HEIGHTWIDTH_MENUBORDER,           //!< Specifies the width and height of menus' non-client area (border).

	HEIGHT_ITEM_SEPARATOR             //!< Specifies the height of the separator item
};

enum FontID
{
	ITEM_DEFAULT_N,	//normal
	ITEM_DEFAULT_S  //selected
};

enum BitmapID
{
	ARROW_N,
	ARROW_S,

	UARROW_N,
	UARROW_S,

	DARROW_N,
	DARROW_S
};

enum ColorID
{
	TEXT_ITEM_NORMAL,
	TEXT_ITEM_SELECTED,

	NC_BORDER,
	NC_PADDING
};

enum StyleID
{
	STYLE_CLASS_MENU_DEFAULT
};

enum BrushID
{
	BG_ITEM_NORMAL,
	BG_ITEM_SELECTED
};

enum DrawItemIconFlags
{
	DIIF_NORMAL = 0,
	DIIF_SELECTED = 1,
	DIIF_DIMMED   = 2
};

//////////////////////////////////////////////////////////////////////////////////////////////

//! Provides methods and data for menu visual representation. Manages colors, item sizes,
//! backgrounds, and other presentation-related aspects.

//TODO: convert to IUnknown
class Theme
{
public:
	virtual ~Theme() {}

public:
	virtual HRESULT  GetBitmap(int theID, BitmapInfo * theOutInfo) const = 0;
	virtual HDC      GetMemoryDC(int theID, int theReserved = 0) const = 0;
	virtual int      GetMetric(int theID) const = 0;
	virtual HFONT    GetFont(int theID) const = 0;
	virtual COLORREF GetColor(int theID) const = 0;
	virtual UINT     GetStyle(int theID) const = 0;
	virtual HBRUSH   GetBrush(int theID) const = 0;



public:
	virtual HRESULT  DrawMenuNcArea(HDC theDC, const RECT & theContainer) const = 0;
	virtual HRESULT  DrawItemBackground(HDC theDC, bool theSelected, const RECT & theContainer) const = 0;
	virtual HRESULT  DrawItemText(HDC theDC, bool theSelected, const RECT & theContainer, LPWSTR theText, int theSize, DWORD theFlags) const = 0;
	virtual HRESULT  DrawItemBitmap(HDC theDC, bool theSelected, const RECT & theContainer, const BitmapInfo & theBmpInfo) const = 0;
	virtual HRESULT  DrawItemIcon(HDC theDC, int theFlags, const RECT & theContainer, HIMAGELIST theImgList, int theIconIndex) const = 0;
	virtual HRESULT  DrawSeparatorItem(HDC theDC, const RECT & theContainer) const = 0;
};
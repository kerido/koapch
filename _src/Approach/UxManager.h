#pragma once

#include <Vsstyle.h>
#include <Vssym32.h>

//////////////////////////////////////////////////////////////////////////////////////////////

//! Provides methods for integrating with uxtheme.dll when available.
class UxManager
{
// UxTheme.dll typedefs
private:
	typedef BOOL    (__stdcall * fnIsAppThemed  )       ();
	typedef HTHEME  (__stdcall * fnOpenThemeData)       (HWND, LPCWSTR);
	typedef HRESULT (__stdcall * fnGetThemeColor)       (HTHEME, int, int, int, COLORREF *);
	typedef HRESULT (__stdcall * fnDrawThemeBackground) (HTHEME, HDC, int, int, LPCRECT, LPCRECT);
	typedef HRESULT (__stdcall * fnCloseThemeData)      (HTHEME);
	typedef HRESULT (__stdcall * fnDrawThemeText)       (HTHEME, HDC, int, int, LPCWSTR, int, DWORD, DWORD, LPCRECT);
	typedef HRESULT (__stdcall * fnGetThemeMetric)      (HTHEME, HDC, int, int, int, int *);
	typedef HRESULT (__stdcall * fnGetThemePartSize)    (HTHEME, HDC, int, int, LPCRECT, THEMESIZE, SIZE *);
	typedef HRESULT (__stdcall * fnGetThemePosition)    (HTHEME, int, int, int, POINT *);
	typedef HRESULT (__stdcall * fnGetThemeMargins)     (HTHEME, HDC, int, int, int, LPRECT, MARGINS *);
	typedef HRESULT (__stdcall * fnGetThemeInt)         (HTHEME, int, int, int, int *);
	typedef HRESULT (__stdcall * fnDrawThemeEdge)       (HTHEME, HDC, int, int, LPCRECT, UINT, UINT, LPRECT );
	typedef HRESULT (__stdcall * fnSetWindowTheme)      (HWND theWnd, LPCWSTR theSubAppName, LPCWSTR theSubIdList);


// Fields
private:
	HMODULE         myUxThemeDll;                  //!< Handle to the Theme dll available on Windows XP and later.
	                                               //!  \remarks The dll is never explicitly loaded and freed. The object
	                                               //!  simply checks if the dll is already loaded through application
	                                               //!  manifest.

	fnIsAppThemed    UxAppThemed;                  //!< Stores the pointer to the IsAppThemed function.
	fnGetThemeColor  UxGetThemeColor;              //!< Stores the pointer to the GetThemeColor function.
	fnOpenThemeData  UxOpenThemeData;              //!< Stores the pointer to the OpenThemeData function.
	fnCloseThemeData UxCloseThemeData;             //!< Stores the pointer to the CloseThemeData function.
	fnDrawThemeBackground UxDrawThemeBackground;   //!< Stores the pointer to the DrawThemeBackground function.
	fnDrawThemeText  UxDrawThemeText;              //!< Stores the pointer to the DrawThemeText function.
	fnGetThemeMetric UxGetThemeMetric;             //!< Stores the pointer to the GetThemeMetric function.
	fnGetThemePartSize UxGetThemePartSize;         //!< Stores the pointer to the GetThemePartSize function.
	fnGetThemePosition UxGetThemePosition;         //!< Stores the pointer to the GetThemePosition function.
	fnGetThemeMargins UxGetThemeMargins;           //!< Stores the pointer to the GetThemeMargins function.
	fnGetThemeInt UxGetThemeInt;                   //!< Stores the pointer to the GetThemeInt function.
	fnDrawThemeEdge UxDrawThemeEdge;               //!< Stores the pointer to the DrawThemeEdge function.
	fnSetWindowTheme UxSetWindowTheme;             //!< Stores the pointer to the SetWindowTheme; function.


// Constructors, destructor
public:
	UxManager()
	{
		ReInitialize();
	}


public:
	void ReInitialize()
	{
		myUxThemeDll = GetModuleHandle( _T("uxtheme.dll") );

		if (myUxThemeDll != 0)
		{
			UxAppThemed           = (fnIsAppThemed)         GetProcAddress(myUxThemeDll, "IsAppThemed");
			UxGetThemeColor       = (fnGetThemeColor)       GetProcAddress(myUxThemeDll, "GetThemeColor");
			UxOpenThemeData       = (fnOpenThemeData)       GetProcAddress(myUxThemeDll, "OpenThemeData");
			UxCloseThemeData      = (fnCloseThemeData)      GetProcAddress(myUxThemeDll, "CloseThemeData");
			UxDrawThemeBackground = (fnDrawThemeBackground) GetProcAddress(myUxThemeDll, "DrawThemeBackground");
			UxDrawThemeText       = (fnDrawThemeText)       GetProcAddress(myUxThemeDll, "DrawThemeText");
			UxGetThemeMetric      = (fnGetThemeMetric)      GetProcAddress(myUxThemeDll, "GetThemeMetric");
			UxGetThemePartSize    = (fnGetThemePartSize)    GetProcAddress(myUxThemeDll, "GetThemePartSize");
			UxGetThemePosition    = (fnGetThemePosition)    GetProcAddress(myUxThemeDll, "GetThemePosition");
			UxGetThemeMargins     = (fnGetThemeMargins)     GetProcAddress(myUxThemeDll, "GetThemeMargins");
			UxGetThemeInt         = (fnGetThemeInt)         GetProcAddress(myUxThemeDll, "GetThemeInt");
			UxDrawThemeEdge       = (fnDrawThemeEdge)       GetProcAddress(myUxThemeDll, "DrawThemeEdge");
			UxSetWindowTheme      = (fnSetWindowTheme)      GetProcAddress(myUxThemeDll, "SetWindowTheme");
		}
		else
		{
			UxAppThemed      = 0;
			UxGetThemeColor  = 0;
			UxOpenThemeData  = 0;
			UxCloseThemeData = 0;
			UxDrawThemeBackground = 0;
			UxDrawThemeText = 0;
			UxGetThemeMetric = 0;
			UxGetThemePartSize = 0;
			UxGetThemePosition = 0;
			UxGetThemeMargins = 0;
			UxGetThemeInt = 0;
			UxDrawThemeEdge = 0;
			UxSetWindowTheme = 0;
		}
	}


//
public:
	bool IsAppThemed() const
	{
		if (UxAppThemed == 0)
			return false;
		else
			return UxAppThemed() != FALSE;
	}

	HRESULT GetThemeColor (HTHEME theTheme, int thePartID, int theStateID, int thePropID, COLORREF * theOutClr) const
	{
		if (UxGetThemeColor == 0)
			return E_FAIL;
		else
			return UxGetThemeColor(theTheme, thePartID, theStateID, thePropID, theOutClr);
	}

	HTHEME OpenThemeData (HWND theWnd, LPCWSTR theClasses) const
	{
		if (UxOpenThemeData == 0)
			return 0;
		else
			return UxOpenThemeData(theWnd, theClasses);
	}

	HRESULT CloseThemeData (HTHEME theTheme) const
	{
		if (UxAppThemed == 0)
			return E_FAIL;
		else
			return UxCloseThemeData(theTheme);
	}

	HRESULT DrawThemeBackground(HTHEME theTheme, HDC theDC, int thePartID, int theStateID, LPCRECT theRect, LPCRECT theClipRect)
	{
		if (UxDrawThemeBackground == 0)
			return E_FAIL;
		else
			return UxDrawThemeBackground(theTheme, theDC, thePartID, theStateID, theRect, theClipRect);
	}

	HRESULT DrawThemeText(HTHEME theTheme, HDC theDC, int thePartID, int theStateID, LPCWSTR theText, int theNumChars, DWORD theTextFlags, DWORD theTextFlags2, LPCRECT theRect)
	{
		if (UxDrawThemeBackground == 0)
			return E_FAIL;
		else
			return UxDrawThemeText(theTheme, theDC, thePartID, theStateID, theText, theNumChars, theTextFlags, theTextFlags2, theRect);
	}

	HRESULT GetThemeMetric(HTHEME theTheme, HDC theDC, int thePartID, int theStateID, int thePropID, int * theOutVal)
	{
		if (UxGetThemeMetric == 0)
			return E_FAIL;
		else
			return UxGetThemeMetric(theTheme, theDC, thePartID, theStateID, thePropID, theOutVal);
	}

	HRESULT GetThemePartSize(HTHEME theTheme, HDC theDC, int thePartID, int theStateID, LPCRECT theRect, THEMESIZE theSizeType, SIZE * theOutVal)
	{
		if (UxGetThemePartSize == 0)
			return E_FAIL;
		else
			return UxGetThemePartSize(theTheme, theDC, thePartID, theStateID, theRect, theSizeType, theOutVal);
	}

	HRESULT GetThemePosition(HTHEME theTheme, int thePartID, int theStateID, int thePropID, POINT * theOutVal)
	{
		if (UxGetThemePosition == 0)
			return E_FAIL;
		else
			return UxGetThemePosition(theTheme, thePartID, theStateID, thePropID, theOutVal);
	}

	HRESULT GetThemeMargins(HTHEME theTheme, HDC theDC, int thePartID, int theStateID, int thePropID, LPRECT theRect, MARGINS * theMargins)
	{
		if (UxGetThemePosition == 0)
			return E_FAIL;
		else
			return UxGetThemeMargins(theTheme, theDC, thePartID, theStateID, thePropID, theRect, theMargins);
	}

	HRESULT GetThemeInt(HTHEME theTheme, int thePartID, int theStateID, int thePropID, int * theOutVal)
	{
		if (UxGetThemeInt == 0)
			return E_FAIL;
		else
			return UxGetThemeInt(theTheme, thePartID, theStateID, thePropID, theOutVal);
	}

	HRESULT DrawThemeEdge(HTHEME theTheme, HDC theDC, int thePartID, int theStateID, LPCRECT theDestRect, UINT theEdge, UINT theFlags, LPRECT theContentRect)
	{
		if (UxDrawThemeEdge == 0)
			return E_FAIL;
		else
			return UxDrawThemeEdge(theTheme, theDC, thePartID, theStateID, theDestRect, theEdge, theFlags, theContentRect);
	}

	HRESULT SetWindowTheme(HWND theWnd, LPCWSTR theSubAppName, LPCWSTR theSubIdList)
	{
		if (UxSetWindowTheme == 0)
			return E_FAIL;
		else
			return UxSetWindowTheme(theWnd, theSubAppName, theSubIdList);
	}
};
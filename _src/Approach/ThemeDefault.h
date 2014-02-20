#pragma once

#include "sdk_Theme.h"

#include "LogicCommon.h"
#include "Preferences.h"

class UxManager;

//////////////////////////////////////////////////////////////////////////////////////////////

//! Represents a theme that is in use on Windows 2000 and Windows XP
class ThemeDefault :
	public Theme,
	public CMessageMap,
	public IPreferencesEventProcessor
{
// Fields
private:
	Theme     * myImpl;           //!< TODO
	UxManager * myUxMan;          //!< TODO
	HWND        myRootHWnd;       //!< Handle of the KO Approach root window.
	UINT        myTimerID;        //!< The ID of the timer fired when global presentation settings are changed.


// Constructors, Destructor
public:
	ThemeDefault();
	~ThemeDefault();


// Theme Members
protected:
	HRESULT GetBitmap(int theID, BitmapInfo * theOutInfo) const;
	HDC GetMemoryDC(int theID, int theReserved = 0) const;
	int GetMetric(int theID) const;
	HFONT GetFont(int theID) const;
	COLORREF GetColor(int theID) const;
	UINT GetStyle(int theID) const;
	HBRUSH GetBrush(int theID) const;

	HRESULT DrawMenuNcArea(HDC theDC, const RECT & theContainer) const;
	HRESULT DrawItemBackground(HDC theDC, bool theSelected, const RECT & theContainer) const;
	HRESULT DrawItemText(HDC theDC, bool theSelected, const RECT & theContainer, LPWSTR theText, int theSize, DWORD theFlags) const;
	HRESULT DrawItemBitmap(HDC theDC, bool theSelected, const RECT & theContainer, const BitmapInfo & theBmpInfo) const;
	HRESULT DrawItemIcon(HDC theDC, int theFlags, const RECT & theContainer, HIMAGELIST theImgList, int theIconIndex) const;
	HRESULT DrawSeparatorItem(HDC theDC, const RECT & theContainer) const;


// IPreferencesEventProcessor Members
protected:
	void OnPreferencesEvent(PreferencesEvent, class ApplicationSettings &);


// WTL Windowing
protected:
	BEGIN_MSG_MAP(ThemeDefault)
		MESSAGE_HANDLER(WM_TIMER,                 MsgHandler_Timer)
		MESSAGE_HANDLER(WM_SETTINGCHANGE,         MsgHandler_SettingChange)
		MESSAGE_HANDLER(WM_THEMECHANGED,          MsgHandler_ThemeChanged)
	END_MSG_MAP()


private:
	LRESULT MsgHandler_Timer(UINT theMsg, WPARAM theWParam, LPARAM theLParam, BOOL & theHandled);

	LRESULT MsgHandler_SettingChange(UINT theMsg, WPARAM theWParam, LPARAM theLParam, BOOL & theHandled);

	LRESULT MsgHandler_ThemeChanged(UINT theMsg, WPARAM theWParam, LPARAM theLParam, BOOL & theHandled);

// Implementation Details
private:
	void Reinitialize();
};

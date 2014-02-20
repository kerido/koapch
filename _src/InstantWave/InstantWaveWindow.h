#pragma once

#include "sdk_MenuWindow.h"

//////////////////////////////////////////////////////////////////////////////////////////////

class InstantWaveWindow : public MenuWindow
{
private:
	IMenuManagerAgent * const myAgent;
	HWND myDialog;
	TCHAR myFileName[256];

private:
	const static UINT WM_USER_PLAY = WM_USER + 1;
	const static UINT WM_USER_STOP = WM_USER + 2;

public:
	InstantWaveWindow(IMenuManagerAgent * theAgent, const TCHAR * theFullPath, HINSTANCE theMainInstance);
	~InstantWaveWindow();

protected:
	virtual LRESULT WndProc(HWND theWnd, UINT theMsg, WPARAM theWParam, LPARAM theLParam);

	static INT_PTR CALLBACK DlgPlayerProc(HWND theDlg, UINT theMsg, WPARAM theWParam, LPARAM theLParam);
};
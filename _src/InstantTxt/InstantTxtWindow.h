#pragma once

#include "sdk_MenuWindow.h"

//////////////////////////////////////////////////////////////////////////////////////////////

class InstantTxtWindow : public MenuWindow
{
private:
	const static int BufferSize = 8192;

private:
	IMenuManagerAgent * const myAgent;
	HWND myEditBox;
	TCHAR myFileName[MAX_PATH];

private:
	const static UINT WM_USER_PLAY = WM_USER + 1;
	const static UINT WM_USER_STOP = WM_USER + 2;

public:
	InstantTxtWindow(IMenuManagerAgent * theAgent, const TCHAR * theFullPath);
	~InstantTxtWindow();

protected:
	virtual LRESULT WndProc(HWND theWnd, UINT theMsg, WPARAM theWParam, LPARAM theLParam);
};
#pragma once

class WindowUtility
{
public:
	static void SetForegroundWindow_ThreadInput(HWND theWnd)
	{
		HWND aCurFgWnd = GetForegroundWindow();

		DWORD aCurFgThr = GetWindowThreadProcessId(aCurFgWnd, NULL);
		DWORD aCurThr = GetCurrentThreadId();

		if (aCurFgThr != aCurThr)
		{
			AttachThreadInput(aCurFgThr, aCurThr, TRUE);
			SetForegroundWindow(theWnd);
			SetFocus(theWnd);
			AttachThreadInput(aCurFgThr, aCurThr, FALSE);
		}
		else
		{
			SetForegroundWindow(theWnd);
			SetFocus(theWnd);
		}
	}

	static void SetForegroundWindow_SimulatedMouseClick(HWND hWnd)
	{
		RECT aRct;
		GetWindowRect(hWnd, &aRct);

		int aMaxX = GetSystemMetrics(SM_CXSCREEN), aMaxY = GetSystemMetrics(SM_CYSCREEN);

		INPUT aInp;
		aInp.type = INPUT_MOUSE;
		aInp.mi.dx = (aRct.left + 1)* 0xFFFF / aMaxX;
		aInp.mi.dy = (aRct.top  + 1)* 0xFFFF / aMaxY;

		aInp.mi.dwFlags = MOUSEEVENTF_LEFTDOWN|MOUSEEVENTF_ABSOLUTE;
		aInp.mi.time = 0;
		aInp.mi.mouseData = 0;
		aInp.mi.dwExtraInfo = 0;

		SendInput(1, &aInp, sizeof(INPUT) );

		aInp.mi.dwFlags = MOUSEEVENTF_LEFTUP|MOUSEEVENTF_ABSOLUTE;

		SendInput(1, &aInp, sizeof(INPUT) );

		SetForegroundWindow(hWnd);
	}
};
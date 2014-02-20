#pragma once

class ShellFolderViewHitTest
{
public:
	typedef int (*HitTestFunc)(HWND theWnd, int theX, int theY);

public:
	static HitTestFunc GetHitTestFunc(HWND theOriginator);
};

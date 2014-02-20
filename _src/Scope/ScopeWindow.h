#pragma once

#include "sdk_MenuWindow.h"

class CxImage;

//////////////////////////////////////////////////////////////////////////////////////////////

class ScopeWindowNew : public MenuWindow
{
	enum Metrics { Margin = 10, Size = 200 };

private:
	CxImage * myImg;
	IMenuManagerAgent * const myAgent;


// Constructor, destructor
public:
	ScopeWindowNew(IMenuManagerAgent * theAgent, const TCHAR * theFullImagePath);

	~ScopeWindowNew();


protected:
	LRESULT WndProc(HWND theWnd, UINT theMsg, WPARAM theWParam, LPARAM theLParam);


private:
	void Draw();
};
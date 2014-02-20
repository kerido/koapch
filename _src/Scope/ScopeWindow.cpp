#include "Common.h"

#include "CxImage\ximage.h"
#include "ScopeWindow.h"

//////////////////////////////////////////////////////////////////////////////////////////////

ScopeWindowNew::ScopeWindowNew(IMenuManagerAgent * theAgent, const TCHAR * theFullImagePath)
	: MenuWindow(theAgent), myAgent(theAgent)
{
	myImg = new CxImage(theFullImagePath, 0);

	RECT aRect = {0, 0, Size, Size };

	AdjustWindowRect( &aRect, GetWindowLong(GetHandle(), GWL_STYLE), false );

	int aWidth = aRect.right - aRect.left,
		aHeight = aRect.bottom - aRect.top;

	SetWindowPos( GetHandle(), NULL, 0, 0, aWidth, aHeight, SWP_NOZORDER|SWP_NOMOVE );
}

//////////////////////////////////////////////////////////////////////////////////////////////

ScopeWindowNew::~ScopeWindowNew()
{
	delete myImg;
}

//////////////////////////////////////////////////////////////////////////////////////////////

LRESULT ScopeWindowNew::WndProc(HWND theWnd, UINT theMsg, WPARAM theWParam, LPARAM theLParam)
{
	switch (theMsg)
	{
	case WM_PAINT:
		Draw();
		break;

	case WM_KEYDOWN:
		{
			UINT aVKeyCode = (UINT) theWParam;
			UINT aPrev = VK_LEFT;

			IMenuKeyboardNav * aNav = 0;
			myAgent->QueryInterface( __uuidof(IMenuKeyboardNav), (void **) &aNav);

			if (aNav != 0)
			{
				aNav->GetKeyCode(IMenuKeyboardNav::KEY_PREV, &aPrev);
				aNav->Release();
			}

			if (aVKeyCode == VK_ESCAPE || aVKeyCode == aPrev)
			{
				myAgent->DestroyMenuChain(this);
			}
			else if (aVKeyCode == VK_UP || aVKeyCode == VK_DOWN)
			{
				if (myParent != 0)
					PostMessage( myParent->GetHandle(), theMsg, theWParam, theLParam );

				myAgent->DestroyMenuChain(this);
			}
		}
		break;

	default:
		return DefWindowProc(theWnd, theMsg, theWParam, theLParam);
	}

	return 0L;
}

//////////////////////////////////////////////////////////////////////////////////////////////

void ScopeWindowNew::Draw()
{
	PAINTSTRUCT aPS = {0};

	BeginPaint (GetHandle(), &aPS );

	int aDestX = 0, aDestY = 0, aDestWidth = 0, aDestHeight = 0;

	if (myImg->GetHeight() < Size - 2*Margin &&
		  myImg->GetWidth()  < Size - 2*Margin)
	{
		aDestWidth = (int)myImg->GetWidth();
		aDestHeight = (int)myImg->GetHeight();
	}
	else if (myImg->GetHeight() > myImg->GetWidth())
	{
		aDestHeight = (int)Size - 2*Margin;
		aDestWidth = myImg->GetWidth() * aDestHeight / myImg->GetHeight();
	}
	else
	{
		aDestWidth = Size - 2*Margin;
		aDestHeight = myImg->GetHeight() * aDestWidth / myImg->GetWidth();
	}

	aDestX = (Size - aDestWidth) / 2;
	aDestY = (Size - aDestHeight) / 2;

	long aRes = myImg->Draw(aPS.hdc,aDestX, aDestY, aDestWidth, aDestHeight);

	if (!aRes)
	{
		RECT aClientRect;

		GetClientRect(GetHandle(), &aClientRect);

		aClientRect.left += 4;
		aClientRect.top += 80;
		aClientRect.right -= 4;
		//aClientRect.bottom -=40;

		SetBkMode(aPS.hdc, TRANSPARENT);
		DrawText(aPS.hdc, _T("ERROR: image could not be displayed"), -1, &aClientRect, DT_WORDBREAK|DT_CENTER);
	}

	EndPaint( GetHandle(), &aPS );
}

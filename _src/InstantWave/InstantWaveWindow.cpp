#include "Common.h"

#include <mmsystem.h>

#include "InstantWaveWindow.h"
#include "InstantWavePlugin.h"
#include "resource.h"

//////////////////////////////////////////////////////////////////////////////////////////////

InstantWaveWindow::InstantWaveWindow(IMenuManagerAgent * theAgent, const TCHAR * theFullPath, HINSTANCE theMainInstance)
	: MenuWindow(theAgent), myAgent(theAgent)
{
	SecureZeroMemory(myFileName, 256);

	myDialog = CreateDialog(theMainInstance, MAKEINTRESOURCE(IDD_PLAYER), GetHandle(), DlgPlayerProc);

	lstrcpyn(myFileName, theFullPath, 255);

	RECT aRect;
	GetWindowRect(myDialog, &aRect);


	AdjustWindowRect( &aRect, GetWindowLong(GetHandle(), GWL_STYLE), false );

	int aWidth = aRect.right - aRect.left,
			aHeight = aRect.bottom - aRect.top;

	SetWindowPos( GetHandle(), NULL, 0, 0, aWidth, aHeight, SWP_NOZORDER|SWP_NOMOVE );
	SetWindowPos( myDialog, NULL, 0, 0, 0, 0, SWP_NOZORDER|SWP_NOSIZE );
}

//////////////////////////////////////////////////////////////////////////////////////////////

InstantWaveWindow::~InstantWaveWindow()
{
	PlaySound(NULL, NULL, SND_ASYNC);
	DestroyWindow(myDialog);
}

//////////////////////////////////////////////////////////////////////////////////////////////

INT_PTR CALLBACK InstantWaveWindow::DlgPlayerProc(HWND theDlg, UINT theMsg, WPARAM theWParam, LPARAM theLParam)
{
	switch(theMsg)
	{
	case WM_COMMAND:
		switch ( LOWORD(theWParam) )
		{
		case IDC_PLAY:
			SendMessage( ::GetParent(theDlg), WM_USER_PLAY, 0, 0);
			return TRUE;

		case IDC_STOP:
			SendMessage( ::GetParent(theDlg), WM_USER_STOP, 0, 0);
			return TRUE;

		default:
			break;
		}
		break;

	case WM_ERASEBKGND:
		//{
		//	HDC aDC = (HDC) theWParam;

		//	RECT aRect;
		//	GetClientRect( theDlg, &aRect );

		//	HBRUSH aBgBrush = (HBRUSH) GetClassLong( ::GetParent(theDlg), GCL_HBRBACKGROUND);

		//	FillRect(aDC, &aRect, aBgBrush);
		//}
		return 1;

	default:
		break;
	}

	return FALSE;
}

//////////////////////////////////////////////////////////////////////////////////////////////

LRESULT InstantWaveWindow::WndProc(HWND theWnd, UINT theMsg, WPARAM theWParam, LPARAM theLParam)
{
	switch (theMsg)
	{
	case WM_USER_PLAY:
		PlaySound(myFileName, NULL, SND_ASYNC|SND_FILENAME|SND_NOSTOP);
		break;

	case WM_USER_STOP:
		PlaySound(NULL, NULL, SND_ASYNC);
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


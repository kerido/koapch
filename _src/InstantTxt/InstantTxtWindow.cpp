#include "Common.h"

#include "InstantTxtWindow.h"
#include "InstantTxtPlugin.h"
#include "InstantTxtCommon.h"
#include "resource.h"

//////////////////////////////////////////////////////////////////////////////////////////////

InstantTxtWindow::InstantTxtWindow(IMenuManagerAgent * theAgent, const TCHAR * theFullPath)
	: MenuWindow(theAgent), myAgent(theAgent)
{
	lstrcpyn(myFileName, theFullPath, 255);

	IMenuManagerAgent::MenuData aDt;
	theAgent->GetMenuData(&aDt);

	myEditBox = CreateWindowEx(
		WS_EX_CLIENTEDGE,
		_T("EDIT"),
		NULL,
		ES_MULTILINE | WS_VSCROLL | WS_VISIBLE | WS_CHILD,
		10,
		10,
		180,
		180,
		GetHandle(),
		NULL,
		aDt.ModuleInstance,
		NULL);

	SendMessage(myEditBox, WM_SETFONT, (LPARAM) GetStockObject(DEFAULT_GUI_FONT), FALSE );

	RECT aRect;
	aRect.left  = aRect.top    =   0;
	aRect.right = aRect.bottom = 200;

	AdjustWindowRect( &aRect, GetWindowLong(GetHandle(), GWL_STYLE), false );

	int aWidth = aRect.right - aRect.left,
			aHeight = aRect.bottom - aRect.top;

	SetWindowPos( GetHandle(), NULL, 0, 0, aWidth, aHeight, SWP_NOZORDER|SWP_NOMOVE );

	HANDLE aFile = CreateFile(
		theFullPath,
		GENERIC_READ,               //open for reading
		FILE_SHARE_READ,            //share for reading
		NULL,                       //no security
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL );

	if (aFile == INVALID_HANDLE_VALUE)
		return;

	unsigned char * aBuf = new unsigned char[BufferSize];

	DWORD aRead = 0;
	BOOL aRes = ReadFile(aFile, aBuf, BufferSize-1, &aRead, NULL);

	CloseHandle(aFile);

	if (aRes == TRUE && aRead > 0)
	{
		aBuf[aRead] = 0;

		if ( aBuf[0] == 0xff && aBuf[1] == 0xfe )	//unicode
		{
			TCHAR * aBuf_Uni = (TCHAR *)(aBuf + 2);
			aBuf_Uni[aRead/2-1] = 0;

			SendMessage(myEditBox, WM_SETTEXT, NULL, (LPARAM) aBuf_Uni);
		}
		else
		{
			TCHAR * aBuf_Uni = new TCHAR[BufferSize];

			if ( aBuf[0] == 0xfe && aBuf[1] == 0xff )	//unicode, big endian
			{
				for (DWORD i = 2; i < aRead; i+=2)
					aBuf_Uni[i/2 - 1] = ( (TCHAR)aBuf[i] << 8 ) | (TCHAR)aBuf[i+1];

				aBuf_Uni[aRead/2 - 1] = 0;
			}
			else if ( aBuf[0] == 0xef && aBuf[1] == 0xbb && aBuf[2] == 0xbf )	//utf8
			{
				int a = MultiByteToWideChar(CP_UTF8, 0, (LPCSTR) aBuf+3, -1, aBuf_Uni, BufferSize);

				aBuf_Uni[a-1] = 0;
			}
			else	//ascii
			{
				int a = MultiByteToWideChar(CP_ACP, 0, (LPCSTR) aBuf, -1, aBuf_Uni, BufferSize);

				aBuf_Uni[a-1] = 0;
			}

			SendMessage(myEditBox, WM_SETTEXT, NULL, (LPARAM) aBuf_Uni);
			delete [] aBuf_Uni;
		}
	}

	delete [] aBuf;
}

//////////////////////////////////////////////////////////////////////////////////////////////

InstantTxtWindow::~InstantTxtWindow()
{
	DestroyWindow(myEditBox);
}

//////////////////////////////////////////////////////////////////////////////////////////////

LRESULT InstantTxtWindow::WndProc(HWND theWnd, UINT theMsg, WPARAM theWParam, LPARAM theLParam)
{
	switch (theMsg)
	{
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


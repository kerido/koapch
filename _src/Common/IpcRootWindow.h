#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////

class IpcRootWindow
{
public:
	static HWND FindSingleInstance()
	{
		return FindWindow( NULL, GetWindowCaption() );
	}

	//! Returns true if the program was terminated within the maximum of 100 attempts. Otherwise returns false
	static bool Quit()
	{
		HWND aWnd = NULL;
		BOOL aSignalSent = FALSE;

		for (int i = 0; i < 100; i++)	//perform fifty attempts
		{
			aWnd = FindSingleInstance();

			if (aWnd == NULL)
				break;

			aSignalSent = ::PostMessage(aWnd, WM_USER_QUIT, 0, 0);
			Sleep(50);
		}

		if (aSignalSent)
			Sleep(1000);	// if we actually send the termination signal, give the OS some time to quit the program

		return (aWnd == NULL);
	}


protected:
	static TCHAR * GetWindowCaption()
	{ return _T("DB135C26-7D1F-4C58-9EBE-748D226F5E05"); }


protected:
	//! Represents the message sent to the Approach Root Window to notify it that
	//! a running instance of KO Approach must end its work and quit gradually.
	//! The value of this message must never be changed.
	static const UINT WM_USER_QUIT               = WM_USER +  3;
	static const UINT WM_USER_FIRST              = WM_USER + 10;

public:
	static const UINT WM_USER_DESTROYNONFLOATING = WM_USER +  4;
};
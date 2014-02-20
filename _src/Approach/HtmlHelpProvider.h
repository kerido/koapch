#pragma once

class HtmlHelpProvider
{
private:
	DWORD myHtmlHelpCookie;
	TCHAR myHelpPath[MAX_PATH];


public:
	HtmlHelpProvider() : myHtmlHelpCookie(0)
	{
		int aSize = GetModuleFileName(_Module.GetModuleInstance(), myHelpPath, MAX_PATH);

		TCHAR aBackSlash = _T('\\');
		TCHAR * aBSOccur = _tcsrchr(myHelpPath, aBackSlash); //find last occurrence of a backslash

		if (aBSOccur)
		{
			size_t aLastCharPos = (aBSOccur - myHelpPath) + 1;
			myHelpPath[aLastCharPos] = 0;
			lstrcat(myHelpPath, _T("Approach.chm") );

			WIN32_FIND_DATA aDt;
			HANDLE aFindHandle = FindFirstFile(myHelpPath, &aDt);

			if (aFindHandle != INVALID_HANDLE_VALUE)
			{
				FindClose(aFindHandle);

#pragma warning(disable:4311)
				HtmlHelp(NULL, NULL, HH_INITIALIZE, (DWORD)&myHtmlHelpCookie);
#pragma warning(default:4311)
			}
			else
				myHelpPath[0] = 0;
		}
		else
			myHelpPath[0] = 0;	//help
	}

	~HtmlHelpProvider()
	{
		if (myHtmlHelpCookie != 0)
			HtmlHelp(NULL, NULL, HH_UNINITIALIZE, myHtmlHelpCookie);
	}


public:
	bool IsHelpEnabled() const
	{
		return myHtmlHelpCookie != 0;
	}

	void DisplayHelp(HWND theParentWindow, TCHAR * theContext = NULL) const
	{
		if ( theContext == NULL)
		{
			HWND aWnd = HtmlHelp(theParentWindow, myHelpPath, HH_DISPLAY_TOC, NULL);
			ATLASSERT(aWnd != 0);
		}
		else
		{
			TCHAR aFull [1000];

			lstrcpy(aFull, myHelpPath);
			lstrcat(aFull, _T("::/content/") );
			lstrcat(aFull, theContext );

			HWND aWnd = HtmlHelp(theParentWindow, aFull, HH_DISPLAY_TOPIC, NULL);
			ATLASSERT(aWnd != 0);
		}
	}
};
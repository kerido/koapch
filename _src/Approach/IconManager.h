#pragma once

#include "resource.h"

//////////////////////////////////////////////////////////////////////////////////////////////

class IconManager
{
	typedef HRESULT (__stdcall * fnLoadIconWithScaleDown)(HINSTANCE, PCWSTR, int, int, HICON *);


private:
	fnLoadIconWithScaleDown mfDpiAwareLoader;

private:
	HICON myAsterisk;
	HICON myInfo;
	HICON myStop;
	HICON myApp;
	HICON myAppSmall;

public:
	IconManager() : mfDpiAwareLoader(0), myAsterisk(0), myInfo(0), myStop(0), myApp(0), myAppSmall(0)
	{
		HMODULE aMod = GetModuleHandle( _T("comctl32.dll") );

		if (aMod != 0)
			mfDpiAwareLoader = (fnLoadIconWithScaleDown) GetProcAddress(aMod, "LoadIconWithScaleDown");
	}

	~IconManager()
	{
		if (myAsterisk != 0)
			DestroyIcon(myAsterisk);

		if (myInfo != 0)
			DestroyIcon(myInfo);

		if (myStop != 0)
			DestroyIcon(myStop);

		if (myApp != 0)
			DestroyIcon(myApp);

		if (myAppSmall != 0)
			DestroyIcon(myAppSmall);
	}


public:
	HICON GetApplicationIcon()
	{
		if (myApp == 0)
			myApp = LoadIconDpiAware( _Module.GetResourceInstance(), MAKEINTRESOURCE(IDI_APPROACH) );

		return myApp;
	}

	HICON GetApplicationIconSmall()
	{
		if (myAppSmall == 0)
		{
			int aIconWidth = GetSystemMetrics(SM_CXSMICON);
			int aIconHeight = GetSystemMetrics(SM_CYSMICON);

			myAppSmall = (HICON) LoadImage( _Module.GetResourceInstance(), MAKEINTRESOURCE(IDI_APPROACH), IMAGE_ICON, aIconWidth, aIconHeight, NULL);
		}

		return myAppSmall;
	}

	HICON GetInformationIcon()
	{
		if (myInfo == 0)
			myInfo = LoadIconDpiAware(NULL, IDI_INFORMATION);

		return myInfo;
	}

	HICON GetAsteriskIcon()
	{
		if (myAsterisk == 0)
			myAsterisk = LoadIconDpiAware(NULL, IDI_EXCLAMATION);

		return myAsterisk;
	}

	HICON GetErrorIcon()
	{
		if (myStop == 0)
			myStop = LoadIconDpiAware(NULL, IDI_HAND);

		return myStop;
	}

	HICON LoadIconDpiAware(HINSTANCE theInst, LPCTSTR theID) const
	{
		return LoadIconDpiAware(theInst, theID, false);
	}

	HICON LoadIconDpiAware(HINSTANCE theInst, LPCTSTR theID, bool theSmall) const
	{
		int aIconWidth = 0;
		int aIconHeight = 0;

		if (theSmall)
		{
			aIconWidth  = GetSystemMetrics(SM_CXSMICON);
			aIconHeight = GetSystemMetrics(SM_CYSMICON);
		}
		else
		{
			aIconWidth  = GetSystemMetrics(SM_CXICON);
			aIconHeight = GetSystemMetrics(SM_CYICON);
		}

		return LoadIconDpiAware(theInst, theID, aIconWidth, aIconHeight);
	}


private:
	HICON LoadIconDpiAware(HINSTANCE theInst, LPCTSTR theID, int theWidth, int theHeight) const
	{
		HICON aRetVal = 0;

		if (mfDpiAwareLoader != 0)
		{
			HRESULT aRes = mfDpiAwareLoader(theInst, theID, theWidth, theHeight, &aRetVal);

			if ( FAILED(aRes) )
				return NULL;
			else
				return aRetVal;
		}
		else
			return LoadIcon(theInst, theID);
	}
};
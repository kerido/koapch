#include "Common.h"

#include <shlwapi.h>

#include "InstantTxtPlugin.h"
#include "InstantTxtWindow.h"
#include "InstantTxtCommon.h"
#include "resource.h"

//////////////////////////////////////////////////////////////////////////////////////////////

static const GUID gGuid  = { 0x3D644B28, 0xCB45, 0x4766, { 0xB2, 0x25, 0x21, 0x7B, 0xAF, 0xDF, 0x77, 0xDD } };

//////////////////////////////////////////////////////////////////////////////////////////////

InstantTxtPlugin::InstantTxtPlugin( HINSTANCE theDllInstance )
: myInstance(theDllInstance), mySupportedExtensionsSize(0)
{ }

//////////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP InstantTxtPlugin::OnLoad(int theFrameworkVersion)
{
	if (theFrameworkVersion != 1)
		return E_FAIL;

	HKEY aKey;

	LONG aRes = RegOpenKeyEx(
			HKEY_CURRENT_USER,
			_T("SOFTWARE\\KO Software\\InstantTxt"),
			0,
			KEY_READ,
			&aKey);


	if (aRes == ERROR_SUCCESS)
	{
		DWORD aSize = sizeof myExtensions * sizeof TCHAR;

		aRes = RegQueryValueEx(aKey, _T("HandledExtensions"), 0, NULL, (LPBYTE) myExtensions, &aSize);

		if ( aRes == ERROR_SUCCESS)
		{
			for (TCHAR * aPos = myExtensions; aPos != 0 && mySupportedExtensionsSize < NumExtensions; )
			{
				TCHAR * aCur = aPos;    // save the current position
				aPos = StrChr( aPos, _T('*') );

				if (aPos != 0)
					*aPos++ = 0;           // null-terminate the current string and set the position for a future iteration

				mySupportedExtensions[mySupportedExtensionsSize++] = aCur;
			}
		}

		RegCloseKey(aKey);
	}
	else
	{
		lstrcpy(myExtensions, _T("txt") );
		mySupportedExtensions[mySupportedExtensionsSize++] = myExtensions;
	}

	fnItemMatch = UtilFileExtensionItemMatch::InitItemMatchImplementation();

	myLetter = LoadBitmap(myInstance, MAKEINTRESOURCE(IDB_LETTER) );

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP InstantTxtPlugin::OnUnload()
{
	DeleteObject(myLetter);

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP InstantTxtPlugin::Build(Item * theItem, const MenuBuilderData * theBuildData, MenuWindow ** theOutMenu)
{
	if (theItem == 0 || theBuildData == 0)
		return E_INVALIDARG;

	if (theOutMenu == 0)
		return E_POINTER;

	TCHAR aName[MAX_PATH];
	int aSize = MAX_PATH;

	HRESULT aRes = theItem->GetDisplayName(true, aName, &aSize);

	if ( SUCCEEDED(aRes) )
	{
		InstantTxtWindow * aWnd = new InstantTxtWindow(theBuildData->Agent, aName);
		aWnd->SetTheme(theBuildData->Theme);

		*theOutMenu = aWnd;
	}
	else
		*theOutMenu = 0;

	return aRes;
}

//////////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP InstantTxtPlugin::CanBuild(Item * theItem, UINT * theOut)
{
	if (theItem == 0)
		return E_INVALIDARG;

	if (theOut == 0)
		return E_POINTER;

	*theOut = fnItemMatch(theItem, (LPCTSTR *)mySupportedExtensions, mySupportedExtensionsSize);
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP InstantTxtPlugin::GetArrowBitmap(UINT theExpCode, bool theSelected, const Theme * theTheme, BitmapInfo * theOutInfo)
{
	if (theOutInfo == 0)
		return E_POINTER;

	theOutInfo->myWidth = 9;
	theOutInfo->myHeight = 9;
	theOutInfo->myBmpHandle = myLetter;

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP InstantTxtPlugin::CreateInstance(IUnknown * theOuter, REFIID theIID, void ** theOut)
{
	if (theIID != IID_IMenuBuilder)
		return E_NOINTERFACE;

	IMenuBuilder * aMB = this;
	aMB->AddRef();
	*theOut = aMB;

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP InstantTxtPlugin::LockServer( BOOL theLock )
{
	return E_NOTIMPL;
}

//////////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP InstantTxtPlugin::GetTypeGuid(LPGUID theOutGuid)
{
	if (theOutGuid == 0)
		return E_POINTER;

	*theOutGuid = gGuid;
	return S_OK;
}

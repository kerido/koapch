#include "Common.h"

#include "sdk_MenuBuilder.h"

#include "InstantWavePlugin.h"
#include "InstantWaveWindow.h"
#include "resource.h"

//////////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP InstantWavePlugin::OnLoad(int theFrameworkVersion)
{
	myWaveform = LoadBitmap(myInstance, MAKEINTRESOURCE(IDB_WAVEFORM) );

	lstrcpy(myExtensions, _T("wav") );
	mySupportedExtensions[0] = myExtensions;

	fnItemMatch = UtilFileExtensionItemMatch::InitItemMatchImplementation();

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP InstantWavePlugin::OnUnload()
{
	DeleteObject(myWaveform);

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP InstantWavePlugin::Build(Item * theItem, const MenuBuilderData * theBuildData, MenuWindow ** theOutMenu)
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
		InstantWaveWindow * aWnd = new InstantWaveWindow( theBuildData->Agent, aName, GetInstance() );
		aWnd->SetTheme(theBuildData->Theme);

		*theOutMenu = aWnd;
	}
	else
		*theOutMenu = 0;

	return aRes;
}
//////////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP InstantWavePlugin::CanBuild(Item * theItem, UINT * theOut)
{
	if (theItem == 0)
		return E_INVALIDARG;

	if (theOut == 0)
		return E_POINTER;

	*theOut = fnItemMatch(theItem, (LPCTSTR *)mySupportedExtensions, 1);
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP InstantWavePlugin::GetArrowBitmap(UINT theExpCode, bool theSelected, const Theme * theTheme, BitmapInfo * theOutInfo)
{
	if (theOutInfo == 0)
		return E_POINTER;

	theOutInfo->myWidth  = 9;
	theOutInfo->myHeight = 9;
	theOutInfo->myBmpHandle = myWaveform;

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP InstantWavePlugin::CreateInstance(IUnknown * theOuter, REFIID theIID, void ** theOut)
{
	if (theIID != IID_IMenuBuilder)
		return E_NOINTERFACE;

	IMenuBuilder * aMB = this;
	aMB->AddRef();
	*theOut = aMB;

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP InstantWavePlugin::LockServer( BOOL theLock )
{
	return E_NOTIMPL;
}

//////////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP InstantWavePlugin::GetTypeGuid(LPGUID theOutGuid)
{
	* theOutGuid = GUID_InstantWave;
	return S_OK;
}
#include "Common.h"

#include "ScopePlugin.h"
#include "ScopeWindow.h"

#include "resource.h"

//////////////////////////////////////////////////////////////////////////////////////////////

ScopePlugin::ScopePlugin(HINSTANCE theDllInstance)
	: myInstance(theDllInstance), myExtensionListSize(0)
{
#ifdef _BMP_ONLY
	mySupportedExtensions[myExtensionListSize++] = _T("bmp");
#else
	mySupportedExtensions[myExtensionListSize++] = _T("png");
	mySupportedExtensions[myExtensionListSize++] = _T("jpe");
	mySupportedExtensions[myExtensionListSize++] = _T("jpg");
	mySupportedExtensions[myExtensionListSize++] = _T("jpeg");
	mySupportedExtensions[myExtensionListSize++] = _T("jfif");
	mySupportedExtensions[myExtensionListSize++] = _T("tif");
	mySupportedExtensions[myExtensionListSize++] = _T("tiff");
	mySupportedExtensions[myExtensionListSize++] = _T("png");
	mySupportedExtensions[myExtensionListSize++] = _T("gif");
	mySupportedExtensions[myExtensionListSize++] = _T("emf");
	mySupportedExtensions[myExtensionListSize++] = _T("wmf");
	mySupportedExtensions[myExtensionListSize++] = _T("pcx");
	mySupportedExtensions[myExtensionListSize++] = _T("bmp");
#endif
}

//////////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP ScopePlugin::Build(Item * theItem, const MenuBuilderData * theBuildData, MenuWindow ** theOutMenu)
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
		ScopeWindowNew * aWnd = new ScopeWindowNew(theBuildData->Agent, aName);
		aWnd->SetTheme(theBuildData->Theme);

		*theOutMenu = aWnd;
	}
	else
		*theOutMenu = 0;

	return aRes;
}

//////////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP ScopePlugin::CanBuild(Item * theItem, UINT * theOut)
{
	if (theItem == 0)
		return E_INVALIDARG;

	if (theOut == 0)
		return E_POINTER;

	*theOut = fnItemMatch(theItem, (LPCTSTR *)mySupportedExtensions, myExtensionListSize);
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP ScopePlugin::GetArrowBitmap(UINT theExpCode, bool theSelected, const Theme * theTheme, BitmapInfo * theOutInfo)
{
	theOutInfo->myWidth = 11;
	theOutInfo->myHeight = 11;
	theOutInfo->myTransparentColor = RGB(0, 255, 0);
	theOutInfo->myBmpHandle = myArrowNormal;

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP ScopePlugin::OnLoad(int theFrakeworkVersionCode)
{
	fnItemMatch = UtilFileExtensionItemMatch::InitItemMatchImplementation();

	myArrowNormal = LoadBitmap(myInstance, MAKEINTRESOURCE(IDB_LENS) );

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP ScopePlugin::OnUnload()
{
	DeleteObject(myArrowNormal);

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP ScopePlugin::CreateInstance(IUnknown * theOuter, REFIID theIID, void ** theOut)
{
	if (theIID != IID_IMenuBuilder)
		return E_NOINTERFACE;

	IMenuBuilder * aMB = this;
	aMB->AddRef();
	*theOut = aMB;

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP ScopePlugin::LockServer( BOOL theLock )
{
	return E_NOTIMPL;
}

//////////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP ScopePlugin::GetTypeGuid(LPGUID theOutGuid)
{
	if (theOutGuid == 0)
		return E_POINTER;

	* theOutGuid = GUID_Scope;
	return S_OK;
}
#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////

#include "sdk_MenuBuilder.h"

#include "PluginBase.h"
#include "UtilFileMatch.h"

//////////////////////////////////////////////////////////////////////////////////////////////

class Item;

class ScopePlugin :
	public ComEntry3<IApproachPlugin, IMenuBuilder, IClassFactory>
{
private:
	HBITMAP myArrowNormal;
	HINSTANCE myInstance;

	int myExtensionListSize;
	TCHAR * mySupportedExtensions[20];
	UtilFileExtensionItemMatch::ITEMMATCHFN fnItemMatch;


private:
	typedef UINT (*CanBuildImpl) (ILogicQueryTarget *);


// Constructors, destructor
public:
	ScopePlugin(HINSTANCE theDllInstance);


// IPlugin Members
protected:
	STDMETHODIMP OnLoad(int theFrameworkVersion);

	STDMETHODIMP OnUnload();


// IMenuBuilder Members
protected:
	STDMETHODIMP Build(Item * theItem, const MenuBuilderData * theBuildData, MenuWindow ** theOutMenu);

	STDMETHODIMP CanBuild(Item * theItem, UINT * theOut);

	STDMETHODIMP GetArrowBitmap(UINT theExpCode, bool theSelected, const Theme * theTheme, BitmapInfo * theOutInfo);


// IClassFactory Members
protected:
	STDMETHODIMP CreateInstance(IUnknown * theOuter, REFIID theIID, void ** theOut);

	STDMETHODIMP LockServer(BOOL theLock);


// IRuntimeObject Members
protected:
	STDMETHODIMP GetTypeGuid(LPGUID theOutGuid);
};

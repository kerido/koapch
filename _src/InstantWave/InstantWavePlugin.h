#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////

#include "sdk_MenuBuilder.h"

#include "PluginBase.h"
#include "UtilFileMatch.h"

//////////////////////////////////////////////////////////////////////////////////////////////

class Item;

class DECLSPEC_NOVTABLE InstantWavePlugin :
	public ComEntry3<IApproachPlugin, IMenuBuilder, IClassFactory>
{
private:
	HBITMAP myWaveform;
	HINSTANCE myInstance;

	UtilFileExtensionItemMatch::ITEMMATCHFN fnItemMatch;

	TCHAR myExtensions[10];
	TCHAR * mySupportedExtensions[1];


public:
	InstantWavePlugin(HINSTANCE theDllInstance) : myInstance(theDllInstance) {}


public:
	HINSTANCE GetInstance() const              { return myInstance; }


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

#include "stdafx.h"

#include "sdk_BinaryStream.h"

#include "Preferences.h"
#include "IpcGuids.h"
#include "BindableEnum.h"
#include "Application.h"
#include "MenuSetSerialization.h"
#include "Utility.h"

//////////////////////////////////////////////////////////////////////////////////////////////

LONG KoApproachItemsRoots::Load( HKEY theKey, const TCHAR * theValue )
{
	CString aStr;
	DWORD aSize;

	RegQueryValueEx(theKey, theValue, 0, NULL, NULL, &aSize);

	LONG aRes = RegQueryValueEx(theKey, theValue, 0, NULL, (LPBYTE) aStr.GetBuffer( aSize/sizeof(TCHAR) + 1 ), &aSize);
	aStr.ReleaseBuffer();

	if (aRes != 0) return aRes;


	int aStart = 0, i = 0;
	for (; i < aStr.GetLength();  i++)
	{
		if ( aStr[i] == _T('*') )
		{
			myDirectories.Add( aStr.Mid(aStart, i-aStart) );
			aStart = i+1;
			continue;
		}
	}

	if (i - aStart >= 1)
		myDirectories.Add( aStr.Mid(aStart,i-aStart) );

	return aRes;
}

//////////////////////////////////////////////////////////////////////////////////////////////

void KoApproachItemsRoots::Save( HKEY theKey, const TCHAR * theValue ) const
{
	CString aStr;

	for (int i = 0; i < myDirectories.GetSize(); i++)
	{
		aStr = aStr + myDirectories[i];
		aStr = aStr + _T('*');
	}

	RegSetValueEx(theKey, theValue, NULL, REG_SZ, (const BYTE *)(const TCHAR *)aStr,
		(aStr.GetLength() + 1) * sizeof(TCHAR) );
}

//////////////////////////////////////////////////////////////////////////////////////////////

bool KoApproachItemsRoots::AddRoot( const TCHAR * theRoot )
{
	bool aContains = false;
	for (int i = 0; i < myDirectories.GetSize(); i++)
		if ( myDirectories[i] == theRoot )    { aContains = true; break; }


		if (!aContains)
			myDirectories.Add(theRoot);

		return !aContains;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

ApplicationSettings::ApplicationSettings() :
	myAutoEnableFolderMenus(0),
	myAutoEnableApproachItems(0),
	myAutoEnableTitleBar(0),
	myUseReleaseTrace(0),
	myAlreadyLaunched(0),
	myTitlebarFlags(TBM_ORDER_PARENT_TO_CHILD)
{
	PrefsSerz::Zero();

	myLocale[0] = 0;

	myUpdateUrl[0] = myActivateUrl[0] = myOrderUrl[0] = myHomeUrl[0] = myFeedbackUrl[0] = myDisabledPlugins[0] = 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////

ApplicationSettings::ApplicationSettings(const ApplicationSettings & theVal)
{
	Set(theVal);
}

//////////////////////////////////////////////////////////////////////////////////////////////

ApplicationSettings::~ApplicationSettings()
{ }

//////////////////////////////////////////////////////////////////////////////////////////////

void ApplicationSettings::Set(const ApplicationSettings & theVal)
{
	PrefsSerz::Set(theVal);

	myAutoEnableFolderMenus   = theVal.myAutoEnableFolderMenus;
	myAutoEnableApproachItems = theVal.myAutoEnableApproachItems;
	myAutoEnableTitleBar      = theVal.myAutoEnableTitleBar;
	myUseReleaseTrace         = theVal.myUseReleaseTrace;
	myAlreadyLaunched         = theVal.myAlreadyLaunched;
	myTitlebarFlags           = theVal.myTitlebarFlags;

	lstrcpyn(myLocale,          theVal.myLocale, 6);

	lstrcpy(myHomeUrl,          theVal.myHomeUrl);
	lstrcpy(myOrderUrl,         theVal.myOrderUrl);
	lstrcpy(myFeedbackUrl,      theVal.myFeedbackUrl);
	lstrcpy(myUpdateUrl,        theVal.myUpdateUrl);
	lstrcpy(myActivateUrl,      theVal.myActivateUrl);
	lstrcpy(myDisabledPlugins,  theVal.myDisabledPlugins);

	myMainMenuSet = theVal.myMainMenuSet;
}

//////////////////////////////////////////////////////////////////////////////////////////////

void ApplicationSettings::SetDefault()
{
	PrefsSerz::SetDefault();

	myAutoEnableFolderMenus     = 1;
	myAutoEnableApproachItems   = 0;
	myAutoEnableTitleBar        = 0;
	myUseReleaseTrace           = 0;
	myAlreadyLaunched           = 0;
	myTitlebarFlags             = TBM_ORDER_PARENT_TO_CHILD;

	lstrcpyn(myLocale, _T("en_US"), 6);

	lstrcpy(myHomeUrl,     _T("http://www.koapproach.com") );
	lstrcpy(myOrderUrl,    _T("http://www.koapproach.com/order.aspx") );
	lstrcpy(myFeedbackUrl, _T("mailto:feedback@koapproach.com?subject=KO Approach feedback") );
	lstrcpy(myUpdateUrl,   _T("http://www.koapproach.com/update_checker.aspx") );
	lstrcpy(myActivateUrl, _T("http://www.koapproach.com/desktop_interact.aspx") );

	myDisabledPlugins[0] = 0;

	InitializeApproachItemsSet();
}

//////////////////////////////////////////////////////////////////////////////////////////////

bool ApplicationSettings::Load()
{
	bool aRetVal = true;

	//-----------------------------------------------------
	// 1. Per-user data
	HKEY aApproachKey = EnsureRegistryKey(GetUserHiveName(), KEY_READ);

	if (aApproachKey != 0)
	{
		LONG aRes = ERROR_SUCCESS;

		DWORD aSize = sizeof(DataType);
		aRes |= RegQueryValueEx( aApproachKey, _T("ScrollingTimeout"),   0, NULL, (LPBYTE) &myTimeoutScroll,             &aSize);

		aSize = sizeof(DataType);
		aRes |= RegQueryValueEx( aApproachKey, _T("ChildWindowTimeout"), 0, NULL, (LPBYTE) &myTimeoutSec,                &aSize);

		aSize = sizeof(DataType);
		aRes |= RegQueryValueEx( aApproachKey, _T("InitTimeout"),        0, NULL, (LPBYTE) &myTimeoutInit,               &aSize);

		aSize = sizeof(DataType);
		aRes |= RegQueryValueEx( aApproachKey, _T("ScrollItemsPage"),    0, NULL, (LPBYTE) &myScrollPage,                &aSize);

		aSize = sizeof(DataType);
		aRes |= RegQueryValueEx( aApproachKey, _T("ScrollItemsWheel"),   0, NULL, (LPBYTE) &myScrollWheel,               &aSize);

		aSize = sizeof(DataType);
		aRes |= RegQueryValueEx( aApproachKey, _T("HiddenFilesMode"),    0, NULL, (LPBYTE) &myHiddenFilesMode,           &aSize);

		aSize = sizeof(DataType);
		aRes |= RegQueryValueEx( aApproachKey, _T("BrowseForFolders"),   0, NULL, (LPBYTE) &myBrowseInNewWindow,         &aSize);

		aSize = sizeof(DataType);
		aRes |= RegQueryValueEx( aApproachKey, _T("ActivateMode"),       0, NULL, (LPBYTE) &myActivateMode,              &aSize);

		aSize = sizeof(DataType);
		aRes |= RegQueryValueEx( aApproachKey, _T("ZipsAreFolders"),     0, NULL, (LPBYTE) &myZipsAreFolders,            &aSize);

		aSize = sizeof(DataType);
		aRes |= RegQueryValueEx( aApproachKey, _T("MaxMenuWidth"),       0, NULL, (LPBYTE) &myMaxMenuWidth,              &aSize);

		aSize = sizeof(DataType);
		aRes |= RegQueryValueEx( aApproachKey, _T("Ordering"),           0, NULL, (LPBYTE) &myOrdering,                  &aSize);

		aSize = sizeof(DataType);
		aRes |= RegQueryValueEx( aApproachKey, _T("AutoEnableTray"),     0, NULL, (LPBYTE) &myAutoEnableApproachItems,   &aSize);

		aSize = sizeof(DataType);
		aRes |= RegQueryValueEx( aApproachKey, _T("AutoEnableFolder"),   0, NULL, (LPBYTE) &myAutoEnableFolderMenus,     &aSize);

		aSize = sizeof(DataType);
		aRes |= RegQueryValueEx( aApproachKey, _T("AutoEnableTitlebar"), 0, NULL, (LPBYTE) &myAutoEnableTitleBar,        &aSize);

		aSize = sizeof(DataType);
		aRes |= RegQueryValueEx( aApproachKey, _T("TitlebarMenusFlags"), 0, NULL, (LPBYTE) &myTitlebarFlags,             &aSize);

		aSize = 6 * sizeof TCHAR; //reserve enough space
		aRes |= RegQueryValueEx( aApproachKey, _T("UiLocale"),           0, NULL, (LPBYTE)  myLocale,                    &aSize);


		aRes |= RegQueryApproachItems ( aApproachKey );

		// Optional parameters

		aSize = sizeof(DataType);
		        RegQueryValueEx( aApproachKey, _T("OutputTraceLog"),     0, NULL, (LPBYTE) &myUseReleaseTrace,           &aSize);

		aSize = sizeof(DataType);
		        RegQueryValueEx( aApproachKey, _T("AlreadyLaunched"),    0, NULL, (LPBYTE) &myAlreadyLaunched,           &aSize);

		aSize = sizeof(DataType);
		        RegQueryValueEx( aApproachKey, _T("IconHandlingFlags"),  0, NULL, (LPBYTE) &myIconHandlingFlags,         &aSize);

		aSize = sizeof(DataType);
		        RegQueryValueEx( aApproachKey, _T("ScrollingFlags"),     0, NULL, (LPBYTE) &myScrollItemFlags,           &aSize);

		aSize = sizeof(myDisabledPlugins);
		        RegQueryValueEx( aApproachKey, _T("DisabledPlugins"),    0, NULL, (LPBYTE)  myDisabledPlugins,           &aSize);

		aSize = sizeof(mySkipAutoUpdates);
		        RegQueryValueEx( aApproachKey, _T("SkipAutoUpdates"),    0, NULL, (LPBYTE) &mySkipAutoUpdates,           &aSize);

		RegCloseKey(aApproachKey);

		aRetVal &= (aRes == ERROR_SUCCESS);
	}


	//-----------------------------------------------------
	// 2. Global data
	aApproachKey = EnsureRegistryKey(GetGlobalHiveName(), KEY_READ);

	if (aApproachKey != 0)
	{
		// 2.1. Mandatory data
		LONG aRes = ERROR_SUCCESS;

		DWORD aSize = sizeof(myHomeUrl);
		aRes |= RegQueryValueEx(aApproachKey, _T("HomeUrl"),             0, NULL, (LPBYTE)myHomeUrl,             &aSize);

		aSize = sizeof(myUpdateUrl);
		aRes |= RegQueryValueEx(aApproachKey, _T("UpdateUrl"),           0, NULL, (LPBYTE)myUpdateUrl,           &aSize);

		aSize = sizeof(myActivateUrl);
		aRes |= RegQueryValueEx(aApproachKey, _T("ActivateUrl"),         0, NULL, (LPBYTE)myActivateUrl,         &aSize);

		aSize = sizeof(myOrderUrl);
		aRes |= RegQueryValueEx(aApproachKey, _T("OrderUrl"),            0, NULL, (LPBYTE)myOrderUrl,            &aSize);

		aSize = sizeof(myFeedbackUrl);
		aRes |= RegQueryValueEx(aApproachKey, _T("FeedbackUrl"),         0, NULL, (LPBYTE)myFeedbackUrl,         &aSize);


		// 2.2. Optional data
		myRoots.Load( aApproachKey, _T("KoApproachItemsRoots") );


		RegCloseKey(aApproachKey);

		aRetVal &= (aRes == ERROR_SUCCESS);
	}


	//-----------------------------------------------------
	// 3. Return
	return aRetVal;
}

//////////////////////////////////////////////////////////////////////////////////////////////

bool ApplicationSettings::Save() const
{
	bool aSuccess = false;


	//-----------------------------------------------------
	// 1. Per-user data
	HKEY aApproachKey = EnsureRegistryKey(GetUserHiveName(), KEY_WRITE);

	if (aApproachKey != 0)
	{
		DWORD aSize = sizeof(DataType);

		LONG aRes = ERROR_SUCCESS;

		aRes |= RegSetValueEx( aApproachKey, _T("ScrollingTimeout"),     0, REG_DWORD, (const LPBYTE) &myTimeoutScroll,             aSize);
		aRes |= RegSetValueEx( aApproachKey, _T("ChildWindowTimeout"),   0, REG_DWORD, (const LPBYTE) &myTimeoutSec,                aSize);
		aRes |= RegSetValueEx( aApproachKey, _T("InitTimeout"),          0, REG_DWORD, (const LPBYTE) &myTimeoutInit,               aSize);
		aRes |= RegSetValueEx( aApproachKey, _T("ScrollItemsPage"),      0, REG_DWORD, (const LPBYTE) &myScrollPage,                aSize);
		aRes |= RegSetValueEx( aApproachKey, _T("ScrollItemsWheel"),     0, REG_DWORD, (const LPBYTE) &myScrollWheel,               aSize);
		aRes |= RegSetValueEx( aApproachKey, _T("HiddenFilesMode"),      0, REG_DWORD, (const LPBYTE) &myHiddenFilesMode,           aSize);
		aRes |= RegSetValueEx( aApproachKey, _T("BrowseForFolders"),     0, REG_DWORD, (const LPBYTE) &myBrowseInNewWindow,         aSize);
		aRes |= RegSetValueEx( aApproachKey, _T("ActivateMode"),         0, REG_DWORD, (const LPBYTE) &myActivateMode,              aSize);
		aRes |= RegSetValueEx( aApproachKey, _T("ZipsAreFolders"),       0, REG_DWORD, (const LPBYTE) &myZipsAreFolders,            aSize);
		aRes |= RegSetValueEx( aApproachKey, _T("MaxMenuWidth"),         0, REG_DWORD, (const LPBYTE) &myMaxMenuWidth,              aSize);
		aRes |= RegSetValueEx( aApproachKey, _T("Ordering"),             0, REG_DWORD, (const LPBYTE) &myOrdering,                  aSize);
		aRes |= RegSetValueEx( aApproachKey, _T("AutoEnableTray"),       0, REG_DWORD, (const LPBYTE) &myAutoEnableApproachItems,   aSize);
		aRes |= RegSetValueEx( aApproachKey, _T("AutoEnableFolder"),     0, REG_DWORD, (const LPBYTE) &myAutoEnableFolderMenus,     aSize);
		aRes |= RegSetValueEx( aApproachKey, _T("AutoEnableTitlebar"),   0, REG_DWORD, (const LPBYTE) &myAutoEnableTitleBar,        aSize);
		aRes |= RegSetValueEx( aApproachKey, _T("TitlebarMenusFlags"),   0, REG_DWORD, (const LPBYTE) &myTitlebarFlags,             aSize);
		aRes |= RegSetValueEx( aApproachKey, _T("UiLocale"),             0, REG_SZ,    (const LPBYTE)  myLocale,                    6 * sizeof TCHAR);
		aRes |= RegSaveApproachItems( aApproachKey );


		//TODO: Save the trace flag
		        RegSetValueEx( aApproachKey, _T("AlreadyLaunched"),      0, REG_DWORD, (const LPBYTE) &myAlreadyLaunched,           aSize);
		        RegSetValueEx( aApproachKey, _T("IconHandlingFlags"),    0, REG_DWORD, (const LPBYTE) &myIconHandlingFlags,         aSize);
		        RegSetValueEx( aApproachKey, _T("ScrollingFlags"),       0, REG_DWORD, (const LPBYTE) &myScrollItemFlags,           aSize);
		        RegSetValueEx( aApproachKey, _T("SkipAutoUpdates"),      0, REG_DWORD, (const LPBYTE) &mySkipAutoUpdates,           aSize);

		aSize = lstrlen(myDisabledPlugins) * sizeof(TCHAR);
		        RegSetValueEx( aApproachKey, _T("DisabledPlugins"),      0, REG_SZ,    (const LPBYTE)  myDisabledPlugins,           aSize);

		RegCloseKey(aApproachKey);

		aSuccess = (aRes == ERROR_SUCCESS);
	}


	//-----------------------------------------------------
	// 2. Global data (does not affect the return value)
	aApproachKey = EnsureRegistryKey(GetGlobalHiveName(), KEY_WRITE);

	if (aApproachKey != 0)
	{
		// 2.1. Mandatory data
		LONG aRes = ERROR_SUCCESS;

		DWORD aSize = lstrlen(myHomeUrl) * sizeof(TCHAR);
		aRes |= RegSetValueEx(aApproachKey, _T("HomeUrl"),             0, REG_SZ,    (const LPBYTE) myHomeUrl,             aSize);

		aSize = lstrlen(myUpdateUrl) * sizeof(TCHAR);
		aRes |= RegSetValueEx(aApproachKey, _T("UpdateUrl"),           0, REG_SZ,    (const LPBYTE) myUpdateUrl,           aSize);

		aSize = lstrlen(myActivateUrl) * sizeof(TCHAR);
		aRes |= RegSetValueEx(aApproachKey, _T("ActivateUrl"),         0, REG_SZ,    (const LPBYTE) myActivateUrl,         aSize);

		aSize = lstrlen(myOrderUrl) * sizeof(TCHAR);
		aRes |= RegSetValueEx(aApproachKey, _T("OrderUrl"),            0, REG_SZ,    (const LPBYTE) myOrderUrl,            aSize);

		aSize = lstrlen(myFeedbackUrl) * sizeof(TCHAR);
		aRes |= RegSetValueEx(aApproachKey, _T("FeedbackUrl"),         0, REG_SZ,    (const LPBYTE) myFeedbackUrl,         aSize);


		// 2.2. Optional data
		myRoots.Save( aApproachKey, _T("KoApproachItemsRoots") );
		RegCloseKey(aApproachKey);
	}


	//-----------------------------------------------------
	// 3. Return
	return aSuccess;
}

//////////////////////////////////////////////////////////////////////////////////////////////

bool ApplicationSettings::AddKoApproachItemsRoot( const TCHAR * theRoot )
{
	return myRoots.AddRoot(theRoot);
}

//////////////////////////////////////////////////////////////////////////////////////////////

const TCHAR * ApplicationSettings::GetUrl( UrlType theType ) const
{
	switch(theType)
	{
	case URL_UPDATE:     return myUpdateUrl;
	case URL_FEEDBACK:   return myFeedbackUrl;
	case URL_ACTIVATION: return myActivateUrl;
	case URL_ORDER:      return myOrderUrl;
	case URL_HOME:
	default:             return myHomeUrl;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////

HKEY ApplicationSettings::EnsureRegistryKey(HKEY theParent, REGSAM theAccess)
{
	HKEY aApproachKey = 0;

	LONG aRes = RegOpenKeyEx
	(
		theParent,
		GetKeyName(),
		0,
		theAccess,
		&aApproachKey
	);

	if (aRes == ERROR_SUCCESS && aApproachKey != 0)
		return aApproachKey;

	else if ( (theAccess & KEY_WRITE) != 0 )
	{
		aRes = RegCreateKeyEx
		(
			theParent,
			GetKeyName(),
			0,
			NULL,
			REG_OPTION_NON_VOLATILE,
			KEY_ALL_ACCESS,
			NULL,
			&aApproachKey,
			NULL
		);
	}

	return aApproachKey;
}

//////////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP ApplicationSettings::RaiseUpdateChanges(ULONG theChangeFlags)
{
	Save();

	Application::Instance().ReportPrefEvent(IPreferencesEventProcessor::PREFEVENT_CHANGE);
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////////////////////////

ULONG STDMETHODCALLTYPE ApplicationSettings::AddRef()
{
	return myNumRefs.AddRef();
}

//////////////////////////////////////////////////////////////////////////////////////////////

ULONG STDMETHODCALLTYPE ApplicationSettings::Release()
{
	return myNumRefs.Release(this);
}

//////////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP ApplicationSettings::QueryInterface( REFIID theIID, void ** theOut )
{
	if (theIID == IID_IUnknown)
		*theOut = static_cast<IUnknown *>(this);

	else if (theIID == IID_IChangeableEntity)
		*theOut = static_cast<IChangeableEntity *>(this);

	else
		return E_NOINTERFACE;

	AddRef();
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////////////////////////

bool ApplicationSettings::IsPluginDisabled( const TCHAR * theName ) const
{
	const TCHAR * aRet = StrStrI(myDisabledPlugins, theName);

	return aRet != NULL;
}

//////////////////////////////////////////////////////////////////////////////////////////////

bool ApplicationSettings::EnablePlugin(const TCHAR * theName)
{
	TCHAR * aNamePos = StrStrI(myDisabledPlugins, theName);

	if (aNamePos == 0)
		return false; // if the string is not found, then the plugin is already enabled

	const TCHAR * aAsterisk = StrStrI(aNamePos, _T("*") );

	bool aLastPlugin = false;

	if (aAsterisk == 0)
		aLastPlugin = true;
	else
	{
		aAsterisk++;
		aLastPlugin = *aAsterisk == 0;
	}

	if (aLastPlugin)
		*aNamePos = 0;
	else
	{
		TCHAR aTempString[1024];
		aTempString[0] = 0;

		TCHAR * aTempPtr = aTempString;

		int aSubstringLength = (int) (aNamePos - myDisabledPlugins);

		if (aSubstringLength != 0)
		{
			lstrcpyn(aTempPtr, myDisabledPlugins, aSubstringLength + 1);
			aTempPtr += aSubstringLength;
		}

		lstrcpy(aTempPtr, aAsterisk);
		lstrcpy(myDisabledPlugins, aTempString);
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////

bool ApplicationSettings::DisablePlugin( const TCHAR * theName )
{
	TCHAR * aNamePos = StrStrI(myDisabledPlugins, theName);

	if (aNamePos != 0)
		return false; // if the string is found, then the plugin is already disabled

	lstrcat(myDisabledPlugins, theName);
	lstrcat(myDisabledPlugins, _T("*") );

	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////

//TEMP
void ApplicationSettings::InitializeApproachItemsSet()
{
	myMainMenuSet.Clear();

	//////////////////////////////////////////////////////////////
	// 1. Approach Items from previous version
	MenuSetEntry aEntry1(MenuSetEntry::LIST, GUID_Hotspot_ApproachItems);
	myMainMenuSet.AddEntry(aEntry1);


	//////////////////////////////////////////////////////////////
	// 2. Separator line
	MenuSetEntry aEntry2(MenuSetEntry::SEPARATOR);
	myMainMenuSet.AddEntry(aEntry2);


	//////////////////////////////////////////////////////////////
	// 3. Run
	MenuSetEntry aEntry3(MenuSetEntry::ITEM, OBJ_RunItem);
	myMainMenuSet.AddEntry(aEntry3);
}

//////////////////////////////////////////////////////////////////////////////////////////////

LONG ApplicationSettings::RegQueryApproachItems( HKEY theKey )
{
	// 1. Define the size required for storing the buffer

	DWORD aSize = 0;
	LONG aRet = RegQueryValueEx( theKey, _T("ApproachItemsData"),   0, NULL, NULL, &aSize);

	if (aRet != ERROR_MORE_DATA && aRet != ERROR_SUCCESS)
		return aRet;	//failure

	ATLASSERT(aSize > 0);

	MemoryStream<char> aStr;

	DWORD aSizeNew = aSize;
	aRet = RegQueryValueEx( theKey, _T("ApproachItemsData"), 0, NULL, (LPBYTE)aStr.EnsureWritableBuffer(aSize), &aSizeNew);

	ATLASSERT(aSizeNew == aSize);

	MenuSetSerializer::Load(myMainMenuSet, aStr);

	return ERROR_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////////////////////

LONG ApplicationSettings::RegSaveApproachItems( HKEY theKey ) const
{
	MemoryStream<char> aOut;
	MenuSetSerializer::Save(myMainMenuSet, aOut);

	DWORD aSize = (DWORD)aOut.GetSize();

	if (aSize == 0)
		return ERROR_BAD_LENGTH;

	return RegSetValueEx( theKey, _T("ApproachItemsData"), 0, REG_BINARY, (LPCBYTE)aOut.GetBuffer(), aSize);
}
//end TEMP
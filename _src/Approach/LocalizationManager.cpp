#include "stdafx.h"

#include "LocalizationManager.h"
#include "LocalizationImpl.h"
#include "Application.h"
#include "RootWindow.h"

//////////////////////////////////////////////////////////////////////////////////////////////

#ifdef _USE_LOCALIZATION_MONITORING

class LocalizationDirectoryMonitorThread : public Thread
{
private:
	ApproachLocalizationManager * myMan;
	HANDLE myMonitorHandle;
	HANDLE myTerminateEvent;

public:
	LocalizationDirectoryMonitorThread(ApproachLocalizationManager * theMan) : myMan(theMan)
	{
		myTerminateEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
		Thread::Run(this);
	}

	virtual ~LocalizationDirectoryMonitorThread()
	{
		Terminate();

		DWORD aWaitResult = WaitForSingleObject(myHandle, 5000);	//wait 5 seconds

		if (aWaitResult != WAIT_OBJECT_0)
			TerminateThread(myHandle, 1);

		CloseHandle(myTerminateEvent);
	}

protected:
	virtual DWORD Run()
	{
		SetThreadPriority(myHandle, THREAD_PRIORITY_IDLE);

		TCHAR aLocaleDir[1000];
		myMan->GetLocalizationBasePath(aLocaleDir, 1000);

		myMonitorHandle = FindFirstChangeNotification(
			aLocaleDir,
			FALSE,
			FILE_NOTIFY_CHANGE_LAST_WRITE);

		if ( myMonitorHandle == INVALID_HANDLE_VALUE)
			return 1;

		HANDLE aObjs[2] = { myMonitorHandle, myTerminateEvent };

		while (true)
		{
			DWORD aRes = WaitForMultipleObjects(2, aObjs, FALSE, INFINITE);

			if ( aRes == WAIT_FAILED)
				break;

			else if (aRes == WAIT_OBJECT_0 + 0)		//directory change
			{
				//HACK
				HWND aWnd = RootWindow::FindSingleInstance();

				if (aWnd == 0)
					break;
				//end HACK

				PostMessage(aWnd, RootWindow::WM_USER_LOADLOCALE, 0, 0);

				BOOL aRet = FindNextChangeNotification(myMonitorHandle);

				if (aRet == 0)
				{
					ATLTRACE("LocalizationDirectoryMonitorThread -- failed on FindNextChangeNotification, returning.\n");
					break;
				}
			}

			else if (aRes == WAIT_OBJECT_0 + 1)	//the terminate event occurred
				break;
		}

		FindCloseChangeNotification(myMonitorHandle);
		return 0;
	}

	virtual void Terminate()
	{
		SetEvent(myTerminateEvent);
	}
};
#endif	//_USE_LOCALIZATION_MONITORING

//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

ApproachLocalizationManager::ApproachLocalizationManager()
{
	Application::Instance().AddPreferencesEventProcessor(this);


#ifdef _USE_LOCALIZATION_MONITORING
	myMonitor = new LocalizationDirectoryMonitorThread(this);
#endif	//_USE_LOCALIZATION_MONITORING
}

//////////////////////////////////////////////////////////////////////////////////////////////

ApproachLocalizationManager::~ApproachLocalizationManager()
{
	for (LocalizationIter aIt = myLocalizations.begin(); aIt != myLocalizations.end(); aIt++)
		delete aIt->second.Localization;

#ifdef _USE_LOCALIZATION_MONITORING
	delete myMonitor;
#endif	//_USE_LOCALIZATION_MONITORING


	Application::Instance().RemovePreferencesEventProcessor(this);
}

//////////////////////////////////////////////////////////////////////////////////////////////

HRESULT ApproachLocalizationManager::SetCurrentLocale(const TCHAR * theLocale, int theSize)
{
	IdType aId(theLocale, theLocale+theSize);

	LocalizationIter aIter = myLocalizations.find(aId);

	HRESULT aRes = S_OK;

	if ( aIter == myLocalizations.end() )
	{
		bool aPartial = false;

		ILocalization * aLoc = CreateLocalization(theLocale, theSize);

		if (aLoc == 0)
			return E_FAIL;

		aRes = aLoc->Initialize(theLocale, theSize, aPartial);

		if ( FAILED(aRes) )
			return aRes;

		aIter = myLocalizations.insert
		(
			aIter,
			LocalizationDataPair(aId, LocalizationData(aLoc, aPartial) )
		);
	}
	else
	{
		bool aPartial = false;

		aRes = aIter->second.Localization->Initialize(theLocale, theSize, aPartial);

		ATLASSERT( SUCCEEDED(aRes) && !aPartial );

		aIter->second.Partial = aPartial;
	}

	myCurLocalization = aId;

	Application::Instance().RaiseLocalizationChanged(aIter->second.Localization, myCurLocalization.c_str(), (int) myCurLocalization.size() );

	return aRes;
}

//////////////////////////////////////////////////////////////////////////////////////////////

HRESULT ApproachLocalizationManager::LocalizationFilePathFromLocaleID(
	TCHAR * theOutPath, int theSize_OutPath,
	const TCHAR * theLocale, int theSize_Locale)
{
	const int NameLength = 20;	//lstrlen( \Approach[en_US].xml )

	HRESULT aRes = GetLocalizationBasePath(theOutPath, theSize_OutPath - NameLength);

	if ( FAILED(aRes) )
		return aRes;

	TCHAR aLocale[6];
	lstrcpyn(aLocale, theLocale, theSize_Locale+1);

	lstrcat( theOutPath, _T( "\\Approach[" ) );
	lstrcat( theOutPath, aLocale );
	lstrcat( theOutPath, _T( "].xml" ) );

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////////////////////////

HRESULT ApproachLocalizationManager::GetLocalizationBasePath(TCHAR * theOutPath, int theOutPathLength)
{
	const FilePathProvider * aFPP = Application::InstanceC().GetFilePathProvider();

	int aSize = theOutPathLength;
	return aFPP->GetPath(FilePathProvider::Dir_Locale, theOutPath, &aSize);
}

//////////////////////////////////////////////////////////////////////////////////////////////

HRESULT ApproachLocalizationManager::GetStringSafe(LocIdKey theInID, TCHAR * theOutString, int * theSize)
{
	ILocalization * aLoc = GetCurrentLocalization();
	HRESULT aRes = E_FAIL;

	if (aLoc != 0)
		aRes = aLoc->GetString(theInID, theOutString, theSize);

	if ( SUCCEEDED(aRes) )
		return aRes;

	aLoc = GetDefaultLocalization();

	if (aLoc != 0 )
		aRes = aLoc->GetString(theInID, theOutString, theSize);

	return SUCCEEDED(aRes) ? S_FALSE : aRes;
}

//////////////////////////////////////////////////////////////////////////////////////////////

HRESULT ApproachLocalizationManager::GetMetricSafe(LocIdKey theInID, Metric & theDestMetric)
{
	ILocalization * aLoc = GetCurrentLocalization();
	HRESULT aRes = E_FAIL;

	if (aLoc != 0)
		aRes = aLoc->GetMetric(theInID, theDestMetric);

	if ( SUCCEEDED(aRes) )
		return aRes;

	aLoc = GetDefaultLocalization();

	if (aLoc != 0 )
		aRes = aLoc->GetMetric(theInID, theDestMetric);

	return SUCCEEDED(aRes) ? S_FALSE : aRes;
}

//////////////////////////////////////////////////////////////////////////////////////////////

HRESULT ApproachLocalizationManager::EnumerateLocalizations(IEnumLocalizationsProcessor * theOut)
{
	TCHAR * aLocale = _T("en_US");
	ILocalization * aLoc = CreateLocalization(aLocale, 5);

	HRESULT aRes = OnLocalizationFoundNew(aLoc, aLocale, 5, theOut);

	if ( FAILED(aRes) )
		return aRes;

	TCHAR aPattern[1000];

	aRes = GetLocalizationBasePath(aPattern, 1000-18);	//18 is the length of \Approach[*_*].xml

	if ( FAILED(aRes) )
		return aRes;

	lstrcat(aPattern, _T("\\Approach[*_*].xml") );

	WIN32_FIND_DATA aDt;

	HANDLE aFindHandle = FindFirstFile(aPattern, &aDt);

	if (aFindHandle == INVALID_HANDLE_VALUE)
	{
		CloseHandle(aFindHandle);
		return S_OK;
	}

	do 
	{
		aLocale = aDt.cFileName + 9;
		aLoc = CreateLocalization(aLocale, 5);
		OnLocalizationFoundNew(aLoc, aLocale, 5, theOut);
	}
	while ( FindNextFile(aFindHandle, &aDt) != 0);

	FindClose(aFindHandle);

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////////////////////////

HRESULT ApproachLocalizationManager::OnLocalizationFoundNew(
		ILocalization * theLoc, const TCHAR * theLocale, int theSize_Locale,
		IEnumLocalizationsProcessor * theOut)
{
	bool aPartial = true;
	HRESULT aRes = theLoc->Initialize(theLocale, theSize_Locale, aPartial);

	if ( FAILED(aRes) )
		delete theLoc;
	else
	{
		IdType aId(theLocale, theLocale+theSize_Locale);

		LocalizationData & aData = myLocalizations[aId];
		aData.Localization = theLoc;
		aData.Partial      = aPartial;

		if (theOut)
			theOut->OnLocalizationFound(theLoc);
	}

	return aRes;
}

//////////////////////////////////////////////////////////////////////////////////////////////

ILocalization * ApproachLocalizationManager::CreateLocalization(const TCHAR * theLocaleID, int theSize_LocaleID)
{
	IdType aId(theLocaleID, theLocaleID+theSize_LocaleID);

	LocalizationIter aIter = myLocalizations.find( aId );

	if ( aIter == myLocalizations.end() )
	{
		if ( aId != _T("en_US") )
			return new LocalizationImpl_Xml();
		else
			return new LocalizationImpl_EnUs_Hardcoded();
	}
	else
		return aIter->second.Localization;
}

//////////////////////////////////////////////////////////////////////////////////////////////

ILocalization * ApproachLocalizationManager::GetCurrentLocalization()
{
	LocalizationIter aIter = myLocalizations.find(myCurLocalization);

	if ( aIter == myLocalizations.end() )
		return 0;
	else
		return aIter->second.Localization;
}

//////////////////////////////////////////////////////////////////////////////////////////////

ILocalization * ApproachLocalizationManager::GetDefaultLocalization()
{
	IdType aId = _T("en_US");
	LocalizationIter aIter;

	if ( myCurLocalization == aId) //this is the current
	{
		aIter = myLocalizations.find(myCurLocalization);
		return aIter->second.Localization;
	}

	aIter = myLocalizations.find(aId);

	if ( aIter == myLocalizations.end() )	//could not find a localization with the desired locale id
	{
		ILocalization * aLoc = CreateLocalization( aId.c_str(), (int) aId.size() );
		if (aLoc == 0)
			return aLoc;

		bool aPartial = false;
		HRESULT aRes = aLoc->Initialize(aId.c_str(), (int) aId.size(), aPartial);

		if ( FAILED(aRes) )
		{
			delete aLoc;
			return 0;
		}

		myLocalizations[aId] = LocalizationData(aLoc, aPartial);

		return aLoc;
	}
	else
		return aIter->second.Localization;
}

//////////////////////////////////////////////////////////////////////////////////////////////

void ApproachLocalizationManager::OnPreferencesEvent(PreferencesEvent theEvent, ApplicationSettings & thePrefs)
{
	int aSize = 0;
	const TCHAR * aLocale = thePrefs.GetLocale(&aSize);

	if (myCurLocalization != aLocale)
	{
		HRESULT aRes = SetCurrentLocale(aLocale, aSize);

		if ( FAILED(aRes) )
		{
			aRes = SetCurrentLocale( _T("en_US"), 5);
			ATLASSERT( SUCCEEDED(aRes) );
		}
	}
}
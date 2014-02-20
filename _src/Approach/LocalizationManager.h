#pragma once

#include "sdk_Localization.h"
#include "sdk_Thread.h"

#include "LogicCommon.h"
#include "Trace.h"

//////////////////////////////////////////////////////////////////////////////////////////////

#ifdef _USE_LOCALIZATION_MONITORING
class LocalizationDirectoryMonitorThread;
#endif

//////////////////////////////////////////////////////////////////////////////////////////////

class ApproachLocalizationManager :
	public ILocalizationManager,
	public IPreferencesEventProcessor
{
protected:

	struct LocalizationData
	{
		ILocalization * Localization;
		bool            Partial;

		LocalizationData(ILocalization * theLoc = 0, bool thePartial = true)
			: Localization(theLoc), Partial(thePartial) {}
	};

	typedef std::wstring IdType;

	typedef std::map<IdType, LocalizationData>                 LocalizationMapNew;
	typedef LocalizationMapNew::value_type                     LocalizationDataPair;
	typedef LocalizationMapNew::iterator                       LocalizationIter;
	typedef LocalizationMapNew::const_iterator                 LocalizationIterCNew;


public:
	ApproachLocalizationManager();
	~ApproachLocalizationManager();


protected:
	ILocalization * GetCurrentLocalization();

	ILocalization * GetDefaultLocalization();

	HRESULT GetStringSafe(LocIdKey theInID, TCHAR * theOutString, int * theSize);

	HRESULT GetMetricSafe(LocIdKey theInID, Metric & theDestMetric);

	HRESULT EnumerateLocalizations(IEnumLocalizationsProcessor * theOut);

	HRESULT SetCurrentLocale(const TCHAR * theLocale, int theSize);


// IPreferencesEventProcessor Members
protected:
	void OnPreferencesEvent(PreferencesEvent theEvent, ApplicationSettings & thePrefs);


protected:
	HRESULT OnLocalizationFoundNew(ILocalization * theLoc, const TCHAR * theLocale,
		int theSize_Locale, IEnumLocalizationsProcessor * theOut);

	ILocalization * CreateLocalization(const TCHAR * theLocaleID, int theSize_LocaleID);

public:
	static HRESULT GetLocalizationBasePath(TCHAR * theOutPath, int theOutPathLength);
	static HRESULT LocalizationFilePathFromLocaleID(TCHAR * theOutPath, int theSize_OutPath,
	                                                const TCHAR * theLocale, int theSize_Locale);


private:
	LocalizationMapNew myLocalizations;
	IdType myCurLocalization;


#ifdef _USE_LOCALIZATION_MONITORING

	LocalizationDirectoryMonitorThread * myMonitor;

public:
	void ProcessChangeEvent()
	{
		if (myCurLocalization != _T("en_US") )
			SetCurrentLocale(myCurLocalization.c_str(), (int) myCurLocalization.size() );
	}

#endif	//_USE_LOCALIZATION_MONITORING
};


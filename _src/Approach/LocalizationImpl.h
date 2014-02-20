#pragma once

#include "sdk_Localization.h"

//////////////////////////////////////////////////////////////////////////////////////////////

class LocalizationCommon : public ILocalization
{
//type definitions
protected:
	typedef std::string IdType;
	typedef std::wstring String;

	typedef std::map<IdType, String>   AuxMap;
	typedef AuxMap::iterator           AuxMapIter;
	typedef AuxMap::const_iterator     AuxMapIterC;

	typedef std::map<LocIdKey, String> StringMap;
	typedef StringMap::iterator        StringMapIter;
	typedef StringMap::const_iterator  StringMapIterC;

	typedef std::map<LocIdKey, Metric> MetricMap;
	typedef MetricMap::iterator        MetricMapIter;
	typedef MetricMap::const_iterator  MetricMapIterC;


//member variables
protected:
	AuxMap myTitles, myComments, myCreatedBys; //[locale, value]
	StringMap myStrings;
	MetricMap myMetrics;


//ILocalization members
protected:
	virtual HRESULT GetString(LocIdKey theInID, TCHAR * theOutString, int * theSize);

	virtual HRESULT GetMetric(LocIdKey theInID, Metric & theDestMetric);

	virtual HRESULT GetSummaryString(SummaryStringID theID, const TCHAR * theInLocale,
		int theSize_Locale, TCHAR * theOutString, int * theSize_Out);
};

//////////////////////////////////////////////////////////////////////////////////////////////

//! Represents a built-in US English localization.
class LocalizationImpl_EnUs_Hardcoded : public LocalizationCommon
{
	class StringAdvanced;

	class LocalizationSerializer;

	enum RenderType { STRING, METRIC, COMMENT };

protected:

	//! Retrieves the locale string for the current localization by copying it into the specified buffer.
	//! Always puts "en_US".
	//! \param [out] theOut      The output buffer that receives the localization string.
	//! \param [in,out] theSize  Initially contains the full size of the buffer.
	//!                          Upon execution, contains the number of characters copied
	//!                          (excluding the null-terminator).
	//! \return  S_OK if the localization string was successfully copied; otherwise, an COM error value.
	virtual HRESULT GetLocale(TCHAR * theOut, int * theSize);



	virtual HRESULT Initialize(const TCHAR * theCurLangID, int theSize_LangID, bool & thePartial);


// Implementation Details
private:

	__declspec(noinline) void DEFINE_METRIC(LocIdKey theType, Metric::DataType theVal)
		{ myMetrics[ theType ] = Metric(theVal); }

	__declspec(noinline) void DEFINE_METRIC(LocIdKey theType, Metric::DataType theVal1, Metric::DataType theVal2)
		{ myMetrics[ theType ] = Metric(theVal1, theVal2); }

	__declspec(noinline) void DEFINE_METRIC(LocIdKey theType, Metric::DataType theVal1, Metric::DataType theVal2,
			Metric::DataType theVal3, Metric::DataType theVal4)
		{ myMetrics[ theType ] = Metric(theVal1, theVal2, theVal3, theVal4); }

	void DEFINE_STRING(LocIdKey theType, const char * theString);

	void DEFINE_STRING(LocIdKey theType, const wchar_t * theString);

	const String & GetString(LocIdKey theID) { return myStrings[theID]; }

	const Metric & GetMetric(LocIdKey theID) { return myMetrics[theID]; }
};


//////////////////////////////////////////////////////////////////////////////////////////////

class LocalizationImpl_Xml : public LocalizationCommon
{
private:
	class Loader;


protected:
	String    myLocale;

//ILocalization members
public:
	HRESULT GetLocale(TCHAR * theOut, int * theSize);

	HRESULT Initialize(const TCHAR * theLocaleID, int theSize_LocaleID, bool & thePartial);

protected:
	void SetString(const TCHAR * theID, int theSize_ID, const TCHAR * theString, int theSize_String);
	void SetMetric(const TCHAR * theID, int theSize_ID, const Metric & theSource);
	void SetLocale(const TCHAR * theLocale, int theSize);
	void SetSummaryString(const TCHAR * theName, int theSize_Name,
		const TCHAR * theLocale, int theSize_Locale, const TCHAR * theVal, int theSize_Val);
};


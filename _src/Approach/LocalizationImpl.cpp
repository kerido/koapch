#include "stdafx.h"

#include <sstream>
#include <fstream>

#include "sdk_ComObject.h"

#include "Trace.h"
#include "LocalizationManager.h"
#include "LocalizationImpl.h"

#include "Utility.h"

#include "resource.h"
#include "auto_LocResource.h"

//////////////////////////////////////////////////////////////////////////////////////////////

HRESULT LocalizationCommon::GetString(LocIdKey theInID, TCHAR * theOutString, int * theSize )
{
	if ( theSize == 0 || theOutString == 0)
		return E_INVALIDARG;

	LocIdKey aKey = theInID;

	StringMapIterC aIt = myStrings.find(aKey);

	if ( aIt == myStrings.end() )
		return E_ACCESSDENIED;

	size_t aLength = aIt->second.size();

	if ( (size_t) *theSize < aLength+1 )
		return E_OUTOFMEMORY;

	lstrcpy( theOutString, aIt->second.c_str() );
	theOutString[aLength] = 0;

	*theSize = (int) aLength;

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////////////////////////

HRESULT LocalizationCommon::GetMetric(LocIdKey theInID, Metric & theDestMetric)
{
	LocIdKey aKey = theInID;

	MetricMapIterC aIt = myMetrics.find(aKey);

	if ( aIt == myMetrics.end() )
		return E_ACCESSDENIED;

	theDestMetric = aIt->second;

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////////////////////////

HRESULT LocalizationCommon::GetSummaryString( SummaryStringID theID, const TCHAR * theInLocale, int theSize_Locale, TCHAR * theOutString, int * theSize_Out )
{
	if ( theSize_Out == 0 || theOutString == 0)
		return E_INVALIDARG;

	AuxMap * aMap = 0;

	if ( theID == TITLE)
		aMap = &myTitles;

	else if (  theID == COMMENT)
		aMap = &myComments;

	else if (  theID == CREATED_BY)
		aMap = &myCreatedBys;

	else
		return E_INVALIDARG;

	IdType aKey;
	aKey.assign(theInLocale, theInLocale+theSize_Locale);

	AuxMapIterC aIt = aMap->find(aKey);

	if ( aIt == aMap->end() )
		return E_ACCESSDENIED;

	size_t aLength = aIt->second.size();

	if ( (size_t) *theSize_Out < aLength+1 )
		return E_OUTOFMEMORY;


	lstrcpy( theOutString, aIt->second.c_str() );
	theOutString[aLength] = 0;

	*theSize_Out = (int) aLength;

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

class LocalizationImpl_EnUs_Hardcoded::StringAdvanced : public std::wstring
{
public:
	__declspec(noinline) StringAdvanced(const char * theString)
	{
		int aNum = MultiByteToWideChar(CP_ACP, 0, theString, -1, 0, 0);

		wchar_t * aUnicode = new wchar_t[aNum];

		MultiByteToWideChar(CP_ACP, 0, theString, -1, aUnicode, aNum);

		assign(aUnicode, aUnicode+aNum-1);

		delete aUnicode;
	}

	StringAdvanced(const std::wstring & theString) : std::wstring(theString) {}
};

//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

class LocalizationImpl_EnUs_Hardcoded::LocalizationSerializer
{
	struct RenderData
	{
		RenderType Type;
		LocIdKey ID;
		IdType Data;
	};

	LocalizationImpl_EnUs_Hardcoded & myObj;
	std::vector<RenderData> myElements;
	size_t myMetricsStartIndex;


public:
	LocalizationSerializer(LocalizationImpl_EnUs_Hardcoded & theObj)
		: myMetricsStartIndex(0), myObj(theObj)
	{ }


public:
	void GenerateReferenceFile(const TCHAR * theOutputDirectory)
	{
		std::wstringstream aStream;


		aStream << L"<Approach_Localization version=\"1\">\r\n";

		//1. summary

		aStream << L"\t<Summary>\r\n";

		//1.0. Locale
		aStream << L"\t\t<Locale>en_US</Locale>\r\n";


		//1.1. Titles
		aStream << L"\t\t<Titles>\r\n";

		for (AuxMapIterC aIt = myObj.myTitles.begin(); aIt != myObj.myTitles.end(); aIt++)
		{
			TCHAR aBuf[6];
			int aLen = MultiByteToWideChar(CP_ACP, 0, aIt->first.c_str(), (int)aIt->first.size(), aBuf, 6);
			aBuf[aLen] = 0;
			aStream << L"\t\t\t<Title locale=\"" << aBuf << "\">" << aIt->second << L"</Title>\r\n";
		}

		aStream << L"\t\t</Titles>\r\n";


		//1.2 Created_Bys
		aStream << L"\t\t<Created_Bys>\r\n";

		for (AuxMapIterC aIt = myObj.myCreatedBys.begin(); aIt != myObj.myCreatedBys.end(); aIt++)
		{
			TCHAR aBuf[6];
			int aLen = MultiByteToWideChar(CP_ACP, 0, aIt->first.c_str(), (int)aIt->first.size(), aBuf, 6);
			aBuf[aLen] = 0;
			aStream << L"\t\t\t<Created_By locale=\"" << aBuf << L"\">" << aIt->second << L"</Created_By>\r\n";
		}

		aStream << L"\t\t</Created_Bys>\r\n";


		//1.3 Comments
		aStream << L"\t\t<Comments>\r\n";

		for (AuxMapIterC aIt = myObj.myComments.begin(); aIt != myObj.myComments.end(); aIt++)
		{
			TCHAR aBuf[6];
			int aLen = MultiByteToWideChar(CP_ACP, 0, aIt->first.c_str(), (int)aIt->first.size(), aBuf, 6);
			aBuf[aLen] = 0;
			aStream << L"\t\t\t<Comment locale=\"" << aBuf << L"\">" << aIt->second << L"</Comment>\r\n";
		}

		aStream << L"\t\t</Comments>\r\n";

		aStream << L"\t</Summary>\r\n";


		//2. Strings
		aStream << L"\r\n\t<Strings>\r\n";

		for (size_t i = 0; i < myMetricsStartIndex; i++)
		{
			if      ( myElements[i].Type == STRING )
				RenderString(aStream, myElements[i].ID, myElements[i].Data );

			else if ( myElements[i].Type == COMMENT )
				RenderComment(aStream, myElements[i].Data);
		}

		aStream << L"\t</Strings>\r\n";


		//3. Metrics
		aStream << L"\t<Metrics>\r\n";

		for (size_t i = myMetricsStartIndex; i < myElements.size(); i++)
		{
			if      ( myElements[i].Type == METRIC )
				RenderMetric(aStream, myElements[i].ID, myElements[i].Data);

			else if ( myElements[i].Type == COMMENT )
				RenderComment(aStream, myElements[i].Data);
		}

		aStream << L"\t</Metrics>\r\n";

		aStream << L"</Approach_Localization>";
		aStream.flush();



		HANDLE aFile = CreateFile (
			_T("Approach[en_US].xml"),
			GENERIC_WRITE,
			FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,
			NULL,
			CREATE_NEW,
			0,
			NULL);

		if ( aFile != INVALID_HANDLE_VALUE )
		{
			std::wstring aBuf = aStream.str();
			const static char * XmlControlTag = "<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n\r\n";

			//determine the size
			int aLen = WideCharToMultiByte(CP_UTF8, 0, aBuf.c_str(), (int) aBuf.size(), NULL, 0, NULL, NULL );

			char * aAllInUtf = new char[aLen];

			// obtain data
			WideCharToMultiByte(CP_UTF8, 0, aBuf.c_str(), (int) aBuf.size(), aAllInUtf, aLen, NULL, NULL );


			DWORD aNumWritten = 0;

			// UTF-8 control word
			WriteFile(aFile, "\xef\xbb\xbf",   3,                     &aNumWritten, NULL);

			// XML control tag control word
			WriteFile(aFile,  XmlControlTag, lstrlenA(XmlControlTag), &aNumWritten, NULL);

			// data
			WriteFile(aFile,  aAllInUtf,      aLen,                   &aNumWritten, NULL);

			delete [] aAllInUtf;

			CloseHandle(aFile);
		}
	}

	void SetElementType(RenderType theType)
	{
		if (theType == METRIC)
			myMetricsStartIndex = myElements.size();
	}

	void AddElement(RenderType theType, LocIdKey theID, const char * theData)
	{
		RenderData aDt;
		aDt.Type = theType;
		aDt.ID = theID;
		aDt.Data = theData;

		myElements.push_back(aDt);
	}


private:
	void RenderString(std::wostream & theOut, LocIdKey theID, const IdType & theData)
	{
		TCHAR aBuf[500];
		int aLen = MultiByteToWideChar(CP_ACP, 0, theData.c_str(), (int) theData.size(), aBuf, 500);
		aBuf[aLen] = 0;

		const String & aStr = myObj.GetString(theID);

		// 1. Render Start Tag
		theOut << L"\t\t<String id=\"" << aBuf << L"\">";

		// 2. Render Inner Text

		String::size_type aCur = 0;
		String::size_type aPos = aStr.find( L"\r\n", aCur );

		while (aPos != String::npos)
		{
			const wchar_t * aTemp = aStr.c_str() + aCur;

			theOut.write( aTemp, aPos-aCur);
			theOut << L"&#0013;&#0010;";

			aCur = aPos + 2;
			aPos = aStr.find( L"\r\n", aCur );
		}

		if ( aCur != aStr.size() )
			theOut.write( aStr.c_str() + aCur, aStr.size()-aCur);


		// 3. Render End Text
		theOut << L"</String>\r\n";
	}

	void RenderMetric(std::wostream & theOut, LocIdKey theID, const IdType & theData)
	{
		const Metric & aM = myObj.GetMetric(theID);

		TCHAR aBuf[500];
		int aLen = MultiByteToWideChar(CP_ACP, 0, theData.c_str(), (int) theData.size(), aBuf, 500);
		aBuf[aLen] = 0;

		theOut << L"\t\t<Metric id=\"" << aBuf << L"\" factor=\"" << aM.Factor << L"\">" << aM.Val1;

		if (aM.Factor == 2)
			theOut << L"," <<  aM.Val2;

		else if (aM.Factor == 4)
			theOut << L"," <<  aM.Val2 << L"," <<  aM.Val3 << L"," <<  aM.Val4;

		theOut << L"</Metric>\r\n";
	}

	void RenderComment(std::wostream & theOut, const LocalizationImpl_EnUs_Hardcoded::IdType & theString)
	{
		theOut << L"\t\t<!-- " << theString.c_str() << L" -->\r\n";
	}
};

//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

HRESULT LocalizationImpl_EnUs_Hardcoded::GetLocale(TCHAR * theOut, int * theSize)
{
	if ( theSize == 0 || theOut == 0)
		return E_INVALIDARG;

	const int Length = 5;

	if (*theSize < Length+1)
		return E_OUTOFMEMORY;

	lstrcpyn(theOut, _T("en_US"), Length+1);

	*theSize = Length;
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////////////////////////

HRESULT LocalizationImpl_EnUs_Hardcoded::Initialize(const TCHAR * theCurLangID, int theSize_LangID, bool & thePartial)
{
#if _APPROACH_USE_REFERENCE_FILE != 0

	LocalizationSerializer aSerializer(*this);

#  define RENDER(type, id, data) aSerializer.AddElement(type, id, data)
#  define DEFINE_COMMENT(data)   RENDER(COMMENT, 0, data)
#  define COMMENT_MODE(type)     aSerializer.SetElementType(type)

#else

#  define RENDER(type, id, data)
#  define DEFINE_COMMENT(data)
#  define COMMENT_MODE(type)

#endif //_APPROACH_USE_REFERENCE_FILE != 0

#define DEFINE_METRIC1(id, v1)             DEFINE_METRIC(RCENTRY_##id::Hash, v1);                                                                                  RENDER(METRIC, RCENTRY_##id::Hash, #id)
#define DEFINE_METRIC2(id, v1, v2)         DEFINE_METRIC(RCENTRY_##id::Hash, v1, v2);                                                                              RENDER(METRIC, RCENTRY_##id::Hash, #id)
#define DEFINE_METRIC3(id, v1, v2, v3)     DEFINE_METRIC(RCENTRY_##id::Hash, v1, v2, v3);                                                                          RENDER(METRIC, RCENTRY_##id::Hash, #id)
#define DEFINE_METRIC4(id, v1, v2, v3, v4) DEFINE_METRIC(RCENTRY_##id::Hash, v1, v2, v3, v4);                                                                      RENDER(METRIC, RCENTRY_##id::Hash, #id)
#define DEFINE_DLG_METRIC(id)              DEFINE_METRIC(RCENTRY_##id::Hash, RCENTRY_##id::Width, RCENTRY_##id::Height);                                           RENDER(METRIC, RCENTRY_##id::Hash, #id)
#define DEFINE_CTL_METRIC(id)              DEFINE_METRIC(RCENTRY_##id::Hash, RCENTRY_##id::X,     RCENTRY_##id::Y,     RCENTRY_##id::Width, RCENTRY_##id::Height); RENDER(METRIC, RCENTRY_##id::Hash, #id)

#define DEFINE_STRINGx(id, v)              DEFINE_STRING(RCENTRY_##id::Hash, v);                     RENDER(STRING, RCENTRY_##id::Hash,    #id)
#define DEFINE_CTL_STRING(id)              DEFINE_STRING(RCENTRY_##id::Hash, RCENTRY_##id::Text() ); RENDER(STRING, RCENTRY_##id::Hash,    #id)
#define DEFINE_MNU_STRING(id)              DEFINE_STRING(RCENTRY_##id::Hash, RCENTRY_##id::Text() ); RENDER(STRING, RCENTRY_##id::Hash,    #id)

#define DEFINE_CTL_TIP(id, tip)            DEFINE_STRING(RCENTRY_##id::TipHash, tip); RENDER(STRING, RCENTRY_##id::TipHash, RCENTRY_##id::TipID() )


	// Step 1. Locale
	myTitles[ "bg_BG" ] = StringAdvanced(L"\x410\x43d\x433\x43b\x438\x439\x441\x43a\x438");       //Английски
	myTitles[ "cs_CZ" ] = StringAdvanced(L"Anglick\375");                                         //y with acute
	myTitles[ "da_DK" ] = StringAdvanced( "Engelsk");
	myTitles[ "de_DE" ] = StringAdvanced( "Englisch");
	myTitles[ "en_US" ] = StringAdvanced( "English");
	myTitles[ "es_ES" ] = StringAdvanced(L"Ingl\351s");                                           //e with acute
	myTitles[ "fi_FI" ] = StringAdvanced( "Englanti");
	myTitles[ "fr_FR" ] = StringAdvanced( "Anglais");
	myTitles[ "he_IL" ] = StringAdvanced(L"\x05D0" L"\x05E0" L"\x05D2" L"\x05DC" L"\x05D9" L"\x05EA");
	myTitles[ "hu_HU" ] = StringAdvanced( "Angol");
	myTitles[ "is_IS" ] = StringAdvanced( "Enska");
	myTitles[ "it_IT" ] = StringAdvanced( "Inglese");
	myTitles[ "nb_NO" ] = StringAdvanced( "Engelsk");                                             //Norwegian Bokmal
	myTitles[ "nl_NL" ] = StringAdvanced( "Engels");
	myTitles[ "nn_NO" ] = StringAdvanced( "Engelsk");                                             //Norwegian Nynorsk
	myTitles[ "pl_PL" ] = StringAdvanced( "Angielski");
	myTitles[ "pt_PT" ] = StringAdvanced(L"Ingl\352s");                                           //e with cap
	myTitles[ "ro_RO" ] = StringAdvanced(L"Englez\403");                                          //a with breve
	myTitles[ "ru_RU" ] = StringAdvanced(L"\x410\x43d\x433\x43b\x438\x439\x441\x43a\x438\x439");  //Английский
	myTitles[ "sk_SK" ] = StringAdvanced(L"Anglick\375");                                         //y with acute
	myTitles[ "sl_SI" ] = StringAdvanced(L"Angle\541ki");                                         //s with caron
	myTitles[ "sv_SE" ] = StringAdvanced( "Engelsk");
	myTitles[ "tr_TR" ] = StringAdvanced(L"\460ngilizce");                                        //Capital I with dot
	myTitles[ "zh_CN" ] = StringAdvanced(L"\x82F1" L"\x8BED");


	// Step 2. Created by
	myCreatedBys[ "en_US" ] = StringAdvanced("Created by KO Software");


	// Step 3. Comments
	myComments[ "en_US" ]  = StringAdvanced("The standard KO Approach localization");

	// Step 4. Strings


	COMMENT_MODE(STRING);

	//   Tab titles
	DEFINE_COMMENT("Tab titles");


	DEFINE_STRINGx( IDD_O_FEAT,                     "Features");
	DEFINE_STRINGx( IDD_O_MNSC,                     "Menus & Scrolling");
	DEFINE_STRINGx( IDD_O_CNTS,                     "Contents");
	DEFINE_STRINGx( IDD_O_MNTC,                     "Maintenance");
	DEFINE_STRINGx( IDD_O_REG,                      "Registration");
	DEFINE_STRINGx( IDD_O_ABT,                      "About");

	//   Exception item
	DEFINE_COMMENT("Exception item");

	DEFINE_STRINGx( IDS_EXCEPT_UNAVAILABLE,         "(Unavailable)");
	DEFINE_STRINGx( IDS_EXCEPT_EMPTY,               "(Empty)");
	DEFINE_STRINGx( IDS_EXCEPT_GENERIC,             "(Generic error)");

	//   Popup menu strings
	DEFINE_COMMENT("Popup menu strings");

	DEFINE_MNU_STRING(IDP_MAIN_ENBL_FLDMNUS);
	DEFINE_MNU_STRING(IDP_MAIN_ENBL_APCHITMS);
	DEFINE_MNU_STRING(IDP_MAIN_ENBL_TTLBARMNUS);
	DEFINE_MNU_STRING(IDP_MAIN_OPTIONS);
	DEFINE_MNU_STRING(IDP_MAIN_EXIT);
	DEFINE_STRINGx( IDP_CXMN_TOAPPROACHITEMS,       "Add shortcut to Approach Items");

	//   Approach Items elements
	DEFINE_COMMENT("Approach Items elements");

	DEFINE_STRINGx( IDS_APCHITEMS_ITEMRUN,          "Run...");
	DEFINE_STRINGx( IDS_APCHITEMS_ITEMPROC,         "Running processes");
	DEFINE_STRINGx( IDS_APCHITEMS_ITEMPROC_SYS,     "System");
	DEFINE_STRINGx( IDS_APCHITEMS_ITEMPROC_QUIT,    "Quit");
	DEFINE_STRINGx( IDS_APCHITEMS_ITEMPROC_REVEAL,  "Reveal");


	//   Global command strings
	DEFINE_COMMENT("Global command strings");

	DEFINE_STRINGx( IDS_GLBL_CMD_OK,                "OK");
	DEFINE_STRINGx( IDS_GLBL_CMD_CANCEL,            "Cancel");
	DEFINE_STRINGx( IDS_GLBL_CMD_HELP,              "Help...");
	DEFINE_STRINGx( IDS_GLBL_CMD_YES,               "Yes");
	DEFINE_STRINGx( IDS_GLBL_CMD_NO,                "No");
	DEFINE_STRINGx( IDS_GLBL_CMD_RETRY,             "Retry");

	//   Global strings
	DEFINE_COMMENT("Global strings");

	DEFINE_STRINGx( IDS_GLBL_WARNING,               "Warning" );
	DEFINE_STRINGx( IDS_GLBL_ERROR,                 "Error" );
	DEFINE_STRINGx( IDS_GLBL_ALWAYS,                "Always");
	DEFINE_STRINGx( IDS_GLBL_NEVER,                 "Never");
	DEFINE_STRINGx( IDS_GLBL_MILLISEC,              "ms");
	DEFINE_STRINGx( IDS_GLBL_USESYSSETTING,         "Use system setting");


	//   Features tab
	DEFINE_COMMENT( "Features tab" );

	DEFINE_CTL_STRING(IDL_O_FEAT_CAT_FLDMENUS);
	DEFINE_CTL_STRING(IDL_O_FEAT_FLDMENUSDESC);

	DEFINE_CTL_STRING(IDL_O_FEAT_INITDELAY);
	DEFINE_CTL_STRING(IDL_O_FEAT_CAT_APCHITEMS);
	DEFINE_CTL_STRING(IDL_O_FEAT_APCHITEMSDESC);
	DEFINE_CTL_STRING(IDC_O_FEAT_APCHITEMSOPT);

	DEFINE_CTL_STRING(IDL_O_FEAT_CAT_TTLBARMENUS);
	DEFINE_CTL_STRING(IDL_O_FEAT_TTLBARMENUSDESC);
	DEFINE_CTL_STRING(IDC_O_FEAT_DESKPOS_0TOP);
	DEFINE_CTL_STRING(IDC_O_FEAT_DESKPOS_1BOTM);
	DEFINE_CTL_STRING(IDC_O_FEAT_CURFLDISFILE);
	DEFINE_CTL_STRING(IDC_O_FEAT_CURFLDAUTOSEL);


	//   Menus and Scrolling tab
	DEFINE_COMMENT("Menus and Scrolling tab");

	DEFINE_CTL_STRING(IDL_O_MNSC_CAT_MENUS);
	DEFINE_CTL_STRING(IDL_O_MNSC_MAXMNUWIDTH);
	DEFINE_CTL_STRING(IDC_O_MNSC_MNUWIDTH_0PXL);
	DEFINE_CTL_STRING(IDC_O_MNSC_MNUWIDTH_1PRC);
	DEFINE_CTL_STRING(IDL_O_MNSC_CHILDDELAY);

	DEFINE_CTL_STRING(IDL_O_MNSC_CAT_SCROLLING);
	DEFINE_CTL_STRING(IDL_O_MNSC_WHEELSCROLLS);
	DEFINE_CTL_STRING(IDL_O_MNSC_WHEELITEMS);
	DEFINE_CTL_STRING(IDL_O_MNSC_PAGESCROLLS);
	DEFINE_CTL_STRING(IDL_O_MNSC_PAGEITEMS);
	DEFINE_CTL_STRING(IDL_O_MNSC_SCROLLARROWS);
	DEFINE_CTL_STRING(IDC_O_MNSC_SCROLLPOS_0TOP);
	DEFINE_CTL_STRING(IDC_O_MNSC_SCROLLPOS_1BOTM);
	DEFINE_CTL_STRING(IDC_O_MNSC_SCROLLPOS_2ENDS);
	DEFINE_CTL_STRING(IDC_O_MNSC_DISPNUMTOSCROLL);
	DEFINE_CTL_STRING(IDL_O_MNSC_SCROLLDELAY);


	//   Contents tab
	DEFINE_COMMENT("Contents tab");

	DEFINE_CTL_STRING(IDL_O_CNTS_CAT_ITEMS);
	DEFINE_CTL_STRING(IDL_O_CNTS_HDNITMS);
	DEFINE_CTL_STRING(IDC_O_CNTS_HDNITMS_DIMICN);
	DEFINE_CTL_STRING(IDL_O_CNTS_BROWSEFLD);
	DEFINE_STRINGx( IDS_O_CNTS_BROWSEFLD_0NEWWND,   "In a new window");         // string for IDC_O_CNTS_BROWSEFLD combobox
	DEFINE_STRINGx( IDS_O_CNTS_BROWSEFLD_1SAMEWND,  "In the existing window");  // string for IDC_O_CNTS_BROWSEFLD combobox
	DEFINE_CTL_STRING(IDL_O_CNTS_SSFISWHAT);
	DEFINE_CTL_STRING(IDC_O_CNTS_SSFIS_0FOLDER);
	DEFINE_CTL_STRING(IDC_O_CNTS_SSFIS_1FILE);
	DEFINE_CTL_STRING(IDL_O_CNTS_ZIPISWHAT);
	DEFINE_CTL_STRING(IDC_O_CNTS_ZIPIS_0FOLDER);
	DEFINE_CTL_STRING(IDC_O_CNTS_ZIPIS_1FILE);
	DEFINE_CTL_STRING(IDL_O_CNTS_MOUSEITEMCLK);
	DEFINE_STRINGx( IDS_O_CNTS_MOUSEITEMCLK_0REG,   "Regular");                 // string for IDC_O_CNTS_MOUSEITEMCLK combobox
	DEFINE_STRINGx( IDS_O_CNTS_MOUSEITEMCLK_1SGL,   "Single-click");            // string for IDC_O_CNTS_MOUSEITEMCLK combobox
	DEFINE_STRINGx( IDS_O_CNTS_MOUSEITEMCLK_2DBL,   "Double-click");            // string for IDC_O_CNTS_MOUSEITEMCLK combobox

	DEFINE_CTL_STRING(IDL_O_CNTS_CAT_ORDER);
	DEFINE_CTL_STRING(IDC_O_CNTS_SORTITEMS);
	DEFINE_CTL_STRING(IDC_O_CNTS_FLDORD_0TOP);
	DEFINE_CTL_STRING(IDC_O_CNTS_FLDORD_1BOTM);
	DEFINE_CTL_STRING(IDC_O_CNTS_FLDORD_2MIX);

	DEFINE_CTL_STRING(IDL_O_CNTS_CAT_ICONS);
	DEFINE_CTL_STRING(IDC_O_CNTS_ICN_0GENERIC);
	DEFINE_CTL_STRING(IDC_O_CNTS_ICN_1REGULAR);
	DEFINE_CTL_STRING(IDC_O_CNTS_ICNOVERLAYS);
	DEFINE_CTL_STRING(IDC_O_CNTS_ICNOPTIMIZ);


	//   Maintenance tab
	DEFINE_COMMENT("Maintenance tab");

	DEFINE_CTL_STRING(IDL_O_MNTC_CAT_MNTC);
	DEFINE_CTL_STRING(IDL_O_MNTC_UILANG);
	DEFINE_STRINGx( IDS_O_MNTC_UILANG_MORE,         "More languages...");
	DEFINE_CTL_STRING(IDL_O_MNTC_UPDATE);
	DEFINE_CTL_STRING(IDC_O_MNTC_UPDATE);
	DEFINE_CTL_STRING(IDC_O_MNTC_UPDATEAUTO);
	DEFINE_CTL_STRING(IDL_O_MNTC_RESTDEF);
	DEFINE_CTL_STRING(IDC_O_MNTC_RESTDEF);
	DEFINE_CTL_STRING(IDL_O_MNTC_AUTORUN);
	DEFINE_CTL_STRING(IDC_O_MNTC_AUTORUN);

	DEFINE_CTL_STRING(IDL_O_MNTC_CAT_PLUGINS);
	DEFINE_CTL_STRING(IDC_O_MNTC_PLGNENBL);
	DEFINE_CTL_STRING(IDC_O_MNTC_PLGNDSBL);

	DEFINE_STRINGx( IDS_O_MNTC_PLGNLST_NAME,        "Name");
	DEFINE_STRINGx( IDS_O_MNTC_PLGNLST_STATUS,      "Status");
	DEFINE_STRINGx( IDS_O_MNTC_PLGNLST_LOADED,      "Loaded");
	DEFINE_STRINGx( IDS_O_MNTC_PLGNLST_NOTLOADED,   "Not loaded");
	DEFINE_STRINGx( IDS_O_MNTC_PLGNLST_STOPPING,    "Stopping");
	DEFINE_STRINGx( IDS_O_MNTC_PLGNLST_DISABLED,    "Disabled");

	DEFINE_STRINGx( IDS_O_MNTC_MSG_UPDYES,          "An update is available. Would you like to download it?");
	DEFINE_STRINGx( IDS_O_MNTC_MSG_UPDNO,           "No update is currently available.");
	DEFINE_STRINGx( IDS_O_MNTC_MSG_RESTDEF,         "The defaults have been restored.");


	//   About tab
	DEFINE_COMMENT("About tab");

	DEFINE_CTL_STRING(IDC_O_ABT_THXCAPTION);

	DEFINE_STRINGx( IDS_O_ABT_VERSION,              "Version");
	DEFINE_STRINGx( IDS_O_ABT_THX1,                 "Turlough O'Connor - for the original idea;");
	DEFINE_STRINGx( IDS_O_ABT_THX2,                 "Dmitry Osipov - for product Documentation, valuable suggestions, pre-release testing, and overall support;");
	DEFINE_STRINGx( IDS_O_ABT_THX3,                L"Andrew Fellows, Denis Gavrilov, Alexander Gusev, Ilya Kirilkin, Ilya Popov, Sergey Samsonov, Jan Schr\366der, \x2020 Alexander Simanov, Roman Soshkin, and Denis Vlasov - for valuable suggestions and pre-release testing;");
	DEFINE_STRINGx( IDS_O_ABT_THX4,                 "All the people on CodeProject for keeping me inspired.");

	DEFINE_CTL_STRING(IDC_O_ABT_FEEDBACK);
	DEFINE_CTL_STRING(IDC_O_ABT_HOMEPAGE);



	////////////////////////////////////////////////////////////////////////////////////////////////
	//   Approach Items Options dialog box
	DEFINE_COMMENT("Approach Items Options dialog box");

	DEFINE_STRINGx( IDD_APCHITMS,                   "Approach Items");

	DEFINE_CTL_STRING(IDL_APCHITMS_CUR);
	DEFINE_CTL_STRING(IDL_APCHITMS_ALL);
	DEFINE_CTL_STRING(IDC_APCHITMS_ADD);
	DEFINE_CTL_STRING(IDC_APCHITMS_RMV);
	DEFINE_CTL_STRING(IDC_APCHITMS_UP);
	DEFINE_CTL_STRING(IDC_APCHITMS_DOWN);
	DEFINE_CTL_STRING(IDC_APCHITMS_BRW);


	////////////////////////////////////////////////////////////////////////////////////////////////
	//   Help popups
	DEFINE_COMMENT("Help popups");

	// Root
	DEFINE_CTL_TIP( IDC_OPTS_OK,                     "Saves the changes and closes the program window.");
	DEFINE_CTL_TIP( IDC_OPTS_CANCEL,                 "Discards the changes and closes the program window.");
	DEFINE_CTL_TIP( IDC_OPTS_HELP,                   "Opens KO Approach Documentation.");

	// Features page
	DEFINE_CTL_TIP( IDC_O_FEAT_FLDMENUSTGL,          "Enables/Disables the Folder Menus feature allowing to display menus that mimic the structure of the selected folder.");
	DEFINE_CTL_TIP( IDC_O_FEAT_INITDELAY,            "Adjusts the amount of time the mouse key needs to be held down before the initial menu expands.");
	DEFINE_CTL_TIP( IDC_O_FEAT_APCHITEMSTGL,         "Enables/Disables the Approach Items feature providing quick access to your favorite files.");
	DEFINE_CTL_TIP( IDC_O_FEAT_APCHITEMSOPT,         "Opens the Approach Items Options window allowing you to administer its contents.");
	DEFINE_CTL_TIP( IDC_O_FEAT_TTLBARMENUSTGL,       "Enables/Disables the Titlebar Menus feature allowing you to browse parent-to-child menus built from an Explorer window's Title Bar.");
	DEFINE_CTL_TIP( IDC_O_FEAT_DESKPOS_0TOP,         "Titlebar Menus will display the Desktop at the top and the current folder at the bottom of the list.");
	DEFINE_CTL_TIP( IDC_O_FEAT_DESKPOS_1BOTM,        "Titlebar Menus will display the current folder at the top and the Desktop at the bottom of the list.");
	DEFINE_CTL_TIP( IDC_O_FEAT_CURFLDISFILE,         "Titlebar Menus will not display submenu arrows for the current folder, treating it as a file.");
	DEFINE_CTL_TIP( IDC_O_FEAT_CURFLDAUTOSEL,        "Titlebar Menus will highlight the item pointing at the current folder.");

	// Menus and Scrolling page
	DEFINE_CTL_TIP( IDC_O_MNSC_MAXMNUWIDTH,          "Specifies the maximum menu width. The maximum width may not exceed 1/3 of the entire screen width.");
	DEFINE_CTL_TIP( IDC_O_MNSC_MNUWIDTH_0PXL,        "Sets the maximum menu width to the specified number of pixels.");
	DEFINE_CTL_TIP( IDC_O_MNSC_MNUWIDTH_1PRC,        "Sets the maximum menu width to the specified percentage of the screen width.");
	DEFINE_CTL_TIP( IDC_O_MNSC_CHILDDELAY,           "Adjusts the amount of time KO Approach will wait before displaying sub-menus.");
	DEFINE_CTL_TIP( IDC_O_MNSC_WHEELSCROLLS,         "Specifies the number of items to be skipped when scrolling with the mouse wheel.");
	DEFINE_CTL_TIP( IDC_O_MNSC_PAGESCROLLS,          "Specifies the number of items to be skipped when scrolling with the Page Up/Page Down buttons.");
	DEFINE_CTL_TIP( IDC_O_MNSC_SCROLLPOS_0TOP,       "Sets both scrolling arrows, if displayed, to appear at the top of the menu.");
	DEFINE_CTL_TIP( IDC_O_MNSC_SCROLLPOS_1BOTM,      "Sets both scrolling arrows, if displayed, to appear at the bottom of the menu.");
	DEFINE_CTL_TIP( IDC_O_MNSC_SCROLLPOS_2ENDS,      "Sets UP and DOWN scrolling arrows, if displayed, to appear at the top and bottom of the menu, respectively.");
	DEFINE_CTL_TIP( IDC_O_MNSC_DISPNUMTOSCROLL,      "Shows/hides a counter of menu items currently outside the boundaries of the screen.");
	DEFINE_CTL_TIP( IDC_O_MNSC_SCROLLDELAY,          "Adjusts the amount of time KO Approach will wait before displaying the next upper/lower item on a menu when scrolling.");

	// Contents page
	DEFINE_CTL_TIP( IDC_O_CNTS_HDNITMS,              "Specifies whether to display hidden files.");
	DEFINE_CTL_TIP( IDC_O_CNTS_HDNITMS_DIMICN,       "Specifies that hidden items will be displayed with a semi-transparent icon.");
	DEFINE_CTL_TIP( IDC_O_CNTS_BROWSEFLD,            "Specifies whether KO Approach should reuse an Explorer window when you click on a folder.");
	DEFINE_CTL_TIP( IDC_O_CNTS_SSFIS_0FOLDER,        "Specifies that folder shortcuts should display contents of their targets.");
	DEFINE_CTL_TIP( IDC_O_CNTS_SSFIS_1FILE,          "Specifies that folder shortcuts should not display contents of their targets.");
	DEFINE_CTL_TIP( IDC_O_CNTS_ZIPIS_0FOLDER,        "Specifies that ZIP archives should display items inside.");
	DEFINE_CTL_TIP( IDC_O_CNTS_ZIPIS_1FILE,          "Specifies that ZIP archives should not display items inside.");
	DEFINE_CTL_TIP( IDC_O_CNTS_MOUSEITEMCLK,         "Specifies whether menu items should be activated with a single- or double mouse click.");
	DEFINE_CTL_TIP( IDC_O_CNTS_SORTITEMS,            "Specifies whether items on a menu should be listed in alphabetical order.");
	DEFINE_CTL_TIP( IDC_O_CNTS_FLDORD_0TOP,          "Specifies whether folders on a menu should be listed first, followed by files.");
	DEFINE_CTL_TIP( IDC_O_CNTS_FLDORD_1BOTM,         "Specifies whether files on a menu should be listed first, followed by folders.");
	DEFINE_CTL_TIP( IDC_O_CNTS_FLDORD_2MIX,          "Makes no distinction between files and folders displaying all items as a continuous list.");
	DEFINE_CTL_TIP( IDC_O_CNTS_ICN_0GENERIC,         "Specifies whether original icons should be replaced with generic type ones (folder, application, or file) to allow for faster menu display.");
	DEFINE_CTL_TIP( IDC_O_CNTS_ICN_1REGULAR,         "Specifies whether items should only display icons similar to those in Windows Explorer.");
	DEFINE_CTL_TIP( IDC_O_CNTS_ICNOVERLAYS,          "Specifies whether items should display icon overlays, such as shortcut arrows.");
	DEFINE_CTL_TIP( IDC_O_CNTS_ICNOPTIMIZ,           "Instructs KO Approach to retrieve item icons in batches to help optimize the icon rendering process.");

	// Maintenance page
	DEFINE_CTL_TIP( IDC_O_MNTC_UILANG,               "Specifies KO Approach localization.");
	DEFINE_CTL_TIP( IDC_O_MNTC_LOCALEINFO,           "Displays brief information about the currently selected localization.");
	DEFINE_CTL_TIP( IDC_O_MNTC_UPDATE,               "Checks whether an update is available on the KO Approach Web site and offers you to download and install it.");
	DEFINE_CTL_TIP( IDC_O_MNTC_UPDATEAUTO,           "Specifies whether KO Approach should periodically check for updates and display a notification when an update becomes available.");
	DEFINE_CTL_TIP( IDC_O_MNTC_RESTDEF,              "Reverts KO Approach settings for the current user to the values specified upon the first program launch.");
	DEFINE_CTL_TIP( IDC_O_MNTC_AUTORUN,              "Creates/Removes the KO Approach shortcut in the Startup folder of your Start Menu.");
	DEFINE_CTL_TIP( IDC_O_MNTC_PLGNLST,              "Displays the list of installed plug-ins." );
	DEFINE_CTL_TIP( IDC_O_MNTC_PLGNENBL,             "Enables the selected plug-in when you press OK and re-open KO Approach." );
	DEFINE_CTL_TIP( IDC_O_MNTC_PLGNDSBL,             "Disables the selected plug-in when you press OK and re-open KO Approach." );

	// About page
	DEFINE_CTL_TIP( IDC_O_ABT_FEEDBACK,              "Creates a new email message, so you can send us your comments and suggestions.");
	DEFINE_CTL_TIP( IDC_O_ABT_HOMEPAGE,              "Opens the KO Approach Web site in your default browser.");


	/////////////////////////////////////////////////////////////////////////////////////////////
	// Step 5. Metrics

	COMMENT_MODE(METRIC);

	DEFINE_COMMENT( "Global" );

	DEFINE_METRIC1( IDM_GLBL_LANGRTL,                          0);
	DEFINE_METRIC1( IDM_GLBL_MILLISECFMT,                      0);

	//   Main dialog box

	DEFINE_COMMENT( "Main dialog box" );

	DEFINE_METRIC1( IDM_OPTS_CONTROLMARGIN,                    7);
	DEFINE_METRIC1( IDM_OPTS_CATEGORIESWIDTH,                 80);
	DEFINE_METRIC2( IDM_OPTS_BUTTONSIZE,                      52,  14);


	//   Features tab
	DEFINE_COMMENT( "Features tab" );

	DEFINE_DLG_METRIC(IDD_O_FEAT);

	DEFINE_CTL_METRIC(IDL_O_FEAT_CAT_FLDMENUS);
	DEFINE_CTL_METRIC(IDL_O_FEAT_FLDMENUSVIS);
	DEFINE_CTL_METRIC(IDL_O_FEAT_FLDMENUSDESC);
	DEFINE_CTL_METRIC(IDL_O_FEAT_INITDELAY);
	DEFINE_CTL_METRIC(IDC_O_FEAT_INITDELAY);
	DEFINE_CTL_METRIC(IDL_O_FEAT_INITDELAYVAL);

	DEFINE_CTL_METRIC(IDL_O_FEAT_CAT_APCHITEMS);
	DEFINE_CTL_METRIC(IDL_O_FEAT_APCHITEMSVIS);
	DEFINE_CTL_METRIC(IDL_O_FEAT_APCHITEMSDESC);
	DEFINE_CTL_METRIC(IDC_O_FEAT_APCHITEMSOPT);

	DEFINE_CTL_METRIC(IDL_O_FEAT_CAT_TTLBARMENUS);
	DEFINE_CTL_METRIC(IDL_O_FEAT_TTLBARMENUSVIS);
	DEFINE_CTL_METRIC(IDL_O_FEAT_TTLBARMENUSDESC);
	DEFINE_CTL_METRIC(IDC_O_FEAT_DESKPOS_0TOP);
	DEFINE_CTL_METRIC(IDC_O_FEAT_DESKPOS_1BOTM);
	DEFINE_CTL_METRIC(IDC_O_FEAT_CURFLDISFILE);
	DEFINE_CTL_METRIC(IDC_O_FEAT_CURFLDAUTOSEL);


	//   Menus and Scrolling tab
	DEFINE_COMMENT( "Menus and Scrolling tab" );

	DEFINE_DLG_METRIC(IDD_O_MNSC);

	DEFINE_CTL_METRIC(IDL_O_MNSC_CAT_MENUS);
	DEFINE_CTL_METRIC(IDL_O_MNSC_MAXMNUWIDTH);
	DEFINE_CTL_METRIC(IDC_O_MNSC_MAXMNUWIDTH);
	DEFINE_CTL_METRIC(IDC_O_MNSC_MNUWIDTH_0PXL);
	DEFINE_CTL_METRIC(IDC_O_MNSC_MNUWIDTH_1PRC);
	DEFINE_CTL_METRIC(IDL_O_MNSC_CHILDDELAY);
	DEFINE_CTL_METRIC(IDC_O_MNSC_CHILDDELAY);
	DEFINE_CTL_METRIC(IDL_O_MNSC_CHILDDELAYVAL);

	DEFINE_CTL_METRIC(IDL_O_MNSC_CAT_SCROLLING);
	DEFINE_CTL_METRIC(IDL_O_MNSC_WHEELSCROLLS);
	DEFINE_CTL_METRIC(IDC_O_MNSC_WHEELSCROLLS);
	DEFINE_CTL_METRIC(IDL_O_MNSC_WHEELITEMS);
	DEFINE_CTL_METRIC(IDL_O_MNSC_PAGESCROLLS);
	DEFINE_CTL_METRIC(IDC_O_MNSC_PAGESCROLLS);
	DEFINE_CTL_METRIC(IDL_O_MNSC_PAGEITEMS);
	DEFINE_CTL_METRIC(IDL_O_MNSC_SCROLLARROWS);
	DEFINE_CTL_METRIC(IDC_O_MNSC_SCROLLPOS_0TOP);
	DEFINE_CTL_METRIC(IDC_O_MNSC_SCROLLPOS_1BOTM);
	DEFINE_CTL_METRIC(IDC_O_MNSC_SCROLLPOS_2ENDS);
	DEFINE_CTL_METRIC(IDC_O_MNSC_DISPNUMTOSCROLL);
	DEFINE_CTL_METRIC(IDL_O_MNSC_SCROLLDELAY);
	DEFINE_CTL_METRIC(IDC_O_MNSC_SCROLLDELAY);
	DEFINE_CTL_METRIC(IDL_O_MNSC_SCROLLDELAYVAL);


	//   Contents tab
	DEFINE_COMMENT( "Contents tab" );

	DEFINE_DLG_METRIC(IDD_O_CNTS);

	DEFINE_CTL_METRIC(IDL_O_CNTS_CAT_ITEMS);
	DEFINE_CTL_METRIC(IDL_O_CNTS_HDNITMS);
	DEFINE_CTL_METRIC(IDC_O_CNTS_HDNITMS);
	DEFINE_CTL_METRIC(IDC_O_CNTS_HDNITMS_DIMICN);
	DEFINE_CTL_METRIC(IDL_O_CNTS_BROWSEFLD);
	DEFINE_CTL_METRIC(IDC_O_CNTS_BROWSEFLD);
	DEFINE_CTL_METRIC(IDL_O_CNTS_SSFISWHAT);
	DEFINE_CTL_METRIC(IDC_O_CNTS_SSFIS_0FOLDER);
	DEFINE_CTL_METRIC(IDC_O_CNTS_SSFIS_1FILE);
	DEFINE_CTL_METRIC(IDL_O_CNTS_ZIPISWHAT);
	DEFINE_CTL_METRIC(IDC_O_CNTS_ZIPIS_0FOLDER);
	DEFINE_CTL_METRIC(IDC_O_CNTS_ZIPIS_1FILE);
	DEFINE_CTL_METRIC(IDL_O_CNTS_MOUSEITEMCLK);
	DEFINE_CTL_METRIC(IDC_O_CNTS_MOUSEITEMCLK);

	DEFINE_CTL_METRIC(IDL_O_CNTS_CAT_ORDER);
	DEFINE_CTL_METRIC(IDC_O_CNTS_SORTITEMS);
	DEFINE_CTL_METRIC(IDC_O_CNTS_FLDORD_0TOP);
	DEFINE_CTL_METRIC(IDC_O_CNTS_FLDORD_1BOTM);
	DEFINE_CTL_METRIC(IDC_O_CNTS_FLDORD_2MIX);

	DEFINE_CTL_METRIC(IDL_O_CNTS_CAT_ICONS);
	DEFINE_CTL_METRIC(IDC_O_CNTS_ICN_0GENERIC);
	DEFINE_CTL_METRIC(IDC_O_CNTS_ICN_1REGULAR);
	DEFINE_CTL_METRIC(IDC_O_CNTS_ICNOVERLAYS);
	DEFINE_CTL_METRIC(IDC_O_CNTS_ICNOPTIMIZ);


	//   Maintenance and Plug-ins tab
	DEFINE_COMMENT( "Maintenance and Plug-ins tab" );

	DEFINE_DLG_METRIC(IDD_O_MNTC);

	DEFINE_CTL_METRIC(IDL_O_MNTC_CAT_MNTC);
	DEFINE_CTL_METRIC(IDL_O_MNTC_UILANG);
	DEFINE_CTL_METRIC(IDC_O_MNTC_UILANG);
	DEFINE_CTL_METRIC(IDC_O_MNTC_LOCALEINFO);
	DEFINE_CTL_METRIC(IDL_O_MNTC_UPDATE);
	DEFINE_CTL_METRIC(IDC_O_MNTC_UPDATE);
	DEFINE_CTL_METRIC(IDC_O_MNTC_UPDATEAUTO);
	DEFINE_CTL_METRIC(IDL_O_MNTC_RESTDEF);
	DEFINE_CTL_METRIC(IDC_O_MNTC_RESTDEF);
	DEFINE_CTL_METRIC(IDL_O_MNTC_AUTORUN);
	DEFINE_CTL_METRIC(IDC_O_MNTC_AUTORUN);

	DEFINE_CTL_METRIC(IDL_O_MNTC_CAT_PLUGINS);
	DEFINE_CTL_METRIC(IDC_O_MNTC_PLGNLST);
	DEFINE_CTL_METRIC(IDC_O_MNTC_PLGNDSBL);
	DEFINE_CTL_METRIC(IDC_O_MNTC_PLGNENBL);


	//   About tab
	DEFINE_COMMENT( "About tab" );

	DEFINE_DLG_METRIC(IDD_O_ABT);

	DEFINE_CTL_METRIC(IDC_O_ABT_APPICON);
	DEFINE_CTL_METRIC(IDC_O_ABT_APPNAME);
	DEFINE_CTL_METRIC(IDC_O_ABT_APPVER);
	DEFINE_CTL_METRIC(IDC_O_ABT_APPCOPY);

	DEFINE_CTL_METRIC(IDC_O_ABT_HORRULE);

	DEFINE_CTL_METRIC(IDC_O_ABT_THXCAPTION);
	DEFINE_CTL_METRIC(IDC_O_ABT_THXLIST);

	DEFINE_CTL_METRIC(IDC_O_ABT_FEEDBACK);
	DEFINE_CTL_METRIC(IDC_O_ABT_HOMEPAGE);

	DEFINE_METRIC1( IDM_O_ABT_NUMTHANKSLINES,                   4);


	////////////////////////////////////////////////////////////////////////////////////////////////
	//   Approach Items Options dialog box
	DEFINE_COMMENT("Approach Items Options dialog box");

	DEFINE_DLG_METRIC(IDD_APCHITMS);

	DEFINE_CTL_METRIC(IDL_APCHITMS_CUR);
	DEFINE_CTL_METRIC(IDL_APCHITMS_ALL);

	DEFINE_CTL_METRIC(IDC_APCHITMS_CUR);
	DEFINE_CTL_METRIC(IDC_APCHITMS_ALL);

	DEFINE_CTL_METRIC(IDC_APCHITMS_ADD);
	DEFINE_CTL_METRIC(IDC_APCHITMS_RMV);
	DEFINE_CTL_METRIC(IDC_APCHITMS_UP);
	DEFINE_CTL_METRIC(IDC_APCHITMS_DOWN);

	DEFINE_CTL_METRIC(IDC_APCHITMS_BRW);

	DEFINE_METRIC4( IDM_APCHITMS_OK,                          196, 154,  50,  14);
	DEFINE_METRIC4( IDM_APCHITMS_CANCEL,                      250, 154,  50,  14);


	thePartial = false;


#if _APPROACH_USE_REFERENCE_FILE != 0

	aSerializer.GenerateReferenceFile(0);

#endif	//_APPROACH_USE_REFERENCE_FILE != 0

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////////////////////////

void LocalizationImpl_EnUs_Hardcoded::DEFINE_STRING(LocIdKey theType, const char * theString)
{
	myStrings[theType] = StringAdvanced(theString);
}

//////////////////////////////////////////////////////////////////////////////////////////////

void LocalizationImpl_EnUs_Hardcoded::DEFINE_STRING(LocIdKey theType, const wchar_t * theString)
{
	myStrings[theType] = StringAdvanced(theString);
}

//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

class LocalizationImpl_Xml::Loader : public ComEntry1<ISAXContentHandler>
{
	//type definitions
protected:
	typedef const wchar_t * UniStr;

	typedef std::vector<std::wstring>      TagDepthStack;
	typedef TagDepthStack::iterator        TagDepthIter;
	typedef TagDepthStack::const_iterator  TagDepthIterC;

	struct Attribute
	{
		UniStr Name;
		std::wstring Value;

		Attribute (UniStr theName) : Name(theName) {}
	};

	typedef std::vector<Attribute>         AttributeVect;
	typedef AttributeVect::iterator        AttributeIter;
	typedef AttributeVect::const_iterator  AttributeIterC;

	enum EntityType
	{
		ENTITY_UNKNOWN,
		ENTITY_NOT_APPLICABLE,

		ENTITY_STRING,
		ENTITY_METRIC,

		ENTITY_LOCALE,
		ENTITY_CREATEDBY,
		ENTITY_TITLE,
		ENTITY_COMMENT
	};

public:
	Loader() : myCurMetricFactor(0), myCurEntityType(ENTITY_UNKNOWN), mySet(0)
	{ }

public:
	HRESULT LoadLocalization(bool thePartial, const TCHAR * theFilePath,
		int theSize_FilePath, LocalizationImpl_Xml * theOut)
	{
		ISAXXMLReader * aRdr = 0;
		HRESULT aRes = DefaultInitialize(thePartial, theOut, &aRdr);

		if ( FAILED(aRes) )
			return aRes;

		aRes = aRdr->parseURL(theFilePath);

		aRdr->Release();

		if (aRes == 0x80001000 && thePartial)
			return S_OK;
		else
			return aRes;
	}


// ISAXContentHandler Members
protected:
	STDMETHODIMP putDocumentLocator(ISAXLocator * theVal)
	{ return S_OK; }


	STDMETHODIMP startPrefixMapping    ( UniStr thePrefix,    int theNumCharsPrefix,
		UniStr theUri,       int theNumCharsUri )
	{ return E_NOTIMPL; }


	STDMETHODIMP endPrefixMapping      ( UniStr thePrefix,    int theNumCharsPrefix )
	{ return E_NOTIMPL; }


	STDMETHODIMP ignorableWhitespace   ( UniStr theWhsp,      int theNumChars )
	{ return E_NOTIMPL; }


	STDMETHODIMP processingInstruction ( UniStr theTarget,    int theNumCharsTarget,
		UniStr theData,      int theNumCharsData )
	{ return E_NOTIMPL; }


	STDMETHODIMP skippedEntity         ( UniStr theName,      int heNumCharsName )
	{ return E_NOTIMPL; }


	STDMETHODIMP startDocument()
	{ return S_OK; }


	STDMETHODIMP endDocument()
	{ return S_OK; }



	virtual HRESULT __stdcall startElement          ( UniStr theUri,       int theNumCharsUri,
		UniStr theLocalName, int theNumCharsLocalName,
		UniStr theQualName,  int theNumCharsQualName,
		ISAXAttributes * theAttribs )
	{
#ifdef _DEBUG
		char aDebugBuf[1000];

		size_t aNum = wcstombs(aDebugBuf, theLocalName, theNumCharsLocalName);
		aDebugBuf[aNum] = 0;

		Trace(aDebugBuf);
		Trace("\n");
#endif	//_DEBUG

		wchar_t aBuf_Tag[1000];
		lstrcpynW(aBuf_Tag, theLocalName, theNumCharsLocalName+1);


#ifdef _DEBUG

		if ( !myStack.empty() )
		{
			const std::wstring & aCur = myStack.back();

			int aAlreadyContains = lstrcmp(aCur.c_str(), aBuf_Tag);

			ATLASSERT(aAlreadyContains != 0);
		}

#endif

		myStack.push_back(aBuf_Tag);

		AttributeVect aVect;

		EntityType aCandidateType = ENTITY_UNKNOWN;

		////////////////////////////// String tag //////////////////////////////////

		if ( lstrcmpW(theLocalName, L"String") == 0)
		{
			aVect.push_back( L"id" );
			aCandidateType = ENTITY_STRING;
		}

		////////////////////////////// Metric tag //////////////////////////////////

		else if ( lstrcmpW(theLocalName, L"Metric") == 0 )
		{
			aVect.push_back( L"id" );
			aVect.push_back( L"factor" );

			aCandidateType = ENTITY_METRIC;
		}

		////////////////////////////// Locale tag //////////////////////////////////

		else if ( lstrcmpW(theLocalName, L"Locale") == 0 )
		{
			aCandidateType = ENTITY_LOCALE;
		}

		//////////////////////////// Created_By tag ////////////////////////////////

		else if ( lstrcmpW(theLocalName, L"Created_By") == 0 )
		{
			aVect.push_back( L"locale");
			aCandidateType = ENTITY_CREATEDBY;
		}

		/////////////////////////////// Title tag //////////////////////////////////

		else if ( lstrcmpW(theLocalName, L"Title") == 0 )
		{
			aVect.push_back( L"locale");
			aCandidateType = ENTITY_TITLE;
		}

		////////////////////////////// Comment tag /////////////////////////////////

		else if ( lstrcmpW(theLocalName, L"Comment") == 0 )
		{
			aVect.push_back( L"locale");
			aCandidateType = ENTITY_COMMENT;
		}

		//////////////////////////// All other tags ////////////////////////////////

		else
			return S_OK;

		//////////////////////////// end tag switch ////////////////////////////////

		HRESULT aRes = FillAttributeVector(theAttribs, aVect);

		if (FAILED (aRes) )
			return aRes;

		myCurEntityType = aCandidateType;

		if      ( aCandidateType == ENTITY_METRIC)
		{
			myCurID           = aVect[0].Value;
			myCurMetricFactor = _wtoi( aVect[1].Value.c_str() );	//TODO: get rid of it
		}

		else if ( aCandidateType == ENTITY_STRING || aCandidateType == ENTITY_CREATEDBY ||
			aCandidateType == ENTITY_TITLE  || aCandidateType == ENTITY_COMMENT    )
		{
			myCurID = aVect[0].Value;
		}

		return S_OK;
	}


	STDMETHODIMP endElement ( UniStr theUri,       int theNumCharsUri,
		UniStr theLocalName, int theNumCharsLocalName,
		UniStr theQualName,  int theNumCharsQualName)
	{
#ifdef _DEBUG
		char aBuf[1000];

		size_t aNumChars = wcstombs(aBuf, theLocalName, theNumCharsLocalName);
		aBuf[aNumChars] = 0;

		Trace("/");
		Trace(aBuf);
		Trace("\n");
#endif	//_DEBUG

		wchar_t aBuf_Tag[1000];

		lstrcpynW(aBuf_Tag, theLocalName, theNumCharsLocalName+1);

		std::wstring & aStackTop = myStack.back();

		if (aStackTop != aBuf_Tag)
		{
			Trace("The end element does not correspond to the start element currently being processed\n");
			return E_FAIL;
		}

		myStack.pop_back();

		//reset the state
		myCurEntityType = ENTITY_UNKNOWN;
		myCurMetricFactor = 0;

		if (myIsBriefMode)	//see if we can abort the operation
			if ( lstrcmpW(theLocalName, L"Summary") == 0 )
				return 0x80001000; //some custom error

		return S_OK;
	}


	STDMETHODIMP characters ( UniStr theString,    int theNumChars )
	{
		if ( myCurEntityType == ENTITY_STRING)
			mySet->SetString( myCurID.c_str(), (int) myCurID.size(), theString, theNumChars);

		else if ( myCurEntityType == ENTITY_METRIC)
		{
			Metric aM;
			aM.Factor = myCurMetricFactor;

			if ( ParseMetric(theString, aM) )
				mySet->SetMetric( myCurID.c_str(), (int) myCurID.size(), aM);
		}

		else if ( myCurEntityType == ENTITY_LOCALE)
			mySet->SetLocale(theString, theNumChars);

		else if ( myCurEntityType == ENTITY_TITLE || myCurEntityType == ENTITY_COMMENT || myCurEntityType == ENTITY_CREATEDBY)
			mySet->SetSummaryString(myStack.back().c_str(), (int) myStack.back().size(),
			myCurID.c_str(), (int) myCurID.size(), theString, theNumChars);

		return S_OK;
	}


//internal members
protected:
	HRESULT FillAttributeVector(ISAXAttributes * theAttribs, AttributeVect & theVect)
	{
		if (theVect.size() == 0)
			return S_OK;

		int aLength = 0;
		HRESULT aRes = theAttribs->getLength(&aLength);

		if ( FAILED (aRes) )
			return aRes;

		if ( (size_t) aLength < theVect.size() )
			return E_FAIL;

		std::vector<bool> aMatchVector( theVect.size(), false );

		wchar_t aBuf[1000];

		for (int i = 0; i < aLength; i++)
		{
			int aNumChars = 0;
			const wchar_t * aTemp;

			aRes = theAttribs->getLocalName(i, &aTemp, &aNumChars);

			if (FAILED (aRes) || aNumChars == 0)
				return aRes;

			lstrcpynW(aBuf, aTemp, aNumChars + 1);

			for ( size_t j = 0; j < theVect.size(); j++)
			{
				if ( lstrcmpW(aBuf, theVect[j].Name) != 0) //the attribute id's do not match
					continue;

				aMatchVector[j] = true;

				aNumChars = 0;

				aRes = theAttribs->getValue(i, &aTemp, &aNumChars);

				if ( FAILED(aRes) || aNumChars == 0 )
					return E_FAIL;

				theVect[j].Value.assign(aTemp, aTemp+aNumChars);
				break;
			}
		}

		for ( size_t j = 0; j < theVect.size(); j++)
			if ( aMatchVector[j] == false)
				return E_FAIL;

		return S_OK;
	}

	HRESULT DefaultInitialize(bool thePartial, LocalizationImpl_Xml * theOut, ISAXXMLReader ** theRdr)
	{
#ifdef _UNICODE
		HRESULT aRes = CoCreateInstance
		(
			__uuidof(SAXXMLReader30),		//explicit version number
			NULL,
			CLSCTX_ALL,
			__uuidof(ISAXXMLReader),
			(void **)theRdr
			);

		if( FAILED(aRes) )
			return aRes;

		aRes = (*theRdr)->putContentHandler(this);

		if( FAILED(aRes) )
		{
			(*theRdr)->Release();
			return aRes;
		}

		myIsBriefMode = thePartial;
		mySet = theOut;

		return aRes;

#else
		Trace("Non-unicode version not implemented");
		return E_NOTIMPL;
#endif	//_UNICODE
	}

	static bool ParseMetric(UniStr theText, Metric & theOutMetric)
	{
		if (theOutMetric.Factor > Metric::MaxNumValues) theOutMetric.Factor = Metric::MaxNumValues;

		int aCurProcessedFactor = 0;
		const wchar_t * aPtr = theText;

		Metric::DataType aTempVal = 0, * aDest = &theOutMetric.Val1;
		bool aStartedProcessingTempVal = false;

		while ( aCurProcessedFactor < theOutMetric.Factor && *aPtr != 0)
		{
			if ( *aPtr >= L'0' && *aPtr <= L'9' )
			{
				aStartedProcessingTempVal = true;
				aTempVal *= 10;
				aTempVal += *aPtr - L'0';
			}
			else if (aStartedProcessingTempVal)
			{
				*aDest = aTempVal;
				aStartedProcessingTempVal = false;

				aTempVal = 0;
				aDest++;
				aCurProcessedFactor++;
			}

			aPtr++;
		}

		if (aStartedProcessingTempVal)
		{
			*aDest = aTempVal;
			aCurProcessedFactor++;
		}

		return aCurProcessedFactor == theOutMetric.Factor;
	}


protected:
	TagDepthStack myStack;
	std::wstring  myCurID;	//also stores locale id when processing the summary section
	int           myCurMetricFactor;
	EntityType    myCurEntityType;
	bool          myIsBriefMode;

	LocalizationImpl_Xml * mySet;
};

//////////////////////////////////////////////////////////////////////////////////////////////

HRESULT LocalizationImpl_Xml::Initialize(const TCHAR * theLocaleID, int theSize_LocaleID, bool & thePartial)
{
	TCHAR aPath[1000];
	HRESULT aRes = ApproachLocalizationManager::LocalizationFilePathFromLocaleID(aPath, 1000, theLocaleID, theSize_LocaleID);

	if ( FAILED(aRes) )
		return aRes;

	myTitles.clear();
	myComments.clear();
	myCreatedBys.clear();

	if ( !thePartial )
	{
		myStrings.clear();
		myMetrics.clear();
	}

	ComInstance<Loader> aLoader;
	return aLoader.LoadLocalization( thePartial, aPath, 1000, this);
}

//////////////////////////////////////////////////////////////////////////////////////////////

void LocalizationImpl_Xml::SetString(const TCHAR * theID, int theSize_ID,
                                     const TCHAR * theString, int theSize_String)
{
	LocIdKey aID = StringUtil::ComputeHash(theID, theSize_ID);

#ifdef _DEBUG

#  ifdef _UNICODE
	char aBuf[1000];
	ZeroMemory(aBuf, 1000 * sizeof(char) );

	WideCharToMultiByte(CP_ACP, 0, theID, theSize_ID, aBuf, 1000, NULL, NULL);
#  else
	const char * aBuf = aID.c_str();
#  endif

	Trace("ApproachLocalization::SetString. ID=[%s]\n", aBuf);

#endif



	if ( myStrings.find(aID) == myStrings.end() )
	{
		myStrings[aID] = String(theString, theString + theSize_String);
	}
	else
	{
		myStrings[aID] = myStrings[aID] + String(theString, theString + theSize_String);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////

void LocalizationImpl_Xml::SetMetric(const TCHAR * theID, int theSize_ID, const Metric & theSource)
{
	Trace("ApproachLocalization::SetMetric\n");

	LocIdKey aID = StringUtil::ComputeHash(theID, theSize_ID);

	myMetrics[aID] = theSource;
}

//////////////////////////////////////////////////////////////////////////////////////////////

void LocalizationImpl_Xml::SetSummaryString(const TCHAR * theName, int theSize_Name,
	const TCHAR * theLocale, int theSize_Locale, const TCHAR * theVal, int theSize_Val)
{
	AuxMap * aMap = 0;

	if ( lstrcmp(theName, _T("Title") ) == 0)
		aMap = & myTitles;

	else if ( lstrcmp(theName, _T("Created_By") ) == 0)
		aMap = &myCreatedBys;

	else if ( lstrcmp(theName, _T("Comment") ) == 0)
		aMap = &myComments;

	else
		return;


	IdType aID (theLocale,  theLocale + theSize_Locale);
	String aVal (theVal, theVal + theSize_Val);

	(*aMap)[aID] = aVal;
}

//////////////////////////////////////////////////////////////////////////////////////////////

void LocalizationImpl_Xml::SetLocale(const TCHAR * theLocale, int theSize)
{
	myLocale.assign(theLocale, theLocale + theSize);
}

//////////////////////////////////////////////////////////////////////////////////////////////

HRESULT LocalizationImpl_Xml::GetLocale( TCHAR * theOut, int * theSize )
{
	if ( theSize == 0 || theOut == 0)
		return E_INVALIDARG;

	size_t aLength = myLocale.size();

	if ( (size_t) *theSize < aLength+1 )
		return E_OUTOFMEMORY;

	lstrcpy(theOut, myLocale.c_str() );
	theOut[aLength] = 0;

	*theSize = (int) aLength;

	return S_OK;
}
#include "stdafx.h"

#include "sdk_ComObject.h"
#include "sdk_Version.h"

#include "UpdateCheck.h"
#include "HttpRequest.h"
#include "Application.h"
#include "Preferences.h"
#include "Utility.h"

//////////////////////////////////////////////////////////////////////////////////////////////

//! Requests XML data from KO Approach Web Site and determines whether
//! an update to the current version of the program is available.
class XmlHandler : public ComEntry1<ISAXContentHandler>
{
// Type definitions
protected:
	typedef const wchar_t * UniStr;
	typedef ProductVersion<unsigned int> Ver;


// Data
protected:
	std::wstring myVersion;
	std::wstring myUrl;

	Ver myCurrent;
	Ver myAvailable;


// Constructor
public:
	XmlHandler() : myCurrent(Ver::VERSION_CURRENT)
	{ }


// Interface
public:

	//! Requests update XML from the specified Web resource and determines
	//! if the current version is outdated (less than the version received
	//! from the Web resource.
	//! return     True if the program is outdated; otherwise, false;
	bool HandleUpdates()
	{
#ifdef _UNICODE
		if (myCurrent.GetMajor() == 0 && myCurrent.GetMinor() == 0)
			return false;

#ifdef _APPROACH_LOCAL_TEST
		const TCHAR * aUrlString = _T("http://localhost:1699/Products/Approach/update_checker.ashx");
#else
		//FLAW: class coupling
		const TCHAR * aUrlString = Application::InstanceC().Prefs()->GetUrl(ApplicationSettings::URL_UPDATE);
#endif

		size_t aUrlLen = lstrlen(aUrlString);

		const HttpUrl aUrl(aUrlString, aUrlLen);
		HttpRequest aRequest(aUrl);

		if ( !aRequest.Open() )
			return false;

		TCHAR aGuidData[37];
		StringUtil::GuidToString(&GUID_ApproachVersion_V1, aGuidData, 37);

		TCHAR aFormatted[256];
		_stprintf
			(
				aFormatted,
				_T("%d.%d.%d.%d"),
				myCurrent.GetMajor(),
				myCurrent.GetMinor(),
				myCurrent.GetRelease(),
				myCurrent.GetRevision()
			);

		//myCurrent.ToString(aFormatted, 256);

		//add headers to the request
		aRequest.AddHeader( _T("Engine"),     aGuidData  );
		aRequest.AddHeader( _T("CurVersion"), aFormatted );

#ifdef _WIN64
		aRequest.AddHeader( _T("Architecture"), _T("x64") );
#else
		aRequest.AddHeader( _T("Architecture"), _T("x86") );
#endif // _WIN64

		if ( !aRequest.Send() )
			return false;

		char aOutput[5000];
		wchar_t aConverted[5000];
		DWORD aSize = 0;

		std::wstring aOut;

		while(true)
		{
			aSize = sizeof aOutput-1;			//leave one for the null terminator
			bool aRes = aRequest.Read(aOutput, &aSize);

			if (!aRes || aSize == 0)
				break;

			aOutput[aSize] = 0;

			//BUG: this not correct for UTF-8 as the string may end with the control sequence
			int aConvSize = MultiByteToWideChar(CP_ACP, 0, aOutput, aSize, aConverted, 5000);
			aConverted[aConvSize] = 0;

			aOut += aConverted;
		}

		if (aOut.size() == 0)
			return false;

		VARIANT aVar;

		wchar_t * aNewString = new wchar_t[ aOut.size()+1 ];

		lstrcpyn(aNewString, aOut.c_str(), (int) aOut.size()+1);

		aVar.bstrVal = aNewString;
		aVar.vt = VT_BSTR;


		// Now that we have the UpdateHandler's response, proceed to reading
		// the actual version information

		bool aRetVal = false;

		CComPtr<ISAXXMLReader> aRdr;
		HRESULT aRes = aRdr.CoCreateInstance( __uuidof(SAXXMLReader30) );

		if( SUCCEEDED(aRes) )
		{
			aRes = aRdr->putContentHandler(this);

			if( SUCCEEDED(aRes) )
				aRes = aRdr->parse(aVar);     // Perform actual SAX reading

			if( SUCCEEDED(aRes) )
				aRetVal = myAvailable > myCurrent;
		}

		delete [] aNewString;
		return aRetVal;

#else
		Trace("Non-unicode version not implemented");
		return false;
#endif	//_UNICODE
	}

	bool GetUpToDateVersion(TCHAR * theOut, size_t & theSize)
	{
		if ( theSize < myVersion.size()+1 )
		{
			theSize = myVersion.size()+1;
			return false;
		}
		else
		{
			theSize = myVersion.size()+1;
			lstrcpyn(theOut, myVersion.c_str(), (int)myVersion.size()+1);
			return true;
		}
	}

	bool GetUpToDateDownloadUrl(TCHAR * theOut, size_t & theSize)
	{
		if ( theSize < myUrl.size()+1 )
		{
			theSize = myUrl.size()+1;
			return false;
		}
		else
		{
			theSize = myUrl.size()+1;
			lstrcpyn(theOut, myUrl.c_str(), (int)myUrl.size()+1);
			return true;
		}
	}


// ISAXContentHandler members
protected:
	STDMETHODIMP putDocumentLocator(ISAXLocator * /*theVal*/)
	{ return S_OK; }


	STDMETHODIMP startPrefixMapping    ( UniStr /*thePrefix*/,    int /*theNumCharsPrefix*/,
		UniStr theUri,       int theNumCharsUri )
	{ return E_NOTIMPL; }


	STDMETHODIMP endPrefixMapping      ( UniStr /*thePrefix*/,    int /*theNumCharsPrefix*/ )
	{ return E_NOTIMPL; }


	STDMETHODIMP ignorableWhitespace   ( UniStr /*theWhsp*/,      int /*theNumChars*/ )
	{ return E_NOTIMPL; }


	STDMETHODIMP processingInstruction ( UniStr /*theTarget*/,    int /*theNumCharsTarget*/,
		UniStr /*theData*/,      int /*theNumCharsData*/ )
	{ return E_NOTIMPL; }


	STDMETHODIMP skippedEntity         ( UniStr /*theName*/,      int /*theNumCharsName*/ )
	{ return E_NOTIMPL; }


	STDMETHODIMP startDocument()
	{ return S_OK; }


	STDMETHODIMP endDocument()
	{ return S_OK; }


	STDMETHODIMP startElement          ( UniStr theUri,       int theNumCharsUri,
		UniStr theLocalName, int theNumCharsLocalName,
		UniStr theQualName,  int theNumCharsQualName,
		ISAXAttributes * theAttribs )
	{
		if ( lstrcmpW(theLocalName, L"update") != 0)
			return S_OK;

		int aLength = 0;
		HRESULT aRes = theAttribs->getLength(&aLength);

		if ( FAILED (aRes) )
			return aRes;

		for (int i = 0; i < aLength; i++)
		{
			int aNumChars = 0;
			const wchar_t * aTemp;	// the pointer to a SAX's internal buffer

			std::wstring aTempTemp;

			aRes = theAttribs->getLocalName(i, &aTemp, &aNumChars);

			if (FAILED (aRes) || aNumChars == 0)
				return aRes;

			aTempTemp.assign(aTemp, aTemp+aNumChars);

			if ( aTempTemp == L"friendly_version" )
			{
				aRes = theAttribs->getValue(i, &aTemp, &aNumChars);

				if (FAILED (aRes) || aNumChars == 0)
					return aRes;

				myVersion.assign(aTemp, aTemp+aNumChars);
			}
			else if ( aTempTemp == L"url" )
			{
				aRes = theAttribs->getValue(i, &aTemp, &aNumChars);

				if (FAILED (aRes) || aNumChars == 0)
					return aRes;

				myUrl.assign(aTemp, aTemp+aNumChars);
			}

			else if ( aTempTemp == L"version" )
			{
				aRes = theAttribs->getValue(i, &aTemp, &aNumChars);

				myAvailable.Set(aTemp, aNumChars);

				if
					(
					myAvailable.GetMajor() == 0 &&
					myAvailable.GetMinor() == 0
					)
					return E_FAIL;
			}
		}

		return S_OK;
	}

	STDMETHODIMP endElement            ( UniStr /*theUri*/,       int /*theNumCharsUri*/,
		UniStr /*theLocalName*/, int /*theNumCharsLocalName*/,
		UniStr /*theQualName*/,  int /*theNumCharsQualName*/)
	{ return S_OK; }


	STDMETHODIMP characters            ( UniStr /*theString*/,    int /*theNumChars*/ )
	{ return S_OK; }
};

//////////////////////////////////////////////////////////////////////////////////////////////

UpdateCheckTask::UpdateCheckTask() : myHandler(0), myData(0), myStatus(0L)
{ }

//////////////////////////////////////////////////////////////////////////////////////////////

UpdateCheckTask::~UpdateCheckTask()
{
	Terminate();

	DWORD aWaitResult = WaitForSingleObject(myHandle, 5000);	//wait 5 seconds

	if (aWaitResult != WAIT_OBJECT_0)
		TerminateThread(myHandle, 1);
}

//////////////////////////////////////////////////////////////////////////////////////////////

void UpdateCheckTask::HandleUpdates(IAsyncUpdateCheckResultHandler * theHandler, UpdateCheckData * theData)
{
	myHandler = theHandler;
	myData = theData;

	Thread::Run(this);
}

//////////////////////////////////////////////////////////////////////////////////////////////

void UpdateCheckTask::Terminate()
{
	InterlockedExchange(&myStatus, 1L);
}

//////////////////////////////////////////////////////////////////////////////////////////////

bool UpdateCheckTask::GetUpdateData(TCHAR * theVersion, int & theSizeVersion, TCHAR * theUrl, int & theSizeUrl) const
{
	return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////

bool UpdateCheckTask::ShouldPerformUpdateCheck()
{
	HKEY aApproachKey = ApplicationSettings::EnsureRegistryKey(HKEY_CURRENT_USER, KEY_READ);

	if (aApproachKey == NULL)
		return false;


	FILETIME aLastUpdateCheckDate;
	DWORD aSize = sizeof FILETIME;
	LONG aRes = RegQueryValueEx( aApproachKey, _T("LastUpdateCheck"), 0, NULL, (LPBYTE) &aLastUpdateCheckDate, &aSize);

	bool aRetVal = true;

	if (aRes == ERROR_SUCCESS)
	{
		FILETIME aCurTime;
		GetSystemTimeAsFileTime(&aCurTime);

		LARGE_INTEGER aLastUpdateCheckDateCalc, aCurTimeCalc;

		aLastUpdateCheckDateCalc.HighPart = aLastUpdateCheckDate.dwHighDateTime;
		aLastUpdateCheckDateCalc.LowPart = aLastUpdateCheckDate.dwLowDateTime;

		aCurTimeCalc.HighPart = aCurTime.dwHighDateTime;
		aCurTimeCalc.LowPart = aCurTime.dwLowDateTime;

		LARGE_INTEGER aDiff;
		aDiff.QuadPart = aCurTimeCalc.QuadPart - aLastUpdateCheckDateCalc.QuadPart;

		const static LONGLONG HundredNanosInWeek = (LONGLONG) 10/* 1mcs */ * 1000 /* 1ms */ * 1000 /* 1s */ * 3600 /* 1h */ * 24 /* 1d */ * 7 /* 1w */;

		if (aDiff.QuadPart < HundredNanosInWeek)
		{
			aRetVal = false;
			Trace("UpdateCheckTask -- The current update check time difference is less than 1 week; update check should not be performed.\n");
		}
		else
			Trace("UpdateCheckTask -- The current update check time difference is greater than 1 week; update check SHOULD be performed.\n");
	}
	else
		Trace("UpdateCheckTask -- Last update check date not found; update check SHOULD be performed.\n");

	RegCloseKey(aApproachKey);

	return aRetVal;
}

//////////////////////////////////////////////////////////////////////////////////////////////

void UpdateCheckTask::SaveUpdateCheckResult()
{
	HKEY aApproachKey = ApplicationSettings::EnsureRegistryKey(HKEY_CURRENT_USER, KEY_WRITE);

	if (aApproachKey == NULL)
		return;

	FILETIME aCurTime;
	GetSystemTimeAsFileTime(&aCurTime);

	LONG aRes = RegSetValueEx(aApproachKey, _T("LastUpdateCheck"), 0, REG_QWORD, (const LPBYTE) &aCurTime, sizeof FILETIME);

	Trace("RootWindow -- Saved the last update check date %x%x with the status of %d\n",
		aCurTime.dwHighDateTime, aCurTime.dwLowDateTime, aRes);

	RegCloseKey(aApproachKey);
}

//////////////////////////////////////////////////////////////////////////////////////////////

DWORD UpdateCheckTask::Run()
{
	CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

	ComInstance<XmlHandler> aHandler;

	bool aHasUpdates = aHandler.HandleUpdates(); // this is the slow operation

	if (myStatus != 0L)
		return 0;

	if (aHasUpdates)
	{
		aHasUpdates &= aHandler.GetUpToDateDownloadUrl(myData->Url(), myData->UrlSize() );
		aHasUpdates &= aHandler.GetUpToDateVersion(myData->Version(), myData->VersionSize() );

		myData->HasUpdates(aHasUpdates);
	}

	if (myStatus == 0L)
		myHandler->HandleUpdateCheckResult(this);

	CoUninitialize();

	return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

UpdateCheckPackage::UpdateCheckPackage() : myUpdateCheckTask(0), myUpdateCheckData(0)
{ }

//////////////////////////////////////////////////////////////////////////////////////////////

UpdateCheckPackage::~UpdateCheckPackage()
{
	DestroyUpdateCheckTask();
	DestroyUpdateCheckData();
}

//////////////////////////////////////////////////////////////////////////////////////////////

void UpdateCheckPackage::HandleUpdates( IAsyncUpdateCheckResultHandler * theHandler )
{
	EnsureUpdateCheckTask();
	EnsureUpdateCheckData();

	myUpdateCheckTask->HandleUpdates(theHandler, myUpdateCheckData);
}

//////////////////////////////////////////////////////////////////////////////////////////////

const UpdateCheckData & UpdateCheckPackage::GetData() const
{
	return *myUpdateCheckData;
}

//////////////////////////////////////////////////////////////////////////////////////////////

void UpdateCheckPackage::EnsureUpdateCheckTask()
{
	DestroyUpdateCheckTask();
	myUpdateCheckTask = new UpdateCheckTask();
}

//////////////////////////////////////////////////////////////////////////////////////////////

void UpdateCheckPackage::EnsureUpdateCheckData()
{
	if (myUpdateCheckData == 0)
		myUpdateCheckData = new UpdateCheckData();
	else
		myUpdateCheckData->Reset();
}

//////////////////////////////////////////////////////////////////////////////////////////////

void UpdateCheckPackage::DestroyUpdateCheckTask()
{
	if (myUpdateCheckTask != 0)
	{
		delete myUpdateCheckTask;
		myUpdateCheckTask = 0;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////

void UpdateCheckPackage::DestroyUpdateCheckData()
{
	if (myUpdateCheckData != 0)
	{
		delete myUpdateCheckData;
		myUpdateCheckData = 0;
	}
}

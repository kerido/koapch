#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////

class HttpRequestHeader
{
	TCHAR * myName,   * myValue,    * myFull;
	int     myLen_Name, myLen_Value,  myLen_Full;

	static const TCHAR * Separator() { return _T(": "); }

public:
	HttpRequestHeader(const TCHAR * theName, int theLen_Name, const TCHAR * theValue, int theLen_Value)
		: myLen_Name(theLen_Name), myLen_Value(theLen_Value), myLen_Full(myLen_Name + 2 + myLen_Value)
	{
		Allocate(theName, theValue);
	}

	HttpRequestHeader(const HttpRequestHeader & theHeader)
		: myLen_Name(theHeader.myLen_Name), myLen_Value(theHeader.myLen_Value), myLen_Full(theHeader.myLen_Full)
	{
		Allocate(theHeader.myName, theHeader.myValue);
	}

	void Allocate(const TCHAR * theName, const TCHAR * theValue)
	{
		myName  = new TCHAR[myLen_Name  + 1];
		myValue = new TCHAR[myLen_Value + 1];
		myFull  = new TCHAR[myLen_Full  + 1];

		lstrcpyn(myName, theName, myLen_Name+1);
		lstrcpyn(myValue, theValue, myLen_Value+1);

		lstrcpyn(myFull, theName, myLen_Name+1);
		lstrcat(myFull, Separator() );
		lstrcat(myFull, myValue);
	}

	~HttpRequestHeader()
	{
		delete [] myName;
		delete [] myValue;
		delete [] myFull;
	}

public:
	operator const TCHAR * () const { return myFull; }

public:
	int GetLength() const { return myLen_Full; }

};

//////////////////////////////////////////////////////////////////////////////////////////////

//! Represents an HTTP Universal Resource Locator
class HttpUrl
{
private:
	TCHAR * myHost;
	TCHAR * myResource;
	int myPort;
	bool myIsHttps;

public:
	HttpUrl(const TCHAR * theUrl, size_t theLength)
		: myHost(0), myResource(0), myPort(0), myIsHttps(false)
	{
		////////////////////////////////////////////////////////////////
		// 1. Parsing protocol
		const TCHAR * aSite = _tcsstr(theUrl, _T("://") );

		if (aSite == 0)
			aSite = theUrl;
		else              // there is the actual protocol specifier
		{
			// the character before the :// is either 'p' or 's' as in http:// or https://
			TCHAR aHttpsEnd = *(aSite-1);

			if ( aHttpsEnd == _T('s') || aHttpsEnd == _T('S') )
				myIsHttps = true;

			aSite += 3;	// the length of the '://' prefix
		}

		const TCHAR * aPortSep     = _tcschr(aSite, _T(':') );
		const TCHAR * aResourceSep = _tcschr(aSite, _T('/') );

		size_t aSiteLength = theLength - (aSite - theUrl);  // the length of the host part (default)

		////////////////////////////////////////////////////////////////
		// 2. Parse port
		if (aPortSep != 0)	//there is an actual port specifier
		{
			aSiteLength = aPortSep - aSite;	// exclude the colon character from site

			while(true)
			{
				aPortSep++;

				if (*aPortSep >= _T('0') && *aPortSep <= _T('9') )
				{
					myPort *= 10;
					myPort += (int) (*aPortSep - _T('0') );
				}
				else
					break;
			}
		}

		if (myPort == 0)
			myPort = myIsHttps ? INTERNET_DEFAULT_HTTPS_PORT : INTERNET_DEFAULT_HTTP_PORT;

		
		////////////////////////////////////////////////////////////////
		// 2. Parse path and set both host and path
		if (aResourceSep == 0)
		{
			SetHost(aSite,   aSiteLength);
			SetPath(_T("/"), 1);          // manually add '/', as the caller is requesting the root
		}
		else
		{
			if (aPortSep == 0)
				aSiteLength = aResourceSep - aSite;	// exclude the slash character from site

			SetHost(aSite, aSiteLength);
			SetPath(aResourceSep, theLength - (aResourceSep - theUrl) );
		}
	}

	~HttpUrl()
	{
		ReleaseHost();
		ReleasePath();
	}

// Properties
public:

	const TCHAR * GetHost() const
		{ return myHost; }

	const TCHAR * GetPath() const
		{ return myResource; }

	int GetPort() const
		{ return myPort; }

	void SetHost(const TCHAR * theHost, size_t theLength)
	{
		ReleaseHost();

		myHost = new TCHAR[theLength+1];
		lstrcpyn(myHost, theHost, (int)theLength+1);    // one extra character for the NULL terminator
	}

	void SetPath(const TCHAR * thePath, size_t theLength)
	{
		ReleasePath();

		myResource = new TCHAR[theLength+1];
		lstrcpyn(myResource, thePath, (int)theLength+1); // one extra character for the NULL terminator
	}

	void SetPort(int thePort)
		{ myPort = thePort; }

	bool IsHttps() const
		{ return myIsHttps; }

private:
	void ReleaseHost()
	{
		if (myHost != 0)
		{
			delete [] myHost;
			myHost = 0;
		}
	}

	void ReleasePath()
	{
		if (myResource != 0)
		{
			delete [] myResource;
			myResource = 0;
		}
	}
};

//////////////////////////////////////////////////////////////////////////////////////////////

class HttpRequest
{
private:
	typedef std::list<HttpRequestHeader> HeaderList;
	typedef HeaderList::iterator         HeaderIter;
	typedef HeaderList::const_iterator   HeaderIterC;

// Fields
private:
	const HttpUrl & myUrl;           //!< The URL which is being requested
	HeaderList myHeaders;            //!< The headers applied to the request
	int myValidCode;                 //!< Zero if everything is OK
	
	HINTERNET Internet;
	HINTERNET Connection;
	HINTERNET Request;


// Constructors, Destructor
public:
	HttpRequest(const HttpUrl & theUrl)
		: myUrl(theUrl), myValidCode(0), Internet(0), Connection(0), Request(0)
	{ }

	~HttpRequest()
	{
		if (Request    != 0) InternetCloseHandle(Request);
		if (Connection != 0) InternetCloseHandle(Connection);
		if (Internet   != 0) InternetCloseHandle(Internet);
	}


public:
	void AddHeader(const TCHAR * theName, const TCHAR * theValue)
	{
		if (myValidCode != 0)
			return;

		if (theValue == 0)
			return;

		int aValueLen = lstrlen(theValue);

		if (aValueLen == 0)
			return;

		myHeaders.push_back( HttpRequestHeader(theName, lstrlen(theName), theValue, aValueLen) );
	}

	bool Open()
	{
		if (myValidCode != 0)
			return false;

		Internet = InternetOpen
		(
			_T("KO Approach"),
			INTERNET_OPEN_TYPE_PRECONFIG,
			NULL,
			NULL,
			0
		);

		if (Internet == NULL)
			return false;

		Connection = InternetConnect
		(
			Internet,
			myUrl.GetHost(),
			(INTERNET_PORT) myUrl.GetPort(),
			NULL,                           // user name
			NULL,                           // password
			INTERNET_SERVICE_HTTP,
			NULL,
			NULL
		);

		if (Connection == NULL)
			return false;

		DWORD aFlags = INTERNET_FLAG_KEEP_CONNECTION|INTERNET_FLAG_DONT_CACHE;

		if ( myUrl.IsHttps() )
			aFlags |= INTERNET_FLAG_SECURE;

		Request = HttpOpenRequest
		(
			Connection,       // connection
			NULL,             // method, use default
			myUrl.GetPath(),  // path within host
			NULL,             // HTTP version string, HTTP/1.1 by default
			NULL,             // referrer
			NULL,             // accept types, TODO: check for compatibility
			aFlags,
			NULL              // pointer to the context
		);

		if (Request == NULL)
			return false;

		return true;
	}


	bool Send()
	{
		if (myValidCode != 0)
			return false;

		BOOL aRes = TRUE;

		// headers
		for (HeaderIterC aIt = myHeaders.begin(); aIt != myHeaders.end(); aIt++)
		{
			const TCHAR * aHdr = aIt->operator const TCHAR *();

			BOOL aRes = HttpAddRequestHeaders(Request, aHdr, aIt->GetLength(),  NULL);

			if ( aRes != TRUE)
				return false;
		}

		//do the query
		aRes = HttpSendRequest(Request, NULL, 0, NULL, 0);

		if (aRes != TRUE)
			return false;	//unable to connect to the Registration service

		//Read output headers
		
		char aOutput[5000];
		DWORD aNumRead = 0;
		DWORD aBufLength = sizeof aOutput;


		aRes = HttpQueryInfoA
		(
			Request,
			HTTP_QUERY_RAW_HEADERS_CRLF,
			aOutput,
			&aBufLength,
			&aNumRead
		);

		if ( aRes != TRUE )
			return false;

		aOutput[aBufLength] = 0;

		char * aFound = strstr(aOutput, "200 OK");
		if (aFound == 0 || aFound > aOutput+20)
			return false;

		return true;
	}


	bool Read(char * theOut, DWORD * theNumRead)
	{
		if (theNumRead == 0)
			return false;

		if (myValidCode != 0)
			return false;

		BOOL aRes = InternetReadFile(Request, theOut, *theNumRead, theNumRead);

		return (aRes == TRUE);
	}
};
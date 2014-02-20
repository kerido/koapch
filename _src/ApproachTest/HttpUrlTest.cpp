#include "stdafx.h"

#include "HttpRequest.h"

//////////////////////////////////////////////////////////////////////////////////////////////

using namespace System;

using namespace Microsoft::VisualStudio::TestTools;
using namespace Microsoft::VisualStudio::TestTools::UnitTesting;

//////////////////////////////////////////////////////////////////////////////////////////////

namespace ApproachTest
{
	[TestClass]
	public ref class HttpUrlTest
	{
	public:

		[TestMethod]
		void Test_HttpUrlParsing()
		{
			// urls with no or root path and default port
			LPTSTR aUrls1[] = 
			{
				_T("www.google.com"),
				_T("http://www.google.com"),
				_T("www.google.com/"),
				_T("http://www.google.com/"),
			};

			for (int i = 0; i < 4; i++)
			{
				HttpUrl aUrl(aUrls1[i], lstrlen(aUrls1[i]) );

				Assert::IsTrue( lstrcmp(aUrl.GetHost(), _T("www.google.com") ) == 0 );
				Assert::IsTrue( lstrcmp(aUrl.GetPath(), _T("/") )              == 0 );
				Assert::IsTrue( aUrl.GetPort() == 80 );
			}

			// urls with no or root path and designated port
			LPTSTR aUrls2[] = 
			{
				_T("www.google.com:121"),
				_T("http://www.google.com:121"),
				_T("www.google.com:121/"),
				_T("http://www.google.com:121/"),
			};

			for (int i = 0; i < 4; i++)
			{
				HttpUrl aUrl(aUrls2[i], lstrlen(aUrls2[i]) );

				Assert::IsTrue( lstrcmp(aUrl.GetHost(), _T("www.google.com") ) == 0 );
				Assert::IsTrue( lstrcmp(aUrl.GetPath(), _T("/") )              == 0 );
				Assert::IsTrue( aUrl.GetPort() == 121 );
			}

			// urls with non-root path and default port
			LPTSTR aUrls3[] = 
			{
				_T("www.google.com/search.html"),
				_T("http://www.google.com/search.html"),
			};

			for (int i = 0; i < 2; i++)
			{
				HttpUrl aUrl(aUrls3[i], lstrlen(aUrls3[i]) );

				Assert::IsTrue( lstrcmp(aUrl.GetHost(), _T("www.google.com") ) == 0 );
				Assert::IsTrue( lstrcmp(aUrl.GetPath(), _T("/search.html") )   == 0 );
				Assert::IsTrue( aUrl.GetPort() == 80 );
			}

			// urls with non-root path and designated port
			LPTSTR aUrls4[] = 
			{
				_T("www.google.com:121/search.html"),
				_T("http://www.google.com:121/search.html"),
			};

			for (int i = 0; i < 2; i++)
			{
				HttpUrl aUrl(aUrls4[i], lstrlen(aUrls4[i]) );

				Assert::IsTrue( lstrcmp(aUrl.GetHost(), _T("www.google.com") ) == 0 );
				Assert::IsTrue( lstrcmp(aUrl.GetPath(), _T("/search.html") )   == 0 );
				Assert::IsTrue( aUrl.GetPort() == 121 );
			}
		}
		
		IMPLEMENT_TEST_CLASS;
	};
}

#include "stdafx.h"

#include "FilePathProvider.h"
#include "util_Process.h"

//////////////////////////////////////////////////////////////////////////////////////////////

using namespace System;

using namespace Microsoft::VisualStudio::TestTools;
using namespace Microsoft::VisualStudio::TestTools::UnitTesting;

//////////////////////////////////////////////////////////////////////////////////////////////

namespace ApproachTest
{
	[TestClass]
	public ref class FilePathProviderTest
	{

	public:

		[TestMethod]
		void Test_GetPath()
		{
			HMODULE aMod = ProcessUtility::GetCurProcessModule();
			FilePathProvider * aPrv = new FilePathProvider(aMod);


			TCHAR aPath[MAX_PATH];


			// X. Provide incorrect arguments
			HRESULT aRes = aPrv->GetPath(FilePathProvider::Dir_Main, 0, 0);
			Assert::IsTrue( FAILED(aRes) );

			// X. Provide not enough space
			int aSize = 0;
			aRes = aPrv->GetPath(FilePathProvider::Dir_Locale, aPath, &aSize);
			Assert::IsTrue( FAILED(aRes) );

			// 1. Provide correct arguments
			aSize = MAX_PATH;
			aRes = aPrv->GetPath(FilePathProvider::Dir_Main, aPath, &aSize);
			Assert::IsTrue( SUCCEEDED(aRes) );

			TCHAR aLast = aPath[aSize-1];
			Assert::IsTrue( aLast != _T('\\'), "The directory path must not end with a backslash" );

			Assert::IsTrue( aSize == lstrlen(aPath) );

			// 2. Provide correct arguments
			aSize = MAX_PATH;
			aRes = aPrv->GetPath(FilePathProvider::Dir_Locale, aPath, &aSize);
			Assert::IsTrue( SUCCEEDED(aRes) );

			aLast = aPath[aSize-1];
			Assert::IsTrue( aLast != _T('\\'), "The directory path must not end with a backslash" );

			Assert::IsTrue( aSize == lstrlen(aPath) );

			// 3. Provide correct arguments
			aSize = MAX_PATH;
			aRes = aPrv->GetPath(FilePathProvider::Dir_Plugins, aPath, &aSize);
			Assert::IsTrue( SUCCEEDED(aRes) );

			aLast = aPath[aSize-1];
			Assert::IsTrue( aLast != _T('\\'), "The directory path must not end with a backslash" );

			Assert::IsTrue( aSize == lstrlen(aPath) );

			// 4. Provide correct arguments
			aSize = MAX_PATH;
			aRes = aPrv->GetPath(FilePathProvider::File_MainModule, aPath, &aSize);
			Assert::IsTrue( SUCCEEDED(aRes) );

			Assert::IsTrue( aSize == lstrlen(aPath) );

			// 5. Provide correct arguments
			aSize = MAX_PATH;
			aRes = aPrv->GetPath(FilePathProvider::File_IpcModule, aPath, &aSize);
			Assert::IsTrue( SUCCEEDED(aRes) );

			Assert::IsTrue( aSize == lstrlen(aPath) );

			// 6. Provide correct arguments
			aSize = MAX_PATH;
			aRes = aPrv->GetPath(FilePathProvider::File_HtmlHelp, aPath, &aSize);
			Assert::IsTrue( SUCCEEDED(aRes) );

			Assert::IsTrue( aSize == lstrlen(aPath) );

			// 7. Provide correct arguments
			aSize = MAX_PATH;
			aRes = aPrv->GetPath(FilePathProvider::File_Key, aPath, &aSize);
			Assert::IsTrue( SUCCEEDED(aRes) );

			Assert::IsTrue( aSize == lstrlen(aPath) );

			// 8. Provide correct arguments
			aSize = MAX_PATH;
			aRes = aPrv->GetPath(FilePathProvider::File_ReleaseTrace, aPath, &aSize);
			Assert::IsTrue( SUCCEEDED(aRes) );


			// 9. Test the temp key
			aSize = MAX_PATH;
			aRes = aPrv->GetPath(FilePathProvider::File_TempKey, aPath, &aSize);
			Assert::IsTrue( SUCCEEDED(aRes) );

			Assert::IsTrue( aSize == lstrlen(aPath) );



			delete aPrv;
		}


		IMPLEMENT_TEST_CLASS;
	};
}

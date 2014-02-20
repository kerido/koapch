#include "stdafx.h"

#include "sdk_Version.h"
#include "util_Process.h"

//////////////////////////////////////////////////////////////////////////////////////////////

using namespace System;

using namespace Microsoft::VisualStudio::TestTools;
using namespace Microsoft::VisualStudio::TestTools::UnitTesting;

//////////////////////////////////////////////////////////////////////////////////////////////

namespace ApproachTest
{
	[TestClass]
	public ref class VersionTest
	{
		typedef ProductVersion<unsigned int> Ver;

	public:
		[TestMethod]
		void Test_Operators()
		{
			// 1.
			Ver aVer1000(1,0,0,0);
			Ver aVer2000(2,0,0,0);

			Assert::IsTrue(aVer2000 > aVer1000);
			Assert::IsTrue(aVer1000 < aVer2000);
			Assert::IsTrue(aVer2000 != aVer1000);


			// 2. 
			Ver aVer1000_1(1,0,0,0);
			Assert::IsTrue(aVer1000_1 == aVer1000);


			// 3. 
			Ver aVer1000_2(aVer1000);
			Assert::IsTrue(aVer1000_2 == aVer1000 && aVer1000_2 == aVer1000_1);
			Assert::IsTrue(aVer1000_2 >= aVer1000 && aVer1000_2 >= aVer1000_1);
			Assert::IsTrue(aVer1000_2 <= aVer1000 && aVer1000_2 <= aVer1000_1);


			// 4.
			Ver aVer1234(1,2,3,4), aVer1235(1,2,3,5);
			Assert::IsTrue(aVer1234 <  aVer1235);
			Assert::IsTrue(aVer1234 <= aVer1235);

			Assert::IsTrue(aVer1235 >  aVer1234);
			Assert::IsTrue(aVer1235 >= aVer1234);

			// 5. 
			Assembly^ aAs = Assembly::GetExecutingAssembly();
			Process ^ aP = Process::GetCurrentProcess();

			HMODULE aMod = 0;

			for each (ProcessModule ^ aIt in aP->Modules)
				if (aAs->Location == aIt->FileName)
				{
					aMod = (HMODULE) aIt->BaseAddress.ToInt64();
					break;
				}

			Ver aVer_Cur(Ver::VERSION_CURRENT, aMod);
			Assert::IsTrue(aVer_Cur > aVer1000 && aVer_Cur < aVer2000);
			Assert::IsTrue(aVer_Cur >= aVer1000 && aVer_Cur <= aVer2000);
		}

		[TestMethod]
		void Test_Set()
		{
			Ver aV;
			Assert::IsTrue
			(
				aV.GetMajor()    == 0 &&
				aV.GetMinor()    == 0 &&
				aV.GetRelease()  == 0 &&
				aV.GetRevision() == 0
			);

			aV.Set(1, 0);
			Assert::IsTrue
			(
				aV.GetMajor()    == 1 &&
				aV.GetMinor()    == 0 &&
				aV.GetRelease()  == 0 &&
				aV.GetRevision() == 0
			);

			aV.Set(1, 0, 256, 1234);
			Assert::IsTrue
			(
				aV.GetMajor()    == 1 &&
				aV.GetMinor()    == 0 &&
				aV.GetRelease()  == 256 &&
				aV.GetRevision() == 1234
			);

			Ver aV1, aV2;
			aV1 = aV2 = aV;

			Assert::IsTrue
			(
				aV1.GetMajor()    == 1 &&
				aV1.GetMinor()    == 0 &&
				aV1.GetRelease()  == 256 &&
				aV1.GetRevision() == 1234
			);
			Assert::IsTrue
			(
				aV2.GetMajor()    == 1 &&
				aV2.GetMinor()    == 0 &&
				aV2.GetRelease()  == 256 &&
				aV2.GetRevision() == 1234
			);


			aV.Set( _T("1.2.3.4"), 7);
			Assert::IsTrue
			(
				aV.GetMajor()    == 1 &&
				aV.GetMinor()    == 2 &&
				aV.GetRelease()  == 3 &&
				aV.GetRevision() == 4
			);


			aV.Set( _T("1, 222:390 4"), 13);
			Assert::IsTrue
			(
				aV.GetMajor()    == 1 &&
				aV.GetMinor()    == 222 &&
				aV.GetRelease()  == 390 &&
				aV.GetRevision() == 4
			);

		}

		IMPLEMENT_TEST_CLASS;
	};
}

#include "stdafx.h"

#include "Utility.h"

//////////////////////////////////////////////////////////////////////////////////////////////

using namespace System;

using namespace Microsoft::VisualStudio::TestTools;
using namespace Microsoft::VisualStudio::TestTools::UnitTesting;

//////////////////////////////////////////////////////////////////////////////////////////////

namespace ApproachTest
{
	[TestClass]
	public ref class UtilityTest
	{
	public:
		[TestMethod]
		void Test_GuidToString()
		{
			GUID aExp = {0x63B01035, 0xB12E, 0x47CA, {0xB5, 0x5B, 0x1F, 0x37, 0x7F, 0x2E, 0x46, 0x34} };

			TCHAR aRes[100] = {0};
			StringUtil::GuidToString(&aExp, aRes, 100);

			String ^ a = gcnew String(aRes);

			Assert::AreEqual("63B01035-B12E-47CA-B55B-1F377F2E4634", a, true);
		}

		
		IMPLEMENT_TEST_CLASS;
	};
}

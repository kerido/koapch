#include "stdafx.h"
#include <iostream>

#include "sdk_WindowNavigator.h"

//////////////////////////////////////////////////////////////////////////////////////////////

using namespace System;
using namespace System::Collections::Generic;

using namespace Microsoft::VisualStudio::TestTools;
using namespace Microsoft::VisualStudio::TestTools::UnitTesting;

//////////////////////////////////////////////////////////////////////////////////////////////

TCHAR ClassName_Progman[]   = _T("Progman");
TCHAR ClassName_ShellView[] = _T("SHELLDLL_DefView");
TCHAR ClassName_ListView[]  = _T("SysListView32");

namespace ApproachTest
{
	public class TestIterator
	{
		int myNumFoundWindows;
	public:
		TestIterator() : myNumFoundWindows(0) { }

	public:
		int GetNumFound() const { return myNumFoundWindows; }

	public:
		bool operator () (HWND theWnd)
		{
			TCHAR aClass[1000];
			GetClassName(theWnd, aClass, 1000);

			if( lstrcmp(aClass, ClassName_ShellView) == 0)
				myNumFoundWindows++;

			return true;
		}
	};


	[TestClass]
	public ref class WindowNavigatorTest
	{
	public:
		[TestMethod]
		void Test_WindowSearch_MaxDepth()
		{
			MaxDepthWindowNavigator
			<
				ClassCondition<ClassName_Progman>,

				0,

				MaxDepthWindowNavigator
				<
					ClassCondition<ClassName_ShellView>,

					0,

					MaxDepthWindowNavigator
					<
						ClassCondition<ClassName_ListView>,
						0
					>
				>
			> aNav_Deterministic;


			MaxDepthWindowNavigator
			<
				ClassCondition<ClassName_Progman>,

				0,

				MaxDepthWindowNavigator
				<
					ClassCondition<ClassName_ListView>,
					1
				>
			> aNav_NonDeterministic;


			HWND aDefView1 = aNav_Deterministic.find(NULL);
			HWND aDefView2 = aNav_NonDeterministic.find(NULL);

			Assert::IsTrue(aDefView1 == aDefView2);
		}

		[TestMethod]
		void Test_WindowSearch_UnlimitedDepth()
		{
			UnlimitedDepthWindowNavigator
			<
				ClassCondition<ClassName_ShellView>
			>
			aNav;


			HWND aDefView = aNav.find(NULL);

			// since there is always one "Progman" window
			// which hosts a shell view, the find operation
			// must always return non-null.
			Assert::IsTrue(aDefView != 0);
		}

		[TestMethod]
		void Test_WindowEnumeration()
		{
			MaxDepthWindowNavigator
			<
				ClassCondition<ClassName_Progman>,

				0,

				MaxDepthWindowNavigator
				<
					ClassCondition<ClassName_ShellView>,
					0
				>
			> aNav_Deterministic;

			MaxDepthWindowNavigator
			<
				ClassCondition<ClassName_ShellView>,
				1
			> aNav_NonDeterministic;

			TestIterator aIter_Deterministic;
			aNav_Deterministic.iterate(0, aIter_Deterministic);

			TestIterator aIter_Nondeterministic;
			aNav_NonDeterministic.iterate(0, aIter_Nondeterministic);

			Assert::IsTrue( aIter_Deterministic.GetNumFound() <= aIter_Deterministic.GetNumFound() );
		}
		
		IMPLEMENT_TEST_CLASS;
	};
}

#include "stdafx.h"

#include "sdk_ComObject.h"

//////////////////////////////////////////////////////////////////////////////////////////////

using namespace System;

using namespace Microsoft::VisualStudio::TestTools;
using namespace Microsoft::VisualStudio::TestTools::UnitTesting;

//////////////////////////////////////////////////////////////////////////////////////////////

namespace ApproachTest
{
	class DECLSPEC_NOVTABLE ISampleInterface : public IUnknown
	{
	public:
		STDMETHOD (SampleMethod) ();
	};

	// {6D31946F-056C-45D9-BFF2-26232B7DE1F2}
	DEFINE_GUID(IID_ISampleInterface, 
		0X6D31946F, 0X56C, 0X45D9, 0XBF, 0XF2, 0X26, 0X23, 0X2B, 0X7D, 0XE1, 0XF2);



	class Implementor : public virtual ComClass, public virtual ISampleInterface
	{
	public:
		STDMETHODIMP SampleMethod()
		{
			return S_OK;
		}

	public:
		STDMETHODIMP QueryInterface(REFIID theIID, void ** theOut)
		{
			if (theOut == 0)
				return E_INVALIDARG;

			if (theIID == IID_IUnknown || theIID == IID_ISampleInterface)
			{
				*theOut = static_cast<void *>(this);
				return S_OK;
			}
			else
				return E_NOINTERFACE;
		}
	};



	[TestClass]
	public ref class UtilityTest
	{
	private:
		TestContext^ myTestCtx;

	public:
		/// <summary> Gets or sets the test context which provides information about
		/// and functionality for the current test run.</summary>
		property TestContext^ Context
		{
			TestContext^ get()
			{ return myTestCtx; }

			Void set(TestContext^ value)
			{ myTestCtx = value; }
		};

		[TestMethod]
		void Test_GuidToString()
		{
			Implementor * aImpl = new	Implementor(); //refs = 1

			ISampleInterface * aSI = 0;
			HRESULT aRes = aImpl->QueryInterface(IID_ISampleInterface, (void **) &aSI); //refs = 2

			Assert::IsTrue( SUCCEEDED(aRes) );

			IUnknown * aUnk = 0;
			aRes = aImpl->QueryInterface(IID_IUnknown, (void **) &aUnk); //refs = 3

			Assert::IsTrue( SUCCEEDED(aRes) );

			bool aPointersEqual = static_cast<void *>(aSI) == static_cast<void *>(aUnk);

			Assert::IsTrue(aPointersEqual);

			ULONG aRefCount = aUnk->Release();
			Assert::AreEqual(2UL, aRefCount);

			aRes = aSI->Release();
			Assert::AreEqual(1UL, aRefCount);

			delete aImpl;
		};
	};
}

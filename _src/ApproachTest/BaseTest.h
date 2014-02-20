#pragma once


#define IMPLEMENT_TEST_CLASS \
public: \
	typedef Microsoft::VisualStudio::TestTools::UnitTesting::TestContext TestCtx; \
protected: \
	TestCtx^ myTestCtx; \
public: \
	property TestCtx^ TestContext \
	{ \
		TestCtx^ get()                    { return myTestCtx; } \
		System::Void set(TestCtx^ value)  { myTestCtx = value; } \
	}
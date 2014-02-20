#pragma once

#include "sdk_ComObject.h"

#include "Framework.h"

//////////////////////////////////////////////////////////////////////////////////////////////

class PropPageFactoryImpl_Default : public ComEntry1<IPropPageFactory>
{
public:
	PropPageFactoryImpl_Default()
	{
		Application::Instance().RegisterPropPageFactory(this);
	}


// IPropPageFactory members
public:
	STDMETHODIMP CreatePropPages(IPropPageHost * theHost)
	{
		HRESULT aRes;

		// 1. Features
		aRes = theHost->AddChildPropPage( ContextID_Root, new DlgOptionsPropSheet_Features() );
		ATLASSERT( SUCCEEDED(aRes) );


		// 2. Menus and Scrolling
		aRes = theHost->AddChildPropPage( ContextID_Root, new DlgOptionsPropSheet_MenusScrolling() );
		ATLASSERT( SUCCEEDED(aRes) );


		// 3. Contents
		aRes = theHost->AddChildPropPage( ContextID_Root, new DlgOptionsPropSheet_Contents() );
		ATLASSERT( SUCCEEDED(aRes) );


		// 4. Maintenance
		aRes = theHost->AddChildPropPage( ContextID_Root, new DlgOptionsPropSheet_Maintenance() );
		ATLASSERT( SUCCEEDED(aRes) );


		// 5. About
		aRes = theHost->AddChildPropPage( ContextID_Root, new DlgOptionsPropSheet_About() );
		ATLASSERT( SUCCEEDED(aRes) );

		return S_OK;
	}
};

class StandardPropPages : public ComInstance<PropPageFactoryImpl_Default>
{
};

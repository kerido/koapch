#pragma once
#include <unknwn.h>

//////////////////////////////////////////////////////////////////////////////////////////////

interface IPropPageHost;
class IChangeableEntity;

//////////////////////////////////////////////////////////////////////////////////////////////

MIDL_INTERFACE("E1279E4B-63A5-4337-975A-8D1034680B4F") IPropPage : public IUnknown
{
	STDMETHOD (GetTitle)     (LPCWSTR * theOutText) = 0;

	//! Returns the GUID of the context represented by a property page. Context IDs are
	//! meant to link property pages allowing them to nest.
	//! 
	//! \param [out] theOutGuid
	//!     Uponn return, contains the Context ID of the property page.
	//! 
	//! \return
	//!   - S_OK     The property page can serve as a parent to other property pages. \a theOutGuid contains
	//!              the context ID for referencing the property page while nesting.
	//!   - S_FALSE  The property page can be identified by a context but cannot contain child property pages.
	//!   - E_*      The property page does not have a context.
	STDMETHOD (GetContext)   (LPGUID theOutGuid) = 0;
	STDMETHOD (Initialize)   (IPropPageHost * theHost) = 0;
	STDMETHOD (GetHwnd)      (HWND * theOutHwnd) = 0;
	STDMETHOD (OnSave)       () = 0;
	STDMETHOD (OnActivating) (bool theActivate) = 0;
	STDMETHOD (OnClosing)    () = 0;
};

// {E1279E4B-63A5-4337-975A-8D1034680B4F}
DFN_GUID(IID_IPropPage, 0xe1279e4b, 0x63a5, 0x4337, 0x97, 0x5a, 0x8d, 0x10, 0x34, 0x68, 0x0b, 0x4f);


// supported context ID's
DFN_GUID(ContextID_Root,    0x9901BF9E, 0x6FA8, 0x404D, 0xA5, 0x40, 0x28, 0xDB, 0x49, 0x62, 0xED, 0x1D);
DFN_GUID(ContextID_Plugins, 0x932028AA, 0x57D6, 0x4510, 0xB6, 0x20, 0xE0, 0x24, 0xFB, 0x35, 0x87, 0xE7);
DFN_GUID(ContextID_Reg,     0x188C0B45, 0xA90E, 0x4385, 0x9F, 0x90, 0x63, 0xE6, 0x71, 0x3F, 0x0F, 0xE3);


//////////////////////////////////////////////////////////////////////////////////////////////



MIDL_INTERFACE("F9ED93E7-2286-446F-BB68-61731B8D03AD") IPropPageHost : public IUnknown
{
//IPropPageHost Members
public:
	STDMETHOD (AddChildPropPage) (REFGUID theParentGuid, IPropPage * thePropPage) = 0;
	STDMETHOD (GetHwnd)          (HWND * theOutHwnd) = 0;
	STDMETHOD (RegisterEntity)   (IChangeableEntity * theEntity) = 0;
	STDMETHOD (GetSettings)      (void ** theOut) = 0;

	//STDMETHOD (GetContext)      (REFGUID theContextID, IPropPageContext ** theOutCtx) = 0;
	//STDMETHOD (RegisterContext) (IPropPageContext * theCtx, REFGUID theContextID) = 0;
};

// {F9ED93E7-2286-446F-BB68-61731B8D03AD}
DFN_GUID(IID_IPropPageHost, 0xf9ed93e7, 0x2286, 0x446f, 0xbb, 0x68, 0x61, 0x73, 0x1b, 0x8d, 0x03, 0xad);

//////////////////////////////////////////////////////////////////////////////////////////////



//! Creates property pages
MIDL_INTERFACE("01DDBC87-6FB8-452E-9A9E-7261A4111889") IPropPageFactory : public IUnknown
{
	STDMETHOD (CreatePropPages) (IPropPageHost * theHost) = 0;
};

// {01DDBC87-6FB8-452E-9A9E-7261A4111889}
DFN_GUID(IID_IPropPageFactory,
         0x1DDBC87, 0x6FB8, 0x452E, 0x9A, 0x9E, 0x72, 0x61, 0xA4, 0x11, 0x18, 0x89);

//////////////////////////////////////////////////////////////////////////////////////////////

MIDL_INTERFACE("0368E089-B9F3-4DE7-A1DE-3D903C5CEFF5") IHelpContext : public IUnknown
{
	//! TODO
	STDMETHOD (GetChmUrl)    (TCHAR * theOut, int * theSize) = 0;
};

DFN_GUID(IID_IHelpContext, 0x368E089, 0xB9F3, 0x4DE7, 0xA1, 0xDE, 0x3D, 0x90, 0x3C, 0x5C, 0xEF, 0xF5);


//////////////////////////////////////////////////////////////////////////////////////////////

MIDL_INTERFACE("EF189636-BCCE-4401-AF6F-E4D65AB4B7A4") IControlHost : public IUnknown
{
	enum
	{
		CTL_CATEGORIES = 1,
		CTL_BTN_OK,
		CTL_BTN_CANCEL,
		CTL_BTN_HELP,
		CTL_TOOLTIP
	};

	STDMETHOD (GetControl) (int theType, HWND * theOutCtl) = 0;
};
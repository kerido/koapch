#include "stdafx.h"

#include "DlgOptions.h"
#include "Application.h"
#include "HtmlHelpProvider.h"

//////////////////////////////////////////////////////////////////////////////////////////////

DlgOptionsPropPages::DlgOptionsPropPages() :
	myPrefs( *Application::InstanceC().Prefs() )
{
	myBindablePropPages[ContextID_Root] = TVI_ROOT;
}

//////////////////////////////////////////////////////////////////////////////////////////////

DlgOptionsPropPages::~DlgOptionsPropPages()
{
	for (PropPageIterC aIt = myPropPages.begin(); aIt != myPropPages.end(); aIt++)
		aIt->second->Release();
}

//////////////////////////////////////////////////////////////////////////////////////////////

void DlgOptionsPropPages::operator () ( IPropPageFactory * theFactory )
{
	theFactory->CreatePropPages(this);
	myCurItem = myPropPages.end();
}

//////////////////////////////////////////////////////////////////////////////////////////////

void DlgOptionsPropPages::ActivatePropPageByTreeItem(HTREEITEM theIt)
{
	PropPageIter aIt = myPropPages.find(theIt);

	if (aIt == myCurItem)
		return;

	if ( myCurItem != myPropPages.end() )
	{
		HWND aWnd = NULL;
		HRESULT aRes = myCurItem->second->GetHwnd(&aWnd);

		if ( SUCCEEDED(aRes) )
		{
			myCurItem->second->OnActivating(false);
			::SetWindowPos(aWnd, NULL, 0, 0, 0, 0, SWP_NOSIZE|SWP_NOMOVE|SWP_NOZORDER|SWP_NOREDRAW|SWP_HIDEWINDOW);
		}
	}

	myCurItem = aIt;

	PositionDynamicControls(true);
	Invalidate();
}

//////////////////////////////////////////////////////////////////////////////////////////////

void DlgOptionsPropPages::ShowHelpOnCurWindow()
{
	ATLASSERT( myCurItem != myPropPages.end() );

	const HtmlHelpProvider * aHP = Application::InstanceC().GetHtmlHelpProvider();

	if ( !aHP->IsHelpEnabled() )
		return;

	CComQIPtr<IHelpContext> aHelper(myCurItem->second);

	if (aHelper == 0)
		return;

	TCHAR aUrl [100];
	int aLength = 100;

	HRESULT aRes = aHelper->GetChmUrl(aUrl, &aLength);

	if ( SUCCEEDED(aRes) )
		aHP->DisplayHelp(m_hWnd, aUrl);
	else
		aHP->DisplayHelp(m_hWnd);
}

//////////////////////////////////////////////////////////////////////////////////////////////

void DlgOptionsPropPages::LoadLocalization()
{
	LocalizationUtility::LocTriplet aLocalizationArray [] = 
	{
		{ KEYOF(IDS_GLBL_CMD_OK),     0, IDOK         },
		{ KEYOF(IDS_GLBL_CMD_CANCEL), 0, IDCANCEL     },
		{ KEYOF(IDS_GLBL_CMD_HELP),   0, IDC_OPTS_HELP }
	};

	const int ArraySize = sizeof aLocalizationArray  /  sizeof aLocalizationArray[0];

	LocalizationUtility::LocalizeDialogBox(m_hWnd, myLocMan, aLocalizationArray, ArraySize);
	LocalizationUtility::SetWindowDirection(myTooltip, myLocMan);
}

//////////////////////////////////////////////////////////////////////////////////////////////

void DlgOptionsPropPages::PositionDynamicControls(bool theActivateCurPropPage)
{
	HWND aWnd = NULL;
	HRESULT aRes = myCurItem->second->GetHwnd(&aWnd);

	if ( FAILED(aRes) )
		return;


	Metric aMetric_Categories;
	Metric aMetric_Margin;
	Metric aMetric_ButtonSize;

	myLocMan->GetMetricSafe( KEYOF(IDM_OPTS_CATEGORIESWIDTH), aMetric_Categories);
	myLocMan->GetMetricSafe( KEYOF(IDM_OPTS_CONTROLMARGIN),   aMetric_Margin);
	myLocMan->GetMetricSafe( KEYOF(IDM_OPTS_BUTTONSIZE),      aMetric_ButtonSize);


	RECT aMarginDummyRect;
	aMarginDummyRect.left = 0;
	aMarginDummyRect.top = 0;
	aMarginDummyRect.right = aMetric_Margin.Val1;
	aMarginDummyRect.bottom = aMetric_Margin.Val1;

	::MapDialogRect( m_hWnd, &aMarginDummyRect);

	// 1. Move the content property page

	RECT aPropPageRect;	//relatively to IDD_O;
	::GetWindowRect(aWnd, &aPropPageRect);

	int aWidth = aPropPageRect.right - aPropPageRect.left;
	int aHeight = aPropPageRect.bottom - aPropPageRect.top;

	aPropPageRect.left = 0;
	aPropPageRect.top = 0;
	aPropPageRect.right = aMetric_Categories.Val1 + 2 * aMetric_Margin.Val1;
	aPropPageRect.bottom = aMetric_Margin.Val1;

	::MapDialogRect( m_hWnd, &aPropPageRect);

	UINT aFlags = SWP_NOZORDER|SWP_NOSIZE|SWP_NOREDRAW;

	if (theActivateCurPropPage)
	{
		myCurItem->second->OnActivating(true);
		aFlags |= SWP_SHOWWINDOW;
	}




	::SetWindowPos
	(
		aWnd,
		NULL,
		aPropPageRect.right,
		aPropPageRect.bottom,
		0,
		0,
		aFlags
	);


	// 2. Size the categories tree view

	RECT aCategoriesRect;
	aCategoriesRect.left   = aMetric_Margin.Val1;
	aCategoriesRect.top    = aMetric_Margin.Val1;
	aCategoriesRect.right  = aMetric_Categories.Val1 + aMetric_Margin.Val1;
	aCategoriesRect.bottom = aCategoriesRect.top;             // dummy value, does not mean anything because it's already in pixels
	                                                          // (as opposed to aMetric_Categories which is in dialog units)

	::MapDialogRect( m_hWnd, &aCategoriesRect );
	aCategoriesRect.bottom = aCategoriesRect.top + aHeight;   //now the rectangle is correct -- entirely in pixels

	myTreeCtrl.SetWindowPos
	(
		NULL,
		aCategoriesRect.left,
		aCategoriesRect.top,
		aCategoriesRect.right-aCategoriesRect.left,
		aCategoriesRect.bottom-aCategoriesRect.top,
		SWP_NOZORDER
	);

	// 3. adjust own size

	RECT aButtonRect;
	aButtonRect.left = 0;
	aButtonRect.top = 0;
	aButtonRect.right = aMetric_ButtonSize.Val1;
	aButtonRect.bottom = aMetric_ButtonSize.Val2;

	::MapDialogRect( m_hWnd, &aButtonRect );


	int aOwnHeight = 3 * aMarginDummyRect.bottom + aCategoriesRect.bottom - aCategoriesRect.top  + aButtonRect.bottom;
	int aOwnWidth  = 3 * aMarginDummyRect.right  + aCategoriesRect.right  - aCategoriesRect.left + aWidth;

	RECT aOwnWndRect;
	aOwnWndRect.left = 0;
	aOwnWndRect.top = 0;
	aOwnWndRect.right = aOwnWidth;
	aOwnWndRect.bottom = aOwnHeight;

	LONG_PTR aStyle = ::GetWindowLongPtr(m_hWnd, GWL_STYLE);
	LONG_PTR aExStyle = ::GetWindowLongPtr(m_hWnd, GWL_EXSTYLE);

	::AdjustWindowRectEx
	(
		&aOwnWndRect,
		(DWORD) aStyle,
		FALSE,
		(DWORD) aExStyle
	);

	SetWindowPos
	(
		NULL,
		0,
		0,
		aOwnWndRect.right - aOwnWndRect.left,
		aOwnWndRect.bottom - aOwnWndRect.top,
		SWP_NOMOVE|SWP_NOZORDER
	);



	// 4. move the Help button (it the rightmost)
	int aButtonsY = aOwnHeight - (aMarginDummyRect.bottom + aButtonRect.bottom);
	int aCurBtnX = aOwnWidth - (aMarginDummyRect.right + aButtonRect.right);

	myBtnHtmlHelp.SetWindowPos
	(
		NULL,
		aCurBtnX,
		aButtonsY,
		aButtonRect.right,
		aButtonRect.bottom,
		SWP_NOZORDER
	);


	// 5. move the Cancel button (it's 2-nd from the right)
	aCurBtnX -= aMarginDummyRect.right + aButtonRect.right;

	myBtnCancel.SetWindowPos
	(
		NULL,
		aCurBtnX,
		aButtonsY,
		aButtonRect.right,
		aButtonRect.bottom,
		SWP_NOZORDER
	);


	// 6. move the OK button (3-rd from the right)
	aCurBtnX -= aMarginDummyRect.right + aButtonRect.right;

	myBtnOk.SetWindowPos
	(
		NULL,
		aCurBtnX,
		aButtonsY,
		aButtonRect.right,
		aButtonRect.bottom,
		SWP_NOZORDER
	);
}

//////////////////////////////////////////////////////////////////////////////////////////////

void DlgOptionsPropPages::UpdatePageTitles()
{
	for(PropPageIter aIt = myPropPages.begin(); aIt != myPropPages.end(); aIt++)
	{
		HTREEITEM aItem = aIt->first;
		IPropPage * aPg = aIt->second;

		const TCHAR * aTitle;
		HRESULT aRes = aPg->GetTitle(&aTitle);

		if ( SUCCEEDED(aRes) )
			myTreeCtrl.SetItemText(aItem, aTitle);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
//                                 Win32 Message Handlers                                   //
//////////////////////////////////////////////////////////////////////////////////////////////

LRESULT DlgOptionsPropPages::MsgHandler_InitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	// bind controls to dlg items
	myTreeCtrl = GetDlgItem(IDC_OPTS_PROPPAGETREE);
	myBtnOk = GetDlgItem(IDOK);
	myBtnCancel = GetDlgItem(IDCANCEL);
	myBtnHtmlHelp = GetDlgItem(IDC_OPTS_HELP);

	myTooltip.Create(m_hWnd);
	myTooltip.Activate(TRUE);


	///////////////////////////////////////////////////////////////////////////////////////
	// Set TreeView item height
	RECT aDummyItemRect;
	aDummyItemRect.left = aDummyItemRect.top = 0;
	aDummyItemRect.right = aDummyItemRect.bottom = 20;                   //dialog units

	MapDialogRect(&aDummyItemRect);                                      //conversion

	myTreeCtrl.SetItemHeight(aDummyItemRect.bottom - aDummyItemRect.top); //pixels


	///////////////////////////////////////////////////////////////////////////////////////
	// Set Max tooltip width
	aDummyItemRect.left = aDummyItemRect.top = 0;
	aDummyItemRect.right = aDummyItemRect.bottom = 120;                  //dialog units

	MapDialogRect(&aDummyItemRect);                                      //conversion

	myTooltip.SetMaxTipWidth(aDummyItemRect.right - aDummyItemRect.left);//pixels



	LoadLocalization();

	CenterWindow();
	
	SetForegroundWindow(m_hWnd);


	Application & aApp = Application::Instance();

	///////////////////////////////////////////////////////////////////////////////////////
	// Enumerate property pages and, upon each step, create property pages
	aApp.EnumPropPageFactories(*this);


	///////////////////////////////////////////////////////////////////////////////////////
	// Activate an appropriate property page
	HTREEITEM aItem = myTreeCtrl.GetChildItem(TVI_ROOT);  // get the first child item;
	myTreeCtrl.SelectItem(aItem);

	aApp.RegisterLocalizationEventProcessor(this);


	///////////////////////////////////////////////////////////////////////////////////////
	// Add tooltips
	LocalizationUtility::LocPair aLocalizationArray [] = 
	{
		{ IDOK,     TIPOF(IDC_OPTS_OK)     },
		{ IDCANCEL, TIPOF(IDC_OPTS_CANCEL) },
		LOCENTRY_H(IDC_OPTS_HELP),
	};

	const int ArraySize = sizeof aLocalizationArray  /  sizeof aLocalizationArray[0];

	return LocalizationUtility::AddTooltips(m_hWnd, myLocMan,
		static_cast<IControlHost *>(this), aLocalizationArray, ArraySize);


	///////////////////////////////////////////////////////////////////////////////////////
	// Complete
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////////

LRESULT DlgOptionsPropPages::MsgHandler_Command_OK( WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/ )
{
	for (PropPageIter aIt = myPropPages.begin(); aIt != myPropPages.end(); aIt++)
	{
		HRESULT aRes = aIt->second->OnClosing();

		aRes = aIt->second->OnSave();

		Trace("Property page returned %x while saving\n", aRes);
	}


#if _USE_CHANGEABLEENTITY
	for (ChangeableEntityIter aItc = myChangeableEntities.begin(); aItc != myChangeableEntities.end(); aItc++)
		(*aItc)->RaiseUpdateChanges(0);
#else
	Application::Instance().SetPrefs(myPrefs);
#endif

	EndDialog(IDOK);
	return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////

LRESULT DlgOptionsPropPages::MsgHandler_Command_Cancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	for (PropPageIter aIt = myPropPages.begin(); aIt != myPropPages.end(); aIt++)
		aIt->second->OnClosing();


	EndDialog(IDCANCEL);
	return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////

LRESULT DlgOptionsPropPages::MsgHandler_Command_Help(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	ShowHelpOnCurWindow();
	return 0L;
}

//////////////////////////////////////////////////////////////////////////////////////////////

LRESULT DlgOptionsPropPages::MsgHandler_Help(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
{
	ShowHelpOnCurWindow();
	return 0L;
}

//////////////////////////////////////////////////////////////////////////////////////////////

LRESULT DlgOptionsPropPages::MsgHandler_Notif_ChangePropSheet(int theControlID, LPNMHDR theHeader, BOOL& bHandled)
{
	NMTREEVIEW * aTV = reinterpret_cast<NMTREEVIEW *>(theHeader);
	ActivatePropPageByTreeItem(aTV->itemNew.hItem);
	return 0L;
}

//////////////////////////////////////////////////////////////////////////////////////////////

LRESULT DlgOptionsPropPages::MsgHandler_Destroy( UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/ )
{
	Application::Instance().UnregisterLocalizationEventProcessor(this);

	return 0L;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//                                 IPropPageHost Members                                    //
//////////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP DlgOptionsPropPages::AddChildPropPage(REFGUID theParentGuid, IPropPage * thePropPage)
{
	HRESULT aRes = thePropPage->Initialize(this);

	if ( FAILED(aRes) )
		return aRes;

	const TCHAR * aTitle;
	aRes = thePropPage->GetTitle(&aTitle);

	if (FAILED(aRes))
	{
		thePropPage->Release();
		return aRes;
	}

	BindablePropPageIter aIt = myBindablePropPages.find(theParentGuid);

	if ( aIt == myBindablePropPages.end() )
	{
		thePropPage->Release();
		return E_FAIL;
	}

	HTREEITEM aItem = myTreeCtrl.InsertItem(aTitle, aIt->second, TVI_LAST);
	myTreeCtrl.Expand(aIt->second);

	// if the property sheet represents a context, we must store its GUID
	// and the associated tree item handle for further referral

	GUID aBindGuid;
	aRes = thePropPage->GetContext(&aBindGuid);

	if ( SUCCEEDED(aRes) )
	{
		aIt = myBindablePropPages.find(aBindGuid);

		if (aIt == myBindablePropPages.end() )
			myBindablePropPages.insert( aIt, BindablePropPagePair(aBindGuid, aItem) );
	}


	HWND aPropPageHwnd = NULL;
	aRes = thePropPage->GetHwnd(&aPropPageHwnd);

	ATLASSERT( SUCCEEDED(aRes) );

	// programmatically set WS_EX_STATICEDGE to make borders visible
	LONG_PTR aExStyle = ::GetWindowLongPtr(aPropPageHwnd, GWL_EXSTYLE);
	aExStyle |= WS_EX_STATICEDGE;
	::SetWindowLongPtr(aPropPageHwnd, GWL_EXSTYLE, aExStyle);

	// because we have changed the extended style, we call SetWindowPos
	// (according to the documentation)
	::SetWindowPos
	(
		aPropPageHwnd,
		NULL,
		0,
		0,
		0,
		0,
		SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER|SWP_FRAMECHANGED
	);

	myPropPages[aItem] = thePropPage;
	return aRes;
}

//////////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP DlgOptionsPropPages::GetHwnd(HWND * theOutHwnd)
{
	*theOutHwnd = m_hWnd;
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP DlgOptionsPropPages::RegisterEntity(IChangeableEntity * theEntity)
{
	ChangeableEntityIterC aIt = std::find(myChangeableEntities.begin(), myChangeableEntities.end(), theEntity);

	if ( aIt != myChangeableEntities.end() )
		return S_FALSE;

	theEntity->AddRef();
	myChangeableEntities.push_back(theEntity);
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP DlgOptionsPropPages::GetSettings(void ** theOut)
{
	if (theOut == 0)
		return E_POINTER;

	*theOut = &myPrefs;
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP DlgOptionsPropPages::GetControl( int theType, HWND * theOutCtl )
{
	if (theOutCtl == 0)
		return E_POINTER;

	switch(theType)
	{
	case CTL_CATEGORIES:
		* theOutCtl = myTreeCtrl;
		break;

	case CTL_BTN_OK:
		* theOutCtl = myBtnOk;
		break;

	case CTL_BTN_CANCEL:
		* theOutCtl = myBtnCancel;
		break;

	case CTL_BTN_HELP:
		* theOutCtl = myBtnHtmlHelp;
		break;

	case CTL_TOOLTIP:
		* theOutCtl = myTooltip;
		break;

	default:
		return E_INVALIDARG;
	}

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////////////////////////

void DlgOptionsPropPages::OnLocalizationChanged( ILocalization *, const TCHAR *, int )
{
	LoadLocalization();
	PositionDynamicControls(false);
	UpdatePageTitles();
	InvalidateRect(NULL);
}
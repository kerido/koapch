#include "stdafx.h"

#include "sdk_ComObject.h"

#include "Framework.h"
#include "HotspotImpl_NotifyIcon.h"
#include "DlgOptionsPropPages.h"
#include "DlgOptionsApproachItems.h"
#include "Application.h"
#include "FilePathProvider.h"
#include "LogicCommon.h"

#include "util_Warning.h"
#include "util_KeyFile.h"

class TrackBarUtility
{
public:
	static void HandleTimeoutTrack(WPARAM wParam, CTrackBarCtrl * aSenderCtrl, CStatic * aValueCtrl)
	{
		int aNotifCode = LOWORD(wParam);
		bool aDragging = (aNotifCode == TB_THUMBPOSITION || aNotifCode == TB_THUMBTRACK);
		int aCurPos = aDragging ? HIWORD(wParam) : aSenderCtrl->GetPos();

		ATLASSERT(aSenderCtrl != 0 && aValueCtrl != 0);

		if (aDragging)
		{
			Trace("Scroll slider, aCurPos=%d\n", aCurPos);

			int aMin = aSenderCtrl->GetRangeMin();
			int aFrq = aSenderCtrl->GetLineSize();

			int aFloor = aMin + ( (aCurPos-aMin) / aFrq ) * aFrq;
			int aCeil = aFloor + aFrq;

			if (aCeil > aSenderCtrl->GetRangeMax() )
				aCeil = aFloor;

			if (aCurPos - aFloor > aFrq/2)
				aCurPos = aCeil;
			else
				aCurPos = aFloor;

			aSenderCtrl->SetPos(aCurPos);
		}

		DisplayTimeoutText(aValueCtrl, aCurPos);
	}

	static void DisplayTimeoutText(CStatic * theCtrl, int theValue)
	{
		LocalizationManagerPtr myLocMan;

		///////////////////////////////////////////////////////
		// 1. Metrics
		Metric aM;

		HRESULT aRes = myLocMan->GetMetricSafe( KEYOF(IDM_GLBL_MILLISECFMT), aM);
		ATLASSERT( SUCCEEDED(aRes) );

		bool myMillisecondsBefore = aM.Val1 != 0;


		///////////////////////////////////////////////////////
		// 2. Strings
		const static int TextSize = 1024;
		TCHAR myMillisecondsString[TextSize];

		int aSize = TextSize;
		aRes = myLocMan->GetStringSafe(KEYOF(IDS_GLBL_MILLISEC), myMillisecondsString, &aSize);
		ATLASSERT( SUCCEEDED(aRes) );

		TCHAR aBuf[200];
		aBuf[0] = 0;

		if (myMillisecondsBefore)
			wsprintf(aBuf, _T("%s %d"), myMillisecondsString, theValue);
		else
			wsprintf(aBuf, _T("%d %s"), theValue, myMillisecondsString);

		theCtrl->SetWindowText(aBuf);
	}
};

//////////////////////////////////////////////////////////////////////////////////////////////
//                  DlgOptionsPropSheet_Features class (Features Tab)                       //
//////////////////////////////////////////////////////////////////////////////////////////////

void DlgOptionsPropSheet_Features::InitControls()
{
	myGroup_FM              = GetDlgItem(IDL_O_FEAT_CAT_FLDMENUS);
	myFolderMenus           = GetDlgItem(IDC_O_FEAT_FLDMENUSTGL);
	mySliderInitial         = GetDlgItem(IDC_O_FEAT_INITDELAY);
	myValueInitial          = GetDlgItem(IDL_O_FEAT_INITDELAYVAL);

	myGroup_AI              = GetDlgItem(IDL_O_FEAT_CAT_APCHITEMS);
	myTrayItems             = GetDlgItem(IDC_O_FEAT_APCHITEMSTGL);

	myGroup_TM              = GetDlgItem(IDL_O_FEAT_CAT_TTLBARMENUS);
	myTitlebarMenus         = GetDlgItem(IDC_O_FEAT_TTLBARMENUSTGL);
	myRbParentToChild       = GetDlgItem(IDC_O_FEAT_DESKPOS_0TOP);
	myRbChildToParent       = GetDlgItem(IDC_O_FEAT_DESKPOS_1BOTM);
	myCbCurFolderAsItem     = GetDlgItem(IDC_O_FEAT_CURFLDISFILE);
	myCbAutoSelectCurFolder = GetDlgItem(IDC_O_FEAT_CURFLDAUTOSEL);


	mySliderInitial.SetRange(300, 1000);
	mySliderInitial.SetTicFreq(100);
	mySliderInitial.SetLineSize(100);
	mySliderInitial.SetPageSize(200);

	HotspotManager * aHSM = Application::Instance().GetHotspotManager();

	myFolderMenus.SetCheck(
		aHSM->IsHotspotEnabled(GUID_Hotspot_FolderMenus)   ? 1 : 0 );

	myTrayItems.SetCheck(
		aHSM->IsHotspotEnabled(GUID_Hotspot_ApproachItems) ? 1 : 0 );

	myTitlebarMenus.SetCheck(
		aHSM->IsHotspotEnabled(GUID_Hotspot_TitlebarMenus) ? 1 : 0 );
}

//////////////////////////////////////////////////////////////////////////////////////////////

void DlgOptionsPropSheet_Features::LoadFromPreferences(ApplicationSettings * thePr)
{
	ApplicationSettings::DataType aTimeout = thePr->GetTimeoutInit();
	mySliderInitial.SetPos(aTimeout);
	TrackBarUtility::DisplayTimeoutText(&myValueInitial, aTimeout);

	ApplicationSettings::DataType aFlags = thePr->GetTitlebarMenusFlags();

	bool aChildToParent = (aFlags & ApplicationSettings::TBM_ORDER_CHILD_TO_PARENT) != 0;
	bool aCurFolderAsItem = (aFlags & ApplicationSettings::TBM_CURFOLDER_AS_ITEM) != 0;
	bool aCurFolderAutoSel  = aCurFolderAsItem  && ( (aFlags & ApplicationSettings::TBM_CURFOLDER_AUTO_SELECT) != 0 );

	myRbChildToParent.SetCheck(aChildToParent ? BST_CHECKED : BST_UNCHECKED);
	myRbParentToChild.SetCheck(aChildToParent ? BST_UNCHECKED : BST_CHECKED);

	myCbCurFolderAsItem.SetCheck(aCurFolderAsItem ? BST_CHECKED : BST_UNCHECKED);
	myCbAutoSelectCurFolder.SetCheck(aCurFolderAutoSel? BST_CHECKED : BST_UNCHECKED);

	UpdateCheckboxState();
}

//////////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP DlgOptionsPropSheet_Features::OnSave()
{
	myPrefs->SetTimeoutInit(   (UINT) mySliderInitial.GetPos() );

	myPrefs->SetAutoEnableFolderMenus    ( myFolderMenus.GetCheck()   != 0 );
	myPrefs->SetAutoEnableKoApproachItems( myTrayItems.GetCheck()     != 0 );
	myPrefs->SetAutoEnableTitleBar       ( myTitlebarMenus.GetCheck() != 0 );

	ApplicationSettings::DataType aFlags = 0;

	if (myRbChildToParent.GetCheck() == BST_CHECKED)
		aFlags |= ApplicationSettings::TBM_ORDER_CHILD_TO_PARENT;

	if ( CheckboxHasFlag(myCbCurFolderAsItem) )
		aFlags |= ApplicationSettings::TBM_CURFOLDER_AS_ITEM;

	if ( CheckboxHasFlag(myCbAutoSelectCurFolder) )
		aFlags |= ApplicationSettings::TBM_CURFOLDER_AUTO_SELECT;

	myPrefs->SetTitlebarMenusFlags(aFlags);

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////////////////////////

void DlgOptionsPropSheet_Features::LoadLocalization()
{
	LocalizationUtility::LocTriplet aLocalizationArray [] = 
	{
		LOCENTRY_SELF(IDD_O_FEAT),

		LOCENTRY_SM(IDL_O_FEAT_CAT_FLDMENUS),
		LOCENTRY_0M(IDL_O_FEAT_FLDMENUSVIS),
		LOCENTRY_SM(IDL_O_FEAT_FLDMENUSDESC),
		LOCENTRY_SM(IDL_O_FEAT_INITDELAY),
		LOCENTRY_0M(IDC_O_FEAT_INITDELAY),
		LOCENTRY_0M(IDL_O_FEAT_INITDELAYVAL),

		LOCENTRY_SM(IDL_O_FEAT_CAT_APCHITEMS),
		LOCENTRY_0M(IDL_O_FEAT_APCHITEMSVIS),
		LOCENTRY_SM(IDL_O_FEAT_APCHITEMSDESC),
		LOCENTRY_SM(IDC_O_FEAT_APCHITEMSOPT),

		LOCENTRY_SM(IDL_O_FEAT_CAT_TTLBARMENUS),
		LOCENTRY_0M(IDL_O_FEAT_TTLBARMENUSVIS),
		LOCENTRY_SM(IDL_O_FEAT_TTLBARMENUSDESC),
		LOCENTRY_SM(IDC_O_FEAT_DESKPOS_0TOP),
		LOCENTRY_SM(IDC_O_FEAT_DESKPOS_1BOTM),
		LOCENTRY_SM(IDC_O_FEAT_CURFLDISFILE),
		LOCENTRY_SM(IDC_O_FEAT_CURFLDAUTOSEL),
	};

	int aArraySize = sizeof aLocalizationArray  /  sizeof aLocalizationArray[0];

	LocalizationUtility::LocalizeDialogBox(m_hWnd, myLocMan, aLocalizationArray, aArraySize);

	AlignCheckbox(myFolderMenus,   myGroup_FM);
	AlignCheckbox(myTrayItems,     myGroup_AI);
	AlignCheckbox(myTitlebarMenus, myGroup_TM);


	LocalizationUtility::LocPair aTips [] = 
	{
		LOCENTRY_H(IDC_O_FEAT_FLDMENUSTGL),
		LOCENTRY_H(IDC_O_FEAT_INITDELAY),

		LOCENTRY_H(IDC_O_FEAT_APCHITEMSTGL),
		LOCENTRY_H(IDC_O_FEAT_APCHITEMSOPT),

		LOCENTRY_H(IDC_O_FEAT_TTLBARMENUSTGL),
		LOCENTRY_H(IDC_O_FEAT_DESKPOS_0TOP),
		LOCENTRY_H(IDC_O_FEAT_DESKPOS_1BOTM),
		LOCENTRY_H(IDC_O_FEAT_CURFLDISFILE),
		LOCENTRY_H(IDC_O_FEAT_CURFLDAUTOSEL),
	};

	aArraySize = sizeof aTips  /  sizeof aTips[0];

	LocalizationUtility::AddTooltips(m_hWnd, myLocMan, myHost, aTips, aArraySize);
}

//////////////////////////////////////////////////////////////////////////////////////////////

void DlgOptionsPropSheet_Features::OnHotspotToggle( const GUID & theGuid, bool theEnable, bool theResult )
{
	CButton * aBtn;

	if (theGuid == GUID_Hotspot_FolderMenus)
		aBtn = &myFolderMenus;

	else if (theGuid == GUID_Hotspot_ApproachItems)
		aBtn = &myTrayItems;

	else if (theGuid == GUID_Hotspot_TitlebarMenus)
		aBtn = &myTitlebarMenus;

	else
		return;

	aBtn->SetCheck( theEnable ^ !theResult  ? 1 : 0 );
}

//////////////////////////////////////////////////////////////////////////////////////////////

LRESULT DlgOptionsPropSheet_Features::MsgHandler_InitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	Application::Instance().GetHotspotManager()->RegisterProcessor(this);

	bHandled = FALSE;
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////////

LRESULT DlgOptionsPropSheet_Features::MsgHandler_Destroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	Application::Instance().GetHotspotManager()->UnregisterProcessor(this);

	bHandled = FALSE;	//involve the base class
	return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////

LRESULT DlgOptionsPropSheet_Features::MsgHandler_Scroll(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	TrackBarUtility::HandleTimeoutTrack(wParam, &mySliderInitial, &myValueInitial);
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////////

LRESULT DlgOptionsPropSheet_Features::MsgHandler_DrawItem(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	int aBitmapID = 0;

	if (wParam == IDL_O_FEAT_FLDMENUSVIS)
		aBitmapID = IDB_FT_FLDMENUS;

	else if (wParam == IDL_O_FEAT_APCHITEMSVIS)
		aBitmapID = IDB_FT_APCHITEMS;

	else if (wParam == IDL_O_FEAT_TTLBARMENUSVIS)
		aBitmapID = IDB_FT_TBARMENUS;
	else
	{
		bHandled = FALSE;
		return 0L;
	}

	DRAWITEMSTRUCT * aStr = reinterpret_cast<DRAWITEMSTRUCT *>(lParam);

	int aPrevMode = SetStretchBltMode(aStr->hDC, HALFTONE);

	HBITMAP aBmp = LoadBitmap( _Module.GetResourceInstance(), MAKEINTRESOURCE(aBitmapID) );
	HDC aDc = CreateCompatibleDC(aStr->hDC);
	HGDIOBJ aPrevObj = SelectObject(aDc, aBmp);

	StretchBlt(aStr->hDC,
		aStr->rcItem.left,
		aStr->rcItem.top,
		aStr->rcItem.right - aStr->rcItem.left,
		aStr->rcItem.bottom - aStr->rcItem.top,
		aDc,
		0, 0, 160, 160,
		SRCCOPY);

	SelectObject(aDc, aPrevObj);
	DeleteDC(aDc);
	DeleteObject(aBmp);

	SetStretchBltMode(aStr->hDC, aPrevMode);
	return 0L;
}

//////////////////////////////////////////////////////////////////////////////////////////////

LRESULT DlgOptionsPropSheet_Features::MsgHandler_Cmd_ToggleFolderMenus(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	SafeToggleHotspot(GUID_Hotspot_FolderMenus);
	return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////

LRESULT DlgOptionsPropSheet_Features::MsgHandler_Cmd_ToggleApproachItems(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	SafeToggleHotspot(GUID_Hotspot_ApproachItems);
	return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////

LRESULT DlgOptionsPropSheet_Features::MsgHandler_Cmd_ToggleTitlebarMenus(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	SafeToggleHotspot(GUID_Hotspot_TitlebarMenus);
	return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////

LRESULT DlgOptionsPropSheet_Features::MsgHandler_Cmd_ApproachItemsOptions(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	DlgApproachItems aDlg(myPrefs);
	aDlg.DoModal();

	return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////

LRESULT DlgOptionsPropSheet_Features::MsgHandler_Cmd_TitlebarMenusCheckBox( WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/ )
{
	UpdateCheckboxState();
	return 0L;
}

//////////////////////////////////////////////////////////////////////////////////////////////

bool DlgOptionsPropSheet_Features::CheckboxHasFlag( const CButton & theCheckbox )
{
	return (theCheckbox.GetCheck() == BST_CHECKED) && theCheckbox.IsWindowEnabled();
}
//////////////////////////////////////////////////////////////////////////////////////////////

void DlgOptionsPropSheet_Features::SafeToggleHotspot(const GUID & theGuid)
{
	HotspotManager * aMan = Application::Instance().GetHotspotManager();

	ATLASSERT(aMan != 0);

	if ( !aMan->ToggleHotspot(theGuid) )
		WarningMessageUtility::Error_FeatureLocked(GetParent(), myLocMan);
}

//////////////////////////////////////////////////////////////////////////////////////////////

void DlgOptionsPropSheet_Features::AlignCheckbox( CButton & theBtn, CStaticWithLine & theAlignWn )
{
	RECT aRct_Ref;
	theAlignWn.GetWindowRect(&aRct_Ref);
	::MapWindowPoints(NULL, m_hWnd, (POINT *) &aRct_Ref, 2);

	RECT aRct_Chk;
	theBtn.GetWindowRect(&aRct_Chk);
	::MapWindowPoints(NULL, m_hWnd, (POINT *) &aRct_Chk, 2);

	aRct_Chk.top      = aRct_Ref.top + 1;
	aRct_Chk.right    = aRct_Ref.left;
	aRct_Chk.bottom   = aRct_Ref.bottom + 1;
	theBtn.SetWindowPos(0, &aRct_Chk, 0);
}

//////////////////////////////////////////////////////////////////////////////////////////////

void DlgOptionsPropSheet_Features::UpdateCheckboxState()
{
	bool aEnable = CheckboxHasFlag(myCbCurFolderAsItem);
	myCbAutoSelectCurFolder.EnableWindow(aEnable);
}

//////////////////////////////////////////////////////////////////////////////////////////////
//              DlgOptionsPropSheet_MenusScrolling class (Menus and Scrolling Tab)          //
//////////////////////////////////////////////////////////////////////////////////////////////

void DlgOptionsPropSheet_MenusScrolling::InitControls()
{
	myCatMenus               = GetDlgItem(IDL_O_MNSC_CAT_MENUS);
	myMaxMenuWidth           = GetDlgItem(IDC_O_MNSC_MAXMNUWIDTH);
	mySpinMaxMenuWidth       = GetDlgItem(IDC_O_MNSC_MAXMNUWIDTHSPIN);

	myCatScrolling           = GetDlgItem(IDL_O_MNSC_CAT_SCROLLING);
	myRbScrollAtTop          = GetDlgItem(IDC_O_MNSC_SCROLLPOS_0TOP);
	myRbScrollAtBottom       = GetDlgItem(IDC_O_MNSC_SCROLLPOS_1BOTM);
	myRbScrollRespective     = GetDlgItem(IDC_O_MNSC_SCROLLPOS_2ENDS);
	myCbShowNumOfScrollItems = GetDlgItem(IDC_O_MNSC_DISPNUMTOSCROLL);

	myScrollItemsWheel       = GetDlgItem(IDC_O_MNSC_WHEELSCROLLS);
	mySpinWheel              = GetDlgItem(IDC_O_MNSC_WHEELSCROLLSSPIN);
	myScrollItemsPage        = GetDlgItem(IDC_O_MNSC_PAGESCROLLS);
	mySpinPage               = GetDlgItem(IDC_O_MNSC_PAGESCROLLSSPIN);

	mySliderChildPopup       = GetDlgItem(IDC_O_MNSC_CHILDDELAY);
	myValueChildPopup        = GetDlgItem(IDL_O_MNSC_CHILDDELAYVAL);
	mySliderScroll           = GetDlgItem(IDC_O_MNSC_SCROLLDELAY);
	myValueScroll            = GetDlgItem(IDL_O_MNSC_SCROLLDELAYVAL);

	mySliderChildPopup.SetRange(100, 800);
	mySliderChildPopup.SetTicFreq(100);
	mySliderChildPopup.SetLineSize(100);
	mySliderChildPopup.SetPageSize(200);


	mySliderScroll.SetRange(50, 200);
	mySliderScroll.SetTicFreq(25);
	mySliderScroll.SetLineSize(25);
	mySliderScroll.SetPageSize(50);


	mySpinWheel.SetRange(2, 20);
	mySpinPage.SetRange(3, 30);
}

//////////////////////////////////////////////////////////////////////////////////////////////

void DlgOptionsPropSheet_MenusScrolling::LoadFromPreferences( ApplicationSettings * thePr )
{
	////////////////////////////////////////////////////////
	// 1. Maximum menu width
	bool aPrc = thePr->GetIsMaxMenuWidthInPercent();
	int aVal = aPrc ? 1 : 0;
	DDX_Radio(IDC_O_MNSC_MNUWIDTH_0PXL, aVal, FALSE);

	aVal = aPrc ? -thePr->GetMaxMenuWidth() : thePr->GetMaxMenuWidth();
	DDX_Int(IDC_O_MNSC_MAXMNUWIDTH, aVal, TRUE, FALSE);

	UpdateWidthConstraints(aPrc);


	////////////////////////////////////////////////////////
	// 2. Child Menu delay
	mySliderChildPopup.SetPos( thePr->GetTimeoutSec() );
	TrackBarUtility::DisplayTimeoutText(&myValueChildPopup, thePr->GetTimeoutSec() );



	////////////////////////////////////////////////////////
	// 3. Scroll offsets (mouse wheel and page up/page down keys)
	DDX_Int(IDC_O_MNSC_WHEELSCROLLS, thePr->myScrollWheel, FALSE, FALSE);
	DDX_Int(IDC_O_MNSC_PAGESCROLLS, thePr->myScrollPage, FALSE, FALSE);



	////////////////////////////////////////////////////////
	// 4. Scroll item positioning
	ApplicationSettings::ScrollItemFlags aScrollFlags = thePr->GetScrollItemPositioning();

	myRbScrollAtTop.SetCheck(aScrollFlags == ApplicationSettings::SIF_POS_TOP);
	myRbScrollAtBottom.SetCheck(aScrollFlags == ApplicationSettings::SIF_POS_BOTTOM);
	myRbScrollRespective.SetCheck(aScrollFlags == ApplicationSettings::SIF_POS_RESPECTIVE);


	////////////////////////////////////////////////////////
	// 5. Show/hide number of remaining items to scroll
	bool aShowNumOfScrollItems = thePr->GetShowNumOfScrollItems();
	myCbShowNumOfScrollItems.SetCheck(aShowNumOfScrollItems ? 1 : 0);


	////////////////////////////////////////////////////////
	// 6. Scrolling delay
	mySliderScroll.SetPos(  thePr->GetTimeoutScroll() );
	TrackBarUtility::DisplayTimeoutText(&myValueScroll, thePr->GetTimeoutScroll() );
}



//////////////////////////////////////////////////////////////////////////////////////////////

void DlgOptionsPropSheet_MenusScrolling::LoadLocalization()
{
	LocalizationUtility::LocTriplet aLocalizationArray [] = 
	{
		LOCENTRY_SELF(IDD_O_MNSC),

		LOCENTRY_SM(IDL_O_MNSC_CAT_MENUS),
		LOCENTRY_SM(IDL_O_MNSC_MAXMNUWIDTH),
		LOCENTRY_SM(IDC_O_MNSC_MAXMNUWIDTH),
		LOCENTRY_SM(IDC_O_MNSC_MNUWIDTH_0PXL),
		LOCENTRY_SM(IDC_O_MNSC_MNUWIDTH_1PRC),
		LOCENTRY_SM(IDL_O_MNSC_CHILDDELAY),
		LOCENTRY_0M(IDC_O_MNSC_CHILDDELAY),
		LOCENTRY_0M(IDL_O_MNSC_CHILDDELAYVAL),

		LOCENTRY_SM(IDL_O_MNSC_CAT_SCROLLING),
		LOCENTRY_SM(IDL_O_MNSC_WHEELSCROLLS),
		LOCENTRY_0M(IDC_O_MNSC_WHEELSCROLLS),
		LOCENTRY_SM(IDL_O_MNSC_WHEELITEMS),
		LOCENTRY_SM(IDL_O_MNSC_PAGESCROLLS),
		LOCENTRY_0M(IDC_O_MNSC_PAGESCROLLS),
		LOCENTRY_SM(IDL_O_MNSC_PAGEITEMS),
		LOCENTRY_SM(IDL_O_MNSC_SCROLLARROWS),
		LOCENTRY_SM(IDC_O_MNSC_SCROLLPOS_0TOP),
		LOCENTRY_SM(IDC_O_MNSC_SCROLLPOS_1BOTM),
		LOCENTRY_SM(IDC_O_MNSC_SCROLLPOS_2ENDS),
		LOCENTRY_SM(IDC_O_MNSC_DISPNUMTOSCROLL),
		LOCENTRY_SM(IDL_O_MNSC_SCROLLDELAY),
		LOCENTRY_0M(IDC_O_MNSC_SCROLLDELAY),
		LOCENTRY_0M(IDL_O_MNSC_SCROLLDELAYVAL),
	};

	int aArraySize = sizeof aLocalizationArray  /  sizeof aLocalizationArray[0];

	LocalizationUtility::LocalizeDialogBox(m_hWnd, myLocMan, aLocalizationArray, aArraySize);



	LocalizationUtility::LocPair aTips [] = 
	{
		LOCENTRY_H(IDC_O_MNSC_MAXMNUWIDTH),
		LOCENTRY_H(IDC_O_MNSC_MNUWIDTH_0PXL),
		LOCENTRY_H(IDC_O_MNSC_MNUWIDTH_1PRC),
		LOCENTRY_H(IDC_O_MNSC_CHILDDELAY),

		LOCENTRY_H(IDC_O_MNSC_WHEELSCROLLS),
		LOCENTRY_H(IDC_O_MNSC_PAGESCROLLS),
		LOCENTRY_H(IDC_O_MNSC_SCROLLPOS_0TOP),
		LOCENTRY_H(IDC_O_MNSC_SCROLLPOS_1BOTM),
		LOCENTRY_H(IDC_O_MNSC_SCROLLPOS_2ENDS),
		LOCENTRY_H(IDC_O_MNSC_DISPNUMTOSCROLL),
		LOCENTRY_H(IDC_O_MNSC_SCROLLDELAY),

		{ IDC_O_MNSC_MAXMNUWIDTHSPIN,  TIPOF(IDC_O_MNSC_MAXMNUWIDTH)  }, // Tips for up-down controls are
		{ IDC_O_MNSC_WHEELSCROLLSSPIN, TIPOF(IDC_O_MNSC_WHEELSCROLLS) }, //  the same as those for their
		{ IDC_O_MNSC_PAGESCROLLSSPIN,  TIPOF(IDC_O_MNSC_PAGESCROLLS)  }, //  corresponding text fields
	};

	aArraySize = sizeof aTips  /  sizeof aTips[0];

	LocalizationUtility::AddTooltips(m_hWnd, myLocMan, myHost, aTips, aArraySize);


	mySpinWheel.SetBuddy(myScrollItemsWheel);
	mySpinPage.SetBuddy(myScrollItemsPage);
	mySpinMaxMenuWidth.SetBuddy(myMaxMenuWidth);
}

//////////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP DlgOptionsPropSheet_MenusScrolling::OnSave()
{
	int aVal = myCbShowNumOfScrollItems.GetCheck();
	myPrefs->SetShowNumOfScrollItems(aVal != 0);

	if ( myRbScrollRespective.GetCheck() & 1)
		myPrefs->SetScrollItemPositioning(ApplicationSettings::SIF_POS_RESPECTIVE);
	else if (myRbScrollAtTop.GetCheck() & 1)
		myPrefs->SetScrollItemPositioning(ApplicationSettings::SIF_POS_TOP);
	else
		myPrefs->SetScrollItemPositioning(ApplicationSettings::SIF_POS_BOTTOM);


	myPrefs->SetTimeoutScroll( (UINT) mySliderScroll.GetPos() );

	myPrefs->SetTimeoutSec(    (UINT) mySliderChildPopup.GetPos() );

	DDX_Int(IDC_O_MNSC_WHEELSCROLLS, myPrefs->myScrollWheel, FALSE, TRUE);
	DDX_Int(IDC_O_MNSC_PAGESCROLLS, myPrefs->myScrollPage, FALSE, TRUE);


	int aMaxMenuWidth = 0;
	DDX_Int(IDC_O_MNSC_MAXMNUWIDTH, aMaxMenuWidth, TRUE, TRUE);

	DDX_Radio(IDC_O_MNSC_MNUWIDTH_0PXL, aVal, TRUE);

	if (aVal == 1)
		aMaxMenuWidth = -aMaxMenuWidth;

	myPrefs->SetMaxMenuWidth(aMaxMenuWidth);

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////////////////////////

LRESULT DlgOptionsPropSheet_MenusScrolling::MsgHandler_Scroll( UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/ )
{
	HWND aSender = (HWND) lParam;

	CTrackBarCtrl * aSenderCtrl = 0;
	CStatic * aValueCtrl = 0;

	if (aSender == mySliderChildPopup)
	{ aSenderCtrl = &mySliderChildPopup; aValueCtrl = & myValueChildPopup; }

	else if (aSender == mySliderScroll)
	{ aSenderCtrl = &mySliderScroll; aValueCtrl = & myValueScroll; }

	TrackBarUtility::HandleTimeoutTrack(wParam, aSenderCtrl, aValueCtrl);
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////////

LRESULT DlgOptionsPropSheet_MenusScrolling::MsgHandler_Cmd_ToggleUnits(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	bool aInPercent = (wID == IDC_O_MNSC_MNUWIDTH_1PRC);

	if (aInPercent == myInPercent) return 0;

	int aScreenWidth = GetSystemMetrics(SM_CXSCREEN), aActualWidth = 0;

	DDX_Int(IDC_O_MNSC_MAXMNUWIDTH, aActualWidth, TRUE, TRUE);

	UpdateWidthConstraints(aInPercent);

	if (aInPercent)	//from pixels to percent
		aActualWidth = aActualWidth * 100 / aScreenWidth;
	else
		aActualWidth = aActualWidth * aScreenWidth / 100;

	DDX_Int(IDC_O_MNSC_MAXMNUWIDTH, aActualWidth, TRUE, FALSE);

	return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////

void DlgOptionsPropSheet_MenusScrolling::UpdateWidthConstraints(bool theAsPercent)
{
	int aMinMax = 100;	//the max cannot be less that 100px;
	int aScreenWidth = GetSystemMetrics(SM_CXSCREEN);

	int aMinMaxActual, aMaxMaxActual;

	if (theAsPercent)
	{
		aMinMaxActual = aMinMax * 100 / aScreenWidth;
		aMaxMaxActual = 33;
	}
	else
	{
		aMinMaxActual = aMinMax;
		aMaxMaxActual = aScreenWidth / 3;
	}

	mySpinMaxMenuWidth.SetRange(aMinMaxActual, aMaxMaxActual);
	myInPercent = theAsPercent;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//                    DlgOptionsPropSheet_Contents class (Contents Tab)                     //
//////////////////////////////////////////////////////////////////////////////////////////////

void DlgOptionsPropSheet_Contents::InitControls()
{
	myGroup_Contents       = GetDlgItem(IDL_O_CNTS_CAT_ITEMS);
	mySelHiddenItems       = GetDlgItem(IDC_O_CNTS_HDNITMS);
	myCbDimmedIcon         = GetDlgItem(IDC_O_CNTS_HDNITMS_DIMICN);
	mySelBrowseFolders     = GetDlgItem(IDC_O_CNTS_BROWSEFLD);
	myRbTreatSsfAsFolders  = GetDlgItem(IDC_O_CNTS_SSFIS_0FOLDER);
	myRbTreatSsfAsFiles    = GetDlgItem(IDC_O_CNTS_SSFIS_1FILE);
	myRbTreatZipAsFolders  = GetDlgItem(IDC_O_CNTS_ZIPIS_0FOLDER);
	myRbTreatZipAsFiles    = GetDlgItem(IDC_O_CNTS_ZIPIS_1FILE);
	mySelMouseInvocation   = GetDlgItem(IDC_O_CNTS_MOUSEITEMCLK);

	myGroup_Order          = GetDlgItem(IDL_O_CNTS_CAT_ORDER);
	myCbSortAlphabetically = GetDlgItem(IDC_O_CNTS_SORTITEMS);
	myRbFoldersPrecede     = GetDlgItem(IDC_O_CNTS_FLDORD_0TOP);
	myRbFoldersFollow      = GetDlgItem(IDC_O_CNTS_FLDORD_1BOTM);
	myRbFoldersMix         = GetDlgItem(IDC_O_CNTS_FLDORD_2MIX);

	myGroup_ClickItems     = GetDlgItem(IDL_O_CNTS_CAT_ICONS);
	myRbGenericIcons       = GetDlgItem(IDC_O_CNTS_ICN_0GENERIC);
	myRbRegularIcons       = GetDlgItem(IDC_O_CNTS_ICN_1REGULAR);
	myCbUseOverlays        = GetDlgItem(IDC_O_CNTS_ICNOVERLAYS);
	myCbOptimizeIconExtr   = GetDlgItem(IDC_O_CNTS_ICNOPTIMIZ);
}

//////////////////////////////////////////////////////////////////////////////////////////////

void DlgOptionsPropSheet_Contents::LoadFromPreferences(ApplicationSettings * thePr)
{
	////////////////////////////////////////////////////////
	// 1. Browse folders mode
	for (int i = 0; i != mySelBrowseFolders.GetCount(); i++)
	{
		DWORD_PTR aData = mySelBrowseFolders.GetItemData(i);
		if ( thePr->GetBrowseForFoldersMode() == aData)
		{
			mySelBrowseFolders.SetCurSel(i);
			break;
		}
	}


	////////////////////////////////////////////////////////
	// 2. Hidden items mode
	for (int i = 0; i != mySelHiddenItems.GetCount(); i++)
	{
		DWORD_PTR aData = mySelHiddenItems.GetItemData(i);
		if ( thePr->GetHiddenFilesMode() == aData)
		{
			mySelHiddenItems.SetCurSel(i);
			break;
		}
	}


	////////////////////////////////////////////////////////
	// 3. Dimmed icon
	myCbDimmedIcon.SetCheck(thePr->GetDimHiddenItems() ? 1 : 0);


	////////////////////////////////////////////////////////
	// 4. Folder shortcut handling mode
	if ( thePr->GetTreatFolderLinksAsFolders() )
	{
		myRbTreatSsfAsFolders.SetCheck(1);
		myRbTreatSsfAsFiles  .SetCheck(0);
	}
	else
	{
		myRbTreatSsfAsFolders.SetCheck(0);
		myRbTreatSsfAsFiles  .SetCheck(1);
	}


	////////////////////////////////////////////////////////
	// 5. ZIP archive handling mode
	if ( thePr->GetTreatZipArchivesAsFolders() )
	{
		myRbTreatZipAsFolders.SetCheck(1);
		myRbTreatZipAsFiles  .SetCheck(0);
	}
	else
	{
		myRbTreatZipAsFolders.SetCheck(0);
		myRbTreatZipAsFiles  .SetCheck(1);
	}


	////////////////////////////////////////////////////////
	// 6. Item mouse invocation mode
	int aVal = thePr->GetActivateMode();
	mySelMouseInvocation.SetCurSel(aVal);


	////////////////////////////////////////////////////////
	// 7. Item list sorting mode
	if ( thePr->GetFoldersPrecedeFiles() )
	{
		myRbFoldersPrecede.SetCheck(1);
		myRbFoldersFollow .SetCheck(0);
		myRbFoldersMix    .SetCheck(0);
	}
	else if (thePr->GetFilesPrecedeFolders() )
	{
		myRbFoldersPrecede.SetCheck(0);
		myRbFoldersFollow .SetCheck(1);
		myRbFoldersMix    .SetCheck(0);
	}
	else
	{
		myRbFoldersPrecede.SetCheck(0);
		myRbFoldersFollow .SetCheck(0);
		myRbFoldersMix    .SetCheck(1);
	}

	myCbSortAlphabetically.SetCheck( thePr->GetSortAlphabetically() );


	////////////////////////////////////////////////////////
	// 8. Icon handling mode
	bool aGenericIcons = thePr->GetUseGenericIcons();
	myRbRegularIcons.SetCheck(aGenericIcons ? 0 : 1);
	myRbGenericIcons.SetCheck(aGenericIcons ? 1 : 0);

	bool aIconOverlays = thePr->GetUseIconOverlays();
	myCbUseOverlays.SetCheck(aIconOverlays ? 1 : 0);

	int aNumWorkerThreads = thePr->GetIconFetchParallelization();
	myCbOptimizeIconExtr.SetCheck(aNumWorkerThreads == 10 ? 1 : 0);

	UpdateNormalIconsRelatedControls();
	UpdateDimmedIconCheckboxStatus();
}

//////////////////////////////////////////////////////////////////////////////////////////////

void DlgOptionsPropSheet_Contents::LoadLocalization()
{
	// ************************************* Single strings and Metrics
	LocalizationUtility::LocTriplet aLocalizationArray [] = 
	{
		LOCENTRY_SELF(IDD_O_CNTS),

		LOCENTRY_SM(IDL_O_CNTS_CAT_ITEMS),
		LOCENTRY_SM(IDL_O_CNTS_HDNITMS),
		LOCENTRY_0M(IDC_O_CNTS_HDNITMS),
		LOCENTRY_SM(IDC_O_CNTS_HDNITMS_DIMICN),
		LOCENTRY_SM(IDL_O_CNTS_BROWSEFLD),
		LOCENTRY_0M(IDC_O_CNTS_BROWSEFLD),
		LOCENTRY_SM(IDL_O_CNTS_SSFISWHAT),
		LOCENTRY_SM(IDC_O_CNTS_SSFIS_0FOLDER),
		LOCENTRY_SM(IDC_O_CNTS_SSFIS_1FILE),
		LOCENTRY_SM(IDL_O_CNTS_ZIPISWHAT),
		LOCENTRY_SM(IDC_O_CNTS_ZIPIS_0FOLDER),
		LOCENTRY_SM(IDC_O_CNTS_ZIPIS_1FILE),
		LOCENTRY_SM(IDL_O_CNTS_MOUSEITEMCLK),
		LOCENTRY_0M(IDC_O_CNTS_MOUSEITEMCLK),

		LOCENTRY_SM(IDL_O_CNTS_CAT_ORDER),
		LOCENTRY_SM(IDC_O_CNTS_SORTITEMS),
		LOCENTRY_SM(IDC_O_CNTS_FLDORD_0TOP),
		LOCENTRY_SM(IDC_O_CNTS_FLDORD_1BOTM),
		LOCENTRY_SM(IDC_O_CNTS_FLDORD_2MIX),

		LOCENTRY_SM(IDL_O_CNTS_CAT_ICONS),
		LOCENTRY_SM(IDC_O_CNTS_ICN_0GENERIC),
		LOCENTRY_SM(IDC_O_CNTS_ICN_1REGULAR),
		LOCENTRY_SM(IDC_O_CNTS_ICNOVERLAYS),
		LOCENTRY_SM(IDC_O_CNTS_ICNOPTIMIZ),
	};

	int ArraySize = sizeof aLocalizationArray  /  sizeof aLocalizationArray[0];

	LocalizationUtility::LocalizeDialogBox(m_hWnd, myLocMan, aLocalizationArray, ArraySize);


	//  ************************************* Combo Box Strings

	LocalizationUtility::LocPair aQueries_Browse [] = 
	{
		{ ApplicationSettings::BROWSE_NEWWINDOW,            KEYOF(IDS_O_CNTS_BROWSEFLD_0NEWWND)  },
		{ ApplicationSettings::BROWSE_EXISTINGWINDOW,       KEYOF(IDS_O_CNTS_BROWSEFLD_1SAMEWND) },
		{ ApplicationSettings::BROWSE_USESYSTEM,            KEYOF(IDS_GLBL_USESYSSETTING)        },
	};

	ArraySize = sizeof aQueries_Browse / sizeof aQueries_Browse[0];

	LocalizationUtility::LocalizeComboBox(mySelBrowseFolders, myLocMan, aQueries_Browse, ArraySize);


	LocalizationUtility::LocPair aQueries_Hidden [] = 
	{
		{ ApplicationSettings::HIDDEN_SHOWALWAYS,           KEYOF(IDS_GLBL_ALWAYS)               },
		{ ApplicationSettings::HIDDEN_SHOWNEVER,            KEYOF(IDS_GLBL_NEVER)                },
		{ ApplicationSettings::HIDDEN_USESYSTEM,            KEYOF(IDS_GLBL_USESYSSETTING)        },
	};

	ArraySize = sizeof aQueries_Hidden / sizeof aQueries_Hidden[0];

	LocalizationUtility::LocalizeComboBox(mySelHiddenItems, myLocMan, aQueries_Hidden, ArraySize);


	LocalizationUtility::LocPair aQueries_Invocation [] = 
	{
		{ ApplicationSettings::ACTIVATE_SINGLE_CLICK_FILES, KEYOF(IDS_O_CNTS_MOUSEITEMCLK_0REG)  },
		{ ApplicationSettings::ACTIVATE_SINGLE_CLICK_ALL,   KEYOF(IDS_O_CNTS_MOUSEITEMCLK_1SGL)  },
		{ ApplicationSettings::ACTIVATE_DOUBLE_CLICK_ALL,   KEYOF(IDS_O_CNTS_MOUSEITEMCLK_2DBL)  },
	};

	ArraySize = sizeof aQueries_Invocation / sizeof aQueries_Invocation[0];

	LocalizationUtility::LocalizeComboBox(mySelMouseInvocation, myLocMan, aQueries_Invocation, ArraySize);


	//  ************************************* Tips

	LocalizationUtility::LocPair aTips [] = 
	{
		LOCENTRY_H(IDC_O_CNTS_HDNITMS),
		LOCENTRY_H(IDC_O_CNTS_HDNITMS_DIMICN),
		LOCENTRY_H(IDC_O_CNTS_BROWSEFLD),
		LOCENTRY_H(IDC_O_CNTS_SSFIS_0FOLDER),
		LOCENTRY_H(IDC_O_CNTS_SSFIS_1FILE),
		LOCENTRY_H(IDC_O_CNTS_ZIPIS_0FOLDER),
		LOCENTRY_H(IDC_O_CNTS_ZIPIS_1FILE),
		LOCENTRY_H(IDC_O_CNTS_MOUSEITEMCLK),

		LOCENTRY_H(IDC_O_CNTS_SORTITEMS),
		LOCENTRY_H(IDC_O_CNTS_FLDORD_0TOP),
		LOCENTRY_H(IDC_O_CNTS_FLDORD_1BOTM),
		LOCENTRY_H(IDC_O_CNTS_FLDORD_2MIX),

		LOCENTRY_H(IDC_O_CNTS_ICN_0GENERIC),
		LOCENTRY_H(IDC_O_CNTS_ICN_1REGULAR),
		LOCENTRY_H(IDC_O_CNTS_ICNOVERLAYS),
		LOCENTRY_H(IDC_O_CNTS_ICNOPTIMIZ),
	};

	ArraySize = sizeof aTips  /  sizeof aTips[0];

	LocalizationUtility::AddTooltips(m_hWnd, myLocMan, myHost, aTips, ArraySize);
}

//////////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP DlgOptionsPropSheet_Contents::OnSave()
{
	int aVal = myRbGenericIcons.GetCheck();
	myPrefs->SetUseGenericIcons(aVal != 0);

	aVal = myCbUseOverlays.GetCheck();
	myPrefs->SetUseIconOverlays(aVal != 0);

	aVal = myCbOptimizeIconExtr.GetCheck();
	myPrefs->SetIconFetchParallelization(aVal == 1 ? 10 : 1);


	aVal = 0;
	if ( myRbTreatSsfAsFiles.GetCheck() )
		aVal |= ApplicationSettings::FTH_IGNORE_FLDLINKS;

	if ( myRbTreatZipAsFolders.GetCheck() )
		aVal |= ApplicationSettings::FTH_EXPAND_ZIPARCH;

	myPrefs->SetFileTypeHandlingFlags(aVal);


	aVal = 0;
	if ( myCbSortAlphabetically.GetCheck() != 0)
		aVal |= ApplicationSettings::ORDER_ALPHABETICAL;

	if ( myRbFoldersPrecede.GetCheck() != 0)
		aVal |= ApplicationSettings::ORDER_FOLDERS_ONTOP;
	else if ( myRbFoldersFollow.GetCheck() != 0)
		aVal |= ApplicationSettings::ORDER_FOLDERS_ONBTM;

	myPrefs->SetItemOrdering(aVal);


	aVal = mySelBrowseFolders.GetCurSel();
	myPrefs->SetBrowseForFoldersMode( (PrefsSerz::DataType) mySelBrowseFolders.GetItemData(aVal) );

	aVal = mySelHiddenItems.GetCurSel();
	myPrefs->SetHiddenFilesMode( (PrefsSerz::DataType) mySelHiddenItems.GetItemData(aVal) );

	aVal = myCbDimmedIcon.GetCheck();
	myPrefs->SetDimHiddenItems(aVal != 0);

	aVal = mySelMouseInvocation.GetCurSel();
	myPrefs->SetActivateMode( (PrefsSerz::DataType) mySelMouseInvocation.GetItemData(aVal) );

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////////////////////////

LRESULT DlgOptionsPropSheet_Contents::MsgHandler_Cmd_Icons( WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/ )
{
	UpdateNormalIconsRelatedControls();

	return 0L;
}

//////////////////////////////////////////////////////////////////////////////////////////////

LRESULT DlgOptionsPropSheet_Contents::MsgHandler_Cmd_HiddenItems( WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/ )
{
	UpdateDimmedIconCheckboxStatus();

	return 0L;
}

//////////////////////////////////////////////////////////////////////////////////////////////

void DlgOptionsPropSheet_Contents::UpdateNormalIconsRelatedControls()
{
	bool aNormalIcons = myRbRegularIcons.GetCheck() == 1;

	myCbUseOverlays.EnableWindow(aNormalIcons);
	myCbOptimizeIconExtr.EnableWindow(aNormalIcons);
}

//////////////////////////////////////////////////////////////////////////////////////////////

void DlgOptionsPropSheet_Contents::UpdateDimmedIconCheckboxStatus()
{
	bool aNotHideAlways = mySelHiddenItems.GetCurSel() != 1;
	myCbDimmedIcon.EnableWindow(aNotHideAlways);
}

//////////////////////////////////////////////////////////////////////////////////////////////
//                 DlgOptionsPropSheet_Maintenance class (Maintenance Tab)                  //
//////////////////////////////////////////////////////////////////////////////////////////////

DlgOptionsPropSheet_Maintenance::DlgOptionsPropSheet_Maintenance() :
	myPlgnMan( Framework::GetPluginManager() ),
	myIsCheckingUpdates(false),
	myPluginListInitialized(false)
{ }

//////////////////////////////////////////////////////////////////////////////////////////////

void DlgOptionsPropSheet_Maintenance::InitControls()
{
	myGroup_Maint          = GetDlgItem(IDL_O_MNTC_CAT_MNTC);
	myLanguages            = GetDlgItem(IDC_O_MNTC_UILANG);
	myCbUsePeriodicUpdates = GetDlgItem(IDC_O_MNTC_UPDATEAUTO);
	myBtnCheckForUpdates   = GetDlgItem(IDC_O_MNTC_UPDATE);
	myCheckBoxAutoRun      = GetDlgItem(IDC_O_MNTC_AUTORUN);

	myGroup_Plugins        = GetDlgItem(IDL_O_MNTC_CAT_PLUGINS);
	myPluginList           = GetDlgItem(IDC_O_MNTC_PLGNLST);
	myBtnEnable            = GetDlgItem(IDC_O_MNTC_PLGNENBL);
	myBtnDisable           = GetDlgItem(IDC_O_MNTC_PLGNDSBL);


	DWORD aExStyle = myPluginList.GetExtendedListViewStyle();
	aExStyle |= LVS_EX_FULLROWSELECT;
	myPluginList.SetExtendedListViewStyle(aExStyle);

	// insert column stubs
	myPluginList.InsertColumn(0, _T("a") );
	myPluginList.InsertColumn(1, _T("b") );


	DisplayLocalizationDescription( myLocMan->GetCurrentLocalization() );
	myLocMan->EnumerateLocalizations(this);


	myCheckBoxAutoRun.SetCheck( StartupShortcut(0) );
}

//////////////////////////////////////////////////////////////////////////////////////////////

void DlgOptionsPropSheet_Maintenance::LoadFromPreferences(ApplicationSettings * thePr)
{
	const TCHAR * aLoc_Cur = thePr->GetLocale(NULL);

	int aN = myLanguages.GetCount();

	for( int i = 0; i < aN; i++)
	{
		DWORD_PTR aItemData = myLanguages.GetItemData(i);
		ILocalization * aLoc = reinterpret_cast<ILocalization *>(aItemData);

		TCHAR aLoc_Itm[6];
		int aSize_Itm = 6;

		HRESULT aRes = aLoc->GetLocale(aLoc_Itm, &aSize_Itm);

		if ( FAILED(aRes) )
			continue;

		int aStringsEq = lstrcmpi(aLoc_Itm, aLoc_Cur);

		if ( aStringsEq == 0)
		{
			myLanguages.SetCurSel(i);
			break;
		}
	}

	PopulatePluginList(thePr);
	UpdateButtonState(0);

	myCbUsePeriodicUpdates.SetCheck( thePr->GetEnablePeriodicUpdateChecks() );
}

//////////////////////////////////////////////////////////////////////////////////////////////

void DlgOptionsPropSheet_Maintenance::LoadLocalization()
{
	LocalizationUtility::LocTriplet aLocalizationArray [] = 
	{
		LOCENTRY_SELF(IDD_O_MNTC),

		LOCENTRY_SM(IDL_O_MNTC_CAT_MNTC),
		LOCENTRY_SM(IDL_O_MNTC_UILANG),
		LOCENTRY_0M(IDC_O_MNTC_UILANG),
		LOCENTRY_0M(IDC_O_MNTC_LOCALEINFO),
		LOCENTRY_SM(IDL_O_MNTC_UPDATE),
		LOCENTRY_SM(IDC_O_MNTC_UPDATE),
		LOCENTRY_SM(IDC_O_MNTC_UPDATEAUTO),
		LOCENTRY_SM(IDL_O_MNTC_RESTDEF),
		LOCENTRY_SM(IDC_O_MNTC_RESTDEF),
		LOCENTRY_SM(IDL_O_MNTC_AUTORUN),
		LOCENTRY_SM(IDC_O_MNTC_AUTORUN),

		LOCENTRY_SM(IDL_O_MNTC_CAT_PLUGINS),
		LOCENTRY_0M(IDC_O_MNTC_PLGNLST),
		LOCENTRY_SM(IDC_O_MNTC_PLGNENBL),
		LOCENTRY_SM(IDC_O_MNTC_PLGNDSBL),
	};

	int ArraySize = sizeof aLocalizationArray  /  sizeof aLocalizationArray[0];
	LocalizationUtility::LocalizeDialogBox(m_hWnd, myLocMan, aLocalizationArray, ArraySize);


	LocalizationUtility::LocPair aTips [] = 
	{
		LOCENTRY_H(IDC_O_MNTC_UILANG),
		LOCENTRY_H(IDC_O_MNTC_UPDATE),
		LOCENTRY_H(IDC_O_MNTC_UPDATEAUTO),
		LOCENTRY_H(IDC_O_MNTC_RESTDEF),
		LOCENTRY_H(IDC_O_MNTC_LOCALEINFO),
		LOCENTRY_H(IDC_O_MNTC_AUTORUN),

		LOCENTRY_H(IDC_O_MNTC_PLGNLST),
		LOCENTRY_H(IDC_O_MNTC_PLGNENBL),
		LOCENTRY_H(IDC_O_MNTC_PLGNDSBL),
	};

	ArraySize = sizeof aTips  /  sizeof aTips[0];

	LocalizationUtility::AddTooltips(m_hWnd, myLocMan, myHost, aTips, ArraySize);



	RECT aRect;
	myPluginList.GetClientRect(&aRect);

	int aWidth = aRect.right - aRect.left;


	TCHAR aText[256];

	LVCOLUMN aCol;
	aCol.pszText = aText;

	//       Name Column
	int aSize = 256;
	LocIdKey aKey = KEYOF(IDS_O_MNTC_PLGNLST_NAME);
	myLocMan->GetStringSafe(aKey, aText, &aSize);

	aCol.mask = LVCF_FMT;
	myPluginList.GetColumn(0, &aCol);

	aCol.cx = aWidth * 45 / 100;
	aCol.mask |= LVCF_TEXT|LVCF_WIDTH;
	myPluginList.SetColumn(0, &aCol);



	//       Status Column
	aSize = 256;
	aKey = KEYOF(IDS_O_MNTC_PLGNLST_STATUS);
	myLocMan->GetStringSafe(aKey, aText, &aSize);

	aCol.mask = LVCF_FMT;
	myPluginList.GetColumn(1, &aCol);

	aCol.cx = aWidth - aCol.cx;
	aCol.mask |= LVCF_TEXT|LVCF_WIDTH;
	myPluginList.SetColumn(1,  &aCol);


	if (myPluginListInitialized)
	{
		PopulatePluginList(myPrefs);
		UpdateButtonState(0);
	}


	//TODO: plug-in status display strings
}

//////////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP DlgOptionsPropSheet_Maintenance::OnSave()
{
	StartupShortcut( myCheckBoxAutoRun.GetCheck() != 0 ? 1 : -1 );

	myPrefs->SetEnablePeriodicUpdateChecks(myCbUsePeriodicUpdates.GetCheck() != 0);

	return ApplyLocalization();
}

//////////////////////////////////////////////////////////////////////////////////////////////

void DlgOptionsPropSheet_Maintenance::OnLocalizationFound( ILocalization * theGet)
{
	// we query three strings : first, describing the localization in the locale of
	// the current localization, second, describing the localization in the target
	// locale, and the third one, describing the localization in the default program
	// locale, English (en_US)

	TCHAR aBuf_Cur[500], aBuf_Targ[500], aBuf_Eng[500];
	TCHAR aLoc_Cur[6], aLoc_Targ[6], aLoc_Eng[6] = _T("en_US");

	aBuf_Cur[0] = aBuf_Targ[0] = aBuf_Eng[0] = 0;
	aLoc_Cur[0] = aLoc_Targ[0] = 0;

	int aLen_Cur = 500, aLen_Targ = 500, aLen_Eng = 500;
	int aLen_LocCur = 6, aLen_LocTarg = 6;
	int aStatus = 0;


	ILocalization * aCurLoc = myLocMan->GetCurrentLocalization();
	HRESULT aRes = aCurLoc->GetLocale(aLoc_Cur, &aLen_LocCur);

	if ( FAILED(aRes) )
	{
		Trace("ERR: Cannot query the default locale ID\n");
		return;
	}

	aRes = theGet->GetLocale(aLoc_Targ, &aLen_LocTarg);

	if ( FAILED(aRes) )
	{
		Trace("ERR: Cannot query the localization's locale ID\n");
		return;
	}


	//Step 1. Query the Title in the localizations's locale
	if (true)
	{
		aRes = theGet->GetSummaryString(TITLE, aLoc_Targ, 5, aBuf_Targ, &aLen_Targ);

		if ( SUCCEEDED(aRes) )
			aStatus |= 0x1;
	}

	//Step 2. Query the Title in the current locale
	bool aIsCurrent = lstrcmpi(aLoc_Cur, aLoc_Targ) == 0;
	if (!aIsCurrent)
	{
		HRESULT aRes = theGet->GetSummaryString(TITLE, aLoc_Cur, 5, aBuf_Cur, &aLen_Cur);

		if ( SUCCEEDED(aRes) )
			aStatus |= 0x2;


		//Step 3. Query the Title in English locale explicitly
		if ( lstrcmpi(aLoc_Eng, aLoc_Targ) != 0 && lstrcmpi(aLoc_Eng, aLoc_Cur) != 0 )
		{
			aRes = theGet->GetSummaryString(TITLE, aLoc_Eng, 5, aBuf_Eng, &aLen_Eng);

			if ( SUCCEEDED(aRes) )
				aStatus |= 0x4;
		}
	}


	if ( (aStatus & 0x1) == 0 )
	{
		Trace("ERR: unable to query description in the localization's locale\n");
		return;
	}

	//Step 4. Buffers initialized, proceed to constructing the localization buffer

	//We display the string in the form: [target] - [current], where:
	//- [current] is the localization's title in the current locale, and
	//- [target]  is the localization's title in the localization's locale
	//The [current] title might be absent and [target] must not
	//Whenever [current] title is absent we try to query the English title, and,
	//if it is found, display the string like [target] -[English], where
	//- [English] is the localization's title in English locale (en_US)

	TCHAR aOut[1000];



	if ( (aStatus & 0x2) != 0 )	//contains the current locale's title
	{
		lstrcpyn(aOut, aBuf_Targ, aLen_Targ + 1);	//1 for the null terminator

		lstrcat(aOut, _T(" - ") );
		lstrcat(aOut, aBuf_Cur);

	}
	else if ( (aStatus & 0x4) != 0 ) //contains the English locale's title
	{
		lstrcpyn(aOut, aBuf_Targ, aLen_Targ + 1);	//1 for the null terminator

		lstrcat(aOut, _T(" - ") );
		lstrcat(aOut, aBuf_Eng);
	}
	else 
		lstrcpyn(aOut, aBuf_Targ, aLen_Targ + 1);	//1 for the null terminator


	int aIndex = myLanguages.AddString(aOut);
	myLanguages.SetItemData( aIndex, reinterpret_cast<DWORD_PTR>(theGet) );

	if (aIsCurrent)
		myLanguages.SetCurSel(aIndex);
}

//////////////////////////////////////////////////////////////////////////////////////////////

void DlgOptionsPropSheet_Maintenance::HandleUpdateCheckResult(UpdateCheckTask *)
{
	PostMessage(WM_USER_HANDLEUPDATERESULT);
}

//////////////////////////////////////////////////////////////////////////////////////////////

LRESULT DlgOptionsPropSheet_Maintenance::MsgHandler_Cmd_RestoreDefaults( WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& theHandled)
{
	Application::Instance().ResetPrefs();

	LocalizationUtility::LocalizedMessageBox(
		m_hWnd, myLocMan, KEYOF(IDS_O_MNTC_MSG_RESTDEF), 0, MB_OK|MB_ICONINFORMATION);

	return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////

LRESULT DlgOptionsPropSheet_Maintenance::MsgHandler_Cmd_ChangeLocale( WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	//generate the string

	int aCurSel = myLanguages.GetCurSel();

	if (aCurSel == -1)
		return 0;

	void * aParam = myLanguages.GetItemDataPtr(aCurSel);
	ILocalization * aLoc = reinterpret_cast<ILocalization *>(aParam);

	DisplayLocalizationDescription(aLoc);

	return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////

LRESULT DlgOptionsPropSheet_Maintenance::MsgHandler_Cmd_CheckForUpdates(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	myBtnCheckForUpdates.EnableWindow(FALSE);

	myUpdateCheckPackage.HandleUpdates(this);

	return 0L;
}

//////////////////////////////////////////////////////////////////////////////////////////////

LRESULT DlgOptionsPropSheet_Maintenance::MsgHandler_Cmd_TogglePlugin( WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/ )
{
	TCHAR aText[128];
	int aItemIndex = GetSelectedItemText(aText, 128);

	if (aItemIndex < 0)
		return 0L;

	TCHAR aStatus[256];
	int aSize = 256;


	if (wID == IDC_O_MNTC_PLGNDSBL)
	{
		if ( myPrefs->DisablePlugin(aText) )
		{
			FormatLocalizedPluginStatus(myPrefs, aText, aStatus, &aSize);
			myPluginList.SetItemText(aItemIndex, 1, aStatus);
			UpdateButtonState(-1);
		}
	}
	else
	{
		if ( myPrefs->EnablePlugin(aText) )
		{
			FormatLocalizedPluginStatus(myPrefs, aText, aStatus, &aSize);
			myPluginList.SetItemText(aItemIndex, 1, aStatus);
			UpdateButtonState(1);
		}
	}

	return 0L;
}

//////////////////////////////////////////////////////////////////////////////////////////////

LRESULT DlgOptionsPropSheet_Maintenance::MsgHandler_Notify_PluginSelect( int theControlID, LPNMHDR theHeader, BOOL& bHandled )
{
	UpdateButtonState(0);
	return 0L;
}

//////////////////////////////////////////////////////////////////////////////////////////////

LRESULT DlgOptionsPropSheet_Maintenance::MsgHandler_HandleUpdateResult(UINT theMsg, WPARAM theWParam, LPARAM theLParam, BOOL& bHandled)
{
	myBtnCheckForUpdates.EnableWindow();

	UpdateCheckTask::SaveUpdateCheckResult();

	if ( !myUpdateCheckPackage.GetData().HasUpdates() )
	{
		LocalizationUtility::LocalizedMessageBox(
			m_hWnd, myLocMan, KEYOF(IDS_O_MNTC_MSG_UPDNO), 0, MB_OK|MB_ICONINFORMATION);
	}
	else
	{
		INT_PTR aAnswer = LocalizationUtility::LocalizedMessageBox(
			m_hWnd, myLocMan, KEYOF(IDS_O_MNTC_MSG_UPDYES), 0, MB_YESNO|MB_ICONQUESTION);

		if (aAnswer == IDYES)
			ShellExecute(m_hWnd, L"open", myUpdateCheckPackage.GetData().Url(), 0, 0, SW_SHOW);
	}

	return 0L;
}

//////////////////////////////////////////////////////////////////////////////////////////////

void DlgOptionsPropSheet_Maintenance::DisplayLocalizationDescription(ILocalization * theLoc)
{
	//order: 1 Current; 2 English; 3; Target

	TCHAR aLocale[6],    aText[1000];
	int aLocaleSize = 6, aSize = 1000;

	ILocalization * aCurLoc = myLocMan->GetCurrentLocalization();
	HRESULT aRes = aCurLoc->GetLocale(aLocale, &aLocaleSize);

	aRes = DisplayLocalizationDescription(theLoc, aLocale, aLocaleSize, aText, &aSize);

	if ( FAILED(aRes) )
	{
		aLocaleSize = 5;
		lstrcpy( aLocale, _T("en_US") );
		aRes = DisplayLocalizationDescription(theLoc, aLocale, aLocaleSize, aText, &aSize);
	}

	if ( FAILED(aRes) )
	{
		aLocaleSize = 6;
		aRes = theLoc->GetLocale(aLocale, &aLocaleSize);
		aRes = DisplayLocalizationDescription(theLoc, aLocale, aLocaleSize, aText, &aSize);
	}

	if ( SUCCEEDED(aRes) )
		SetDlgItemText(IDC_O_MNTC_LOCALEINFO, aText);
}

//////////////////////////////////////////////////////////////////////////////////////////////

HRESULT DlgOptionsPropSheet_Maintenance::DisplayLocalizationDescription(
	ILocalization * theLoc,
	const TCHAR * theLocale, int theSizeLocale,
	TCHAR * theOut, int * theSizeOut)
{
	TCHAR * aPtr = theOut;
	int aSize = *theSizeOut, aCurLen = aSize;

	HRESULT aRes = theLoc->GetSummaryString(COMMENT, theLocale, theSizeLocale, aPtr, &aCurLen);

	if ( SUCCEEDED(aRes) )
	{
		aPtr[aCurLen+0] = _T('\r');
		aPtr[aCurLen+1] = _T('\n');

		aPtr += aCurLen + 2;
		aCurLen = aSize - aCurLen - 2;

		aRes = theLoc->GetSummaryString(CREATED_BY, theLocale, theSizeLocale, aPtr, &aCurLen);

		if (SUCCEEDED(aRes) )
			*theSizeOut = aCurLen;
	}
	return aRes;
}

//////////////////////////////////////////////////////////////////////////////////////////////

HRESULT DlgOptionsPropSheet_Maintenance::ApplyLocalization()
{
	DWORD_PTR aItemData = myLanguages.GetItemData( myLanguages.GetCurSel() );
	ILocalization * aGet = reinterpret_cast<ILocalization *> (aItemData);

	if (aGet == 0)
		return E_FAIL;

	TCHAR aLocale[6];
	int aSize = 6;

	HRESULT aRes = aGet->GetLocale(aLocale, &aSize);

	if ( SUCCEEDED(aRes) )
		myPrefs->SetLocale(aLocale, aSize);

	return aRes;
}

//////////////////////////////////////////////////////////////////////////////////////////////

void DlgOptionsPropSheet_Maintenance::PopulatePluginList(ApplicationSettings * thePr)
{
	myPluginList.DeleteAllItems();

	const FilePathProvider * aFPP = Application::InstanceC().GetFilePathProvider();

	//TODO: move to a separate utility class
	TCHAR aPath[1024];
	int aSize = 1024;
	HRESULT aRes = aFPP->GetPath(FilePathProvider::Dir_Plugins, aPath, &aSize);

	if ( FAILED(aRes) )
		return;

	HANDLE hDirValid = CreateFile(
		aPath,
		GENERIC_READ,                      // open for reading
		FILE_SHARE_READ|FILE_SHARE_WRITE,  // allow read/write access, but disallow delete access
		NULL,                              // no security
		OPEN_EXISTING,
		FILE_FLAG_BACKUP_SEMANTICS,        // trying to obtain a handle to a directory
		NULL );

	if (hDirValid == INVALID_HANDLE_VALUE)	//dir doesn't exist
		return;

	lstrcpyn(aPath + aSize, _T("\\*.dll"), 6     + 1);

	//begin the enumeration process
	WIN32_FIND_DATA aWFD;
	HANDLE aSearchRes = FindFirstFile(aPath, &aWFD);

	if (aSearchRes != INVALID_HANDLE_VALUE)
	{
		do
		{
			TCHAR aStatus[256];
			int aStatusSize = 256;

			FormatLocalizedPluginStatus(myPrefs, aWFD.cFileName, aStatus, &aStatusSize);

			myPluginList.AddItem(0, 0, aWFD.cFileName);
			myPluginList.AddItem(0, 1, aStatus);

		} while ( FindNextFile(aSearchRes, &aWFD) );

		FindClose(aSearchRes);
	}

	CloseHandle(hDirValid);
	myPluginListInitialized = true;
}

//////////////////////////////////////////////////////////////////////////////////////////////

void DlgOptionsPropSheet_Maintenance::UpdateButtonState(int theState)
{
	bool aIsDisabled = false;
	bool aIsEnabled  = false;

	if (theState == 0)
	{
		TCHAR aText[128];
		if ( GetSelectedItemText(aText, 128) >= 0 )
		{
			aIsDisabled = myPrefs->IsPluginDisabled(aText);
			aIsEnabled = !aIsDisabled;
		}
	}
	else
	{
		aIsDisabled = (theState < 0);
		aIsEnabled = !aIsDisabled;
	}

	myBtnEnable.EnableWindow(aIsDisabled);
	myBtnDisable.EnableWindow(aIsEnabled);
}

//////////////////////////////////////////////////////////////////////////////////////////////

int DlgOptionsPropSheet_Maintenance::GetSelectedItemText(TCHAR * theOut, int theTextBufSize)
{
	LVITEM aL;
	aL.mask = LVIF_TEXT;
	aL.pszText = theOut;
	aL.cchTextMax = theTextBufSize;
	aL.iItem = 0;
	aL.iSubItem = 0;

	BOOL aSucc = myPluginList.GetSelectedItem(&aL);

	if (aSucc == FALSE)
		return -1;
	else
		return aL.iItem;
}

//////////////////////////////////////////////////////////////////////////////////////////////

void DlgOptionsPropSheet_Maintenance::FormatLocalizedPluginStatus(ApplicationSettings * thePr, const TCHAR * thePluginName, TCHAR * theOutStatus, int * theStatusSize )
{
	PluginInfo aInfo;
	HRESULT aHRes = myPlgnMan->GetPluginInfo(thePluginName, &aInfo);

	bool aIsDisabled = thePr->IsPluginDisabled(thePluginName);
	LocIdKey aKey;

	if ( SUCCEEDED(aHRes) )
	{
		if (aIsDisabled)
			aKey = KEYOF(IDS_O_MNTC_PLGNLST_STOPPING);
		else
			aKey = KEYOF(IDS_O_MNTC_PLGNLST_LOADED);
	}
	else
	{
		if (aIsDisabled)
			aKey = KEYOF(IDS_O_MNTC_PLGNLST_DISABLED);
		else
			aKey = KEYOF(IDS_O_MNTC_PLGNLST_NOTLOADED);
	}

	aHRes = myLocMan->GetStringSafe(aKey, theOutStatus, theStatusSize);
}

//////////////////////////////////////////////////////////////////////////////////////////////

bool DlgOptionsPropSheet_Maintenance::StartupShortcut(int theCreate)	//-1 delete; 1 create; 0 do nothing
{
	TCHAR aPath[MAX_PATH];

	SHGetSpecialFolderPath(NULL, aPath, CSIDL_STARTUP, FALSE);

	lstrcat(aPath, _T("\\KO Approach.lnk") );

	WIN32_FIND_DATA aDt;

	HANDLE aFindHandle = FindFirstFile(aPath, &aDt);

	if (theCreate == 1)
	{
		if (aFindHandle == INVALID_HANDLE_VALUE)
		{
			CComPtr<IShellLink> aSL;
			HRESULT aRes = aSL.CoCreateInstance(CLSID_ShellLink);

			if ( SUCCEEDED(aRes) )
			{
				TCHAR aModulePath[1024];
				const FilePathProvider * aFPP = Application::InstanceC().GetFilePathProvider();

				////////////////////////////////////////////////////////////////////
				// 1. Path
				int aSize = 1024;
				aRes = aFPP->GetPath(FilePathProvider::File_MainModule, aModulePath, &aSize);

				if ( SUCCEEDED(aRes) )
					aRes = aSL->SetPath(aModulePath);

				////////////////////////////////////////////////////////////////////
				// 2. Working Directory
				aSize = 1024;
				aRes = aFPP->GetPath(FilePathProvider::Dir_Main, aModulePath, &aSize);

				if ( SUCCEEDED(aRes) )
					aRes = aSL->SetWorkingDirectory(aModulePath);

				////////////////////////////////////////////////////////////////////
				// TODO: Set KO Approach shortcut description

				if (SUCCEEDED(aRes))
				{
					CComQIPtr<IPersistFile> aPF(aSL);

					if (aPF != 0)
						aRes = aPF->Save(aPath, FALSE);
				}
			}
		}
	}
	else if (theCreate == -1)
	{
		if (aFindHandle != INVALID_HANDLE_VALUE)
			DeleteFile(aPath);
	}

	FindClose(aFindHandle);
	return aFindHandle != INVALID_HANDLE_VALUE;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//                        DlgOptionsPropSheet_About class (About Tab)                       //
//////////////////////////////////////////////////////////////////////////////////////////////

void DlgOptionsPropSheet_About::DoCreate( IPropPageHost * theHost, HWND theOwnerWnd )
{
	myPrefs = Application::InstanceC().Prefs();

	BaseClass::DoCreate(theHost, theOwnerWnd);
}

//////////////////////////////////////////////////////////////////////////////////////////////

LRESULT DlgOptionsPropSheet_About::MsgHandler_InitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	myLblThanks = GetDlgItem(IDC_O_ABT_THXLIST);
	myHorLine = GetDlgItem(IDC_O_ABT_HORRULE);


	//insert a LTR mark into the copyright string as it is always in English
	TCHAR aCopy[1000];
	aCopy[0] = _T('\x200E');
	TCHAR * aText = aCopy + 1;

	int aLength = GetDlgItemText(IDC_O_ABT_APPCOPY, aText, 1000);
	SetDlgItemText(IDC_O_ABT_APPCOPY, aCopy);


	SendDlgItemMessage(IDC_O_ABT_APPICON, STM_SETICON, (WPARAM) Application::Instance().GetIconManager().GetApplicationIcon(), 0);

	return FALSE;
}

//////////////////////////////////////////////////////////////////////////////////////////////

void DlgOptionsPropSheet_About::LoadLocalization()
{
	// ************************************* Controls
	LocalizationUtility::LocTriplet aLocalizationArray [] = 
	{
		LOCENTRY_SELF(IDD_O_ABT),

		LOCENTRY_0M(IDC_O_ABT_APPNAME),
		LOCENTRY_0M(IDC_O_ABT_APPVER),
		LOCENTRY_0M(IDC_O_ABT_APPCOPY),
		LOCENTRY_0M(IDC_O_ABT_APPICON),

		LOCENTRY_SM(IDC_O_ABT_FEEDBACK),
		LOCENTRY_SM(IDC_O_ABT_HOMEPAGE),

		LOCENTRY_0M(IDC_O_ABT_HORRULE),

		LOCENTRY_SM(IDC_O_ABT_THXCAPTION),
		LOCENTRY_0M(IDC_O_ABT_THXLIST),
	};

	int ArraySize = sizeof aLocalizationArray  /  sizeof aLocalizationArray[0];

	LocalizationUtility::LocalizeDialogBox(m_hWnd, myLocMan, aLocalizationArray, ArraySize);


	if (myLnkFeedback == 0)
		myLnkFeedback.SubclassWindow( GetDlgItem(IDC_O_ABT_FEEDBACK) );

	if (myLnkHomepage == 0)
		myLnkHomepage.SubclassWindow( GetDlgItem(IDC_O_ABT_HOMEPAGE) );

	myLnkFeedback.SetHyperLinkExtendedStyle(HLINK_NOTOOLTIP);
	myLnkHomepage.SetHyperLinkExtendedStyle(HLINK_NOTOOLTIP);

	myLnkFeedback.SetHyperLink( myPrefs->GetUrl(ApplicationSettings::URL_FEEDBACK) );
	myLnkHomepage.SetHyperLink( myPrefs->GetUrl(ApplicationSettings::URL_HOME) );


	// ************************************* Thanks
	HRESULT aRes = ProcessSpecialThanksSection( myLocMan->GetCurrentLocalization() );

	if (FAILED(aRes) )
		aRes = ProcessSpecialThanksSection( myLocMan->GetDefaultLocalization() );

	ATLASSERT( SUCCEEDED(aRes) );


	// ************************************* Version string

	TCHAR aModuleFileName[MAX_PATH];
	int aLength = GetModuleFileName(0, aModuleFileName, MAX_PATH);
	DWORD aHandle = 0;
	DWORD aSize = GetFileVersionInfoSize(aModuleFileName, &aHandle);

	if (aSize != 0)
	{
		char * aVersionBuf = new char[aSize];

		BOOL aVerRes = GetFileVersionInfo(aModuleFileName, aHandle, aSize, aVersionBuf);

		if (aVerRes != 0)
		{
			VS_FIXEDFILEINFO * aVersionInfo = 0;

			BOOL aVerRes = VerQueryValue(aVersionBuf, _T("\\"), (void **) &aVersionInfo, (LPUINT) &aSize);

			if (aVerRes != 0)
			{
				short
					aW1 = HIWORD(aVersionInfo->dwProductVersionMS),
					aW2 = LOWORD(aVersionInfo->dwProductVersionMS),
					aW3 = HIWORD(aVersionInfo->dwProductVersionLS);

				TCHAR aVersion[500], * aPtr = aVersion;
				int aStringSize = 400;	//leave 100 chars for the version number (quite enough)

				aRes = myLocMan->GetStringSafe( KEYOF(IDS_O_ABT_VERSION), aVersion, &aStringSize);

				if ( SUCCEEDED(aRes) )
				{
					aPtr = aVersion + aStringSize;

					if (aW3 != 0)
						_stprintf(aPtr, _T(" \x200E%d.%d.%d"), aW1, aW2, aW3);
					else
						_stprintf(aPtr, _T(" \x200E%d.%d"), aW1, aW2);


					//Added 2006-12-02
					TCHAR * aOutBuild = 0;
					aVerRes = VerQueryValue(aVersionBuf, _T("\\StringFileInfo\\040904b0\\SpecialBuild"),
						(void **) &aOutBuild, (LPUINT) &aSize);

					if (aVerRes != 0)
					{
						lstrcat(aPtr, _T(" ") );
						lstrcat(aPtr, aOutBuild);
					}
					//End added 2006-12-02

					::SetDlgItemText(m_hWnd, IDC_O_ABT_APPVER, aVersion);
				}
			}
		}
		delete [] aVersionBuf;
	}


	// ************************************* Tips
	LocalizationUtility::LocPair aTips [] = 
	{
		LOCENTRY_H(IDC_O_ABT_FEEDBACK),
		LOCENTRY_H(IDC_O_ABT_HOMEPAGE),
	};

	ArraySize = sizeof aTips  /  sizeof aTips[0];

	LocalizationUtility::AddTooltips(m_hWnd, myLocMan, myHost, aTips, ArraySize);
}

//////////////////////////////////////////////////////////////////////////////////////////////

HRESULT DlgOptionsPropSheet_About::ProcessSpecialThanksSection(ILocalization * theLoc)
{
	Metric aM;
	HRESULT aRes = theLoc->GetMetric( KEYOF(IDM_O_ABT_NUMTHANKSLINES), aM);

	if ( FAILED(aRes) )
		return aRes;

	char aStringID[16];
	lstrcpyA (aStringID, STRINGOF(IDS_O_ABT_THX1) );

	TCHAR aBuf[1000], * aCurBufPointer = aBuf;
	int aLength = 1000, aCurLen = aLength;

	for ( int i = 1; i <= aM.Val1 && i < 10; i++)
	{
		if ( i != 1)
		{
			aCurBufPointer[0] = _T('\n');
			aCurBufPointer[1] = _T('\n');
			aCurBufPointer[2] = 0;

			aCurLen -= 2;
			aLength -= 2;
			aCurBufPointer += 2;
		}

		LocIdKey aKey = StringUtil::ComputeHash(aStringID);
		aRes = theLoc->GetString(aKey, aCurBufPointer, &aCurLen);

		if ( FAILED(aRes) )
			break;

		aCurBufPointer += aCurLen;

		aCurLen = aLength - aCurLen;
		aLength = aCurLen;

		aStringID[13]++;	//advance to IDS_O_ABT_THANKS_2, IDS_O_ABT_THANKS_3, IDS_O_ABT_THANKS_4, etc
	}

	if ( SUCCEEDED(aRes) )
		::SetDlgItemText(m_hWnd, IDC_O_ABT_THXLIST, aBuf);

	return aRes;
}
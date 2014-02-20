#pragma once

/*

(c) 2004-2008 Kirill Osipov

This file contains classes that implement the behavior of the
program's main window. Classes defined here:

PropSheetPartial
DlgOptionsPropSheet_Features
DlgOptionsPropSheet_Contents
DlgOptionsPropSheet_Settings
DlgOptionsPropSheet_Settings::XmlHandler
DlgOptionsPropSheet_Registered
DlgOptionsPropSheet_Unregistered
DlgOptionsPropSheet_Plugins
DlgOptionsPropSheet_About

*/

#include "sdk_PropPages.h"
#include "sdk_ChangeableEntity.h"
#include "sdk_Version.h"
#include "sdk_Localization.h"
#include "sdk_Theme.h"

#include "Trace.h"
#include "StaticControl.h"
#include "HttpRequest.h"
#include "HotspotNew.h"
#include "UpdateCheck.h"

#include "Utility.h"
#include "util_Localization.h"


#include "resource.h"
#include "auto_LocResource.h"

//////////////////////////////////////////////////////////////////////////////////////////////
//                            BASIC PROPERTY SHEET IMPLEMENTATION                           //
//////////////////////////////////////////////////////////////////////////////////////////////

template <typename T_Impl>
class PropSheetPartial : public ComEntry1<IPropPage>
{
protected:
	const static int TextSize = 100;

// IPropPage members
protected:
	TCHAR myText[TextSize];            //!< Stores the window's caption
	LocalizationManagerPtr myLocMan;   //!< Stores the pointer to the localization manager.
	ComRefCount<> myRefs;              //!< Stores the number of references to the current object.

protected:
	PropSheetPartial()
	{
		UpdateTitle();
	}


// IPropPage Partial Implementation
protected:
	STDMETHODIMP GetTitle   (LPCWSTR * theOutText)
		{ * theOutText = myText; return S_OK; }

	STDMETHODIMP OnActivating (bool theActivate)
		{ return S_OK; }

	STDMETHODIMP Initialize (IPropPageHost * theHost)
	{
		HWND aOwnerHwnd = NULL;
		HRESULT aRes = theHost->GetHwnd(&aOwnerHwnd);

		if ( SUCCEEDED(aRes) )
		{
			T_Impl * aImpl = static_cast<T_Impl *>(this);
			aImpl->DoCreate(theHost, aOwnerHwnd); // call the implementer method
		}

		return aRes;
	}

	STDMETHODIMP OnSave()
		{ return S_OK; }


	STDMETHODIMP GetContext(LPGUID theOutGuid)
		{ return E_NOTIMPL; }


protected:
	ULONG STDMETHODCALLTYPE Partial_AddRef()
	{
		return myRefs.AddRef();
	}

	ULONG STDMETHODCALLTYPE Partial_Release()
	{
		return myRefs.Release( static_cast<T_Impl *>(this) );
	}

	HRESULT STDMETHODCALLTYPE Partial_QueryInterface (REFIID theIID, void ** theOut)
	{
		HRESULT aRes = InternalQueryInterface(theIID, static_cast<T_Impl *>(this), theOut);

		if ( SUCCEEDED(aRes) )
			Partial_AddRef();

		return aRes;
	}

	void UpdateTitle()
	{
		LocIdKey aTitleKey = T_Impl::GetTitleKey();

		int aSize = TextSize;
		HRESULT aRes = myLocMan->GetStringSafe(aTitleKey, myText, &aSize);

		ATLASSERT( SUCCEEDED(aRes) );
	}
};

//////////////////////////////////////////////////////////////////////////////////////////////

template<typename T_Impl, int T_DlgID>
class DialogPropSheetPartial :
	public CDialogImpl<T_Impl>,
	public PropSheetPartial<T_Impl>,
	public IHelpContext,
	public ILocalizationEventProcessor
{
public:
	enum { IDD = T_DlgID };


	DialogPropSheetPartial() : myHost(0), myUxMan( Application::Instance().GetUxManager() )
		{ }


// PropSheetPartial required implementation func
public:
	void DoCreate(IPropPageHost * theHost, HWND theOwnerWnd)
	{
		myHost = theHost;

		Create(theOwnerWnd);
		Application::Instance().RegisterLocalizationEventProcessor(this);

		T_Impl * aThis = static_cast<T_Impl *>(this);
		aThis->LoadLocalization();
	}

	static void SetPrefs(IPropPageHost * theHost, ApplicationSettings *& thePrefs)
	{
#if _USE_CHANGEABLEENTITY
		thePrefs = Application::Instance().Prefs();
		theHost->RegisterEntity(thePrefs);
#else
		theHost->GetSettings( (void **) &thePrefs );
#endif
	}

protected:
	STDMETHODIMP GetHwnd(HWND * theOutHwnd)
	{
		if (theOutHwnd == 0)
			return E_POINTER;

		* theOutHwnd = m_hWnd;
		return S_OK;
	}

	STDMETHODIMP OnClosing()
	{
		Application::Instance().UnregisterLocalizationEventProcessor(this);
		return S_OK;
	}


	// IHelpContext Members
protected:
	STDMETHODIMP GetChmUrl(TCHAR * theOut, int * theSize)
	{
		if (theOut == 0 || theSize == 0)
			return E_POINTER;

		int aBufSize = *theSize;

		const TCHAR * aUrl = T_Impl::GetHelpUrl();
		const int aPageSize = lstrlen(aUrl);

		if (aBufSize < aPageSize + 1)
			return E_OUTOFMEMORY;

		lstrcpyn( theOut, aUrl, aPageSize + 1 );
		*theSize = aPageSize;

		return S_OK;
	}


// ILocalizationEventProcessor members
protected:
	void OnLocalizationChanged(ILocalization *, const TCHAR *, int)
	{
		UpdateTitle();

		T_Impl * aThis = static_cast<T_Impl *>(this);
		aThis->LoadLocalization();

		InvalidateRect(NULL);
	}


// IUnknown members
protected:
	ULONG STDMETHODCALLTYPE AddRef()
		{ return Partial_AddRef(); }

	ULONG STDMETHODCALLTYPE Release()
		{ return Partial_Release(); }

	HRESULT STDMETHODCALLTYPE QueryInterface (REFIID theIID, void ** theOut)
	{
		if ( theIID == IID_IHelpContext)
			*theOut = static_cast<IHelpContext *> (this);

		else
			return Partial_QueryInterface(theIID, theOut);

		AddRef();
		return S_OK;
	}

	BEGIN_MSG_MAP(DialogPropSheetPartial)
		MESSAGE_RANGE_HANDLER(WM_CTLCOLORMSGBOX, WM_CTLCOLORSTATIC, MessageHandler_CtlColor)
		MESSAGE_HANDLER(WM_NCPAINT,                                 MessageHandler_NcPaint)
	END_MSG_MAP()


protected:
	LRESULT MessageHandler_CtlColor(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL & bHandled)
	{
		if ( myUxMan->IsAppThemed() )
			return (INT_PTR)GetStockObject(WHITE_BRUSH);

		bHandled = FALSE;
		return 0L;
	}

	LRESULT MessageHandler_NcPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL & bHandled)
	{
		if ( !myUxMan->IsAppThemed() )
		{
			bHandled = FALSE;
			return 0L;
		}

		RECT aRect;
		GetWindowRect(&aRect);
		::MapWindowPoints(NULL, m_hWnd, (LPPOINT) &aRect, 2);
		OffsetRect(&aRect, 1, 1);

		HDC hdc = GetWindowDC();
		SelectClipRgn(hdc, NULL);

		IntersectClipRect(hdc, aRect.left,   aRect.top,   aRect.right,   aRect.bottom);
		ExcludeClipRect  (hdc, aRect.left+2, aRect.top+2, aRect.right-2, aRect.bottom-2);


		HTHEME aTh = (HTHEME) myUxMan->OpenThemeData(NULL, L"ListView");
		myUxMan->DrawThemeBackground(aTh, hdc, 0, 0, &aRect, 0);
		myUxMan->CloseThemeData(aTh);


		SelectClipRgn(hdc, NULL);
		ReleaseDC(hdc);
		return 0L;
	}

protected:
	IPropPageHost * myHost;
	UxManager * myUxMan;
};

#define LOCALIZ_KEY(id) #id

//////////////////////////////////////////////////////////////////////////////////////////////

template<typename T_Impl, int T_DlgID>
class DialogPropSheetPrefProcessor :
	public DialogPropSheetPartial<T_Impl, T_DlgID>,
	public IPreferencesEventProcessor
{
private:
	typedef DialogPropSheetPartial<T_Impl, T_DlgID> BaseClass;

public:
	DialogPropSheetPrefProcessor() : myPrefs(0) { }


// PropSheetPartial required implementation func
public:
	void DoCreate(IPropPageHost * theHost, HWND theOwnerWnd)
	{
		BaseClass::SetPrefs(theHost, myPrefs);
		BaseClass::DoCreate(theHost, theOwnerWnd);

		T_Impl * aThis = static_cast<T_Impl *>(this);
		aThis->LoadFromPreferences(myPrefs);
	}

public:
	BEGIN_MSG_MAP(DialogPropSheetPrefProcessor)
		MESSAGE_HANDLER(WM_INITDIALOG,                    MsgHandler_InitDialog)
		MESSAGE_HANDLER(WM_DESTROY,                       MsgHandler_Destroy)
	CHAIN_MSG_MAP(BaseClass)
	END_MSG_MAP()


protected:
	LRESULT MsgHandler_InitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		Application::Instance().AddPreferencesEventProcessor(this);

		T_Impl * aThis = static_cast<T_Impl *>(this);
		aThis->InitControls();

		myUxMan->SetWindowTheme(m_hWnd, L"Explorer", 0);

		return TRUE;

	}

	LRESULT MsgHandler_Destroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		Application::Instance().RemovePreferencesEventProcessor(this);
		return 0L;
	}


// IPreferencesEventProcessor Members
private:
	void OnPreferencesEvent(PreferencesEvent theEvent, ApplicationSettings & thePrefs)
	{
		if ( theEvent != PREFEVENT_RESET)
			return;

		T_Impl * aThis = static_cast<T_Impl *>(this);
		aThis->LoadFromPreferences(&thePrefs);
	}


// Data
protected:
	ApplicationSettings * myPrefs;
};

//////////////////////////////////////////////////////////////////////////////////////////////
//                                 GENERAL PROPERTY SHEET                                   //
//////////////////////////////////////////////////////////////////////////////////////////////

class DlgOptionsPropSheet_Features :
	public DialogPropSheetPrefProcessor<DlgOptionsPropSheet_Features, IDD_O_FEAT>,
	public IHotspotToggleProcessor
{
	typedef DialogPropSheetPrefProcessor<DlgOptionsPropSheet_Features, IDD_O_FEAT> BaseClass;


// PropSheetPartial required implementation func
public:
	static LocIdKey      GetTitleKey() { return KEYOF(IDD_O_FEAT); }
	static const TCHAR * GetHelpUrl()  { return _T("tab_features.html"); }

	void InitControls();

	void LoadFromPreferences(ApplicationSettings * thePr);

	void LoadLocalization();


// WTL Messaging
public:
	BEGIN_MSG_MAP(DlgOptionsPropSheet_Features)
		MESSAGE_HANDLER(WM_INITDIALOG,                    MsgHandler_InitDialog)
		MESSAGE_HANDLER(WM_DESTROY,                       MsgHandler_Destroy)
		MESSAGE_HANDLER(WM_HSCROLL,                       MsgHandler_Scroll)
		MESSAGE_HANDLER(WM_DRAWITEM,                      MsgHandler_DrawItem)

		COMMAND_ID_HANDLER(IDC_O_FEAT_FLDMENUSTGL,        MsgHandler_Cmd_ToggleFolderMenus)

		COMMAND_ID_HANDLER(IDC_O_FEAT_APCHITEMSTGL,       MsgHandler_Cmd_ToggleApproachItems)
		COMMAND_ID_HANDLER(IDC_O_FEAT_APCHITEMSOPT,       MsgHandler_Cmd_ApproachItemsOptions)

		COMMAND_ID_HANDLER(IDC_O_FEAT_TTLBARMENUSTGL,     MsgHandler_Cmd_ToggleTitlebarMenus)
		COMMAND_ID_HANDLER(IDC_O_FEAT_CURFLDISFILE,       MsgHandler_Cmd_TitlebarMenusCheckBox)
		COMMAND_ID_HANDLER(IDC_O_FEAT_CURFLDAUTOSEL,      MsgHandler_Cmd_TitlebarMenusCheckBox)
	CHAIN_MSG_MAP(BaseClass)
	END_MSG_MAP()


// IHotspotToggleProcessor members
protected:
	void OnHotspotToggle(const GUID & theGuid, bool theEnable, bool theResult);


// IPropPage Members - partial implementation
protected:
	STDMETHODIMP OnSave();


// Win32 Message Handlers
protected:
	LRESULT MsgHandler_InitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT MsgHandler_Destroy   (UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT MsgHandler_Scroll    (UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT MsgHandler_DrawItem  (UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);


	LRESULT MsgHandler_Cmd_ToggleFolderMenus    (WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT MsgHandler_Cmd_ToggleApproachItems  (WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT MsgHandler_Cmd_ToggleTitlebarMenus  (WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT MsgHandler_Cmd_ApproachItemsOptions (WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT MsgHandler_Cmd_TitlebarMenusCheckBox(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);


// Implementation Details
private:
	static bool CheckboxHasFlag(const CButton & theCheckbox);

	void SafeToggleHotspot(const GUID & theGuid);
	void AlignCheckbox(CButton & theBtn, CStaticWithLine & theAlignWn);
	void UpdateCheckboxState();


// Controls
protected:
	CStaticWithLine myGroup_FM;            // -----------------------------
	CButton myFolderMenus;
	CTrackBarCtrl mySliderInitial;
	CStatic myValueInitial;

	CStaticWithLine myGroup_AI;            // -----------------------------
	CButton myTrayItems;

	CStaticWithLine myGroup_TM;            // -----------------------------
	CButton myTitlebarMenus;
	CButton myRbParentToChild, myRbChildToParent;
	CButton myCbCurFolderAsItem, myCbAutoSelectCurFolder;
};

//////////////////////////////////////////////////////////////////////////////////////////////
//                                MENUS & SCROLLING   PAGE                                  //
//////////////////////////////////////////////////////////////////////////////////////////////

class DlgOptionsPropSheet_MenusScrolling :
	public DialogPropSheetPrefProcessor<DlgOptionsPropSheet_MenusScrolling, IDD_O_MNSC>,
	public CWinDataExchange<DlgOptionsPropSheet_MenusScrolling>
{
	typedef DialogPropSheetPrefProcessor<DlgOptionsPropSheet_MenusScrolling, IDD_O_MNSC> BaseClass;


// PropSheetPartial required implementation func
public:
	static LocIdKey      GetTitleKey() { return KEYOF(IDD_O_MNSC); }
	static const TCHAR * GetHelpUrl()  { return _T("tab_menusscrolling.html"); }

	void InitControls();

	void LoadFromPreferences(ApplicationSettings * thePr);

	void LoadLocalization();


// IPropPage Members - partial implementation
protected:
	STDMETHODIMP OnSave();


// WTL Messaging
public:
	BEGIN_MSG_MAP(DlgOptionsPropSheet_MenusScrolling)
		MESSAGE_HANDLER(WM_HSCROLL,    MsgHandler_Scroll)
		COMMAND_RANGE_HANDLER(IDC_O_MNSC_MNUWIDTH_0PXL, IDC_O_MNSC_MNUWIDTH_1PRC, MsgHandler_Cmd_ToggleUnits)
		CHAIN_MSG_MAP(BaseClass)
	END_MSG_MAP()


private:
	LRESULT MsgHandler_Scroll(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

	LRESULT MsgHandler_Cmd_ToggleUnits(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);


private:
	void UpdateWidthConstraints(bool theAsPercent);


private:
	CStaticWithLine myCatMenus;            //-------------------------------
	CEdit myMaxMenuWidth;
	CUpDownCtrl mySpinMaxMenuWidth;

	CStaticWithLine myCatScrolling;        //-------------------------------
	CButton myRbScrollAtTop, myRbScrollAtBottom, myRbScrollRespective;
	CButton myCbShowNumOfScrollItems;
	CEdit myScrollItemsWheel, myScrollItemsPage;
	CUpDownCtrl mySpinWheel, mySpinPage;
	CTrackBarCtrl mySliderChildPopup, mySliderScroll;
	CStatic myValueChildPopup, myValueScroll;

private:
	bool myInPercent;
};

//////////////////////////////////////////////////////////////////////////////////////////////
//                                   CONTENTS    PAGE                                       //
//////////////////////////////////////////////////////////////////////////////////////////////

class DlgOptionsPropSheet_Contents :
	public DialogPropSheetPrefProcessor<DlgOptionsPropSheet_Contents, IDD_O_CNTS>,
	public CWinDataExchange<DlgOptionsPropSheet_Contents>
{
	typedef DialogPropSheetPrefProcessor<DlgOptionsPropSheet_Contents, IDD_O_CNTS> BaseClass;


public:
	BEGIN_MSG_MAP(DlgOptionsPropSheet_Contents)
		COMMAND_HANDLER(IDC_O_CNTS_HDNITMS, CBN_SELCHANGE, MsgHandler_Cmd_HiddenItems)
		COMMAND_RANGE_HANDLER(IDC_O_CNTS_ICN_0GENERIC, IDC_O_CNTS_ICN_1REGULAR, MsgHandler_Cmd_Icons)
	CHAIN_MSG_MAP(BaseClass)
	END_MSG_MAP()


// PropSheetPartial required implementation func
public:
	static LocIdKey      GetTitleKey() { return KEYOF(IDD_O_CNTS); }
	static const TCHAR * GetHelpUrl()  { return _T("tab_contents.html"); }

	void InitControls();

	void LoadFromPreferences(ApplicationSettings * thePr);

	void LoadLocalization();


// IPropPage members - partial implementation
protected:
	STDMETHODIMP OnSave();


// Win32 Message Handlers
private:
	LRESULT MsgHandler_Cmd_Icons      (WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT MsgHandler_Cmd_HiddenItems(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);


// Implementation Details
private:
	void UpdateNormalIconsRelatedControls();
	void UpdateDimmedIconCheckboxStatus();


// Controls
protected:
	CStaticWithLine myGroup_Contents;     //---------------------
	CComboBox       mySelHiddenItems;
	CButton         myCbDimmedIcon;
	CComboBox       mySelBrowseFolders;
	CButton         myRbTreatSsfAsFolders, myRbTreatSsfAsFiles;
	CButton         myRbTreatZipAsFolders, myRbTreatZipAsFiles;
	CComboBox       mySelMouseInvocation;
	CStaticWithLine myGroup_Order;        //---------------------
	CButton         myCbSortAlphabetically;
	CButton         myRbFoldersPrecede, myRbFoldersFollow, myRbFoldersMix;
	CStaticWithLine myGroup_ClickItems;   //---------------------
	CButton         myRbGenericIcons, myRbRegularIcons;
	CButton         myCbUseOverlays;
	CButton         myCbOptimizeIconExtr;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//                                SETTINGS PROPERTY SHEET                                   //
//////////////////////////////////////////////////////////////////////////////////////////////

class IPluginManager;

class DlgOptionsPropSheet_Maintenance :
	public DialogPropSheetPrefProcessor<DlgOptionsPropSheet_Maintenance, IDD_O_MNTC>,
	public IEnumLocalizationsProcessor,
	public IAsyncUpdateCheckResultHandler
{
	typedef DialogPropSheetPrefProcessor<DlgOptionsPropSheet_Maintenance, IDD_O_MNTC> BaseClass;
	const static UINT WM_USER_HANDLEUPDATERESULT = WM_USER;

// Constructors, destructor
public:
	DlgOptionsPropSheet_Maintenance();


// PropSheetPartial required implementation func
public:
	static LocIdKey      GetTitleKey() { return KEYOF(IDD_O_MNTC); }
	static const TCHAR * GetHelpUrl()  { return _T("tab_maintenance.html"); }

	void InitControls();

	void LoadFromPreferences(ApplicationSettings * thePr);

	void LoadLocalization();


// IPropPage members -- partial implementation
protected:

	STDMETHODIMP GetContext (LPGUID theOutGuid)
		{ *theOutGuid = ContextID_Plugins; return S_OK; }

	STDMETHODIMP OnSave();


// IEnumLocalizationsProcessor members
protected:
	void OnLocalizationFound(ILocalization * theGet);


// IAsyncUpdateCheckResultHandler members
protected:
	void HandleUpdateCheckResult(UpdateCheckTask * theTask);


// WTL Messaging
public:
	BEGIN_MSG_MAP(DlgOptionsPropSheet_Maintenance)
		COMMAND_HANDLER   (IDC_O_MNTC_UILANG, CBN_SELCHANGE, MsgHandler_Cmd_ChangeLocale)
		COMMAND_ID_HANDLER(IDC_O_MNTC_UPDATE,                MsgHandler_Cmd_CheckForUpdates)
		COMMAND_ID_HANDLER(IDC_O_MNTC_RESTDEF,               MsgHandler_Cmd_RestoreDefaults)
		COMMAND_ID_HANDLER(IDC_O_MNTC_PLGNENBL,              MsgHandler_Cmd_TogglePlugin)
		COMMAND_ID_HANDLER(IDC_O_MNTC_PLGNDSBL,              MsgHandler_Cmd_TogglePlugin)

		NOTIFY_HANDLER(IDC_O_MNTC_PLGNLST, LVN_ITEMCHANGED,  MsgHandler_Notify_PluginSelect)

		MESSAGE_HANDLER(WM_USER_HANDLEUPDATERESULT,          MsgHandler_HandleUpdateResult)

	CHAIN_MSG_MAP(BaseClass)
	END_MSG_MAP()


// Win32 Message Handlers
protected:
	LRESULT MsgHandler_Cmd_ChangeLocale   (WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT MsgHandler_Cmd_CheckForUpdates(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT MsgHandler_Cmd_RestoreDefaults(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT MsgHandler_Cmd_TogglePlugin   (WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	LRESULT MsgHandler_Notify_PluginSelect(int theControlID, LPNMHDR theHeader, BOOL& bHandled);

	LRESULT MsgHandler_HandleUpdateResult (UINT theMsg, WPARAM theWParam, LPARAM theLParam, BOOL& bHandled);



// Implementation details
private:

	//! TODO
	static bool StartupShortcut(int theCreate);

	//! TODO
	void DisplayLocalizationDescription(ILocalization * theLoc);

	//! TODO
	HRESULT DisplayLocalizationDescription(ILocalization * theLoc,
		const TCHAR * theLocale, int theSizeLocale,
		TCHAR * theOut, int * theSizeOut);

	//! TODO
	HRESULT ApplyLocalization();

	//! TODO
	void InitPluginListColumns();

	//! TODO
	void PopulatePluginList(ApplicationSettings * thePr);

	//! TODO
	void UpdateButtonState(int theState);

	//! TODO
	int GetSelectedItemText(TCHAR * theOut, int theTextBufSize);

	//! TODO
	void FormatLocalizedPluginStatus(ApplicationSettings * thePr, const TCHAR * thePluginName, TCHAR * theOutStatus, int * theStatusSize);


// Controls
private:
	CStaticWithLine myGroup_Maint;   //---------------------
	CComboBox       myLanguages;
	CButton         myCbUsePeriodicUpdates;
	CButton         myBtnCheckForUpdates;
	CButton         myCheckBoxAutoRun;
	CStaticWithLine myGroup_Plugins;    //---------------------
	CListViewCtrl   myPluginList;
	CButton         myBtnEnable, myBtnDisable;


private:
	IPluginManager * myPlgnMan;
	UpdateCheckPackage myUpdateCheckPackage;
	bool myPluginListInitialized;
	bool myIsCheckingUpdates;
};

//////////////////////////////////////////////////////////////////////////////////////////////
//                                 ABOUT PROPERTY SHEET                                     //
//////////////////////////////////////////////////////////////////////////////////////////////

class DlgOptionsPropSheet_About :
	public DialogPropSheetPartial<DlgOptionsPropSheet_About, IDD_O_ABT>
{
	typedef DialogPropSheetPartial<DlgOptionsPropSheet_About, IDD_O_ABT> BaseClass;


// PropSheetPartial required implementation func
public:
	static LocIdKey      GetTitleKey() { return KEYOF(IDD_O_ABT); }
	static const TCHAR * GetHelpUrl()  { return _T("tab_about.html"); }

	void DoCreate(IPropPageHost * theHost, HWND theOwnerWnd);

	void LoadLocalization();


// WTL Messaging
public:
	BEGIN_MSG_MAP(DlgOptionsPropSheet_About)
		MESSAGE_HANDLER(WM_INITDIALOG, MsgHandler_InitDialog)
	CHAIN_MSG_MAP(BaseClass)
	END_MSG_MAP()


// Win32 Message Handlers
protected:
	LRESULT MsgHandler_InitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);


// Implementation Details
protected:
	HRESULT ProcessSpecialThanksSection(ILocalization * theLoc);


// Controls
protected:
	CStatic myLblThanks;
	CStaticWithLine myHorLine;
	CHyperLink myLnkFeedback, myLnkHomepage;

	const ApplicationSettings * myPrefs;
};


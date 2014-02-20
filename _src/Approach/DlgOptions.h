#pragma once

/*

(c) 2004-2008 Kirill Osipov

This file contains classes the interface of the 
DlgOptionsPropPages class.

*/

#include "sdk_GuidComparer.h"
#include "sdk_ChangeableEntity.h"
#include "sdk_ComObject.h"
#include "sdk_PropPages.h"
#include "sdk_Localization.h"

#include "Trace.h"
#include "StaticControl.h"
#include "HttpRequest.h"
#include "Preferences.h"

#include "Utility.h"
#include "util_Localization.h"

#include "resource.h"
#include "auto_LocResource.h"

//////////////////////////////////////////////////////////////////////////////////////////////

//! Represents the application Options dialog box.
class DlgOptionsPropPages :
	public CDialogImpl<DlgOptionsPropPages>,
	public ComEntry2<IPropPageHost, IControlHost>,
	public ILocalizationEventProcessor
{
	typedef std::map<HTREEITEM, IPropPage*> PropPages;
	typedef PropPages::value_type           PropPagePair;
	typedef PropPages::iterator             PropPageIter;
	typedef PropPages::const_iterator       PropPageIterC;

	typedef std::vector<IChangeableEntity *>     ChangeableEntityVect;
	typedef ChangeableEntityVect::iterator       ChangeableEntityIter;
	typedef ChangeableEntityVect::const_iterator ChangeableEntityIterC;

	typedef std::map<GUID, HTREEITEM, GuidComparer> BindablePropPageMap;
	typedef BindablePropPageMap::value_type         BindablePropPagePair;
	typedef BindablePropPageMap::iterator           BindablePropPageIter;
	typedef BindablePropPageMap::const_iterator     BindablePropPageIterC;


//Controls
protected:
	CTreeViewCtrl myTreeCtrl;
	CButton myBtnOk;
	CButton myBtnCancel;
	CButton myBtnHtmlHelp;
	CToolTipCtrl myTooltip;


protected:
	PropPages myPropPages;                           //!< TODO
	PropPageIter myCurItem;                          //!< TODO
	BindablePropPageMap myBindablePropPages;         //!< TODO
	ChangeableEntityVect myChangeableEntities;       //!< TODO
	LocalizationManagerPtr myLocMan;                 //!< TODO
	ApplicationSettings myPrefs;                     //!< TODO


public:
	DlgOptionsPropPages();
	~DlgOptionsPropPages();


// CDialogImpl Members
public:
	enum  { IDD = IDD_O };

	BEGIN_MSG_MAP(DlgOptionsPropPages)
		MESSAGE_HANDLER(WM_INITDIALOG,     MsgHandler_InitDialog)
		MESSAGE_HANDLER(WM_HELP,           MsgHandler_Help)
		MESSAGE_HANDLER(WM_DESTROY,        MsgHandler_Destroy)

		COMMAND_ID_HANDLER(IDOK,           MsgHandler_Command_OK)
		COMMAND_ID_HANDLER(IDCANCEL,       MsgHandler_Command_Cancel)
		COMMAND_ID_HANDLER(IDC_OPTS_HELP,  MsgHandler_Command_Help)

		NOTIFY_HANDLER(IDC_OPTS_PROPPAGETREE, TVN_SELCHANGED, MsgHandler_Notif_ChangePropSheet)
	END_MSG_MAP()


// IPropPageHost Members
public:

	//! Adds a property page to a list of child property pages of a giving parent property page
	STDMETHODIMP AddChildPropPage(REFGUID theParentGuid, IPropPage * thePropPage);

	//! TODO
	STDMETHODIMP GetHwnd         (HWND * theOutHwnd);

	//! TODO
	STDMETHODIMP RegisterEntity  (IChangeableEntity * theEntity);

	//! TODO
	STDMETHODIMP GetSettings(void ** theOut);


// IControlHost Members
public:
	STDMETHODIMP GetControl(int theType, HWND * theOutCtl);


protected:
	void OnLocalizationChanged(ILocalization *, const TCHAR *, int);


	// Operations
public:

	//! Called when enumerating property page factories.
	//! \param theFactory  The property page factory currently being enumerated.
	//!                    Upon each iteration it should be instructed to create child property pages.
	void operator () ( IPropPageFactory * theFactory );

	//! Activates a property sheet corresponding to the specified tree item
	//! and computes control positioning so that the entire property page is visible
	//! in the container. Buttons are moved with the anchor to bottom and right.
	//! Categories tree-view control is sized with the anchor to bottom.
	//! \param theIt   A tree item that has been activated.
	void ActivatePropPageByTreeItem(HTREEITEM theIt);


	//! Displays the HTML help window about the currently active property sheet.
	void ShowHelpOnCurWindow();



protected:
	void LoadLocalization();

	void PositionDynamicControls(bool theActivateCurPropPage);

	void UpdatePageTitles();


	// Message handlers
protected:
	LRESULT MsgHandler_InitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

	//! Called either when F1 is pressed or due to a click in the context help dialog mode.
	LRESULT MsgHandler_Help(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

	//! TODO
	LRESULT MsgHandler_Destroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);


	LRESULT MsgHandler_Command_OK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT MsgHandler_Command_Cancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT MsgHandler_Command_Help(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	LRESULT MsgHandler_Notif_ChangePropSheet(int theControlID, LPNMHDR theHeader, BOOL& bHandled);
};

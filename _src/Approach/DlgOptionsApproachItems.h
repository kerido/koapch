#pragma once

#include "sdk_ItemDesigner.h"
#include "sdk_ItemFactory.h"

#include "MenuSet.h"
#include "Preferences.h"

//////////////////////////////////////////////////////////////////////////////////////////////

class DlgApproachItems :
	public CDialogImpl<DlgApproachItems>,
	public CWinDataExchange<DlgApproachItems>,
	public ILocalizationEventProcessor
{
private:

	typedef std::vector<MenuSetEntry>    ItemDataVect;
	typedef ItemDataVect::iterator       ItemDataIter;
	typedef ItemDataVect::const_iterator ItemDataIterC;


public:
	enum { IDD = IDD_APCHITMS };


public:
	BEGIN_DDX_MAP(DlgApproachItems)
		DDX_CONTROL_HANDLE(IDC_APCHITMS_CUR,  myCurItems)
		DDX_CONTROL_HANDLE(IDC_APCHITMS_ALL,  myAvailItems)

		DDX_CONTROL_HANDLE(IDC_APCHITMS_UP,   myItemUp)
		DDX_CONTROL_HANDLE(IDC_APCHITMS_DOWN, myItemDown)
		DDX_CONTROL_HANDLE(IDC_APCHITMS_ADD,  myAddItem)
		DDX_CONTROL_HANDLE(IDC_APCHITMS_RMV,  myDeleteItem)
	END_DDX_MAP();


public:
	DlgApproachItems(ApplicationSettings * theSettings) : mySt(theSettings)
	{
		const MenuSet * aSet = mySt->GetApproachItemsSet();

		for(int i = 0; i < aSet->GetEntryCount(); i++)
			myCurItemList.push_back( aSet->GetEntry(i) );

		Application::Instance().RegisterLocalizationEventProcessor(this);
	}

	~DlgApproachItems()
	{
		Application::Instance().UnregisterLocalizationEventProcessor(this);
	}


protected:
	void OnLocalizationChanged(ILocalization *, const TCHAR *, int)
		{ LoadLocalization(); }


	BEGIN_MSG_MAP(DlgApproachItems)
		MESSAGE_HANDLER(WM_INITDIALOG,        MsgHandler_InitDialog)

		COMMAND_ID_HANDLER(IDOK,              CmdHandler_Ok)
		COMMAND_ID_HANDLER(IDCANCEL,          CmdHandler_Cancel)

		COMMAND_ID_HANDLER(IDC_APCHITMS_ADD,  CmdHandler_ItemAdd)
		COMMAND_ID_HANDLER(IDC_APCHITMS_RMV,  CmdHandler_ItemRemove)
		COMMAND_ID_HANDLER(IDC_APCHITMS_UP,   CmdHandler_ItemUp)
		COMMAND_ID_HANDLER(IDC_APCHITMS_DOWN, CmdHandler_ItemDown)
		COMMAND_ID_HANDLER(IDC_APCHITMS_BRW,  CmdHandler_BrowseItems)

		COMMAND_HANDLER(IDC_APCHITMS_CUR, LBN_SELCHANGE,           CmdHandler_CurItems_SelChange)
		COMMAND_HANDLER(IDC_APCHITMS_ALL, LBN_SELCHANGE,           CmdHandler_AvailItems_SelChange)

	END_MSG_MAP()


private:
	LRESULT MsgHandler_InitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		LoadLocalization();

		DoDataExchange(FALSE);

		AddStringsFromFactoriesAndLists();
		AddStringsFromCurrentItems();

		UpdateUpDownDeleteButtonsState();
		UpdateAddButtonState();

		return TRUE;
	}

	LRESULT CmdHandler_Ok(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		MenuSet * aSet = mySt->GetApproachItemsSet();

		aSet->Clear();

		int aN = myCurItems.GetCount();

		for(int i = 0; i < aN; i++)
		{
			ItemDataVect::size_type aIndex = myCurItems.GetItemData(i);
			const MenuSetEntry & aEntry = myCurItemList[aIndex];
			aSet->AddEntry(aEntry);
		}

		EndDialog(IDOK);
		return 0L;
	}

	LRESULT CmdHandler_Cancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		EndDialog(IDCANCEL);
		return 0L;
	}

	LRESULT CmdHandler_BrowseItems(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		HotspotManager * aHM = Application::Instance().GetHotspotManager();

		HotspotImpl_NotifyIcon * aTI = reinterpret_cast<HotspotImpl_NotifyIcon *>
			( aHM->GetHotspot(GUID_Hotspot_ApproachItems) );

		if (aTI != 0)
			aTI->ShowItems();

		return 0;
	}

	LRESULT CmdHandler_ItemAdd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		int aIndex = myAvailItems.GetCurSel();

		if (aIndex < 0)
			return 0;

		ItemDataVect::size_type aEntryIndex = myCurItemList.size();

		const MenuSetEntry & aEntry = myAvailItemList[aIndex];
		myCurItemList.push_back(aEntry);

		int aItemIndex = myCurItems.AddString(aEntry.Name);
		myCurItems.SetItemData(aItemIndex, aEntryIndex);

		myAvailItems.SetCurSel(-1);	//remove selection
		UpdateAddButtonState();

		return 0;
	}

	LRESULT CmdHandler_ItemRemove(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		int aIndex = myCurItems.GetCurSel();

		if (aIndex >= 0)
			myCurItems.DeleteString(aIndex);

		if (aIndex <= myCurItems.GetCount() )
			myCurItems.SetCurSel(aIndex);
		else
			myCurItems.SetCurSel(-1);

		UpdateUpDownDeleteButtonsState();

		return 0;
	}

	LRESULT CmdHandler_ItemUp(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		int aIndex = myCurItems.GetCurSel();

		if (aIndex <= 0)
			return 0;

		CString aStr;
		myCurItems.GetText(aIndex, aStr);

		ItemDataVect::size_type aEntryIndex = myCurItems.GetItemData(aIndex);

		myCurItems.DeleteString(aIndex);

		myCurItems.InsertString(aIndex-1, aStr);
		myCurItems.SetItemData(aIndex-1, aEntryIndex);

		myCurItems.SetCurSel(aIndex-1);

		return 0;
	}

	LRESULT CmdHandler_ItemDown(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		int aIndex = myCurItems.GetCurSel();

		if ( aIndex >= myCurItems.GetCount() - 1 )
			return 0;

		CString aStr;
		myCurItems.GetText(aIndex, aStr);

		ItemDataVect::size_type aEntryIndex = myCurItems.GetItemData(aIndex);

		myCurItems.DeleteString(aIndex);

		myCurItems.InsertString(aIndex+1, aStr);
		myCurItems.SetItemData(aIndex+1, aEntryIndex);

		myCurItems.SetCurSel(aIndex+1);

		return 0;
	}

	LRESULT CmdHandler_CurItems_SelChange(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		UpdateUpDownDeleteButtonsState();
		return 0;
	}

	LRESULT CmdHandler_AvailItems_SelChange(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		UpdateAddButtonState();
		return 0;
	}


private:
	void UpdateUpDownDeleteButtonsState()
	{
		int aIndex = myCurItems.GetCurSel();

		if (aIndex < 0)
		{
			myItemUp.EnableWindow(FALSE);
			myItemDown.EnableWindow(FALSE);
			myDeleteItem.EnableWindow(FALSE);
		}
		else
		{
			myItemUp.EnableWindow(aIndex > 0);
			myItemDown.EnableWindow(aIndex < myCurItems.GetCount() - 1);
			myDeleteItem.EnableWindow(TRUE);
		}
	}

	void UpdateAddButtonState()
	{
		int aIndex = myAvailItems.GetCurSel();
		myAddItem.EnableWindow(aIndex >= 0);
	}



	void AddStringsFromFactoriesAndLists()
	{
		Application & aApp = Application::Instance();

		/////////////////////////////////////////////////////////////////////
		// 1. Lists
		int aC = aApp.GetItemListCount();

		for (int i = 0; i < aC; i++)
		{
			ObjectData aDt;
			aApp.GetItemListData(i, &aDt);

			IItemList * aLst = aApp.GetItemList(aDt.ObjectID);
			CComQIPtr<IItemDesigner> aPtr(aLst);

			if (aPtr == 0)
				continue;

			ProcessItemsFromDesigner(aPtr);
		}

		/////////////////////////////////////////////////////////////////////
		// 2. Items
		aC = aApp.GetItemFactoryCount();

		for (int i = 0; i < aC; i++)
		{
			ObjectData aDt;
			aApp.GetItemFactoryData(i, &aDt);

			IItemFactory * aFact = aApp.GetItemFactory(aDt.ObjectID);
			CComQIPtr<IItemDesigner> aPtr(aFact);

			if (aPtr == 0)
				continue;

			ProcessItemsFromDesigner(aPtr);
		}

		/////////////////////////////////////////////////////////////////////
		// 3. Separator
		MenuSetEntry aSep;
		aSep.SetType(MenuSetItemData::SEPARATOR);
		aSep.SetName( _T("Separator") );

		myAvailItemList.push_back(aSep);


		// 4. Construct List Box Items

		for(ItemDataIter aIt = myAvailItemList.begin(); aIt != myAvailItemList.end(); aIt++)
		{
			int aIndex = myAvailItems.AddString(aIt->Name);
			myAvailItems.SetItemData(aIndex, aIt - myAvailItemList.begin() );
		}
	}

	void AddStringsFromCurrentItems()
	{
		Application & aApp = Application::Instance();

		for (ItemDataIter aIt = myCurItemList.begin(); aIt != myCurItemList.end(); aIt++)
		{
			MenuSetEntry & aEntry = *aIt;

			int aNameLength = lstrlen(aEntry.Name);

			int aIndex = -1;

			if (aNameLength != 0)
				aIndex = myCurItems.AddString(aEntry.Name);

			else
			{
				if (aEntry.GetType() == MenuSetEntry::SEPARATOR)
					aIndex = myCurItems.AddString( _T("Separator") );

				else
				{
					CComQIPtr<IItemDesigner> aDsgn;

					if (aEntry.GetType() == MenuSetEntry::ITEM)
						aDsgn = aApp.GetItemFactory( aEntry.GetObjectID() );

					else // if (aEntry.GetType() == MenuSetEntry::LIST)
						aDsgn = aApp.GetItemList( aEntry.GetObjectID() );

					if (aDsgn == 0)
						continue;

					HRESULT aRes = aDsgn->GetItemName(&aEntry, aEntry.Name, aEntry.NameSize);
					if ( FAILED(aRes) )
						continue;	//TODO: leave the entry in the list

					aIndex = myCurItems.AddString(aEntry.Name);
				}
			}

			myCurItems.SetItemData(aIndex, aIt - myCurItemList.begin() );
		}
	}

	void ProcessItemsFromDesigner(IItemDesigner * theItemDesigner)
	{
		int aNumItems = 0;
		HRESULT aRes = theItemDesigner->GetNumItems(&aNumItems);

		if (aNumItems == 0)
			return;

		for(int j = 0; j < aNumItems; j++)
		{
			MenuSetEntry aItem;
			aRes = theItemDesigner->GetItemData(j, &aItem);

			if ( FAILED(aRes) )
				continue;

			myAvailItemList.push_back(aItem);
		}
	}

	void LoadLocalization()
	{
		LocalizationUtility::LocTriplet aLocalizationArray [] = 
		{
			LOCENTRY_SELF(IDD_APCHITMS),

			LOCENTRY_SM(IDL_APCHITMS_CUR),
			LOCENTRY_0M(IDC_APCHITMS_ALL),

			LOCENTRY_SM(IDL_APCHITMS_ALL),
			LOCENTRY_0M(IDC_APCHITMS_CUR),

			LOCENTRY_SM(IDC_APCHITMS_ADD),
			LOCENTRY_SM(IDC_APCHITMS_RMV),
			LOCENTRY_SM(IDC_APCHITMS_UP),
			LOCENTRY_SM(IDC_APCHITMS_DOWN),

			LOCENTRY_SM(IDC_APCHITMS_BRW),

			{ KEYOF(IDS_GLBL_CMD_OK),     KEYOF(IDM_APCHITMS_OK),     IDOK     },
			{ KEYOF(IDS_GLBL_CMD_CANCEL), KEYOF(IDM_APCHITMS_CANCEL), IDCANCEL }
		};

		const int ArraySize = sizeof aLocalizationArray  /  sizeof aLocalizationArray[0];

		LocalizationUtility::LocalizeDialogBox(m_hWnd, myLocMan, aLocalizationArray, ArraySize);
	}


private:
	ApplicationSettings * mySt;      //!< Stores a pointer to the current application settings.
	LocalizationManagerPtr myLocMan; //!< Stores a pointer to the current localization manager.

	ItemDataVect myAvailItemList;    //!< Stores all available items that can be used.
	ItemDataVect myCurItemList;      //!< Stores entries currently in use. This list does not shrink when items are deleted.




// Controls
private:
	CListBox myAvailItems, myCurItems;
	CButton myItemUp, myItemDown, myAddItem, myDeleteItem;
};


#pragma once

#include "sdk_Launchable.h"

//////////////////////////////////////////////////////////////////////////////////////////////

class DECLSPEC_NOVTABLE IDispayItemEventHandler
{
public:
	virtual void DoProcess(MenuWindow_DisplayItemList * theWnd, const class ItemData & theItemData) const = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////////

class ClickItemHandler_Expand : public IDispayItemEventHandler
{
protected:
	virtual void DoProcess(MenuWindow_DisplayItemList * theWnd, const ItemData & theItemData) const
	{
		if (theWnd->IsChildMenuPresentFromItem(theItemData))
			return;

		DisplayItem * aDI = theItemData.GetItem();

		if (aDI == 0)
			return;

		Item * aIt = aDI->GetLogicalItem();

		if (aIt == 0)
			return;

		if (theWnd->GetChild() != 0)
			MenuManager::DestroyMenuChain(theWnd->GetChild());

		RECT aRect;
		theItemData.GetWindowRect(aRect);

		MapWindowPoints(theWnd->GetHandle(), NULL, (POINT *) &aRect, 2);

		MenuManager::ConstructByDataItem(aIt, theWnd, &aRect);

		theWnd->OnChildMenuConstructedFromItem(theItemData);
	}
};

//////////////////////////////////////////////////////////////////////////////////////////////

class ClickItemHandler_Activate : public IDispayItemEventHandler
{
protected:
	virtual void DoProcess(MenuWindow_DisplayItemList * theWnd, const ItemData & theItemData) const
	{
		DisplayItem * aDI = theItemData.GetItem();

		if (aDI == 0)
			return;

		Item * aLogItem = aDI->GetLogicalItem();

		ProcessLogicalItem(theWnd, aLogItem, true);
	}



public:
	static void ProcessLogicalItem(MenuWindow_DisplayItemList * theWnd, Item * theItem, bool theDefaultOnly)
	{
		if (theItem == 0) 
			return;

		CComQIPtr<ILaunchable, &IID_ILaunchable> aL(theItem);

		if (aL == 0)
			return;

		ULONG aOpts = theDefaultOnly ? ILaunchable::RUN_DEFAULT_ACTION : ILaunchable::SHOW_CONTEXT_MENU;

		MenuManager::SetOperationInProgress(1);
		HRESULT aRes = aL->DoLaunch(theWnd->GetHandle(), aOpts);
		MenuManager::SetOperationInProgress(0);

		if (aRes == S_OK)
			MenuManager::DestroyNonFloatingMenus();
	}
};

//////////////////////////////////////////////////////////////////////////////////////////////

class ClickItemHandler_Dumb : public IDispayItemEventHandler
{
protected:
	virtual void DoProcess(MenuWindow_DisplayItemList * /*theWnd*/, const ItemData & /*theItemData*/) const { }
};

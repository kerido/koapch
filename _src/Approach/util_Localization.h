#pragma once

#include "sdk_Localization.h"

//////////////////////////////////////////////////////////////////////////////////////////

class LocalizationUtility
{
public:
	struct LocTriplet
	{
		LocIdKey StringID;
		LocIdKey MetricID;
		int ControlID;
	};

	struct LocPair
	{
		DWORD_PTR    Data;
		LocIdKey StringID;
	};

public:

	static void LocalizeDialogBox(HWND theDlgBox, ILocalizationManager * theMan,
		const LocTriplet * theControlsArr, int theSize);

	static void LocalizeMenu(HMENU theMenu, ILocalizationManager * theMan,
		const LocPair * theItemsArr, int theSize);

	static HRESULT GetWindowsHelpString(HELPINFO * theInfo, ILocalizationManager * theMan,
		const LocTriplet * theControlsArr, int theSize, TCHAR * theOutString, int * theOutSize);

	static INT_PTR LocalizedMessageBox(HWND theParent, ILocalizationManager * theMan,
		LocIdKey theTextID, LocIdKey theCaptionID, UINT theStyle);

	static HRESULT SetWindowDirection(HWND theWnd, ILocalizationManager * theMan);

	static HRESULT IsDirectionLtr(ILocalizationManager * theMan, bool * theOut);

	static HRESULT AddTooltips(HWND theWnd, ILocalizationManager * theMan,
		IUnknown * thePropPageHost, const LocPair * theControlsArr, int theSize);

	static HRESULT LocalizeComboBox(HWND theCombo, ILocalizationManager * theMan,
		const LocPair * theQuery, int theSize);

private:
	static void UpdateWindowRtlStyle(HWND theWnd, bool theSet);
};

//////////////////////////////////////////////////////////////////////////////////////////

template<typename TLocManType>
class LocalizationManagerPtrBase
{
private:
	TLocManType * myPtr;

public:
	LocalizationManagerPtrBase(TLocManType * thePtr) : myPtr(thePtr) { }

public:
	TLocManType * operator *  () { return myPtr; }
	TLocManType * operator -> () { return myPtr; }
	operator TLocManType *    () { return myPtr; }
};

class LocalizationManagerPtrC: public LocalizationManagerPtrBase<const ILocalizationManager>
{
public:
	LocalizationManagerPtrC();
};

class LocalizationManagerPtr : public LocalizationManagerPtrBase<ILocalizationManager>
{
public:
	LocalizationManagerPtr();
};


//////////////////////////////////////////////////////////////////////////////////////////

#define LOCENTRY_SELF(id)     { RCENTRY_##id::Hash,  RCENTRY_##id::Hash, -1 }
#define LOCENTRY_SM(id)       { RCENTRY_##id::Hash,  RCENTRY_##id::Hash, id }
#define LOCENTRY_0M(id)       { 0,                   RCENTRY_##id::Hash, id }
#define LOCENTRY_S0(id)       { RCENTRY_##id::Hash,  0,                  id }
#define LOCENTRY_TM(text, id) { text,                RCENTRY_##id::Hash, id }

#define LOCENTRY_S(id)        { id,                  RCENTRY_##id::Hash     }
#define LOCENTRY_H(id)        { id,                  RCENTRY_##id::TipHash  }


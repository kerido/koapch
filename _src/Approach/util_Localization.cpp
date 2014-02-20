#include "stdafx.h"
#include <mlang.h>

#include "sdk_ComObject.h"
#include "sdk_PropPages.h"

#include "Application.h"

#include "util_Localization.h"

#include "resource.h"
#include "auto_LocResource.h"

//////////////////////////////////////////////////////////////////////////////////////////////

void LocalizationUtility::LocalizeDialogBox(
	HWND theDlgBox, ILocalizationManager * theMan,
	const LocTriplet * theControlsArr, int theSize)
{
	SetWindowDirection(theDlgBox, theMan);

	static const int Size = 1000;
	TCHAR aTempBuf[Size];

	for ( int i = 0; i < theSize; i++)
	{
		int aSize = Size;

		HWND aCtl;
		if ( theControlsArr[i].ControlID == -1 )
			aCtl = theDlgBox;
		else
			aCtl = ::GetDlgItem(theDlgBox, theControlsArr[i].ControlID);


		HRESULT aRes = theMan->GetStringSafe( theControlsArr[i].StringID, aTempBuf, &aSize);

		if ( SUCCEEDED (aRes) )
			::SetWindowText(aCtl, aTempBuf);

		Metric aMetric;

		aRes = theMan->GetMetricSafe(theControlsArr[i].MetricID, aMetric);

		if ( SUCCEEDED(aRes) )
		{
			RECT aRct = {0};

			UINT aFlags = SWP_FRAMECHANGED|SWP_NOZORDER|SWP_NOREPOSITION|SWP_NOSENDCHANGING;

			if ( aMetric.Factor == 2)		//as size
			{
				aRct.right  = aMetric.Val1;
				aRct.bottom = aMetric.Val2;

				aFlags |= SWP_NOMOVE;
			}

			else
			{
				aRct.left    = aMetric.Val1;
				aRct.top     = aMetric.Val2;
				aRct.right   = aMetric.Val1+aMetric.Val3;
				aRct.bottom  = aMetric.Val2+aMetric.Val4;

				if (aMetric.Factor == 4 && aMetric.Val3 == 0 && aMetric.Val4 == 0)
					aFlags |= SWP_NOSIZE;
				else if (aMetric.Factor == 1)
					aFlags |= SWP_NOMOVE|SWP_NOSIZE;
			}

			::MapDialogRect(theDlgBox, &aRct);

			if ( theControlsArr[i].ControlID == -1 )
			{
				LONG_PTR aStyle = ::GetWindowLongPtr(aCtl, GWL_STYLE);
				AdjustWindowRect(&aRct, (DWORD)aStyle, ::GetMenu(aCtl) != 0);
			}

			int aWidth  = aRct.right  - aRct.left;
			int aHeight = aRct.bottom - aRct.top;

#ifdef _DEBUG
			RECT aRect = {0};
			::GetWindowRect(aCtl, &aRect);
			::MapWindowPoints(NULL, theDlgBox, (LPPOINT) &aRect, 2);

			if ( aRect.left    != aRct.left    ||
				aRect.right   != aRct.right   ||
				aRect.top     != aRct.top     ||
				aRect.bottom  != aRct.bottom )
			{
				Trace
					(
					"Metric [%d] changed the size/position of control %d in the Dialog Box %x. Prev(LTWH) = [%d,%d,%d,%d], Cur(LTWH) = [%d,%d,%d,%d].\n",
					theControlsArr[i].MetricID,
					theControlsArr[i].ControlID,
					theDlgBox,
					aRect.left,   aRect.top,   aRect.right-aRect.left,   aRect.bottom - aRect.top,
					aRct.left,    aRct.top,    aWidth,                   aHeight
					);
			}
#endif	//_DEBUG


			::SetWindowPos(aCtl, NULL, aRct.left, aRct.top, aWidth, aHeight, aFlags);
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////

void LocalizationUtility::LocalizeMenu(
																			 HMENU theMenu, ILocalizationManager * theMan, const LocPair * theItemsArr, int theSize)
{
	TCHAR aBuf[1000];

	MENUITEMINFO aMII;
	SecureZeroMemory(&aMII, sizeof aMII);

	aMII.cbSize      = sizeof MENUITEMINFO;
	aMII.fMask       = MIIM_STRING;
	aMII.dwTypeData  = aBuf;
	// the cch field is not yet set, we'll do it later, when the string is obtained

	for (int i = 0; i < theSize; i++)
	{
		int aNumChars = 1000;	// the initial size of the buffer

		HRESULT aRes = theMan->GetStringSafe(theItemsArr[i].StringID, aBuf, &aNumChars);

		ATLASSERT ( SUCCEEDED(aRes) );

		aMII.cch = aNumChars;
		SetMenuItemInfo(theMenu, (UINT) theItemsArr[i].Data, FALSE, &aMII);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////

HRESULT LocalizationUtility::GetWindowsHelpString(
	HELPINFO * theInfo, ILocalizationManager * theMan, const LocTriplet * theControlsArr,
	int theSize, TCHAR * theOutString, int * theOutSize)
{
	if (theInfo == 0 || theMan       == 0 || theControlsArr == 0 ||
		theSize == 0 || theOutString == 0 || theOutSize     == 0)
		return E_INVALIDARG;

	for (int i = 0; i < theSize; i++)
		if ( theInfo->iCtrlId == theControlsArr[i].ControlID )
			return theMan->GetStringSafe(theControlsArr[i].StringID, theOutString, theOutSize);

	return E_FAIL;
}

//////////////////////////////////////////////////////////////////////////////////////////////

INT_PTR LocalizationUtility::LocalizedMessageBox(HWND theParent,
																								 ILocalizationManager * theMan, LocIdKey theTextID, LocIdKey theCaptionID, UINT theStyle)
{
	TCHAR aText[1000], aCaption[200];
	int aSz_Text = 1000, aSz_Caption = 200;

	HRESULT aRes = theMan->GetStringSafe(theTextID, aText, &aSz_Text);

	bool aLtr = true;
	aRes = IsDirectionLtr(theMan, &aLtr);

	if ( FAILED(aRes) )
		return -1;

	if (!aLtr)
		theStyle |= MB_RTLREADING|MB_RIGHT;

	if ( theCaptionID != 0 )
	{
		aRes = theMan->GetStringSafe(theCaptionID, aCaption, &aSz_Caption);

		if ( FAILED (aRes) )
			lstrcpy(aCaption, _T("KO Approach") );
	}
	else
		lstrcpy(aCaption, _T("KO Approach") );

	// Obtain the language ID
	TCHAR aLocale[6];
	int aSize = 6;
	theMan->GetCurrentLocalization()->GetLocale(aLocale, &aSize);

	// replace the underscore
	if (aLocale[2] == _T('_') )
		aLocale[2] = _T('-');

	// obtain the language id
	WORD aLangID = 0;

	CComPtr<IMultiLanguage> pml;
	aRes = pml.CoCreateInstance(CLSID_CMultiLanguage);

	if ( SUCCEEDED(aRes) )
	{
		LCID aLcid = 0;
		aRes = pml->GetLcidFromRfc1766(&aLcid, aLocale);

		if ( SUCCEEDED(aRes) )
			aLangID = LANGIDFROMLCID(aLcid);
	}

	return MessageBoxEx(theParent, aText, aCaption, theStyle, aLangID );
}

//////////////////////////////////////////////////////////////////////////////////////////////

HRESULT LocalizationUtility::SetWindowDirection(HWND theWnd, ILocalizationManager * theMan)
{
	bool aLtr = true;
	HRESULT aRes = IsDirectionLtr(theMan, &aLtr);

	if ( FAILED(aRes) )
		return aRes;

	UpdateWindowRtlStyle(theWnd, !aLtr);

	for ( HWND aWnd = GetTopWindow(theWnd); aWnd != NULL; aWnd = GetNextWindow(aWnd, GW_HWNDNEXT) )
		UpdateWindowRtlStyle(aWnd, !aLtr);

	return aRes;
}

//////////////////////////////////////////////////////////////////////////////////////////////

void LocalizationUtility::UpdateWindowRtlStyle(HWND theWnd, bool theSet)
{
	LONG_PTR aStyle = ::GetWindowLongPtr(theWnd, GWL_EXSTYLE);

	if (theSet)
		aStyle |= WS_EX_LAYOUTRTL;
	else
		aStyle &= ~WS_EX_LAYOUTRTL;

	::SetWindowLongPtr(theWnd, GWL_EXSTYLE, aStyle);
}

//////////////////////////////////////////////////////////////////////////////////////////////

HRESULT LocalizationUtility::IsDirectionLtr(ILocalizationManager * theMan, bool * theOut)
{
	Metric aRtl;
	HRESULT aRes = theMan->GetMetricSafe( KEYOF(IDM_GLBL_LANGRTL), aRtl);

	if ( SUCCEEDED(aRes) )
		*theOut = (aRtl.Val1 == 0);

	return aRes;
}

//////////////////////////////////////////////////////////////////////////////////////////////

HRESULT LocalizationUtility::AddTooltips(HWND theWnd, ILocalizationManager * theMan,
																				 IUnknown * thePropPageHost, const LocPair * theControlsArr, int theSize)
{
	CComQIPtr<IControlHost> aCtlHost(thePropPageHost);

	if (aCtlHost == 0)
		return E_NOINTERFACE;


	HWND aTip;
	aCtlHost->GetControl(IControlHost::CTL_TOOLTIP, &aTip);

	if (aTip == NULL)
		return E_FAIL;

	CToolTipCtrl aTipCtl(aTip);

	TCHAR aText[1000];
	for (int i = 0; i < theSize; i++)
	{
		int aTextSize = sizeof aText / sizeof TCHAR;
		HRESULT aRes = theMan->GetStringSafe(theControlsArr[i].StringID, aText, &aTextSize);

		if ( SUCCEEDED(aRes) )
		{
			TOOLINFO aInfo;
			aInfo.cbSize = sizeof TOOLINFO;
			aInfo.hwnd = NULL;
			aInfo.lParam = NULL;
			aInfo.uId = (UINT_PTR)::GetDlgItem(theWnd, (UINT) theControlsArr[i].Data);
			aInfo.lpszText = aText;
			aInfo.uFlags = TTF_IDISHWND|TTF_SUBCLASS;
			aInfo.hinst = 0;


			aTipCtl.DelTool(&aInfo);
			aTipCtl.AddTool(&aInfo);
		}
	}

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////////////////////////

HRESULT LocalizationUtility::LocalizeComboBox(HWND theCombo, ILocalizationManager * theMan,
																							const LocPair * theQuery, int theSize)
{
	ATLASSERT(theCombo != 0 && IsWindow(theCombo) );

	int aPrevCurSel = (int) SendMessage(theCombo, CB_GETCURSEL, 0, 0);

	SendMessage(theCombo, CB_RESETCONTENT, 0L, 0L);

	TCHAR aTempBuf[1000];

	for (int i = 0; i < theSize; i++)
	{
		LocIdKey aKey = theQuery[i].StringID;

		int aSize = 1000;
		HRESULT aRes = theMan->GetStringSafe(aKey, aTempBuf, &aSize);
		ATLASSERT( SUCCEEDED(aRes) );

		INT_PTR aItemIndex = SendMessage(theCombo, CB_ADDSTRING, 0, (LPARAM)aTempBuf);

		DWORD_PTR aData = theQuery[i].Data;
		SendMessage(theCombo, CB_SETITEMDATA, aItemIndex, aData);
	}

	if (aPrevCurSel >= 0 && aPrevCurSel < theSize)
		SendMessage(theCombo, CB_SETCURSEL, aPrevCurSel, 0);

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////////////////////////

LocalizationManagerPtrC::LocalizationManagerPtrC()
	: LocalizationManagerPtrBase( Application::InstanceC().GetLocalizationManager() )
{ }

//////////////////////////////////////////////////////////////////////////////////////////////

LocalizationManagerPtr::LocalizationManagerPtr()
	: LocalizationManagerPtrBase( Application::Instance().GetLocalizationManager() )
{ }

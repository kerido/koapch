#pragma once

#include "util_Localization.h"

#include "resource.h"
#include "auto_LocResource.h"

//////////////////////////////////////////////////////////////////////////////////////////////

//! Provides utility methods for displaying warning and error messages.
class WarningMessageUtility
{
public:

	//! Displays a localized message saying that a feature cannot be enabled
	//! in the unregistered mode.
	static INT_PTR Error_FeatureLocked(HWND theOwnerWnd, ILocalizationManager * theLocMan)
	{
		return LocalizationUtility::LocalizedMessageBox
		(
			theOwnerWnd,
			theLocMan,
			KEYOF(IDS_WARNTXT_FEATURELOCKED),
			KEYOF(IDS_GLBL_ERROR),
			MB_OK|MB_ICONERROR
		);
	}


	static INT_PTR Warning_ProgramUnregistered(HWND theOwnerWnd, ILocalizationManager * theLocMan)
	{
		return LocalizationUtility::LocalizedMessageBox
		(
			theOwnerWnd,
			theLocMan,
			KEYOF(IDS_WARNTXT_INVALIDKEY),
			KEYOF(IDS_GLBL_WARNING),
			MB_OK|MB_ICONWARNING
		);
	}
};

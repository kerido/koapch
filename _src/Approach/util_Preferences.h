#pragma once

#include "sdk_ItemProvider.h"

#include "Preferences.h"
#include "Application.h"


//////////////////////////////////////////////////////////////////////////////////////////////

class PreferencesUtility
{
public:
	static ULONG GetEnumItemsOptions()
	{
		ULONG aFlags = 0;

		ApplicationSettings * aPr = Application::Instance().Prefs();

		if ( aPr->GetFoldersPrecedeFiles() )
			aFlags |= ENUMOPTIONS_FOLDERS_ON_TOP;

		else if (aPr->GetFilesPrecedeFolders() )
			aFlags |= ENUMOPTIONS_FOLDERS_ON_BTM;

		if ( aPr->GetSortAlphabetically() )
			aFlags |= ENUMOPTIONS_SORT_ITEMS;

		return aFlags;
	}
};

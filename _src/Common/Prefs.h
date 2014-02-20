#pragma once

//Be sure to include standard include files BEFORE including this one

//////////////////////////////////////////////////////////////////////////////////////////////

#include <shellapi.h>
#include <shlobj.h>

//////////////////////////////////////////////////////////////////////////////////////////////

//TODO: convert to class
struct PrefsSerz
{
	//! Specifies the item click mode.
	enum ActivateMode
	{
		ACTIVATE_SINGLE_CLICK_FILES = 0,   //!< Single click to open files, double click to open folders.
		ACTIVATE_SINGLE_CLICK_ALL   = 1,   //!< Single click to open files and folders.
		ACTIVATE_DOUBLE_CLICK_ALL   = 2    //!< Double click to open files and folders.
	};

	//! Specifies the mode for displaying hidden files.
	enum HiddenFilesMode
	{
		HIDDEN_USESYSTEM   = 0,            //!< Use system setting to determine visibility of hidden files.
		HIDDEN_SHOWALWAYS  = 1,            //!< Always show hidden files.
		HIDDEN_SHOWNEVER   = 2,            //!< Never show hidden files.
		HIDDEN_DIMICON     = 4             //!< Dim icons of hidden files.
	};

	//! Specifies the way folders and shortcuts to folders are opened (in a new or in the existing window).
	enum BrowseMode
	{
		BROWSE_USESYSTEM      = 0,         //!< Use system setting to determine the browse folders mode.
		BROWSE_NEWWINDOW      = 1,         //!< Always browse folders in a new window.
		BROWSE_EXISTINGWINDOW = 2          //!< Always browse folders in the existing window.
	};

	//! Specifies item ordering options. Treated as a set of flags.
	enum Ordering
	{
		ORDER_ALPHABETICAL  = 0x1,         //!< Sort items by name (preferably by pidl)
		ORDER_FOLDERS_ONTOP = 0x2,         //!< Folders precede files
		ORDER_FOLDERS_ONBTM = 0x4,         //!< Files precede folders
	};

	//! Specifies scroll item positioning mode and other options.
	enum ScrollItemFlags
	{
		SIF_POS_RESPECTIVE    = 0,         //!< Specifies that each of the scroll items must be placed on its respective side.
		SIF_POS_TOP           = 1,         //!< Specifies that scroll items must be placed at the top.
		SIF_POS_BOTTOM        = 2,         //!< Specifies that scroll items must be placed at the bottom.
		SIF_SHOWNUMBEROFITEMS = 4,         //!< When set, instructs scroll items to display the number of items left to scroll.
	};

	//! Specifies the way item icons are displayed in KO Approach menus.
	enum IconHandlingFlags
	{
		ICF_ICONS_NORMAL          = 0,     //!< Display standard shell items. This is the default behavior.
		ICF_ICONS_GENERIC         = 1,     //!< Displays only a few icons to distinguish between item types (folder, file, program)
		ICF_ICONS_OVERLAYS        = 2,     //!< When set, draws overlays (shortcut, version control) over items' icons. Applicable with ICF_ICONS_NORMAL.

		ICF_PARALLELIZATION_BIT   = 4,     //!< The starting bit at which the number of icon extractor threads begins.
		ICF_PARALLELIZATION_SHIFT = 2      //!< The number of bits for left-shifting the number of icon extractor thread, must equal to log2(ICF_PARALLELIZATION_BIT).
	};

	enum FileTypeHandlingFlags
	{
		FTH_NONE            = 0,
		FTH_EXPAND_ZIPARCH  = 1,
		FTH_IGNORE_FLDLINKS = 2
	};

	typedef LONG DataType;

	DataType myTimeoutScroll;            //!< Scrolling interval in milliseconds.
	DataType myTimeoutInit;              //!< Folder Menus root menu interval in milliseconds.
	DataType myTimeoutSec;               //!< Child menu interval in milliseconds.
	DataType myScrollWheel;              //!< Number of items scrolled by mouse wheel rotation.
	DataType myScrollPage;               //!< Number of items scrolled by PAGE UP / PAGE DOWN keys.
	DataType myHiddenFilesMode;          //!< Value is one of the HiddenFilesMode options.
	DataType myBrowseInNewWindow;        //!< Value is one of the BrowseMode options.
	DataType myActivateMode;             //!< Value is one of the ActivateMode options.
	DataType myMaxMenuWidth;             //!< Positive -- pixels, negative -- percent screen width.
	DataType myZipsAreFolders;           //!< 0 -- do not treat zip archives as folders, 1 -- treat (in windows xp).
	DataType myOrdering;                 //!< Value is one or more options from the Ordering enumeration.
	DataType myIconHandlingFlags;        //!< Controls the way item icons are displayed.
	DataType myScrollItemFlags;          //!< Controls the way scroll items are positioned and displayed.
	DataType mySkipAutoUpdates;          //!< Controls update checks are automatically performed once a week.

	void SetDefault()
	{
		myTimeoutScroll = 75;
		myTimeoutInit = 500;
		myTimeoutSec = 300;

		myScrollWheel = 3;
		myScrollPage = 5;
		myHiddenFilesMode = HIDDEN_USESYSTEM & ~HIDDEN_DIMICON;

		myBrowseInNewWindow = 0;
		myActivateMode = 0;

		myMaxMenuWidth = 200;

		myZipsAreFolders    = FTH_NONE;
		myOrdering          = ORDER_FOLDERS_ONTOP;
		myIconHandlingFlags = ICF_ICONS_NORMAL;
		myScrollItemFlags   = SIF_POS_RESPECTIVE;

		mySkipAutoUpdates = FALSE;
	}

	void Set (const PrefsSerz & theVal)
	{
		myTimeoutScroll = theVal.myTimeoutScroll;
		myTimeoutInit = theVal.myTimeoutInit;
		myTimeoutSec = theVal.myTimeoutSec;

		myScrollWheel = theVal.myScrollWheel;
		myScrollPage = theVal.myScrollPage;
		myHiddenFilesMode = theVal.myHiddenFilesMode;

		myBrowseInNewWindow = theVal.myBrowseInNewWindow;
		myActivateMode = theVal.myActivateMode;

		myMaxMenuWidth = theVal.myMaxMenuWidth;
		myZipsAreFolders = theVal.myZipsAreFolders;

		myOrdering = theVal.myOrdering;

		myIconHandlingFlags = theVal.myIconHandlingFlags;
		myScrollItemFlags = theVal.myScrollItemFlags;

		mySkipAutoUpdates = theVal.mySkipAutoUpdates;
	}

	void Zero () { SecureZeroMemory(this, sizeof(PrefsSerz) ); }

	DataType GetTimeoutScroll() const              { return myTimeoutScroll; }
	void SetTimeoutScroll(DataType theValue)       { myTimeoutScroll = theValue; }

	DataType GetTimeoutSec() const                 { return myTimeoutSec; }
	void SetTimeoutSec(DataType theValue)          { myTimeoutSec = theValue; }

	DataType GetTimeoutInit() const                { return myTimeoutInit; }
	void SetTimeoutInit(DataType theValue)         { myTimeoutInit = theValue; }

	DataType GetNumScrollItemsWheel() const        { return myScrollWheel; }
	void SetNumScrollItemsWheel(DataType theValue) { myScrollWheel = theValue; }

	DataType GetNumScrollItemsPage() const         { return myScrollPage; }
	void SetNumScrollItemsPage(DataType theValue)  { myScrollPage = theValue; }

	DataType GetHiddenFilesMode() const            { return myHiddenFilesMode & ~HIDDEN_DIMICON; }
	void SetHiddenFilesMode(DataType theValue)     { myHiddenFilesMode &= HIDDEN_DIMICON; myHiddenFilesMode |= theValue; }

	DataType GetActivateMode() const               { return myActivateMode; }
	void SetActivateMode(DataType theValue)        { myActivateMode = theValue; }

	DataType GetIconHandlingFlags() const          { return myIconHandlingFlags; }
	void SetIconHandlingFlags(DataType theValue)   { myIconHandlingFlags = theValue; }

	bool GetUseGenericIcons() const                { return (myIconHandlingFlags & ICF_ICONS_GENERIC) != 0; }
	void SetUseGenericIcons(bool theVal)
	{
		if (theVal) myIconHandlingFlags |= ICF_ICONS_GENERIC;
		else        myIconHandlingFlags &= ~ICF_ICONS_GENERIC;
	}

	bool GetUseIconOverlays() const                { return (myIconHandlingFlags & ICF_ICONS_OVERLAYS) != 0; }
	void SetUseIconOverlays(bool theVal)
	{
		if (theVal) myIconHandlingFlags |= ICF_ICONS_OVERLAYS;
		else        myIconHandlingFlags &= ~ICF_ICONS_OVERLAYS;
	}

	bool GetDimHiddenItems() const                 { return (myHiddenFilesMode & HIDDEN_DIMICON) != 0; }
	void SetDimHiddenItems(bool theVal)
	{
		if (theVal) myHiddenFilesMode |= HIDDEN_DIMICON;
		else        myHiddenFilesMode &= ~HIDDEN_DIMICON;
	}

	int GetIconFetchParallelization() const
	{
		int aRetVal = myIconHandlingFlags >> ICF_PARALLELIZATION_SHIFT;

		if      (aRetVal < 1)
			return 1;
		else if (aRetVal > 10)
			return 10;
		else
			return aRetVal;
	}

	void SetIconFetchParallelization(int theVal)
	{
		int aEffective = theVal;

		if (aEffective < 1)
			aEffective = 1;
		else if (aEffective > 10)
			aEffective = 10;

		myIconHandlingFlags &= ICF_PARALLELIZATION_BIT-1;
		myIconHandlingFlags |= aEffective << ICF_PARALLELIZATION_SHIFT;
	}

	bool GetShowNumOfScrollItems() const { return (myScrollItemFlags & SIF_SHOWNUMBEROFITEMS) != 0; }
	void SetShowNumOfScrollItems(bool theVal)
	{
		if (theVal) myScrollItemFlags |= SIF_SHOWNUMBEROFITEMS;
		else        myScrollItemFlags &= ~SIF_SHOWNUMBEROFITEMS;
	}

	ScrollItemFlags GetScrollItemPositioning() const
	{
		return ScrollItemFlags(myScrollItemFlags & ~SIF_SHOWNUMBEROFITEMS);
	}

	void SetScrollItemPositioning(ScrollItemFlags theVal)
	{
		myScrollItemFlags &= SIF_SHOWNUMBEROFITEMS;
		myScrollItemFlags |= theVal;
	}

	//! Correctly processes the HIDDEN_USESYSTEM setting
	//! and returns either HIDDEN_SHOWALWAYS or HIDDEN_SHOWNEVER
	DataType GetActualHiddenFilesMode() const
	{
		DataType aRet = GetHiddenFilesMode();
		if (aRet != HIDDEN_USESYSTEM)
			return aRet;

		SHELLFLAGSTATE aSt;
		SHGetSettings( &aSt, SSF_SHOWALLOBJECTS);

		return aSt.fShowAllObjects ? HIDDEN_SHOWALWAYS : HIDDEN_SHOWNEVER;
	}

	DataType GetBrowseForFoldersMode() const       { return myBrowseInNewWindow; }
	void SetBrowseForFoldersMode(DataType theValue){ myBrowseInNewWindow = theValue; }

	DataType GetActualBrowseFoldersMode() const
	{
		if (myBrowseInNewWindow != BROWSE_USESYSTEM)
			return myBrowseInNewWindow;

		// OSSPECIFIC
		
		HKEY aKey;

		LONG aRes = RegOpenKeyEx
		(
			HKEY_CURRENT_USER,
			_T("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\CabinetState"),
			0,
			KEY_READ,
			&aKey
		);

		if (aRes != ERROR_SUCCESS)
			return BROWSE_EXISTINGWINDOW;


		DWORD aRaw[8];	//reserve enough to store the value
		DWORD aSize = sizeof(aRaw);

		aRes = RegQueryValueEx(aKey, _T("Settings"), 0, NULL, (LPBYTE) aRaw, &aSize);

		RegCloseKey(aKey);

		if (aRes != ERROR_SUCCESS)
			return BROWSE_EXISTINGWINDOW;

		if ( (aRaw[1] & 0x00000020) != 0 )
			return BROWSE_NEWWINDOW;
		else
			return BROWSE_EXISTINGWINDOW;

		// end OSSPECIFIC
	}



	bool GetTreatZipArchivesAsFolders() const       { return (myZipsAreFolders & FTH_EXPAND_ZIPARCH) != 0; }
	bool GetTreatFolderLinksAsFolders() const       { return (myZipsAreFolders & FTH_IGNORE_FLDLINKS) == 0; }
	void SetFileTypeHandlingFlags(int theVal)       { myZipsAreFolders = theVal; }

	bool GetFoldersPrecedeFiles() const             { return (myOrdering & ORDER_FOLDERS_ONTOP) != 0; }
	bool GetFilesPrecedeFolders() const             { return (myOrdering & ORDER_FOLDERS_ONBTM) != 0; }
	bool GetSortAlphabetically() const              { return (myOrdering & ORDER_ALPHABETICAL ) != 0; }
	void SetItemOrdering(DataType theVal)           { myOrdering = theVal; }

	int  GetMaxMenuWidth() const                    { return (int) myMaxMenuWidth; }
	bool GetIsMaxMenuWidthInPercent() const         { return myMaxMenuWidth < 0; }
	void SetMaxMenuWidth(int theVal)                { myMaxMenuWidth = (DataType) theVal; }

	bool GetEnablePeriodicUpdateChecks() const      { return mySkipAutoUpdates == FALSE; }
	void SetEnablePeriodicUpdateChecks(bool theVal) { mySkipAutoUpdates = !theVal; }
};


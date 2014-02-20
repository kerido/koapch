#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////

//! Represents options for icon extraction.
enum IconExtractFlags
{
	EXTRACT_DEFAULT    = 0,  //!< Perform icon extraction in a default manner.
	EXTRACT_PREFERFAST = 1,  //!< Do not perform lengthy extraction, return S_FALSE in case a lengthy
	                         //!  operation is required in order to successfully extract the icon.

	CACHE_IGNORE       = 4,  //!< Ignore the Shell Icon Cache.
};


//! Represents data necessary to draw item icons.
struct ItemIconData
{
	HIMAGELIST ImageList;      //!< Image list that contains the icon. This can be the icon cache or a per-menu image list.
	int        IconIndex;      //!< The index of the icon in the image list.
	int        OverlayIndex;   //!< The index of the icon overlay (in the system image list) or -1 of no overlay applicable.
	bool       IsIconDimmed;   //!< Specifies whether the icon is dimmed, typically for hidden items.

	ItemIconData() : ImageList(0), IconIndex(-1), OverlayIndex(-1), IsIconDimmed(false) { }

	bool operator == (const ItemIconData & theX) const
	{
		return
			ImageList    == theX.ImageList &&
			IconIndex    == theX.IconIndex &&
			OverlayIndex == theX.OverlayIndex &&
			IsIconDimmed == theX.IsIconDimmed;
	}

	bool operator != (const ItemIconData & theX) const
	{
		return ! operator == (theX);
	}
};


//! TODO
struct IconExtractParams : public ItemIconData
{
	DWORD Flags;

	IconExtractParams() : Flags(0) { }
};

//////////////////////////////////////////////////////////////////////////////////////////////

//! Performs the actual icon updating. The SetIconData method gets a pointer to the instance-
//! based image list (the list which is owned by the window). The implementor can either use
//! this list as the container and insert the icon there, or use any other list (if
//! appropriate). This allows to use the global icon cache index for optimized icon storage
MIDL_INTERFACE("422105A4-029F-4ABB-BA60-98F11EA6424E") IIconAcceptor : public IUnknown
{
	enum IconChangeFlags
	{
		CHANGE_ICON = 4,
		CHANGE_OVERLAY = 8,
		CHANGE_ALL = CHANGE_ICON|CHANGE_OVERLAY
	};


	STDMETHOD (RetrieveIconData)      (IconExtractParams * theParams, void ** theOptAsyncArg) = 0;
	STDMETHOD (RetrieveIconDataAsync) (IconExtractParams * theParams, void * theAsyncArg) = 0;
	STDMETHOD (ReleaseAsyncArg)       (void * theAsyncArg) = 0;
};

// {422105A4-029F-4ABB-BA60-98F11EA6424E}
DFN_GUID(IID_IIconAcceptor,
         0x422105a4, 0x29f, 0x4abb, 0xba, 0x60, 0x98, 0xf1, 0x1e, 0xa6, 0x42, 0x4e);

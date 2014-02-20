#pragma once

#include "sdk_IconExtractor.h"
#include "sdk_GuidComparer.h"

#include "UtilShellFunc.h"

//////////////////////////////////////////////////////////////////////////////////////////////

class ShellItemGenericIconManager
{
public:
	enum GenericIcon
	{
		GENERIC_ICON_DOCUMENT    = 0,
		GENERIC_ICON_APPLICATION = 1,
		GENERIC_ICON_FOLDER      = 2
	};

	friend class Application;	//only the application can create the object

	typedef std::map<GUID, int, GuidComparer>    WellKnownIconMap;
	typedef WellKnownIconMap::value_type         WellKnownIconPair;
	typedef WellKnownIconMap::iterator           WellKnownIconIter;
	typedef WellKnownIconMap::const_iterator     WellKnownIconIterC;


private:
	HIMAGELIST mySysImageList;           //!< TODO
	int myExeIconIndex;                  //!< TODO
	int myFolderIconIndex;               //!< TODO
	int myDocumentIconIndex;             //!< TODO
	WellKnownIconMap myWellKnownIcons;   //!< TODO
	bool myNeedToDestroyImgList;         //!< TODO


private:
	ShellItemGenericIconManager()
		: mySysImageList(0), myExeIconIndex(0), myFolderIconIndex(0), myDocumentIconIndex(0), myNeedToDestroyImgList(false)
	{
		OSVERSIONINFO aVer;
		aVer.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		GetVersionEx(&aVer);

		// add a generic EXE icon depending on the OS version

		if (aVer.dwMajorVersion == 5 && aVer.dwMinorVersion == 0)
		{
			// Because on Windows 2000 the system image list cannot be modified,
			// we create an extra one here

			int aW = GetSystemMetrics(SM_CXSMICON);
			int aH = GetSystemMetrics(SM_CYSMICON);
			mySysImageList = ImageList_Create(aW, aH, ILC_COLOR32|ILC_MASK, 1, 1);
			myNeedToDestroyImgList = true;
		}
		else
		{
			// On other operating system the system image list can be safely
			// modified, so there is no need to create an extra one.
			// Furthermore, on Windows Vista and later, we rely on the system
			// image list which seems to readily contain several required icons.

			HIMAGELIST aLarge = 0, aSmall = 0;	//unneeded

			BOOL aRes = Shell_GetImageLists(&aLarge, &aSmall);

			ATLASSERT(aRes == TRUE);

			mySysImageList = aSmall;
		}


		if (aVer.dwMajorVersion >= 6)	// windows vista, windows 7 or server 2008
		{
			myDocumentIconIndex = 0;   // the document and folder icons are
			myFolderIconIndex = 1;     // already present in the system image list

			myExeIconIndex = InsertExtractedIcon( _T("imageres.dll"), 11);

			if (myExeIconIndex < 0)
				myExeIconIndex = myDocumentIconIndex;
		}
		else
		{
			myDocumentIconIndex = InsertExtractedIcon( _T("shell32.dll"), 1);

			myFolderIconIndex = InsertExtractedIcon( _T("shell32.dll"), 4);

			myExeIconIndex = InsertExtractedIcon( _T("shell32.dll"), 2);
		}
	}

	~ShellItemGenericIconManager()
	{
		if (myNeedToDestroyImgList)
			ImageList_Destroy(mySysImageList);
	}

public:
	virtual HRESULT GetIconType(IShellFolder * theSF, LPCITEMIDLIST thePidl) const
	{
		ULONG aAttribs = SFGAO_FOLDER;
		HRESULT aRes = theSF->GetAttributesOf(1, &thePidl, &aAttribs);

		if ( FAILED(aRes) )
			return aRes;

		if (aAttribs & SFGAO_FOLDER)
			return MAKE_HRESULT(0, 0, GENERIC_ICON_FOLDER);

		TCHAR aPath[MAX_PATH];
		aRes = UtilShellFunc::GetDisplayName(theSF, thePidl, SHGDN_NORMAL|SHGDN_FORADDRESSBAR|SHGDN_FORPARSING, aPath, MAX_PATH);

		if ( FAILED(aRes) )
			return aRes;

		int aPathLen = lstrlen(aPath);

		if (aPathLen < 3)
			return MAKE_HRESULT(0, 0, GENERIC_ICON_DOCUMENT); 

		const TCHAR * aExtension = aPath + aPathLen - 3;	//3 is the size of "exe"

		if ( lstrcmpi(aExtension, _T("exe") ) == 0)
			return MAKE_HRESULT(0, 0, GENERIC_ICON_APPLICATION);
		else
			return MAKE_HRESULT(0, 0, GENERIC_ICON_DOCUMENT);
	}


	virtual HRESULT GetIconLocation(int theIconType, ItemIconData * theOut) const
	{
		theOut->ImageList = mySysImageList;

		//TODO: implement for other OS's

		switch(theIconType)
		{
			case GENERIC_ICON_APPLICATION:
				theOut->IconIndex = myExeIconIndex; break;
			case GENERIC_ICON_FOLDER:
				theOut->IconIndex = myFolderIconIndex; break;
			default:
				theOut->IconIndex = myDocumentIconIndex; break;
		}

		return S_OK;
	}

	virtual HRESULT EnsureWellKnownIcon(const GUID & theID, HICON theIcon)
	{
		WellKnownIconIter aIt = myWellKnownIcons.find(theID);

		if ( aIt != myWellKnownIcons.end() )
			return S_FALSE;

		int aIconIndex = ImageList_AddIcon(mySysImageList, theIcon);
		aIt = myWellKnownIcons.insert(aIt, WellKnownIconPair(theID, aIconIndex) );

		return S_OK;
	}

	virtual HRESULT GetWellKnownIconData(const GUID & theID, ItemIconData * theOut) const
	{
		WellKnownIconIterC aIt = myWellKnownIcons.find(theID);

		if ( aIt == myWellKnownIcons.end() )
			return E_FAIL;

		theOut->ImageList = mySysImageList;
		theOut->IconIndex = aIt->second;

		return S_OK;
	}


private:
	int InsertExtractedIcon(const TCHAR * aPath, int aIndex) const
	{
		int aRetVal = -1;

		HICON aIcon = 0;
		UINT aNumExtracted = ExtractIconEx(aPath, aIndex, NULL, &aIcon, 1);

		ATLASSERT(aNumExtracted == 1 && aIcon != 0);

		aRetVal = ImageList_AddIcon(mySysImageList, aIcon);

		DestroyIcon(aIcon);

		return aRetVal;
	}
};
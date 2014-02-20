#pragma once

#include "sdk_Item.h"

#include <shlwapi.h>
#include "Trace.h"

class ILogicQueryTarget;

//////////////////////////////////////////////////////////////////////////////////////////////

class UtilFileExtensionItemMatch
{
public:
	typedef bool  (__stdcall *ITEMMATCHFN) (Item *, LPCTSTR * theExtList, size_t);

public:
	static ITEMMATCHFN InitItemMatchImplementation()
	{
		OSVERSIONINFO aVer;
		aVer.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

		GetVersionEx(&aVer);

		//OSSPECIFIC
		if (aVer.dwMajorVersion == 5 && aVer.dwMinorVersion == 0) //2000
			return InitItemMatchImpl_W2K;

		else                                                      //xp, 2003, 2003 R2, MCE, vista
			return InitItemMatchImpl_WXP;
		//end OSSPECIFIC
	}

public:
	static bool __stdcall ItemMatchByAttributes(Item * theItem)
	{
		bool aRetVal = false;

		IApproachShellItem * aASI = 0;
		HRESULT aRes = theItem->QueryInterface(IID_IApproachShellItem, (void **) &aASI);

		if ( SUCCEEDED(aRes) && aASI != 0 )
		{
			LPITEMIDLIST aAbsolutePidl = 0;

			aRes = aASI->GetShellItemData(IApproachShellItem::PARENTFOLDER_ABSOLUTE, NULL, &aAbsolutePidl);

			if ( SUCCEEDED(aRes) )
			{
				LPCITEMIDLIST aPidlLast = 0;
				IShellFolder * aParent = 0;

				aRes = SHBindToParent(aAbsolutePidl, IID_IShellFolder, (void **) &aParent, &aPidlLast);

				if ( SUCCEEDED(aRes) )
				{
					ULONG aAttr = SFGAO_STREAM;

					aRes = aParent->GetAttributesOf(1, &aPidlLast, &aAttr);

					CoTaskMemFree(aAbsolutePidl);
					aParent->Release();

					if ( SUCCEEDED(aRes) && (aAttr & SFGAO_STREAM) == 0 )
						aRetVal = true;
				}
			}
			aASI->Release();
		}
		return aRetVal;
	}


	static bool __stdcall ItemNameMatchesExtension(Item * theItem, LPCTSTR * theExtList, size_t theListSize)
	{
		// Step 1. Get item display name
		TCHAR aDisplayName[MAX_PATH];
		int aSize = MAX_PATH;

		HRESULT aRes = theItem->GetDisplayName(true, aDisplayName, &aSize);

		if ( FAILED(aRes) )
			return false;


		// Step 2. Find the last dot to separate the extension part
		TCHAR * aExtensionDot = StrRChr( aDisplayName, NULL, _T('.') );

		if (aExtensionDot == 0)
			return false;


		// Step 3. copy the extension from the item display name
		TCHAR aExt[20];

		int aNumToCopy = __min( 19, (int) ( (aDisplayName + lstrlen(aDisplayName) ) - aExtensionDot ) );

		lstrcpyn(aExt, aExtensionDot + 1, aNumToCopy);
		aExt[aNumToCopy] = 0;


		// Step 4. Do the OR compare with each of the items in the list
		for (size_t i = 0; i < theListSize; i++)
			if ( lstrcmpi(aExt, theExtList[i]) == 0 )
				return true;

		return false;
	}

protected:
	static bool __stdcall InitItemMatchImpl_W2K(Item * theItem, LPCTSTR * theExtList, size_t theListSize)
	{
		return ItemNameMatchesExtension(theItem, theExtList, theListSize);
	}

	static bool __stdcall InitItemMatchImpl_WXP(Item * theItem, LPCTSTR * theExtList, size_t theListSize)
	{
		if ( ItemNameMatchesExtension(theItem, theExtList, theListSize) )
			return ItemMatchByAttributes(theItem);

		return false;
	}

};
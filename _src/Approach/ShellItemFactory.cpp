#include "stdafx.h"

#include "sdk_MemoryWriter.h"

#include "ShellItemFactory.h"
#include "ShellFolder.h"
#include "ShellFolderLink.h"
#include "UtilPidlFunc.h"
#include "UtilShellFunc.h"
#include "Trace.h"

//////////////////////////////////////////////////////////////////////////////////////////////

ShellItem * ShellItemFactoryImpl_Base::CreateChild(IShellFolder * theSF, LPITEMIDLIST theItemPidl, int theFlags)
{
	LPCITEMIDLIST aConstPidl = theItemPidl;


	//////////////////////////////////////////////////////////////
	// 1. Query desired attributes of a shell item
	ULONG aAttribs = 0;
	HRESULT aRes = GetAttributes(theSF, aConstPidl, aAttribs);


	if ( CanCreateAsFolder(aAttribs) )
		return new ShellFolder( ProcessPidl(theItemPidl, theFlags), theSF);

	//////////////////////////////////////////////////////////////
	// 2. Resolve the type of theItem
	else if ( !myExpandFolderShortcuts || (aAttribs & SFGAO_LINK) == 0 )
		return  new ShellItem  ( ProcessPidl(theItemPidl, theFlags), theSF) ;


	//////////////////////////////////////////////////////////////
	// 3. In case of a shell link, load it and see if it points to a folder
	CComPtr<IShellLink> aSL;
	aRes = theSF->GetUIObjectOf(NULL, 1, &aConstPidl, IID_IShellLink, NULL, (void **)&aSL);

	if ( FAILED (aRes)  )
		return new ShellItem(ProcessPidl(theItemPidl, theFlags), theSF);


	//////////////////////////////////////////////////////////////
	//4. The file loaded successfully, proceed to decision-making
	LPITEMIDLIST aPidl;	           //a full pidl that represents link's target
	aRes = aSL->GetIDList(&aPidl); //acquire the full pidl

	//the GetIDList method can return S_FALSE. in this case there is no pidl extracted in aPidl.
	//we must not treat this file as a shortcut, but rather, as an individual file

	if (aPidl == NULL)
		return new ShellItem(ProcessPidl(theItemPidl, theFlags), theSF);


	//////////////////////////////////////////////////////////////
	// 5. Obtain the owner of the target pidl
	IShellFolder * aParentSF;	    //the folder which will be queried for attributes
	LPCITEMIDLIST aRelativePidl;  //points somewhere inside aPidl, needs not be released
	aRes = UtilShellFunc::SHBindToParentEx(aPidl, IID_IShellFolder, (void **) &aParentSF, &aRelativePidl);


	//////////////////////////////////////////////////////////////
	// 6. Analyze the attributes of the target
	ShellItem * aRetVal = 0;
	if ( SUCCEEDED(aRes) && aParentSF != 0)
	{
		aRes = GetAttributes(aParentSF, aRelativePidl, aAttribs);

		if ( SUCCEEDED(aRes) && CanCreateAsFolder(aAttribs) )
		{
			// copy the data as it will be used in the shell folder link
			LPITEMIDLIST aLinkPidl = UtilPidlFunc::Copy<MemoryWriter_Crt>(aRelativePidl);
			aParentSF->AddRef();

			aRetVal = new ShellFolderLink( aParentSF, aLinkPidl, theSF, ProcessPidl(theItemPidl, theFlags) );
		}

		aParentSF->Release();
	}

	if (aRetVal == 0)
		aRetVal = new ShellItem( ProcessPidl(theItemPidl, theFlags), theSF);


	//////////////////////////////////////////////////////////////
	// 7. Clean up
	CoTaskMemFree(aPidl);

	return aRetVal;
}

//////////////////////////////////////////////////////////////////////////////////////////////

INLINE LPITEMIDLIST ShellItemFactoryImpl_Base::ProcessPidl(LPITEMIDLIST theItemPidl, int theFlags)
{
	if (theFlags & CREATE_CLONE_PIDL)
		return UtilPidlFunc::Copy<MemoryWriter_Crt>(theItemPidl);
	else
		return theItemPidl;
}

//////////////////////////////////////////////////////////////////////////////////////////////

HRESULT ShellItemFactoryImpl_Base::GetAttributes(IShellFolder * theSF, LPCITEMIDLIST theItemPidl, ULONG & theOutAttribs)
{
#if defined _DEBUG && defined _USE_ATTRIBUTE_CHECKING
	ULONG aBaseAttribs = 0xFFFFFFFF;
#else
	ULONG aBaseAttribs = GetDesiredAttributes();
#endif	//defined _DEBUG && defined _USE_ATTRIBUTE_CHECKING

	ULONG aAttribs = aBaseAttribs|SFGAO_LINK;
	LPCITEMIDLIST aConstPidl = theItemPidl;
	HRESULT aRes = theSF->GetAttributesOf(1, &aConstPidl, &aAttribs);

	if ( FAILED(aRes) )
		return aRes;


	//////////////////////////////////////////////////////////////
	// 2. Analyze attributes (if necessary)

#if defined _DEBUG && defined _USE_ATTRIBUTE_CHECKING

#define CheckAttrib( theBuf, theAttr, theVal ) if (theAttr & theVal) strcat( theBuf, #theVal "|" )
#define ChkAttr(theVal) CheckAttrib(aBuf, aAttribs, theVal)

	char aBuf [1024] = {0};

	strcat (aBuf, "Attribs: ");

	CheckAttrib(aBuf,aAttribs,SFGAO_CANCOPY);
	CheckAttrib(aBuf,aAttribs,SFGAO_CANMOVE );
	CheckAttrib(aBuf,aAttribs,SFGAO_CANLINK);
	CheckAttrib(aBuf,aAttribs,SFGAO_STORAGE);
	CheckAttrib(aBuf,aAttribs,SFGAO_CANRENAME);
	CheckAttrib(aBuf,aAttribs,SFGAO_CANDELETE);
	CheckAttrib(aBuf,aAttribs,SFGAO_HASPROPSHEET);
	CheckAttrib(aBuf,aAttribs,SFGAO_DROPTARGET);
	CheckAttrib(aBuf,aAttribs,SFGAO_CAPABILITYMASK);
	CheckAttrib(aBuf,aAttribs,SFGAO_ENCRYPTED);
	CheckAttrib(aBuf,aAttribs,SFGAO_ISSLOW);
	CheckAttrib(aBuf,aAttribs,SFGAO_GHOSTED);
	CheckAttrib(aBuf,aAttribs,SFGAO_LINK);
	CheckAttrib(aBuf,aAttribs,SFGAO_SHARE);
	CheckAttrib(aBuf,aAttribs,SFGAO_READONLY);
	CheckAttrib(aBuf,aAttribs,SFGAO_HIDDEN);
	CheckAttrib(aBuf,aAttribs,SFGAO_DISPLAYATTRMASK);
	CheckAttrib(aBuf,aAttribs,SFGAO_FILESYSANCESTOR);
	CheckAttrib(aBuf,aAttribs,SFGAO_FOLDER);
	CheckAttrib(aBuf,aAttribs,SFGAO_FILESYSTEM);
	CheckAttrib(aBuf,aAttribs,SFGAO_HASSUBFOLDER);
	CheckAttrib(aBuf,aAttribs,SFGAO_CONTENTSMASK);
	CheckAttrib(aBuf,aAttribs,SFGAO_VALIDATE);
	CheckAttrib(aBuf,aAttribs,SFGAO_REMOVABLE);
	CheckAttrib(aBuf,aAttribs,SFGAO_COMPRESSED);
	CheckAttrib(aBuf,aAttribs,SFGAO_BROWSABLE);
	CheckAttrib(aBuf,aAttribs,SFGAO_NONENUMERATED);
	CheckAttrib(aBuf,aAttribs,SFGAO_NEWCONTENT);
	CheckAttrib(aBuf,aAttribs,SFGAO_CANMONIKER);
	CheckAttrib(aBuf,aAttribs,SFGAO_HASSTORAGE);
	CheckAttrib(aBuf,aAttribs,SFGAO_STREAM);
	CheckAttrib(aBuf,aAttribs,SFGAO_STORAGEANCESTOR);
	CheckAttrib(aBuf,aAttribs,SFGAO_STORAGECAPMASK);

	strcat(aBuf, "\n");

	Trace(aBuf);
#endif	//defined _DEBUG && defined _USE_ATTRIBUTE_CHECKING

	theOutAttribs = aAttribs;
	return S_OK;
}


#pragma once

#include "sdk_Item.h"
#include "sdk_Launchable.h"
#include "sdk_IconExtractor.h"

//////////////////////////////////////////////////////////////////////////////////////////////

//! \brief Represents an Shell file or other non-navigable item. 
//! 
//! Class instances are normally created during child enumeration process.
class ShellItem :
	public Item,
	public ILaunchable,
	public IIconAcceptor,
	public IApproachShellItem
{
protected:
	ULONG myRefs;               //!< The number of references to the object.
	IShellFolder * myParentSF;  //!< Pointer to the Shell Folder that contains the item.
	LPITEMIDLIST myPidl;        //!< A Pidl relative to #myParentSF.

#ifdef _DEBUG
	TCHAR myDbgFullName[MAX_PATH];
#endif


// Constructors, destructor
public:

	//! Default constructor
	ShellItem();

	ShellItem(LPITEMIDLIST theInitPidl, IShellFolder * theInitSF = NULL);

	ShellItem(const TCHAR * thePath);

	virtual ~ShellItem();


public:
	virtual LPCITEMIDLIST GetPidl() const { return myPidl; }
	virtual bool IsFolder() const { return false; }


// Item Members
protected:
	STDMETHODIMP GetDisplayName(bool theFullName, TCHAR * theOutName, int * theSize);


// IUnknown Members
protected:
	ULONG STDMETHODCALLTYPE AddRef();

	ULONG STDMETHODCALLTYPE Release();

	STDMETHODIMP QueryInterface(REFIID theIID,  void ** theOut);


// ILaunchable methods
protected:
	STDMETHODIMP DoLaunch(HWND theParentWnd, ULONG theOptions);


//IIconAcceptor members
protected:
	STDMETHODIMP RetrieveIconData(IconExtractParams * theParams, void ** theOutAsyncArg);

	STDMETHODIMP RetrieveIconDataAsync(IconExtractParams * theParams, void * theAsyncArg);

	STDMETHODIMP ReleaseAsyncArg(void * theAsyncArg);


//IApproachShellItem members
protected:
	STDMETHODIMP GetShellItemData(PidlType theType, IShellFolder ** theOutSF, LPITEMIDLIST * theOutPidl);


// Implementation Details
protected:
	void Init();

	static SHGDNF GetDisplayNameFlags(bool theFullName);
};



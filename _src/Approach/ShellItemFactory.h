#pragma once

#include "ShellItem.h"

//////////////////////////////////////////////////////////////////////////////////////////////

class IShellItemFactory
{
public:
	enum CreationFlags
	{
		CREATE_DEFAULT = 0,
		CREATE_CLONE_PIDL = 1
	};

public:
	virtual ~IShellItemFactory() {}


	//! Creates a Shell Item based on the provided PIDL type.
	//!
	//! \param [in] theSF        A Shell Folder that contains the item.
	//! \param [in] theItemPidl  A single level pidl identifying an item inside \a theSF.
	//! \param [in] theFlags     Specifies whether the object should clone the pidl pointed by \a theItemPidl.
	virtual ShellItem * CreateChild(IShellFolder * theSF, LPITEMIDLIST theItemPidl, int theFlags = CREATE_DEFAULT) = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////////

class ShellItemFactoryImpl_Base : public IShellItemFactory
{
// Constructors, destructor
protected:
	ShellItemFactoryImpl_Base(bool theExpandFolderShortcuts)
		: myExpandFolderShortcuts(theExpandFolderShortcuts)
	{ }


// IShellItemFactory Members
private:
	ShellItem * CreateChild(IShellFolder * theSF, LPITEMIDLIST theItemPidl, int theFlags = CREATE_DEFAULT);


// Additional Overridables
protected:

	//! Returns the attributes that must be extracted from the item.
	//! \return   An integral value representing the minimum attributes required.
	virtual ULONG GetDesiredAttributes() const = 0;


	//! Returns if specified attributed correspond to a folder.
	//! \param [in] theAttribs   The attributes being checked.
	//! \return   True if the attributes correspond to a folder; otherwise, false.
	virtual bool CanCreateAsFolder(ULONG theAttribs) const = 0;


// Implementation Details
private:
	HRESULT GetAttributes(IShellFolder * theSF, LPCITEMIDLIST theItemPidl, ULONG & theOutAttribs);
	static LPITEMIDLIST ProcessPidl(LPITEMIDLIST theItemPidl, int theFlags);


protected:
	bool myExpandFolderShortcuts;
};

//////////////////////////////////////////////////////////////////////////////////////////////

class ShellItemFactoryImpl_ZipNo : public ShellItemFactoryImpl_Base
{
public:
	ShellItemFactoryImpl_ZipNo(bool theExpandFolderLinks)
		: ShellItemFactoryImpl_Base(theExpandFolderLinks) { }

// ShellItemFactoryImpl_Base Members
protected:
	ULONG GetDesiredAttributes() const
		{ return SFGAO_FOLDER|SFGAO_STREAM; }

	bool CanCreateAsFolder(ULONG theAttribs) const 
		{ return (theAttribs & SFGAO_FOLDER) && !(theAttribs & SFGAO_STREAM); }
};

//////////////////////////////////////////////////////////////////////////////////////////////


class ShellItemFactoryImpl_ZipOK : public ShellItemFactoryImpl_Base
{
public:
	ShellItemFactoryImpl_ZipOK(bool theExpandFolderLinks)
		: ShellItemFactoryImpl_Base(theExpandFolderLinks) { }

// ShellItemFactoryImpl_Base Members
protected:
	ULONG GetDesiredAttributes() const
		{ return SFGAO_FOLDER; }

	bool CanCreateAsFolder(ULONG theAttribs) const 
		{ return (theAttribs & SFGAO_FOLDER) != 0; }
};



#pragma once

#include "sdk_ItemDesigner.h"

//////////////////////////////////////////////////////////////////////////////////////////////

//! \brief Represents the way user clicks on an Approach Notification Icon
//! in order to open a Menu for a particular Menu Set.
//! 
//! When opening Approach Items Menus, the user can combine mouse click
//! with modifier keys thus having a reasonable amount of combinations.
class MenuSetActivator
{
public:

	//! Represents a combination of mouse clicks and modifier keys that are used
	//! to open a particular set.
	enum Flags
	{
		MOUSE_LCLICK =   1, //!< Specifies that the menu is activated using the left mouse button click
		MOUSE_MCLICK =   2, //!< Specifies that the menu is activated using the middle mouse button click
		MOUSE_RCLICK =   4, //!< Specifies that the menu is activated using the right mouse button click

		KBMOD_SHIFT  =  32, //!< Specifies that the SHIFT key must be pressed together with the mouse click
		KBMOD_ALT    =  64, //!< Specifies that the ALT key must be pressed together with the mouse click
		KBMOD_CTRL   = 128  //!< Specifies that the CTRL key must be pressed together with the mouse click
	};

	int myValue;

public:
	MenuSetActivator() : myValue(0) { }

	MenuSetActivator(Flags theFlags) : myValue(theFlags) { }

public:
	MenuSetActivator & operator = (int theVal)
	{
		myValue = theVal;
		return *this;
	}

public:
	static MenuSetActivator GetFromWindowsMessage(UINT theWMsg)
	{
		MenuSetActivator aNew;

		switch(theWMsg)
		{
		case WM_LBUTTONDOWN:
			aNew.myValue = MOUSE_LCLICK; break;

		case WM_MBUTTONDOWN:
			aNew.myValue = MOUSE_MCLICK; break;

		case WM_RBUTTONDOWN:
			aNew.myValue = MOUSE_RCLICK; break;

		default:
			break;
		}

		if ( (GetKeyState(VK_SHIFT) & 0x8000) != 0)
			aNew.myValue |= KBMOD_SHIFT;

		if ( (GetKeyState(VK_CONTROL) & 0x8000) != 0)
			aNew.myValue |= KBMOD_CTRL;

		if ( (GetKeyState(VK_MENU) & 0x8000) != 0)
			aNew.myValue |= KBMOD_ALT;

		return aNew;
	}

public:
	bool Empty() const
	{
		return ( myValue & (MOUSE_LCLICK|MOUSE_MCLICK|MOUSE_RCLICK) ) != 0;
	}
};

//////////////////////////////////////////////////////////////////////////////////////////////

class MenuSetKeystroke
{
	//TODO: value format
	int myValue;

public:
	MenuSetKeystroke() : myValue(0) { }

public:
	MenuSetKeystroke & operator = (int theVal)
	{
		myValue = theVal;
		return *this;
	}

public:
	bool Empty() const { return myValue == 0; }
};

//////////////////////////////////////////////////////////////////////////////////////////////

//! Represents a single entry in a Menu Set. Each entry can either represent
//! a single item, a single Item list, or a separator as specified in MenuSetEntry::EntryType
class MenuSetEntry : public MenuSetItemData
{
	TCHAR myName[256];

// Constructors, destructor
public:
	MenuSetEntry(EntryType theType = SEPARATOR)
	{
		Set(theType);
	}

	MenuSetEntry(EntryType theType, REFGUID theProviderGuid)
	{
		Set(theType, theProviderGuid);
	}

	MenuSetEntry(const MenuSetEntry & theSrc)
	{
		Set(theSrc.Type, theSrc.ObjectID);
		SetParam(theSrc.Param, theSrc.ParamSize);
		SetName(theSrc.Name);
	}

	~MenuSetEntry()
	{
		DestroyParam();
	}


// Operations
public:
	EntryType GetType() const       { return Type; }
	void SetType(EntryType theType) { Type = theType; }

	REFGUID GetObjectID() const         { return ObjectID; }
	void SetObjectID(REFGUID theObjID)  { ObjectID = theObjID; }

	int GetIconIndex() const             { return IconIndex; }
	void SetIconIndex(int theIndex)      { IconIndex = theIndex; }

	const TCHAR * GetName() const       { return myName; }
	void SetName(const TCHAR * theName) { lstrcpy(myName, theName); }

	void SetParam(const void * theBuf, int theBufSize)
	{
		DestroyParam();

		Param = new BYTE[ParamSize = theBufSize];
		CopyMemory(Param, theBuf, theBufSize);
	}

	const BYTE * GetParam() const
		{ return Param; }

	int GetParamSize() const
		{ return ParamSize; }

public:
	bool operator == (const MenuSetEntry & theY) const
	{
		if (Size != theY.Size)
			return false;

		if ( GetType() != theY.GetType() )
			return false;

		if ( GetType() == SEPARATOR)
			return true;

		if ( GetObjectID() != theY.GetObjectID() )
			return false;

		if ( GetIconIndex() != theY.GetIconIndex() )
			return false;

		int aNamesEq = lstrcmp( GetName(), theY.GetName() );

		if (aNamesEq != 0)
			return false;

		if ( ParamSize != theY.ParamSize )
			return false;

		if ( ParamSize != 0)
		{
			if (GetParam() == 0 || theY.GetParam() == 0)
				return false;

			int aParamEq = memcmp( GetParam(), theY.GetParam(), ParamSize );

			if (aParamEq != 0)
				return false;
		}

		return true;
	}

	bool operator != (const MenuSetEntry & theY)
	{
		return !operator == (theY);
	}


// Implementation Details
private:
	void Set(EntryType theType)
	{
		Size = sizeof MenuSetItemData;

		Type = theType;

		Param = 0;
		ParamSize = 0;

		Name = myName;
		myName[0] = 0;
		NameSize = 256;

		IconIndex = -1;
	}

	void Set(EntryType theType, REFGUID theProviderGuid)
	{
		Set(theType);
		ObjectID = theProviderGuid;
	}

	void DestroyParam()
	{
		if (Param != 0)
		{
			delete [] Param;
			Param = 0;
		}
	}
};

//////////////////////////////////////////////////////////////////////////////////////////////

//! \brief Represents a set of items displayed when the user clicks the Approach Items menu.
//! 
//! Multiple sets can be invoked, depending on the type of click made. For example,
//! a left click can bring up favorite programs, a right click - favorite Web addresses,
//! and CTLR + middle click - the contents of My Computer.
class MenuSet
{
public:
	typedef std::vector<MenuSetEntry> EntryList;
	typedef EntryList::iterator       EntryIter;
	typedef EntryList::const_iterator EntryIterC;

private:
	MenuSetActivator myActivation;
	MenuSetKeystroke myKeystroke;
	EntryList myEntries;

public:
	MenuSet() : myActivation(MenuSetActivator::MOUSE_LCLICK)
		{ }

	explicit MenuSet(MenuSetActivator theActivation) : myActivation(theActivation)
		{ }

public:
	MenuSet & operator = (const MenuSet & theVal)
	{
		myActivation = theVal.myActivation;
		myKeystroke = theVal.myKeystroke;

		myEntries.clear();

		if (theVal.myEntries.size() > myEntries.capacity() )
			myEntries.reserve( theVal.myEntries.size() );

		for(EntryIterC aIt = theVal.myEntries.begin(); aIt != theVal.myEntries.end(); aIt++)
			myEntries.push_back(*aIt);

		return *this;
	}

public:
	int GetEntryCount() const
		{ return (int) myEntries.size(); }

	const MenuSetEntry & GetEntry(int theIndex) const
		{ return myEntries[theIndex]; }

	void AddEntry(const MenuSetEntry & theObj)
		{ myEntries.push_back(theObj); }

	void Clear()
		{ myEntries.clear(); }
};

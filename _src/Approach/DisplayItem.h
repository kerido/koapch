#pragma once

#include "sdk_DisplayItem.h"

//////////////////////////////////////////////////////////////////////////////////////////////

/// <summary>
/// This is a base class for view iterators. A const reference to ItemData
/// is returned each time the iterator advances.
/// </summary>
class ItemData
{
public:
	virtual ~ItemData() {}

public:
	/// <summary>Returns a pointer to an item being enumerated.</summary>
	virtual DisplayItem * GetItem() const = 0;


	/// <summary>Returns the pointer to the view that owns the item.</summary>
	virtual class IDisplayItemHost * GetHost() const = 0;


	/// <summary>Gets the item rectangle in view coordinates.</summary>
	/// <param name="theOut">The destination rectangle that receives the value.</param>
	virtual void GetViewRect  (RECT & theOut) const = 0;


	/// <summary>Gets the item rectangle in window coordinates.</summary>
	/// <param name="theOut">The destination rectangle that receives the value.</param>
	virtual void GetWindowRect(RECT & theOut) const = 0;


	/// <summary>Creates an exact copy of the instance without
	/// modifying the instance's state.</summary>
	virtual ItemData * Clone() const = 0;


	//! TODO
	virtual int GetID() const = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////////

/// <summary>
/// Represents an 'auto pointer' analog that wraps a pointer to ItemData
/// and destroys it when the object goes out of scope.
/// </summary>
class ItemDataWrapper
{
protected:
	ItemData * myData;

public:
	ItemDataWrapper(ItemData * theData = 0) : myData(theData)
		{}

	~ItemDataWrapper()
		{ SafeRelease(); }

private:
	// this class must not be copy-constructed
	ItemDataWrapper(const ItemDataWrapper & theSrc) : myData( theSrc->Clone() )
		{}


//operators
public:
	ItemDataWrapper & operator = (ItemData * theData)
	{
		SafeRelease();
		myData = theData;

		return *this;
	}

	ItemData * operator ->() const
		{ return myData; }

	bool operator == (const ItemData * theData) const
		{ return myData == theData; }

	bool operator != (const ItemData * theData) const
		{ return myData != theData; }

	operator ItemData * ()
		{ return myData; }

	operator const ItemData * () const
		{ return myData; }

private:
	void SafeRelease()
	{
		if (myData != 0)
			{ delete myData; myData = 0; }
	}
};

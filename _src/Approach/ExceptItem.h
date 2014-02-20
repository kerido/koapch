#pragma once

// (c) 2005 Kirill Osipov

#include "sdk_Selectable.h"
#include "sdk_DisplayItem.h"

//////////////////////////////////////////////////////////////////////////////////////////////

//TODO: rename ExceptItemType values to longer names
enum ExceptItemType
{
	SUCCESS = 0,	//never actually used in this class

	EMPTY,
	UNAVAILABLE,
	GENERIC
};

//////////////////////////////////////////////////////////////////////////////////////////////

class ExceptItem :
	public DisplayItem,
	public ISelectable
{
private:
	ULONG myNumRefs;         //!< Stores the number of references to the curent object
	ExceptItemType myType;   //!< Stores the exception type.  


public:
	ExceptItem (ExceptItemType theType);


// DisplayItem methods
protected:
	virtual void Measure (HDC theDC, ItemMeasure & theMeasure, const Theme * theTheme);

	void Draw(HDC theDC, bool theSelected, const RECT & theContainer, const Theme * theTheme);
	virtual void Draw(const ItemDrawData * theData, const class Theme * theTheme);

	virtual class Item * GetLogicalItem() const { return 0; }
	virtual void OnEvent(EventType theEvent, class MenuWindow * theHost) { }


// ISelectable members
protected:
	STDMETHODIMP OnSelect(ULONG theFlags)
		{ return MAKE_HRESULT(0, 0, DEFAULT); }


// IUnknown members
public:

	ULONG STDMETHODCALLTYPE AddRef();

	ULONG STDMETHODCALLTYPE Release();

	STDMETHODIMP QueryInterface(REFIID theIID, void ** theOut);


// Implementation details
private:
	HRESULT GetText(TCHAR * theOutBuf, int * theBufSize);
};

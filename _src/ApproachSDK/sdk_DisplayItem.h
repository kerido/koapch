#pragma once

MIDL_INTERFACE("6B6CEC31-E47B-4681-B516-28D5D9EEA843") IDisplayItemGetName : public IUnknown
{
	STDMETHOD (GetDisplayString) (TCHAR * theOutBuf, int * theSize) = 0;
};

DFN_GUID(IID_IDisplayItemGetName,
         0x6B6CEC31, 0xE47B, 0x4681, 0xB5, 0x16, 0x28, 0xD5, 0xD9, 0xEE, 0xA8, 0x43);

//////////////////////////////////////////////////////////////////////////////////////////////

//! Represents the desired dimensions of a DisplayItem
struct ItemMeasure
{
	//if zero then the default value is used
	int Width, Height;
	UINT Mask;	//one of the MaskVal

	enum MaskVal { MASKVAL_NONE = 0, WIDTH = 1, HEIGHT = 2 };

	ItemMeasure(UINT theMask = MASKVAL_NONE, int theWidth = 0, int theHeight = 0)
		: Width(theWidth), Height(theHeight), Mask(theMask) {}
};

//////////////////////////////////////////////////////////////////////////////////////////////

//! Stores data necessary to draw a DisplayItem instance.
struct ItemDrawData
{
	HDC DC;          //!< The Device Context onto which drawing will take place.
	RECT Rect;       //!< The rectangle of the Item being drawn.

	ULONG
		State,         //!< The state of the item as defined in the ItemState enumeration.

		ContentMask,   //!< The content of the item that needs to be drawn as defined in the
		               //!  ItemContent enumeration. The drawing entity should take advantage
		               //!  of this field to draw only necessary parts.

		Flags,         //!< Reserved for future use.
		FlagsEx;       //!< Reserved for future use.


	ItemDrawData() : DC(0UL), State(0UL), ContentMask(0UL), Flags(0UL), FlagsEx(0UL)
	{
		Rect.left = 0;
		Rect.top = 0;
		Rect.right = 0;
		Rect.bottom = 0;
	}

	ItemDrawData( HDC theDC, const RECT& theRect, ULONG theState = 0UL, ULONG theContentMask = 0UL,
	              ULONG theFlags = 0UL, ULONG theFlagsEx = 0UL) :
			DC(theDC), Rect(theRect), State(theState), ContentMask(theContentMask),
			Flags(theFlags), FlagsEx(theFlagsEx)
	{ }
};

//////////////////////////////////////////////////////////////////////////////////////////////

class MenuWindow;
class Theme;
class Item;

//! Serves as visual representation of a Logical Item.
//! \deprecated Possibly this class must be merged with #Item.
class DECLSPEC_NOVTABLE DisplayItem : public IUnknown
{
protected:
	UINT myExpansionCode;

// Constructors, destructor
public:
	DisplayItem() : myExpansionCode(0U) { }
	virtual ~DisplayItem()              { }

	//! Represents an event that occurs on an instance. Each event is processed by the
	//! OnEvent method.
	enum EventType
	{
		EVENT_LOAD   = 1,    //!< Called when the item is loaded and added to the hosting menu.
		EVENT_UNLOAD,        //!< Called when the item is hosting menu is about to unload the item.
		EVENT_SHOW,          //!< Not currently used.
		EVENT_HIDE,          //!< Not currently used.
		EVENT_INIT,          //!< Called before the item is added to the hosting menu.

		EVENT_RECTCHANGED    //!< Called when the item's rectangle is changed.
		                     //!  the Param contains the client rect of the item.
		                     //!  Currently not in use.
	};

	//! Represents the state of the DisplayItem.
	enum ItemState
	{
		ITEMSTATE_NORMAL = 0,    //<! Item is deselected
		ITEMSTATE_SELECTED = 1   //<! Item is selected
	};

	//! Represents the content region of the DisplayItem.
	enum ItemContent
	{
		ITEMCONTENT_NONE = 0,   //<! No content.

		ITEMCONTENT_ICON = 1,   //<! The rectangule containing the DisplayItem's icon.
		ITEMCONTENT_TEXT = 2,   //<! The rectangule containing the DisplayItem's text.

		ITEMCONTENT_ALL = ITEMCONTENT_ICON|ITEMCONTENT_TEXT //!< The entire DisplayItem's rectangle.
	};


// Interface methods
public:

	//! TODO
	virtual void Measure(HDC theDC, ItemMeasure & theMeasure, const Theme * theTheme) = 0;

	//! TODO
	virtual void Draw(const ItemDrawData * theData, const Theme * theTheme) = 0;

	//TODO: Instead of returning a pointer to Item, a more abstract class should be returned
	//      This will help binding to IContentProvider which is not part or the
	//      Item hierarchy
	virtual Item * GetLogicalItem() const  = 0;


	//! Processes the event that occurs on the instance.
	//! \param [in] theEvent      The code of the event that occurred.
	//! \param [in] theHost       The Menu that contains the current instance.
	virtual void OnEvent(EventType theEvent, MenuWindow * theHost) = 0;


public:
	UINT GetExpansionCode() const { return myExpansionCode; }
};

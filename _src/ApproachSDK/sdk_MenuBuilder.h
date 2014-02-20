#pragma once

class MenuWindow;
class Theme;
interface IMenuManagerAgent;
class Item;
struct BitmapInfo;

//////////////////////////////////////////////////////////////////////////////////////////////

struct MenuBuilderData
{
	IMenuManagerAgent * Agent;
	Theme             * Theme;
	RECT                Rect;

	MenuBuilderData() : Agent(0), Theme(0)
	{
		SecureZeroMemory(&Rect, sizeof RECT);
	}
};

//////////////////////////////////////////////////////////////////////////////////////////////

//! When implemented, represents an object that can build child menus.
//! This interface should be implemented in Approach Plugins which provide
//! preview functionality to certain file types (such graphic file previewers).
MIDL_INTERFACE("FA3859CC-A1AF-42E7-B441-356998A93FF0") IMenuBuilder : public IUnknown
{
	//! Specifies whether an item is expandable, i.e. a child menu can be built from it.
	enum ItemExpansionCode
	{
		NON_EXPANDABLE =          0x00000000, //!< Specifies that an item is not expandable (i.e. a file).

		EXPANDABLE_BASIC_FOLDER = 0xFFFFFFFF  //!< Specifies that an item must be treated as regular folder
	};


	//! Constructs a menu from the specified data item.
	//! \param theItem       [in]  The Item from which a menu must be constructed.
	//! \param theBuildData  [in]  The data necessary to construct a menu.
	//! \param theOutMenu    [out] The pointer that will hold the constructed menu object.
	//! \return  TODO
	STDMETHOD (Build) (Item * theItem, const MenuBuilderData * theBuildData, MenuWindow ** theOutMenu) = 0;


	//! Returns a code that which specifies whether a child menu can be build
	//! from a given item.
	//! \param [in] theItem
	//!         The item being checked.
	//! 
	//! \param [out] theOut
	//!         The implementation must return #NON_EXPANDABLE if no child menu can be built.
	//!         If the implementation returns #EXPANDABLE_BASIC_FOLDER, a standard folder arrow
	//!         will be displayed next to the item and the item will be treated as a folder
	//!         (i.e. when the user Preferences instruct folders to appear on top of the item list,
	//!         the item will be put together with normal folders. The implementation <em>must not</em>
	//!         return #EXPANDABLE_BASIC_FOLDER for files that have been extended with a preview menu.
	//!         If some other value is returned, the client must call #GetArrowBitmap to retrieve
	//!         the arrow bitmap.
	//! \return  TODO
	STDMETHOD (CanBuild) (Item * theItem, UINT * theOut) = 0;


	//! Retrieves the arrow bitmap for the specified item
	//! \param [in] theSelected
	//!         True, if the items is currently selected; otherwise, false.
	//! 
	//! \param [in] theExpCode
	//!         The code previously returned by CanBuild
	//! 
	//! \param [in] theTheme
	//!         The Theme currently being active application-wide.
	//! 
	//! \param [out] theOutInfo
	//!         The information about the bitmap that was extracted.
	//! 
	//! \return  TODO
	STDMETHOD (GetArrowBitmap) (UINT theExpCode, bool theSelected, const Theme * theTheme, BitmapInfo * theOutInfo) = 0;
};


// {FA3859CC-A1AF-42E7-B441-356998A93FF0}
DFN_GUID(IID_IMenuBuilder,
         0xFA3859CC, 0xA1AF, 0x42E7, 0xB4, 0x41, 0x35, 0x69, 0x98, 0xA9, 0x3F, 0xF0);

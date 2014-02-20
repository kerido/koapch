#pragma once

#include "sdk_ContextMenuExtension.h"

//////////////////////////////////////////////////////////////////////////////////////////////

//! \brief
//!     Performs invocation of Shell Item context menu commands.
//! 
//! \details
//!     The main responsibility of this class is to determine whether a synchronous
//!     or asynchronous operation must be performed. This is required because the
//!     only way to browse an item in the existing Explorer window is by executing
//!     the command in the address space of the Explorer process.
//! 
//!     If an asynchronous operation needs to be performed, the class queries an
//!     IContextMenu pointer, serializes all the data from the context menu command
//!     into shared memory and transfers execution to the host process (i.e. Explorer).
//! 
//!     On the end of the host process the data is deserialized, an IContextMenu pointer
//!     is queried again and the command whose id has been deserialized is executed on the
//!     IContextMenu pointer.
//! 
//!     If the object is a folder or must be treated as a folder, an IShellBrowser
//!     pointer will be queried from the Shell View window and a call to BrowserObject
//!     will be made.
//! 
//! \sa
//!   - InterprocessContextMenuInfo
//!   - IpcModule::LLExecuteContextMenu
class ItemContextMenuHandlerDefault : public IItemContextMenuHandler
{
	// the Data for context menu extensions provided by plugins or
	// other entities that utilize RegisterContextMenuExtension

	class ExecutionContext;


// Fields
private:
	ExecutionContext * myCurContext;


public:
	ItemContextMenuHandlerDefault();


// IItemContextMenuHandler Members
protected:
	HRESULT DoHandle(Item * theItem, HWND theParentWnd, bool theDefaultOnly);
};

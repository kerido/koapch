#pragma once

class Item;
class DisplayItem;
class ApplicationSettings;

//////////////////////////////////////////////////////////////////////////////////////////////

//! Executes context menu commands.
class DECLSPEC_NOVTABLE IItemContextMenuHandler
{
public:

	//! Executes a context menu command on a specified Item instance. If \a theDefaultOnly
	//! is set to false, displays a context menu prior to execution.
	//!
	//! \param [in] theItem
	//!     The Item on which a context menu command will be executed.
	//!
	//! \param [in] theParentWnd
	//!     Handle to the approach menu that contains the item.
	//!
	//! \param [in] theDefaultOnly
	//!     True if a context menu needs to be displayed prior to execution; otherwise, false.
	//!
	//! \return
	//!     TODO
	virtual HRESULT DoHandle(Item * theItem, HWND theParentWnd, bool theDefaultOnly) = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////////

/*
//!Represents an object responding to the WM_SETTINGCHANGE message.
class DECLSPEC_NOVTABLE IOnSettingChangeHandler
{
public:
	//! Processes the WM_SETTINGCHANGE message.
	//! \param theWParam   The code of the change.
	//! \param theLParam   Specific to the code.
	virtual void OnSettingChange(WPARAM theWParam, LPARAM theLParam) = 0;
};*/

//////////////////////////////////////////////////////////////////////////////////////////////

class DECLSPEC_NOVTABLE IMessageProcessor : public IUnknown
{
public:
	STDMETHOD (ProcessMessage) (HWND theWindow, UINT theMessage, WPARAM theWParam, LPARAM theLParam) = 0;
};

DFN_GUID(IID_IMessageProcessor,
         0x4DF51169, 0x21D5, 0x41E5, 0xA1, 0xB3, 0x9D, 0x83, 0xD4, 0xAB, 0x30, 0x77);

//////////////////////////////////////////////////////////////////////////////////////////////

MIDL_INTERFACE("F2103669-1DA7-4A4E-9AB0-432695371C3E") IMessageProcessorStorage : public IUnknown
{
public:
	STDMETHOD (RegisterProcessor)   (IMessageProcessor * theProcessor) = 0;
	STDMETHOD (UnregisterProcessor) (IMessageProcessor * theProcessor) = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////////

//! Implemented by classes that want to respond to application preferences changes.
class DECLSPEC_NOVTABLE IPreferencesEventProcessor
{
public:
	//! Represents the event which occurred on application preferences
	enum PreferencesEvent
	{
		PREFEVENT_LOAD,   //!< The preferences were successfully loaded.

		PREFEVENT_CHANGE, //!< The preferences were modified by the user.

		PREFEVENT_RESET   //!< The preferences were reset to their default values.
		                  //!  This happens, for example, when the preferences cannot
		                  //!  be loaded from a persistent storage.
	};

public:

	//! Processes the application preferences event.
	//! 
	//! \param theEvent  The event that occurred on the application settings.
	//! 
	//! \param thePrefs  The present application settings. Always points to the settings
	//!                  which are currently in effect.
	virtual void OnPreferencesEvent(PreferencesEvent theEvent, ApplicationSettings & thePrefs) = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////////

class DECLSPEC_NOVTABLE ISetRegisteredUser
{
public:
	virtual bool IsRegistered() const = 0;

	virtual void SetRegistered(bool theRegistered) = 0;

	virtual const TCHAR * GetRegisteredUser() const = 0;

	virtual void SetRegisteredUser(const TCHAR * theUser, int theSize_User) = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////////

struct ItemIconData;

class DECLSPEC_NOVTABLE IIconCache
{
public:
	virtual bool Lookup (wchar_t * thePath, int theIconIndex, ItemIconData & theData) = 0;

	virtual void Add    (wchar_t * thePath, int theIconIndex, HICON theIcon, ItemIconData & theOutData) = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////////

interface IApproachPlugin;

struct PluginInfo
{
	PluginInfo(HMODULE theMod = 0, IApproachPlugin * theClass = 0) : myModule(theMod), myPlugin(theClass) {}

	HMODULE myModule;
	IApproachPlugin * myPlugin;
};

class DECLSPEC_NOVTABLE IPluginManager
{
public:
	virtual ~IPluginManager() {}

public:
	virtual bool LoadPlugins(const TCHAR * thePath, int theSize_Path) = 0;

	virtual void ReleasePlugins() = 0;

	virtual int GetPluginCount() const = 0;

	virtual bool HasInstalledPlugins() const = 0;

	virtual HRESULT GetPluginInfo(const TCHAR * theFullPath, PluginInfo * theOut) = 0;
};

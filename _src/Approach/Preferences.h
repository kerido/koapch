#pragma once

#include "sdk_ComObject.h"
#include "sdk_ChangeableEntity.h"

#include "Prefs.h"
#include "MenuSet.h"

//////////////////////////////////////////////////////////////////////////////////////////////

class MenuSet;

class KoApproachItemsRoots
{
private:
	typedef ATL::CSimpleArray<CString> DirectoryVector;


private:
	DirectoryVector myDirectories;


public:
	LONG Load(HKEY theKey, const TCHAR * theValue);

	void Save(HKEY theKey, const TCHAR * theValue) const;

	bool AddRoot(const TCHAR * theRoot);

	const TCHAR * operator [](int theIndex) const { return myDirectories[theIndex]; }
	int GetLength() const                         { return myDirectories.GetSize(); }
};

//////////////////////////////////////////////////////////////////////////////////////////////

class ApplicationSettings :
	public PrefsSerz,
	public IChangeableEntity
{
private:
	const static int UrlSize = 1000;


public:
	//! Specifies the URL type to be retrieved by the GetUrl method.
	enum UrlType
	{
		URL_HOME = 1,                     //!< The URL of KO Approach Web Site.
		URL_ORDER,                        //!< The URL of the Order HTTP resource.
		URL_ACTIVATION,                   //!< The URL of the update request processor.
		URL_UPDATE,                       //!< The URL of the activation request processor.
		URL_FEEDBACK                      //!< The URL of the Send Feedback resource.
	};

	enum TitlebarMenusFlags
	{
		TBM_ORDER_PARENT_TO_CHILD  = 0,   //!< TODO
		TBM_ORDER_CHILD_TO_PARENT  = 1,   //!< TODO

		TBM_CURFOLDER_AS_ITEM      = 2,   //!< TODO
		TBM_CURFOLDER_AUTO_SELECT  = 4    //!< TODO
	};


// Fields
private:
	DataType myAutoEnableFolderMenus;    //!< Nonzero if Folder Menus should be automatically enabled at program startup; otherwise, zero.
	DataType myAutoEnableApproachItems;  //!< Nonzero if Approach Items should be automatically enabled at program startup; otherwise, zero.
	DataType myAutoEnableTitleBar;       //!< Nonzero if Titlebar Menus should be automatically enabled at program startup; otherwise, zero.
	DataType myUseReleaseTrace;          //!< Nonzero if Approach should gather various status usage events and dump them into the trace file.
	DataType myAlreadyLaunched;          //!< Nonzero if the program has been already launched at least once; otherwise, zero.
	DataType myTitlebarFlags;            //!< Stores options that control the behavior of the Titlebar Menus feature.
	TCHAR myLocale[6];                   //!< TODO

	TCHAR myHomeUrl[UrlSize];            //!< Stores the URL of KO Approach Web Site. \sa #URL_HOME
	TCHAR myOrderUrl[UrlSize];           //!< Stores the URL of the Order HTTP resource. \sa #URL_ORDER
	TCHAR myUpdateUrl[UrlSize];          //!< Stores the URL of the update request processor. \sa #URL_UPDATE
	TCHAR myActivateUrl[UrlSize];        //!< Stores the URL of the activation request processor. \sa #URL_ACTIVATION
	TCHAR myFeedbackUrl[UrlSize];        //!< Stores the URL of the Send Feedback resource. \sa #URL_FEEDBACK

	TCHAR myDisabledPlugins[1024];       //!< Stores the string with the names of disabled plugins, separated by an asterisk (*)

	KoApproachItemsRoots myRoots;        //!< Stores paths of KO Approach Items directories
	                                     //!  belonging to <em>all</em> users on the computer
	                                     //!  on which KO Approach is installed.

	MenuSet myMainMenuSet;               //!< TODO

	ComRefCount<> myNumRefs;             //!< Stores the number of references to the current object.

// Constructors, Destructor
public:
	ApplicationSettings();

	ApplicationSettings(const ApplicationSettings & theVal);

	~ApplicationSettings();


// Interface
public:
	bool GetAutoEnableFolderMenus() const          { return myAutoEnableFolderMenus != 0; }
	void SetAutoEnableFolderMenus(bool theVal)     { myAutoEnableFolderMenus = theVal ? 1 : 0; }

	bool GetAutoEnableKoApproachItems() const      { return myAutoEnableApproachItems != 0; }
	void SetAutoEnableKoApproachItems(bool theVal) { myAutoEnableApproachItems = theVal ? 1 : 0; }

	bool GetAutoEnableTitleBar() const             { return myAutoEnableTitleBar != 0; }
	void SetAutoEnableTitleBar(bool theVal)        { myAutoEnableTitleBar = theVal ? 1 : 0; }

	bool GetUseReleaseLogging() const              { return myUseReleaseTrace != 0; }

	bool GetAlreadyLaunched() const                { return myAlreadyLaunched != 0; }
	void SetAlreadyLaunched(bool theVal)           { myAlreadyLaunched = theVal ? 1 : 0; }

	DataType GetTitlebarMenusFlags() const             { return myTitlebarFlags; }
	void SetTitlebarMenusFlags(DataType theVal){ myTitlebarFlags = theVal; }


	const TCHAR * GetLocale(int * theLength) const
	{
		if (theLength != 0)
			*theLength = lstrlen(myLocale);

		return myLocale;
	}

	void SetLocale(const TCHAR * theLocale, int theLength)
	{
		lstrcpyn(myLocale, theLocale, theLength+1);
	}

	const MenuSet * GetApproachItemsSet() const
		{ return &myMainMenuSet; }

	MenuSet * GetApproachItemsSet()
		{ return &myMainMenuSet; }

	//! Fills the current instance with default values.
	void SetDefault();


	//! Loads preferences from a persistent storage.
	//! \return    True if all values were loaded successfully; otherwise, false.
	bool Load();


	//! Writes the preferences to a persistent storage.
	//! \return    True if all values were saved successfully; otherwise, false.
	bool Save() const;


	void InitializeApproachItemsSet();

	// TODO: move to a dedicated persistence class
	LONG RegQueryApproachItems(HKEY theKey);

	LONG RegSaveApproachItems(HKEY theKey) const;
	// end TODO


	//! Stores a reference to the KO Approach Items directory of a particular user.
	//! \param [in] theRoot    A non-null C string pointing to the Approach Items folder.
	//! \return                True if the object was modified; otherwise, false.
	bool AddKoApproachItemsRoot(const TCHAR * theRoot);


	//! Retrieves a URL of the specified type
	//! \param [in] theType   The type of the URL to retrieved.
	//! \return               A non-null C string if the URL is found; otherwise, a null pointer.
	const TCHAR * GetUrl(UrlType theType) const;


	//! Assigns values from the source settings and raises the change event.
	//! \param [in] theVal            The source settings.
	void Set(const ApplicationSettings & theVal);


	//! Returns true if a plugin with a specified name has been disabled.
	//! \param [in] theName           The plugin name.
	//! \return                       True if the plugin is disabled; otherwise, false.
	bool IsPluginDisabled(const TCHAR * theName) const;


	//! Ensures that the plugin may be loaded next time KO Approach starts.
	//! \param [in] theName           The plugin name.
	//! \return                       True if the plugin state (enabled vs disabled) changes.
	bool EnablePlugin(const TCHAR * theName);


	//! Ensures that the plugin is not loaded next time KO Approach starts.
	//! \param [in] theName           The plugin name.
	//! \return                       True if the plugin state (enabled vs disabled) changes.
	bool DisablePlugin(const TCHAR * theName);

	//TODO: get rid of explicit registry coupling
	static HKEY EnsureRegistryKey(HKEY theParent, REGSAM theAccess);


// IChangeableEntity members
protected:
	virtual STDMETHODIMP RaiseUpdateChanges(ULONG theChangeFlags);


// IUnknown members
public:
	ULONG STDMETHODCALLTYPE AddRef();

	ULONG STDMETHODCALLTYPE Release();

	STDMETHODIMP QueryInterface(REFIID theIID, void ** theOut);


// Implementation Details
private:
	static TCHAR * GetKeyName()     { return _T("SOFTWARE\\KO Software\\Approach"); }
	static HKEY GetUserHiveName()   { return HKEY_CURRENT_USER; }
	static HKEY GetGlobalHiveName() { return HKEY_LOCAL_MACHINE; }
};


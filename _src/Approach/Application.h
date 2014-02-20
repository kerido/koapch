#pragma once

#include "sdk_PropPages.h"
#include "sdk_ContextMenuExtension.h"
#include "sdk_GuidComparer.h"

#include "ShellItemContextMenuExtension.h"
#include "Preferences.h"
#include "LocalizationManager.h"
#include "HotspotNew.h"
#include "Logging.h"
#include "HtmlHelpProvider.h"
#include "FilePathProvider.h"
#include "IconManager.h"

//////////////////////////////////////////////////////////////////////////////////////////////

// Forward declarations
interface IItemList;
interface IItemFactory;
interface IMenuBuilder;
class IShellItemFactory;
class StandardPropPages;
class IpcModule;
class IItemContextMenuHandler;
class ItemContextMenuHandlerDefault;
class IIconCache;
class ShellIconCacheImplNew;
class IFrameworkAgent;
class UxManager;
class RootWindow;
class ISetRegisteredUser;
class RegistrationManager;
class MenuBuilder_Shell;
class ShellItemGenericIconManager;

//////////////////////////////////////////////////////////////////////////////////////////////

struct ObjectData
{
	GUID ObjectID;
	HMODULE Module;
};

template<typename tObj>
struct PluginObjectData
{
	tObj   * Object;
	HMODULE  Module;

	PluginObjectData(tObj * theObject, HMODULE theModule)
		: Object(theObject), Module(theModule)
	{ }
};

typedef PluginObjectData<IContextMenuExtension> ContextMenuExtensionData;
typedef PluginObjectData<IMenuBuilder>          PluginMenuBuilderData;

//////////////////////////////////////////////////////////////////////////////////////////////

//! Represents global functionality of the program.
//! \remarks  This class implements the Singleton pattern.
//!           An const reference to the object can be obtained through a call to #InstanceC.
//!           An non-const reference to the object can be obtained through a call to #Instance.
class Application
{
private:
	typedef ATL::CSimpleArray<IPreferencesEventProcessor *> PreferencesEventProcessors;
	typedef ATL::CSimpleArray<IPropPageFactory *>           PropPageFactories;
	typedef ATL::CSimpleArray<ContextMenuExtensionData>     ContextMenuExtensions;

	typedef std::vector<ILocalizationEventProcessor *>      LocalizationEventProcessors;
	typedef LocalizationEventProcessors::iterator           LocalizationEventProcessorIter;
	typedef LocalizationEventProcessors::const_iterator     LocalizationEventProcessorIterC;

	typedef std::map<GUID, IItemFactory *, GuidComparer>    ItemFactoryMap;
	typedef ItemFactoryMap::value_type                      ItemFactoryPair;
	typedef ItemFactoryMap::iterator                        ItemFactoryIter;
	typedef ItemFactoryMap::const_iterator                  ItemFactoryIterC;

	typedef std::map<GUID, IItemList*, GuidComparer>        ItemListMap;
	typedef ItemListMap::value_type                         ItemListPair;
	typedef ItemListMap::iterator                           ItemListIter;
	typedef ItemListMap::const_iterator                     ItemListIterC;

	typedef std::list<PluginMenuBuilderData>                MenuBuilderList;
	typedef MenuBuilderList::iterator                       MenuBuilderIter;
	typedef MenuBuilderList::const_iterator                 MenuBuilderIterC;

private:

	class GenerationStub
	{
#ifdef _DEBUG
	public:
		GenerationStub()   { ourGeneration++; }
		~GenerationStub()  { ourGeneration--; }
#endif
	};

	class SingletonStub
	{
	public:
		SingletonStub(Application * theApp)
		{
			ourInstance = theApp;

#ifdef _DEBUG
			ourGeneration = -1;
#endif
		}

		~SingletonStub()
		{
			ourInstance = 0;

#ifdef _DEBUG
			ourGeneration = -1;
#endif
		}
	};


	template<typename tObj, int tGen>
	class GenerationObject
	{
	private:
		tObj myObj;

	public:
		GenerationObject() : myObj() { }

		template<typename tArg1>
		GenerationObject(tArg1 theArg1) : myObj(theArg1) { }

		//TODO: add more constructors


	public:
		tObj & GetObject()
		{
#ifdef _DEBUG
			ATLASSERT(ourGeneration >= tGen);
#endif

			return myObj;
		}

		const tObj & GetObject() const
		{
#ifdef _DEBUG
			ATLASSERT(ourGeneration >= tGen);
#endif

			return myObj;
		}

		void SetObject(const tObj & theObj)
			{ myObj = theObj; }

	public:
		tObj * operator * ()
			{ return &GetObject(); }

		const tObj * operator * () const
			{ return &GetObject(); }

		tObj * operator -> ()
			{ return &GetObject(); }

		const tObj * operator -> () const
			{ return &GetObject(); }

		tObj & operator & ()
			{ return GetObject(); }

		const tObj & operator & () const
			{ return GetObject(); }

		GenerationObject<tObj, tGen> & operator = (const tObj & theObj)
		{
			SetObject(theObj);
			return *this;
		}
	};

	enum ShellItemFactoryType
	{
		Unknown                     = -1,
		ZipsAsFolders               = 0,
		ZipsAsFiles                 = 1,
		FolderShortcutsAreFolders   = 0,
		FolderShortcutsAreFiles     = 2
	};

	template<int tGen>
	class GenerationShellItemFactory : public GenerationObject<IShellItemFactory *, tGen>
	{
	private:
		int myType;

	public:
		GenerationShellItemFactory() : myType(Unknown) { }

	public:
		int GetType() const { return myType; }
		void Set(int theType, IShellItemFactory * theObj)
		{
			myType = theType;
			SetObject(theObj);
		}
	};


private:
	SingletonStub                                   myStub;

	GenerationObject<FilePathProvider,           0> myFilePathPrv;          // Gen 0
	GenerationObject<ApplicationSettings,        0> myPrefs;                // Gen 0
	GenerationObject<PreferencesEventProcessors, 0> myPrefEventProcessors;  // Gen 0
	GenerationObject<ReleaseLogger,              0> myLogger;               // Gen 0
	GenerationObject<HtmlHelpProvider,           0> myHtmlHelp;             // Gen 0
	GenerationObject<IconManager,                0> myIconManager;          // Gen 0
	GenerationStub                                  myGen0;

	PropPageFactories                 myPropPageFactories;        // Gen 1
	MenuBuilderList                   myMenuBuilders;             // Gen 1
	ContextMenuExtensions             myContextMenuExtensions;    // Gen 1
	ApproachLocalizationManager       myLocManager;               // Gen 1
	LocalizationEventProcessors       myProcessors;               // Gen 1
	ItemFactoryMap                    myItemFactories;            // Gen 1
	ItemListMap                       myItemLists;                // Gen 1
	ShellItemContextMenuExtensionImpl myShellExt;                 // Gen 1
	GenerationStub                    myGen1;

	IpcModule                       * myIpcModule;                // Gen 2

	GenerationShellItemFactory<3>     mySFF;                      // Gen 3
	RootWindow                      * myRootWindow;               // Gen 3
	UxManager                       * myUxManager;                // Gen 3
	StandardPropPages               * myStandardPropPages;        // Gen 3

	ShellIconCacheImplNew           * myIconCache;                // Gen 4
	ItemContextMenuHandlerDefault   * myItemCtxMenuHandler;       // Gen 4
	HotspotManager                  * myHSManager;                // Gen 4
	MenuBuilder_Shell               * myMenuBuilderDefault;       // Gen 4
	ShellItemGenericIconManager     * myShellItemGenericIconMan;  // Gen 4



// Constructors, destructor

public:

	//! Creates an instance of the class.
	Application();

	//! Destroys an instance of the class and releases any resources used.
	~Application();

public:
	void Initialize();


// Preferences-related methods
public:
	const ApplicationSettings * Prefs() const
		{ return *myPrefs; }

	ApplicationSettings * Prefs()
		{ return *myPrefs; }

	bool LoadPrefs();

	bool SavePrefs();

	bool SetPrefs(const ApplicationSettings & thePrefs);

	bool ResetPrefs();

	void AddPreferencesEventProcessor(IPreferencesEventProcessor * theProcessor);

	void RemovePreferencesEventProcessor(IPreferencesEventProcessor * theProcessor);

	void ReportPrefEvent(IPreferencesEventProcessor::PreferencesEvent theEvent);



// Item Lists and Item Factories
public:
	int GetItemListCount();

	void GetItemListData(int theIndex, ObjectData * theData);

	IItemList * GetItemList(REFGUID theListID);

	void RegisterItemList(REFGUID theListID, IItemList * theList, HMODULE theModule);

	void UnregisterItemList(REFGUID theListID, IItemList * theList);

	int GetItemFactoryCount();

	void GetItemFactoryData(int theIndex, ObjectData * theData);

	IItemFactory * GetItemFactory(REFGUID theFactID);

	void RegisterItemFactory(REFGUID theFactID, IItemFactory * theFact, HMODULE theModule);


// Property page handling methods
public:
	void RegisterPropPageFactory(IPropPageFactory * theFactory);

	void UnregisterPropPageFactory(IPropPageFactory * theFactory);

	//FLAW
	template<typename T>
	void EnumPropPageFactories (T & theEnumerator) const
	{
		for (int i = 0; i < myPropPageFactories.GetSize(); i++)
			theEnumerator( myPropPageFactories[i] );
	}

	//FLAW
	void AddKoApproachItemsRoot(const TCHAR * theRoot);


// Localization Manager
public:
	ApproachLocalizationManager * GetLocalizationManager()
		{ return &myLocManager; }

	const ApproachLocalizationManager * GetLocalizationManager()  const
		{ return &myLocManager; }


	void RegisterLocalizationEventProcessor(ILocalizationEventProcessor * theProcessor);

	void UnregisterLocalizationEventProcessor(ILocalizationEventProcessor * theProcessor);

	void RaiseLocalizationChanged(ILocalization * theLoc, const TCHAR * theLocale, int theSize_Locale);




// Context Menu Extensions
public:
	int GetNumContextMenuExtensions() const
		{ return myContextMenuExtensions.GetSize(); }

	const ContextMenuExtensionData & GetContextMenuExtension(int theIndex) const
		{ return myContextMenuExtensions[theIndex]; }

	void RegisterContextMenuExtension(IContextMenuExtension * theExt, HMODULE theModule);

	void UnregisterContextMenuExtension(IContextMenuExtension * theExt);

	void ReleaseContextMenuExtensions(HMODULE theModule);


// Hotspot Manager
public:
	HotspotManager * GetHotspotManager();


public:
	IItemContextMenuHandler * GetItemContextMenuHandler() const;


public:
	ReleaseLogger * GetReleaseLogger() { return *myLogger; }


public:
	IIconCache * GetIconCache();

	IShellItemFactory * GetShellItemFactory();

public:
	IMenuBuilder * GetMenuBuilder(Item * theTarget, UINT & theOutExpansionCode);

	void RegisterMenuBuilder(IMenuBuilder * theBuilder, HMODULE theModule);

	void UnregisterMenuBuilder(IMenuBuilder * theBuilder);

	void ReleaseMenuBuilders(HMODULE theModule);


public:
	HWND GetRootWindow() const;

	RootWindow * GetRootWindowInstance();

	const RootWindow * GetRootWindowInstance() const;


public:
	const HtmlHelpProvider * GetHtmlHelpProvider() const
	{
		return *myHtmlHelp;
	}


// IPC Module
public:

	//! Returns a pointer to the IPC module.
	//! \return  A valid pointer to the #IpcModule class.
	//!          The pointer returned by this function <em>must not</em> be deleted
	//!          by callers. It is deleted upon application exit.
	IpcModule * GetIpcModule() const;


public:
	UxManager * GetUxManager() const;


public:
	const FilePathProvider * GetFilePathProvider() const
		{ return *myFilePathPrv; }

	const ShellItemGenericIconManager * GetShellItemGenericIconManager() const;
	ShellItemGenericIconManager * GetShellItemGenericIconManager();

	IconManager & GetIconManager()
		{ return &myIconManager; }


private:
	void UpdateShellItemFactory();



// Singleton pattern
public:
	//! Returns a non-const reference to the only object of this class.
	//! \return A non-const reference to the only object of this class.
	static Application & Instance();

	//! Returns a const reference to the only object of this class.
	//! \return A const reference to the only object of this class.
	static const Application & InstanceC();


private:
	static Application * ourInstance;	//!< Stores a pointer to the only class instance.

#ifdef _DEBUG
	static int ourGeneration;
#endif
};



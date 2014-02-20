#pragma once

#include "sdk_Plugin.h"

#include "Prefs.h"
#include "Trace.h"

//////////////////////////////////////////////////////////////////////////////////////////////

extern int CurMenuDepthLevel;

interface IMenuBuilder;
class IPluginManager;
class IShellItemFactory;
class Item;

//////////////////////////////////////////////////////////////////////////////////////////////

class Framework
{
	class IMenuBuilderStorage
	{
	public:
		virtual ~IMenuBuilderStorage() {}
	public:
		virtual IMenuBuilder * GetMenuBuilder(Item * theTarget, UINT & theOutExpansionCode) = 0;
	};

public:

	static void InitializeNew();

	static void UnIninitialize();

	static IMenuBuilder * GetMenuBuilder(Item * theTarget, UINT & theOutExpansionCode);

	static IMenuBuilder * GetMenuBuilder_Act(Item *, UINT &);

	static class IHotspotEnabler * GetHotspotEnabler()
		{ return HotspotEnabler; }

	static IPluginManager * GetPluginManager()
		{ return PluginManager; }


	static void * GetMenuBuilderStorage() { return &MenuBuilderStorage; }


//static data
private:
	static IMenuBuilderStorage *      MenuBuilderStorage;
	static IPluginManager *           PluginManager;
	static IHotspotEnabler *          HotspotEnabler;

	////////////////////////////////////////////////////////////////////////////////////////////

	class MenuBuilderStorageImpl_Unlimited : public IMenuBuilderStorage
	{
	public:
		MenuBuilderStorageImpl_Unlimited()
		{
			ReleaseTraceCode(FEATURE_MENUBUILDERS_UNLOCKED);
		}

	protected:
		virtual IMenuBuilder * GetMenuBuilder(Item * theTarget, UINT & theOutExpansionCode)
		{
			return Framework::GetMenuBuilder_Act(theTarget, theOutExpansionCode);
		}
	};
};


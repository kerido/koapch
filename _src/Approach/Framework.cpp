#include "stdafx.h"

#include "MenuBuilder_Shell.h"
#include "Framework.h"
#include "MenuManager.h"
#include "PluginManager.h"
#include "HotspotNew.h"
#include "Application.h"
#include "Preferences.h"

//////////////////////////////////////////////////////////////////////////////////////////////

Framework::IMenuBuilderStorage *  Framework::MenuBuilderStorage = 0;
IPluginManager *                  Framework::PluginManager = 0;
IHotspotEnabler *                 Framework::HotspotEnabler = 0;

//////////////////////////////////////////////////////////////////////////////////////////////

void Framework::InitializeNew()
{
	ReleaseTraceCode(FRAMEWORK_INITIALIZE);
	Trace("Framework::Initialize\n");

	OleInitialize(NULL);
	InitCommonControls();

	// 1. menu builder
	MenuBuilderStorage = new MenuBuilderStorageImpl_Unlimited;

	// 2. plugin manager
	PluginManager = new PluginManagerImpl_Unlimited;

	// 3. hotspot enabler
	HotspotEnabler = new HotspotEnablerImpl_Registered;
}

//////////////////////////////////////////////////////////////////////////////////////////////

void Framework::UnIninitialize()
{
	PluginManager->ReleasePlugins();

	delete MenuBuilderStorage;
	delete PluginManager;
	delete HotspotEnabler;

	OleUninitialize();

	ReleaseTraceCode(FRAMEWORK_UNINITIALIZE);
}

//////////////////////////////////////////////////////////////////////////////////////////////

IMenuBuilder * Framework::GetMenuBuilder(Item * theTarget, UINT & theOutExpansionCode)
{
	return MenuBuilderStorage->GetMenuBuilder(theTarget, theOutExpansionCode);
}

//////////////////////////////////////////////////////////////////////////////////////////////

IMenuBuilder * Framework::GetMenuBuilder_Act(Item * theTarget, UINT & theOutExpansionCode)
{
	return Application::Instance().GetMenuBuilder(theTarget, theOutExpansionCode);
}
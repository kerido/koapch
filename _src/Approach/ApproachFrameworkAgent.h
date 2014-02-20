#pragma once

#include "ApiAgent.h"
#include "AppModule.h"

//////////////////////////////////////////////////////////////////////////////////////////////

class FrameworkAgentImpl_Approach :
	public IFrameworkAgent,
	public IPreferencesEventProcessor,
	public IFrameworkAgent2
{
public:
	FrameworkAgentImpl_Approach()
	{
		Application::Instance().AddPreferencesEventProcessor(this);
	}

	virtual ~FrameworkAgentImpl_Approach()
	{
		Application::Instance().RemovePreferencesEventProcessor(this);
	}


//IFrameworkAgent members
protected:
	virtual void RegisterMenuBuilder(class IMenuBuilder * theBuilder)
		{ Framework::RegisterMenuBuilder(theBuilder); }

	virtual void UnregisterMenuBuilder(class IMenuBuilder * theBuilder)
		{ Framework::UnregisterMenuBuilder(theBuilder); }

	virtual ILocalizationManager * GetLocalizationManager()
	{ return Application::Instance().GetLocalizationManager(); }


//IFrameworkAgent2 members
protected:
	virtual void RegisterContextMenuExtension(IContextMenuExtension * theExtension, HMODULE theModule)
		{ Application::Instance().RegisterContextMenuExtension(theExtension, theModule); }

	virtual void UnregisterContextMenuExtension(IContextMenuExtension * theExtension)
		{ Application::Instance().UnregisterContextMenuExtension(theExtension); }


//IPreferencesEventProcessor members
protected:
	virtual void OnPreferencesEvent(PreferencesEvent theEvent, ApplicationSettings & thePrefs)
	{
		Framework::UpdateShellItemFactory();
	}

//ILogicQueryTarget members
public:
	virtual bool RequestLogic(LogicID theID, void ** theOut, void * theParam = 0)
	{
		if (theID == LOGIC_FRAMEWORKAGENT2)
		{
			*theOut = (IFrameworkAgent2 *) this;
			return true;
		}
		else
			return false;
	}

	virtual void ReleaseLogic(LogicID theID, ILogic * theLogic)
	{
	}
};
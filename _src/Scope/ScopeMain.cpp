#include "Common.h"

#include "ScopePlugin.h"
#include "Trace.h"

//////////////////////////////////////////////////////////////////////////////////////////////

ComInstance<ScopePlugin> * gPlugin = NULL;

//////////////////////////////////////////////////////////////////////////////////////////////

IApproachPlugin * GetKOApproachPlugin() { return gPlugin; }

//////////////////////////////////////////////////////////////////////////////////////////////

BOOL WINAPI DllMain(HINSTANCE theDllInstance, DWORD theReason, void *)
{
	switch(theReason)
	{
	case DLL_PROCESS_ATTACH:
		gPlugin = new ComInstance<ScopePlugin>(theDllInstance);
		break;

	case DLL_PROCESS_DETACH:
		delete gPlugin;
		break;
	}

	return TRUE;
}
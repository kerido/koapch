#include "Common.h"

#include "InstantWavePlugin.h"
#include "Trace.h"

//////////////////////////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG
#  define EntryPoint DllMain
#else
#  define EntryPoint DllMainCRTStartup
#endif //_DEBUG

ComInstance<InstantWavePlugin> * gPlugin = NULL;

//////////////////////////////////////////////////////////////////////////////////////////////

IApproachPlugin * GetKOApproachPlugin() { return gPlugin; }

//////////////////////////////////////////////////////////////////////////////////////////////

BOOL WINAPI EntryPoint(HINSTANCE theDllInstance, DWORD theReason, void *)
{
	switch(theReason)
	{
	case DLL_PROCESS_ATTACH:
		gPlugin = new ComInstance<InstantWavePlugin>(theDllInstance);
		break;

	case DLL_PROCESS_DETACH:
		delete gPlugin;
		break;
	}

	return TRUE;
}

#include "Common.h"

#include <Windows.h>
#include <Psapi.h>
#include <tchar.h>

#include "Trace.h"

#ifdef _USE_FOLDERMENUS_USAGE_TRACING


#  pragma comment(lib, "psapi.lib")
//#  pragma comment(lib, "msvcrt.lib")

//////////////////////////////////////////////////////////////////////////////////////////////

void ProcessRunningProcess(DWORD theProcID)
{
	HANDLE aHndl = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, theProcID);

	if (aHndl == NULL)
	{
		Trace("Cannot open process handle\n");
		return;
	}

	Trace("Process handle open, PID=%d\n", theProcID);

	HMODULE aModules[1000];

	DWORD aSize = 0;
	BOOL aRes = EnumProcessModules(aHndl, aModules, sizeof aModules, &aSize);

	if (aRes == 0)
	{
		CloseHandle(aHndl);
		Trace("Cannot enumerate modules\n");
		return;
	}

	aSize  /= sizeof DWORD;

	for (DWORD i = 0; i != aSize; i++)
	{
		TCHAR a[1000];

		DWORD aModFNSize = GetModuleFileNameEx(aHndl, aModules[i], a, 1000);

		TCHAR  * aOccur = _tcsstr(a, _T("FolderMenus.dll") );

		if (aOccur != 0)
		{
			Trace ("Dll found\n");
		}
	}
	
	CloseHandle(aHndl);
}

//////////////////////////////////////////////////////////////////////////////////////////////

void DisplayUsage()
{

	DWORD aProcessIds[1000];
	DWORD aSize;

	BOOL aRet = EnumProcesses(aProcessIds, sizeof aProcessIds, &aSize);

	if ( aRet == FALSE)
	{
		Trace("unable to enumerate processes\n");
		return;
	}


	aSize /= sizeof DWORD;

	for (DWORD i = 0; i < aSize; i++)
		ProcessRunningProcess(aProcessIds[i]);
}

#endif //_USE_FOLDERMENUS_USAGE_TRACING
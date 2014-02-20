#include "Common.h"

#include <Sddl.h>

#include "ApproachNsis.h"
#include "IpcGuids.h"
#include "IpcRootWindow.h"
#include "RegKeyPermissionModifier.h"

//////////////////////////////////////////////////////////////////////////////////////////////

typedef BOOL (WINAPI * LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);

//////////////////////////////////////////////////////////////////////////////////////////////

extern "C"
BOOL __stdcall DllMainCRTStartup(HINSTANCE hinstDLL, DWORD theReason, void *)
	{ return TRUE; }

//////////////////////////////////////////////////////////////////////////////////////////////

bool prvIsApproachRunning()
{
	HWND aWnd = IpcRootWindow::FindSingleInstance();

	return aWnd != 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////

bool IsWin64()
{
	HMODULE aKernel32 = GetModuleHandle( _T("kernel32") );
	LPFN_ISWOW64PROCESS fnIsWow64Process = (LPFN_ISWOW64PROCESS) GetProcAddress(aKernel32, "IsWow64Process");

	if(fnIsWow64Process == NULL)
		return false;

	BOOL aRetVal = FALSE;
	fnIsWow64Process(GetCurrentProcess(), &aRetVal);

	return aRetVal != FALSE;
}

//////////////////////////////////////////////////////////////////////////////////////////////

void IsVersionOK(HWND, int theStringSize, char * theVars, NSIS::Stack ** theStackTop)
{
	bool aIsVersionOK;

	OSVERSIONINFO aInfo;
	aInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx( &aInfo );

	aIsVersionOK = (aInfo.dwMajorVersion >= 5);

	//TODO: separate checks for x86 and x64

	NSIS::Push(theStackTop, aIsVersionOK ? "1" : "0", theStringSize);
}

//////////////////////////////////////////////////////////////////////////////////////////////

void DlgRunningPrepare(HWND, int theStringSize, char *, NSIS::Stack ** theStackTop)
{
	const USHORT aIconField = 2;
	const USHORT aIconCtlID = 1200 + aIconField - 1;

	char * aTextHwnd = (*theStackTop)->Text;
	HWND aWnd = (HWND) NSIS::DwordFromString_Dec(aTextHwnd);

	NSIS::Pop(theStackTop);

	HICON aIcn = LoadIcon(NULL, MAKEINTRESOURCE(IDI_WARNING) );
	SendDlgItemMessage(aWnd, aIconCtlID, STM_SETICON, (WPARAM)aIcn, 0);

	MessageBeep(MB_ICONEXCLAMATION);
}

//////////////////////////////////////////////////////////////////////////////////////////////

void QuitProgram(HWND, int theStringSize, char *, NSIS::Stack ** theStackTop)
{
	bool aIsRunning = prvIsApproachRunning();

	if (aIsRunning)
		aIsRunning = !IpcRootWindow::Quit();

	NSIS::Push(theStackTop, aIsRunning ? "0" : "1", theStringSize);
}

//////////////////////////////////////////////////////////////////////////////////////////////

void UpdateRegPermissions(HWND, int, char *, NSIS::Stack ** theStackTop)
{
	REGSAM aAccess = IsWin64() ? KEY_WOW64_64KEY|KEY_ALL_ACCESS : KEY_ALL_ACCESS;

	HKEY aKey;
	LONG aRes = RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("Software\\KO Software\\Approach"),
		0, aAccess, &aKey);

	if (aRes != ERROR_SUCCESS)
		return;

	PSID aSid = 0;
	ConvertStringSidToSid( _T("S-1-5-32-545"), &aSid);

	if (aSid != 0)
	{
		RegKeyPermissionModifier aMod;
		aMod.AddAccessRights(aKey, aSid, KEY_ALL_ACCESS);

		LocalFree(aSid);
	}

	RegCloseKey(aKey);
}
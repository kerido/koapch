#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////

class UtilShellNotify
{
public:
	struct SHNOTIFYSTRUCT
	{
		DWORD Item1;    //previous PIDL or name of the folder. 
		DWORD Item2;    //new PIDL or name of the folder. 
	};


public:
	static ULONG ChangeNotifyRegister(LPCITEMIDLIST thePidl, HWND theWnd, UINT theMessage, bool theRecursive);

	static void OutputChangeNotifyEvent(DWORD theCode, const SHNOTIFYSTRUCT * thePidls, char * theOutInfo);

	static bool CommonCanIgnoreEventByCode(DWORD theCode)
	{
		return (  theCode ==  SHCNE_FREESPACE        ||
		          theCode ==  SHCNE_DISKEVENTS       ||
		          theCode ==  SHCNE_GLOBALEVENTS     ||
		          theCode ==  SHCNE_INTERRUPT        ||
		          theCode ==  SHCNE_EXTENDED_EVENT   ||
		         (theCode  &  SHCNE_UPDATEITEM) != 0 ||
		          theCode == (SHCNE_GLOBALEVENTS|SHCNE_FREESPACE) );
	}
};
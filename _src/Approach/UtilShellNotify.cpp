#include "stdafx.h"

#include "UtilShellNotify.h"

//////////////////////////////////////////////////////////////////////////////////////////////

ULONG UtilShellNotify::ChangeNotifyRegister(LPCITEMIDLIST thePidl, HWND theWnd, UINT theMessage, bool theRecursive)
{
	SHChangeNotifyEntry aEntry;

	aEntry.fRecursive = theRecursive;
	aEntry.pidl       = thePidl;

	LONG aFlags = 0x0002L;

	if (theRecursive)
		aFlags |= 0x0001L|0x1000L;

	return SHChangeNotifyRegister(theWnd, aFlags, SHCNE_ALLEVENTS, theMessage, 1, &aEntry);
}

//////////////////////////////////////////////////////////////////////////////////////////////

void UtilShellNotify::OutputChangeNotifyEvent(
	DWORD theCode, const SHNOTIFYSTRUCT * thePidls, char * theOutInfo)
{
	theOutInfo[0] = 0;

#  define CheckAttrib( _Buf, _Attr, _Val ) if (_Attr & _Val) strcat( _Buf, #_Val "|" )


	if (theCode == SHCNE_ALLEVENTS)
		strcat(theOutInfo, "SHCNE_ALLEVENTS");

	else
	{
		CheckAttrib(theOutInfo,theCode,SHCNE_ASSOCCHANGED);
		CheckAttrib(theOutInfo,theCode,SHCNE_ATTRIBUTES);
		CheckAttrib(theOutInfo,theCode,SHCNE_CREATE);
		CheckAttrib(theOutInfo,theCode,SHCNE_DELETE);
		CheckAttrib(theOutInfo,theCode,SHCNE_DRIVEADD);
		CheckAttrib(theOutInfo,theCode,SHCNE_DRIVEADDGUI);
		CheckAttrib(theOutInfo,theCode,SHCNE_DRIVEREMOVED);
		CheckAttrib(theOutInfo,theCode,SHCNE_EXTENDED_EVENT);
		CheckAttrib(theOutInfo,theCode,SHCNE_MEDIAINSERTED);
		CheckAttrib(theOutInfo,theCode,SHCNE_MEDIAREMOVED);
		CheckAttrib(theOutInfo,theCode,SHCNE_MKDIR);
		CheckAttrib(theOutInfo,theCode,SHCNE_NETSHARE);
		CheckAttrib(theOutInfo,theCode,SHCNE_NETUNSHARE);
		CheckAttrib(theOutInfo,theCode,SHCNE_RENAMEFOLDER);
		CheckAttrib(theOutInfo,theCode,SHCNE_RENAMEITEM);
		CheckAttrib(theOutInfo,theCode,SHCNE_RMDIR);
		CheckAttrib(theOutInfo,theCode,SHCNE_SERVERDISCONNECT);
		CheckAttrib(theOutInfo,theCode,SHCNE_UPDATEDIR);
		CheckAttrib(theOutInfo,theCode,SHCNE_UPDATEIMAGE);
		CheckAttrib(theOutInfo,theCode,SHCNE_UPDATEITEM);
		CheckAttrib(theOutInfo,theCode,SHCNE_FREESPACE);
		CheckAttrib(theOutInfo,theCode,SHCNE_DISKEVENTS);
		CheckAttrib(theOutInfo,theCode,SHCNE_GLOBALEVENTS);
		CheckAttrib(theOutInfo,theCode,SHCNE_INTERRUPT);
	}
}
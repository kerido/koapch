#include "stdafx.h"

#include <fstream>

#include "UtilPidlFunc.h"

/////////////////////////////////////////////////////////////////////////////

void UtilPidlFunc::PidlDump (LPITEMIDLIST thePidl)
{
	using namespace std;

	LPBYTE SingleByte = LPBYTE(thePidl);
	LPITEMIDLIST CopyPidl = thePidl;
	int cb = 0;

	ofstream aFile("pidldump.txt", ios::out);

	while (CopyPidl->mkid.cb != 0)
	{
		for (int i = 1; i <= CopyPidl->mkid.cb; i++)
		{
			aFile << *SingleByte;
			SingleByte++;
			cb++;
			aFile << " ";
		}
		CopyPidl = LPITEMIDLIST(LPBYTE(CopyPidl) + CopyPidl->mkid.cb);
	}
	SingleByte ++;
	cb++;
	aFile <<  *SingleByte << " ";
	SingleByte ++;
	cb++;
	aFile <<  *SingleByte << " ";
	aFile << dec << "\n\nsize : " << cb << "\n\n";
}

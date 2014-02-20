/*


*/

#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.h>
#include <shlobj.h>

//////////////////////////////////////////////////////////////////////////////////////////////

class UtilPidlFunc
{
public:
	template <typename T_Writer>
	static LPITEMIDLIST  Copy (LPCITEMIDLIST theSource)
	{
		return Copy<T_Writer>(theSource, PidlSize(theSource) );
	}

	template <typename T_Writer>
	static LPITEMIDLIST Copy (LPCITEMIDLIST theSource, UINT theNumBytes)
	{
		if(NULL == theSource)
			return (NULL);

		// Allocate the new pidl plus the size of the null-terminator
		LPITEMIDLIST aTarget = (LPITEMIDLIST)CoTaskMemAlloc( theNumBytes + sizeof(USHORT) );

		if(!aTarget)
			return (NULL);

		T_Writer aWr;
		void * aNew = aWr.AdvanceWrite(aTarget, theSource, theNumBytes);

		SecureZeroMemory(aNew, sizeof USHORT);

		return aTarget;
	}


	template <typename T_Writer>
	static LPITEMIDLIST PidlAppend (LPCITEMIDLIST theParentPidl, LPCITEMIDLIST theAppendPidl) throw()
	{
		if(theParentPidl == NULL)		return NULL;

		UINT cb1 = PidlSize(theParentPidl) - sizeof(theParentPidl->mkid.cb);
		UINT cb2 = PidlSize(theAppendPidl);

		LPITEMIDLIST aNew = (LPITEMIDLIST)	CoTaskMemAlloc(cb1 + cb2);

		if (aNew)
		{
			void * aPtr = aNew;

			T_Writer aWr;
			aPtr = aWr.AdvanceWrite(aPtr, theParentPidl, cb1);
			aPtr = aWr.AdvanceWrite(aPtr, theAppendPidl, cb2);
		}
		return aNew;
	}


	static LPCITEMIDLIST GetNextItemID(LPCITEMIDLIST thePidl) throw()
	{
		if (thePidl == NULL) return NULL; 	      //Check for valid pidl

		if (thePidl->mkid.cb == 0)	return NULL; 	//If the size is zero, it is the end of the list
		
		// Add cb to pidl (casting to increment by bytes)
		thePidl = (LPITEMIDLIST) (((char *) thePidl) + thePidl->mkid.cb);

		// Return NULL if it is null-terminating, or a pidl otherwise
		return (thePidl->mkid.cb == 0) ? NULL : (LPITEMIDLIST) thePidl;
	}


	static UINT PidlSize (LPCITEMIDLIST thePidl) throw()
	{
		LPCITEMIDLIST aPidl = thePidl;

		if (aPidl == 0) return 0;

		UINT aTotal = sizeof(aPidl->mkid.cb);
		
		while (aPidl)
		{
			aTotal += aPidl->mkid.cb;
			aPidl = GetNextItemID(aPidl);
		}
		return aTotal;
	}

	static bool IsZero(LPCITEMIDLIST thePidl) throw()
	{
		if (thePidl == 0)
			return true;
		else if (thePidl->mkid.cb == 0)
			return true;
		else
			return false;
	}

	BOOL IsPartOf(LPCITEMIDLIST theWhole, LPCITEMIDLIST thePart) throw()
	{
		LPCITEMIDLIST aIter = theWhole;

		while (aIter != NULL)
		{
			UINT aPartSize = PidlSize(thePart);
			if ( PidlSize (aIter) < aPartSize ) break;

			if ( 0 == memcmp(aIter, thePart, (size_t) aPartSize) ) return true;

			aIter = GetNextItemID(aIter);
		}

		return false;
	}

	//from code project
	static LPBYTE GetByPos(LPCITEMIDLIST pidl, int nPos) throw()
	{
		if (!pidl)
			return 0;
		int nCount = 0;

		BYTE * pCur = (BYTE *) pidl;
		while (((LPCITEMIDLIST) pCur)->mkid.cb)
		{
			if (nCount == nPos)
				return pCur;
			nCount++;
			pCur += ((LPCITEMIDLIST) pCur)->mkid.cb;	// + sizeof(pidl->mkid.cb);
		}
		if (nCount == nPos) 
			return pCur;
		return NULL;
	}

	static UINT Count(LPCITEMIDLIST pidl) throw()
	{
		if (!pidl)
			return 0;

		UINT nCount = 0;
		BYTE*  pCur = (BYTE *) pidl;
		while (((LPCITEMIDLIST) pCur)->mkid.cb)
		{
			nCount++;
			pCur += ((LPCITEMIDLIST) pCur)->mkid.cb;
		}
		return nCount;
	}

	static BOOL PidlsEqual(LPCITEMIDLIST thePidl1, LPCITEMIDLIST thePidl2) throw()
	{
		BOOL aRetVal = FALSE;
		LPCITEMIDLIST aIter1 = thePidl1, aIter2 = thePidl2;

		while (true)
		{
			if (aIter1 == 0 && aIter2 != 0)
				return FALSE;

			else if (aIter2 == 0 && aIter1 != 0)
				return FALSE;

			else if (aIter1 == 0 && aIter2 == 0)
				return TRUE;

			else if (aIter1->mkid.cb != aIter2->mkid.cb)
				return FALSE;

			int a = memcmp(aIter1->mkid.abID, aIter2->mkid.abID, aIter1->mkid.cb - sizeof USHORT);

			if ( a != 0 )
				return FALSE;

			aIter1 = GetNextItemID(aIter1);
			aIter2 = GetNextItemID(aIter2);
		}
	}


	static void PidlDump(LPITEMIDLIST thePidl);
};


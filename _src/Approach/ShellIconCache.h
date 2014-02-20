#pragma once

#include "sdk_CriticalSection.h"

#include "LogicCommon.h"
#include "Utility.h"

class RootWindow;

//////////////////////////////////////////////////////////////////////////////////////////////

class ShellIconCacheEntry
{
	typedef INT_PTR HashType;

private:
	HashType myHash;
	int myIconIndex;
	int myImageListIndex;
	UINT64 myLastAccessTime;


public:
	ShellIconCacheEntry(wchar_t * thePath, int theIconIndex) :
			myIconIndex(theIconIndex), myLastAccessTime(0), myImageListIndex(-1),
			myHash( StringUtil::ComputeHash(thePath) )
	{ }


public:
	UINT64 GetLastAccessTime() const         { return myLastAccessTime; }
	void   SetLastAccessTime(UINT64 theVal ) { myLastAccessTime = theVal; }

	int  GetImageListIndex() const           { return myImageListIndex;  }
	void SetImageListIndex(int theVal)       { myImageListIndex = theVal; }


//operators
public:
	bool operator == (const ShellIconCacheEntry & theY) const
	{
		return myHash == theY.myHash && myIconIndex == theY.myIconIndex;
	}

	bool operator != (const ShellIconCacheEntry & theY) const
	{
		return ! ( operator == (theY) );
	}

	bool operator > (const  ShellIconCacheEntry & theY) const
	{
		if      (myHash >  theY.myHash) return true;
		else if (myHash == theY.myHash) return myIconIndex > theY.myIconIndex;
		else                            return false;
	}

	bool operator <= (const ShellIconCacheEntry & theY) const
	{
		return ! ( operator > (theY) );
	}

	bool operator < (const ShellIconCacheEntry & theY) const
	{
		if      (myHash <  theY.myHash) return true;
		else if (myHash == theY.myHash) return myIconIndex < theY.myIconIndex;
		else                            return false;
	}

	bool operator >= (const ShellIconCacheEntry & theY) const
	{
		return ! ( operator < (theY) );
	}
};

//////////////////////////////////////////////////////////////////////////////////////////////

class ShellIconCacheImplNew :
	public IIconCache,
	public CMessageMap
{
private:
	typedef std::vector<ShellIconCacheEntry>    CacheEntryList;
	typedef CacheEntryList::iterator            CacheEntryIter;
	typedef CacheEntryList::const_iterator      CacheEntryIterC;

	static const int MaxTime = 10;                                                     //!< Maximum icon lifespan in minutes.
	static const int MaxTime_In_Milli = MaxTime * 60 * 1000;                           //!< Maximum icon lifespan in milliseconds.
	static const UINT64 MaxTime_In_Nano = (UINT64) MaxTime_In_Milli * (UINT64) 1000000;//!< Maximum icon lifespan in nanoseconds.


private:
	CacheEntryList  myEntriesArr;     //!< The array of entries that contain icon indexes within #myImageList.
	HIMAGELIST      myImageList;      //!< Image list that stores actual icons.
	CriticalSection myCS;             //!< Critical section meant to synchronize access to the object's data.
	int             myTimerID;        //!< Timer that is fired up for periodic cache cleanup.
	RootWindow    * myRootWindow;     //!< Pointer to the Approach Root Window that is subclassed to allow periodic cleanups.



public:
	ShellIconCacheImplNew();

	~ShellIconCacheImplNew();


protected:
	bool Lookup (wchar_t * thePath, int theIconIndex, ItemIconData & theOut);

	void Add (wchar_t * thePath, int theIconIndex, HICON theIcon, ItemIconData & theOut);


private:
	void Cleanup();

	void Clear();

	int BinarySearchUnsafe(ShellIconCacheEntry & theEntry);

	CacheEntryIter RemoveEntryUnsafe(CacheEntryIter theEntry);

	//! Ensures correct sort order in DEBUG mode.
	void ValidateUnsafe();

	void GetIconDataUnsafe(int thePos, ItemIconData & theOut);


// WTL Windowing
protected:
	BEGIN_MSG_MAP(ShellIconCacheImplNew)
		MESSAGE_HANDLER(WM_TIMER,         MsgHandler_Timer)
	END_MSG_MAP()


// Message Handlers
private:
	LRESULT MsgHandler_Timer(UINT theMsg, WPARAM theWParam, LPARAM theLParam, BOOL & theHandled);
};

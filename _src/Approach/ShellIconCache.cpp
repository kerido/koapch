#include "stdafx.h"

#include "sdk_IconExtractor.h"

#include "ShellIconCache.h"
#include "Application.h"
#include "RootWindow.h"
#include "Trace.h"
#include "MenuManager.h"

////////////////////////////////////////////////////////////////////////////////////////////////

ShellIconCacheImplNew::ShellIconCacheImplNew()
{
	myCS.Enter();

	Application & aApp = Application::Instance();

	myRootWindow = aApp.GetRootWindowInstance();
	myRootWindow->AddMessageMap(this);

	myTimerID = myRootWindow->GetAvailableTimerID();

	int aW = GetSystemMetrics(SM_CXSMICON);
	int aH = GetSystemMetrics(SM_CYSMICON);
	myImageList = ImageList_Create(aW, aH, ILC_COLOR32|ILC_MASK, 1, 1);

	::SetTimer(*myRootWindow, myTimerID, MaxTime_In_Milli, NULL);

	myCS.Leave();
}

////////////////////////////////////////////////////////////////////////////////////////////////

ShellIconCacheImplNew::~ShellIconCacheImplNew()
{
	myCS.Enter();

	myRootWindow->RemoveMessageMap(this);
	ImageList_Destroy(myImageList);

	myCS.Leave();
}

////////////////////////////////////////////////////////////////////////////////////////////////

bool ShellIconCacheImplNew::Lookup(wchar_t * thePath, int theIconIndex, ItemIconData & theOut)
{
	int aPos = -1;

	myCS.Enter();

	if ( !myEntriesArr.empty() )
	{
		ValidateUnsafe();

		ShellIconCacheEntry aEntry(thePath, theIconIndex);
		aPos = BinarySearchUnsafe(aEntry);

		if (aPos >= 0)
			GetIconDataUnsafe(aPos, theOut);
	}

	myCS.Leave();


	ATLTRACE
	(
		_T("ShellIconCacheImplNew: querying icon presence for %s,%d; returning %d\n"),
		thePath, theIconIndex, aPos
	);

	return aPos >= 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////

void ShellIconCacheImplNew::Add(wchar_t * thePath, int theIconIndex, HICON theIcon, ItemIconData & theOut)
{
	ShellIconCacheEntry aEntry( thePath, theIconIndex );

	myCS.Enter();

	int aPos = BinarySearchUnsafe(aEntry);

	if (aPos < 0)
	{
		aPos = ~aPos;

		int aImageIndex = ImageList_AddIcon(myImageList, theIcon);
		aEntry.SetImageListIndex(aImageIndex);

		if (aPos >= (int) myEntriesArr.size() )
			myEntriesArr.push_back(aEntry);

		else
			//FLAW: this is a slow operation for a vector
			myEntriesArr.insert( myEntriesArr.begin() + aPos, aEntry );
	}

	ValidateUnsafe();

	GetIconDataUnsafe(aPos, theOut);

	myCS.Leave();
}

////////////////////////////////////////////////////////////////////////////////////////////////

void ShellIconCacheImplNew::Cleanup()
{
	myCS.Enter();

	if ( !MenuManager::HasNonFloatingMenus() )
	{
		unsigned __int64 aFileTimeNew;
		GetSystemTimeAsFileTime( (FILETIME *) &aFileTimeNew );

		for (CacheEntryIter aIt = myEntriesArr.begin(); aIt != myEntriesArr.end(); )
		{
			__int64 aDiff = aFileTimeNew - aIt->GetLastAccessTime();

			if (aDiff > MaxTime_In_Nano)
				aIt = RemoveEntryUnsafe(aIt);
			else
				aIt++;
		}
	}

	myCS.Leave();
}

////////////////////////////////////////////////////////////////////////////////////////////////

void ShellIconCacheImplNew::Clear()
{
	myCS.Enter();

	myEntriesArr.clear();
	ImageList_RemoveAll(myImageList);

	myCS.Leave();
}

////////////////////////////////////////////////////////////////////////////////////////////////

int ShellIconCacheImplNew::BinarySearchUnsafe( ShellIconCacheEntry & theEntry )
{
	int aLow = 0;
	int aHigh = (int) myEntriesArr.size()  - 1;	//zero based array

	while( aLow <= aHigh )
	{
		int aMiddle = (aHigh + aLow) / 2;

		if      ( theEntry == myEntriesArr[aMiddle] ) //match
			return aMiddle;

		else if ( theEntry <  myEntriesArr[aMiddle] )
			aHigh = aMiddle - 1;    //search low end of array

		else
			aLow = aMiddle + 1;     //search high end of array
	}

	return ~aLow;		//search key not found, return the bitwise-complement
}

////////////////////////////////////////////////////////////////////////////////////////////////

ShellIconCacheImplNew::CacheEntryIter ShellIconCacheImplNew::RemoveEntryUnsafe( CacheEntryIter theEntry )
{
	int aImageListIndex = theEntry->GetImageListIndex();

	if (aImageListIndex < ImageList_GetImageCount(myImageList) - 1)	//last
	{
		//need to decrease the index of all items that go AFTER the one to be deleted
		for (CacheEntryIter aIt = myEntriesArr.begin(); aIt != myEntriesArr.end(); aIt++)
			if (aIt->GetImageListIndex() > aImageListIndex)
				aIt->SetImageListIndex ( aIt->GetImageListIndex() - 1 );
	}

	CacheEntryIter aRet = myEntriesArr.erase(theEntry);
	ImageList_Remove(myImageList, aImageListIndex);

	return aRet;
}

////////////////////////////////////////////////////////////////////////////////////////////////

inline void ShellIconCacheImplNew::ValidateUnsafe()
{
#ifdef _DEBUG
	if (myEntriesArr.size() > 1)
		for (CacheEntryIterC aIt = myEntriesArr.begin(), aIt1 = aIt+1; aIt1 != myEntriesArr.end(); aIt++, aIt1++)
			ATLASSERT( *aIt1 >= *aIt );
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////

void ShellIconCacheImplNew::GetIconDataUnsafe(int thePos, ItemIconData & theOut)
{
	ShellIconCacheEntry & aEntry = myEntriesArr[thePos];

	UINT64 aFileTimeNew;
	GetSystemTimeAsFileTime( (FILETIME *) &aFileTimeNew );

	aEntry.SetLastAccessTime(aFileTimeNew);

	theOut.ImageList = myImageList;
	theOut.IconIndex = aEntry.GetImageListIndex();
}

////////////////////////////////////////////////////////////////////////////////////////////////

LRESULT ShellIconCacheImplNew::MsgHandler_Timer( UINT theMsg, WPARAM theWParam, LPARAM theLParam, BOOL & theHandled )
{
	if (theWParam == myTimerID)
		Cleanup();

	else
		theHandled = FALSE;

	return 0L;
}
#pragma once

#include "sdk_Thread.h"
#include "sdk_IconExtractor.h"
#include "sdk_CriticalSection.h"

////////////////////////////////////////////////////////////////////////////////////////////////

struct IconExtractionDataBase 
{
	int                ItemID;
	DisplayItem      * Item;
	IconExtractParams  IconExtractionParams;
	void             * AsyncArg;
};

struct IconExtractionData : public IconExtractionDataBase
{
	HWND OwnerWnd;
	UINT Message;
};

////////////////////////////////////////////////////////////////////////////////////////////////

class PerWindowIconExtraction;

class IconExtractorThread : public Thread
{
private:
	volatile LONG             myStatus;
	PerWindowIconExtraction & myHost;

public:
	IconExtractorThread(PerWindowIconExtraction & theHost);

private:
	IconExtractorThread(const PerWindowIconExtraction & theSrc);	//purposely undefined

public:
	void Terminate();

protected:
	DWORD Run();

private:
	void ProcessItem(IconExtractionData & theItem);
};

////////////////////////////////////////////////////////////////////////////////////////////////

class PerWindowIconExtraction
{
private:
	friend class IconExtractorThread;

	typedef std::vector<IconExtractorThread *> ThreadDataList;
	typedef ThreadDataList::iterator           ThreadDataIter;

	typedef std::deque<IconExtractionDataBase>  IconExtractionDataList;
	typedef IconExtractionDataList::iterator    IconExtractionDataIter;


private:
	HWND                   myMenu;                     //!< Handle to the menu that owns the item for which icon(s) are to be extracted.
	UINT                   myPostMsg;                  //!< The message that will be posted to #myMenu when each icon is extracted.
	CriticalSection        myCS;                       //!< Critical section to synchronize access to #myData.
	ThreadDataList         myThreadData;               //!< Stores icon extractor threads.
	IconExtractionDataList myData;                     //!< Stores a list of individual icon extraction operation.
	volatile LONG          myNumWaitingThreads;        //!< Stores the number of worker threads that are waiting for icon extraction tasks.
	HANDLE                 myEvt_HasNewItemOrStopping; //!< Event handle for notifying waiting threads that an icon extraction task is available.
	const size_t           myMaxWorkerThreads;         //!< Stores the maximum number of worker threads (retrieved from ApplicationSettings).

public:
	PerWindowIconExtraction(HWND theMenu = 0, UINT thePostMsg = 0);
	~PerWindowIconExtraction();

private:
	PerWindowIconExtraction(const PerWindowIconExtraction & theSrc);	//purposely undefined

public:
	void QueueIconExtraction(int theItemID, DisplayItem * theItem, const IconExtractParams & theParams, void * theAsyncArg);

	void StopIconExtraction();

private:
	bool GetNextItem(IconExtractionData & theOut);

	bool WaitForWorkerThreadResumption(int theTimeout);

	static size_t GetMaxNumberOfWorkerThreads();
};

////////////////////////////////////////////////////////////////////////////////////////////////

class BackgroundIconExtractionManager
{
private:

	typedef std::map<HWND, PerWindowIconExtraction *>  IconExtractionMap;
	typedef IconExtractionMap::iterator                IconExtractionIter;
	typedef IconExtractionMap::value_type              IconExtractionPair;

private:
	IconExtractionMap myData;

public:
	//! Schedules background icon extraction for a given item.
	//! 
	//! \param [in] theOwnerWnd           Handle to the window that owns the item.
	//! \param [in] theMsg                Message that will be posted to the window when icon extraction completes.
	//! \param [in] theItemID             Item's unique identifier within the owner window
	//! \param [in] theItem               Pointer to the interface whose SetIconData method will be called.
	//! \param [in] theIconExtractParams  Initial parameters for icon extraction.
	//! \param [in] theAsyncArg           Arguments for asynchronous operation.
	void Queue(HWND theOwnerWnd, UINT theMsg, int theItemID, DisplayItem * theItem,
		const IconExtractParams & theIconExtractParams, void * theAsyncArg);


	PerWindowIconExtraction * EnsurePerWindowData(HWND theWindow, UINT theMessage);


	//! Removes all items that are scheduled for background icon extraction and belong to the specified window.
	//! 
	//! \param [in] theOwnerWnd      Handle to the window whose items should be removed.
	void DeleteItems(HWND theOwnerWnd);


	static BackgroundIconExtractionManager & Instance()
	{
		static BackgroundIconExtractionManager * ourObject = new BackgroundIconExtractionManager();
		return *ourObject;
	}
};
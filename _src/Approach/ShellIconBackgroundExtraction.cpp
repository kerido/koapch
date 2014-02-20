#include "stdafx.h"

#include "sdk_Item.h"
#include "sdk_DisplayItem.h"

#include "ShellIconBackgroundExtraction.h"
#include "Trace.h"
#include "DisplayItem.h"
#include "Application.h"

////////////////////////////////////////////////////////////////////////////////////////////////

IconExtractorThread::IconExtractorThread(PerWindowIconExtraction & theHost)
	: myHost(theHost), myStatus(0L)
{
	Thread::Run(this);
}

////////////////////////////////////////////////////////////////////////////////////////////////

DWORD IconExtractorThread::Run()
{
	CoInitializeEx(NULL, COINIT_APARTMENTTHREADED/*COINIT_MULTITHREADED*/);

	while (myStatus == 0L)
	{
		IconExtractionData aDt;
		bool aEndOfList = myHost.GetNextItem(aDt);

		if (aEndOfList)
		{
			if ( !myHost.WaitForWorkerThreadResumption(200) )
				break;
			else
				continue;	// we will have to first check if we're not instructed to terminate and only then repeat icon processing
		}

		ProcessItem(aDt);
	}

	CoUninitialize();

	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////

void IconExtractorThread::Terminate()
{
	InterlockedExchange(&myStatus, 1L);
}

////////////////////////////////////////////////////////////////////////////////////////////////

void IconExtractorThread::ProcessItem(IconExtractionData & theItem)
{
	try
	{
		CComQIPtr<IIconAcceptor> aAcc(theItem.Item);

		if (aAcc != 0)
		{
			HRESULT aRes = aAcc->RetrieveIconDataAsync(&theItem.IconExtractionParams, theItem.AsyncArg);
			PostMessage(theItem.OwnerWnd, theItem.Message, theItem.ItemID, aRes);

			aRes = aAcc->ReleaseAsyncArg(theItem.AsyncArg);
		}
	}
	catch (...)
	{
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////

PerWindowIconExtraction::PerWindowIconExtraction( HWND theMenu /*= 0*/, UINT thePostMsg /*= 0*/ )
	: myMenu(theMenu), myPostMsg(thePostMsg), myNumWaitingThreads(0L), myMaxWorkerThreads( GetMaxNumberOfWorkerThreads() )
{
	myEvt_HasNewItemOrStopping = CreateEvent(NULL, TRUE, FALSE, NULL);
}

////////////////////////////////////////////////////////////////////////////////////////////////

PerWindowIconExtraction::~PerWindowIconExtraction()
{
	for (ThreadDataIter aIt = myThreadData.begin(); aIt != myThreadData.end(); aIt++)
		delete *aIt;

	for (IconExtractionDataIter aIt = myData.begin(); aIt != myData.end(); aIt++)
	{
		CComQIPtr<IIconAcceptor> aAcc(aIt->Item);

		if (aAcc != 0)
			aAcc->ReleaseAsyncArg(aIt->AsyncArg);
	}

	CloseHandle(myEvt_HasNewItemOrStopping);
}

////////////////////////////////////////////////////////////////////////////////////////////////

void PerWindowIconExtraction::QueueIconExtraction
	(
		int theItemID, DisplayItem * theItem,
		const IconExtractParams & theParams, void * theAsyncArg
	)
{
	myCS.Enter();                // prohibit list modification

	bool aNeedToCreateNewThread = myThreadData.empty() || !myData.empty() && myThreadData.size() <= myMaxWorkerThreads;

	IconExtractionDataBase aDt;
	aDt.ItemID = theItemID;
	aDt.Item = theItem;
	aDt.IconExtractionParams = theParams;
	aDt.IconExtractionParams.Flags &= ~EXTRACT_PREFERFAST;
	aDt.AsyncArg = theAsyncArg;

	myData.push_back(aDt);

	SetEvent(myEvt_HasNewItemOrStopping);

	if (aNeedToCreateNewThread)
	{
		if (myNumWaitingThreads == 0L)
		{
			myThreadData.push_back( new IconExtractorThread(*this) );
			Trace("PerWindowIconExtraction: the number of worker threads: %d\n", myThreadData.size() );
		}
		// otherwise, waiting threads will resume as a result
		// of myAvailableItems being set to the signaled state.
	}

	myCS.Leave();                // permit list modification
}

////////////////////////////////////////////////////////////////////////////////////////////////

void PerWindowIconExtraction::StopIconExtraction()
{
	HANDLE aAllThreads[16];	//sufficient enough
	int aCurActiveThreads = 0;


	for (ThreadDataIter aIt = myThreadData.begin(); aIt != myThreadData.end(); aIt++)
	{
		IconExtractorThread * aT = *aIt;

		if ( !aT->IsRunning() )
			continue;

		aAllThreads[aCurActiveThreads++] = aT->GetHandle();

		aT->Terminate();	// send a signal to the thread that it must quit. The thread may be waiting so we'll wake it up below.
	}

	SetEvent(myEvt_HasNewItemOrStopping);	// prevent worker threads from waiting

	WaitForMultipleObjects(aCurActiveThreads, aAllThreads, TRUE, INFINITE);
}

////////////////////////////////////////////////////////////////////////////////////////////////

bool PerWindowIconExtraction::GetNextItem( IconExtractionData & theOut )
{
	myCS.Enter();                // prohibit list modification

	bool aEndOfList = false;

	try
	{
		IconExtractionDataIter aIt = myData.begin();

		aEndOfList = ( aIt == myData.end() );

		if (!aEndOfList)
		{
			theOut.ItemID = aIt->ItemID;
			theOut.Item = aIt->Item;
			theOut.IconExtractionParams = aIt->IconExtractionParams;
			theOut.AsyncArg = aIt->AsyncArg;
			theOut.OwnerWnd = myMenu;
			theOut.Message = myPostMsg;

			myData.erase(aIt);
		}
		else
			ResetEvent(myEvt_HasNewItemOrStopping);
	}
	catch(...)
	{ }

	myCS.Leave();                // permit list modification

	return aEndOfList;
}

////////////////////////////////////////////////////////////////////////////////////////////////

bool PerWindowIconExtraction::WaitForWorkerThreadResumption( int theTimeout )
{
	InterlockedIncrement(&myNumWaitingThreads);
	Trace("PerWindowIconExtraction: %d threads waiting for new items to arrive...\n", myNumWaitingThreads);
	DWORD aRes = WaitForSingleObject(myEvt_HasNewItemOrStopping, theTimeout);
	InterlockedDecrement(&myNumWaitingThreads);

	return aRes == WAIT_OBJECT_0;
}

////////////////////////////////////////////////////////////////////////////////////////////////

size_t PerWindowIconExtraction::GetMaxNumberOfWorkerThreads()
{
	const Application & aApp = Application::InstanceC();
	const ApplicationSettings * aSt = aApp.Prefs();

	return (size_t) aSt->GetIconFetchParallelization();
}

////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////

void BackgroundIconExtractionManager::Queue
	(
		HWND theOwnerWnd, UINT theMsg, int theItemID, DisplayItem * theItem,
		const IconExtractParams & theIconExtractParams, void * theAsyncArg
	)
{
	PerWindowIconExtraction * aData = EnsurePerWindowData(theOwnerWnd, theMsg);
	aData->QueueIconExtraction(theItemID, theItem, theIconExtractParams, theAsyncArg);
}

////////////////////////////////////////////////////////////////////////////////////////////////

PerWindowIconExtraction * BackgroundIconExtractionManager::EnsurePerWindowData(HWND theWindow, UINT theMessage)
{
	IconExtractionIter aIt = myData.find(theWindow);

	if ( aIt == myData.end() )
	{
		PerWindowIconExtraction * aObj = new PerWindowIconExtraction(theWindow, theMessage);
		aIt = myData.insert(aIt, IconExtractionPair(theWindow, aObj));
	}

	return aIt->second;
}

////////////////////////////////////////////////////////////////////////////////////////////////

void BackgroundIconExtractionManager::DeleteItems( HWND theOwnerWnd )
{
	IconExtractionIter aIt = myData.find(theOwnerWnd);

	if ( aIt == myData.end() )
		return;

	aIt->second->StopIconExtraction();
	delete aIt->second;

	myData.erase(aIt);
}

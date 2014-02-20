#pragma once


//
class DECLSPEC_NOVTABLE Thread
{
protected:
	HANDLE myHandle;
	DWORD myThreadID;

public:
	Thread() : myHandle(NULL), myThreadID(0) {}
	virtual ~Thread()
	{
		CloseHandle(myHandle);
	}

public:
	virtual void Terminate() = 0;
	virtual DWORD Run() = 0;

	bool IsRunning() const
	{
		DWORD aRes = 0;
		GetExitCodeThread(myHandle, &aRes);

		return aRes == STILL_ACTIVE;
	}

	HANDLE GetHandle() const
		{ return myHandle; }

	DWORD GetID() const
		{ return myThreadID; }

public:
	static void Run( Thread * theThread, bool theSuspended = false )
	{
		if (!theSuspended)	//try to resume if possible
			if ( theThread->IsRunning() )
				{ ResumeThread(theThread->myHandle); return; }

		DWORD aCreationFlags = theSuspended ? CREATE_SUSPENDED : 0;

		if (theThread->myHandle != 0)
			CloseHandle(theThread->myHandle);

		theThread->myHandle = CreateThread(NULL, 0, ThreadProcDefault,
			(void *) theThread, aCreationFlags, &theThread->myThreadID);
	}

	static DWORD __stdcall ThreadProcDefault(void * theParam)
	{
		Thread * aThread = (Thread *) theParam;
		DWORD aRes = aThread->Run();

		return aRes;
	}
};
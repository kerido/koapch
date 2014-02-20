#include "stdafx.h"

#include "Logging.h"
#include "Application.h"
#include "Framework.h"
#include "Utility.h"
#include "Trace.h"

//////////////////////////////////////////////////////////////////////////////////////////////

#ifdef _USE_RELEASE_TRACE

void ReleaseTraceCode(int theReleaseTraceCode)
{
	ReleaseLogger * aLog = Application::Instance().GetReleaseLogger();
	aLog->LogCode(theReleaseTraceCode);
}

void ReleaseTraceBytes(void * theBytes, int theNumBytes)
{
	ReleaseLogger * aLog = Application::Instance().GetReleaseLogger();
	aLog->LogBytes(theBytes, theNumBytes);
}

#endif

//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

ReleaseLogger::ReleaseLogger() : myReleaseTraceCodes(0)
{

}

//////////////////////////////////////////////////////////////////////////////////////////////

ReleaseLogger::~ReleaseLogger()
{
	if (myReleaseTraceCodes != 0)
	{
#ifdef _USE_RELEASE_TRACE
		FinalizeReleaseTrace();
#endif

		delete myReleaseTraceCodes;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////

void ReleaseLogger::InitReleaseTrace()
{
#ifdef _USE_RELEASE_TRACE
	myReleaseTraceCodes = new ATL::CSimpleArray<ReleaseTraceEntry>();
#endif
}

//////////////////////////////////////////////////////////////////////////////////////////////

void ReleaseLogger::FinalizeReleaseTrace()
{
	const FilePathProvider * aFPP = Application::InstanceC().GetFilePathProvider();

	TCHAR aPath[MAX_PATH];
	int aLength = MAX_PATH;
	HRESULT aRes = aFPP->GetPath(FilePathProvider::Dir_Main, aPath, &aLength);

	lstrcat(aPath, _T("ApproachTrace.txt") );

	HANDLE aFile = CreateFile
	(
		aPath,
		GENERIC_WRITE,
		0,
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);

	if (aFile == INVALID_HANDLE_VALUE)
		return;

	DWORD aNumWritten = 0;

	for (int i = 0; i < myReleaseTraceCodes->GetSize(); i++)
	{
		const ReleaseTraceEntry & aEntry = (*myReleaseTraceCodes)[i];

		if (aEntry.Type == CODE)
		{
			char a[20];
			sprintf( a, "%d\r\n", aEntry.Data.DataAsCode );

			WriteFile(aFile, a, lstrlenA(a), &aNumWritten, NULL);
		}
		else if (aEntry.Type == BYTES)
		{
			char b[128];	//because DataAsBytes is only 60 chars,
			              //128 will be enough (60*2 = 120,
			              //plus 1 for the minus, plus zero terminator, 122 totally)

			b[0] = '-';

			int aLen = (int) aEntry.Data.DataAsBytes.SizeBytes;

			StringUtil::ByteToStringA
			(
				(const char *)aEntry.Data.DataAsBytes.Bytes,
				b+1,
				aLen
			);

			WriteFile(aFile, b,      aLen*2+1, &aNumWritten, NULL);
			WriteFile(aFile, "\r\n", 2,      &aNumWritten, NULL);
		}
	}

	CloseHandle(aFile);
}

//////////////////////////////////////////////////////////////////////////////////////////////

void ReleaseLogger::LogCode(int theReleaseTraceCode)
{
	if (myReleaseTraceCodes != 0)
		myReleaseTraceCodes->Add( ReleaseTraceEntry(theReleaseTraceCode) );
}

//////////////////////////////////////////////////////////////////////////////////////////////

void ReleaseLogger::LogBytes(void * theBytes, int theNumBytes)
{
	if (myReleaseTraceCodes != 0)
		myReleaseTraceCodes->Add
		(
			ReleaseTraceEntry
			(
				(unsigned char *) theBytes,
				__min(theNumBytes, 60)
			)
		);
}
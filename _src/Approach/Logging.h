#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////

class ReleaseLogger
{
private:
	enum ReleaseTraceEntryType
	{
		CODE,
		BYTES
		//TODO: add string
	};

	struct ReleaseTraceEntry
	{
		SYSTEMTIME            Time;
		ReleaseTraceEntryType Type;
		union
		{
			int   DataAsCode;
			struct ByteData
			{
				size_t        SizeBytes;
				unsigned char Bytes[60];
			} DataAsBytes;
		} Data;


		ReleaseTraceEntry(int theCode) : Type(CODE)
		{
			GetSystemTime(&Time);

			Data.DataAsCode = theCode;
		}

		ReleaseTraceEntry(unsigned char * theBytes, size_t theSize) : Type(BYTES)
		{
			GetSystemTime(&Time);

			Data.DataAsBytes.SizeBytes = theSize;
			CopyMemory(Data.DataAsBytes.Bytes, theBytes, __min(theSize, 60) );
		}
	};


private:
	ATL::CSimpleArray<ReleaseTraceEntry> * myReleaseTraceCodes;


// Constructors, Destructor
public:
	ReleaseLogger();
	~ReleaseLogger();


// Interface
public:

	//! Initialized the release trace mechanism which results various program
	//! events written to the ApproachTrace.txt file.
	void InitReleaseTrace();


	//! Writes an integer code to the release trace.
	//! \param theReleaseTraceCode    The status code to be written.
	void LogCode(int theReleaseTraceCode);


	//! Writes an array of bytes to the release trace.
	//! \param theBytes     The array of bytes to be written.
	//! \param theNumBytes  The number of bytes in \a theBytes.
	void LogBytes(void * theBytes, int theNumBytes);


// Implementation Details
private:
	void FinalizeReleaseTrace();
};

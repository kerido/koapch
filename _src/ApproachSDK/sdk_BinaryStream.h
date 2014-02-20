#pragma once

#define NOMINMAX

#include <tchar.h>
#include <Windows.h>

//////////////////////////////////////////////////////////////////////////////////////////////

template <typename tByte>
class CrtMemMgr
{
public:
	typedef tByte byte;
	const static size_t bytesize = sizeof byte;

public:
	static byte * Alloc(size_t theNumElements)
		{ return reinterpret_cast<byte *>( malloc(theNumElements * bytesize) ); }

	static byte * Realloc(void * theBuf, size_t theNumElements)
		{ return reinterpret_cast<byte *>( realloc(theBuf, theNumElements * bytesize) ); }

	static void Free(byte * theBuf)
		{ return free(theBuf); }

	static void Copy(byte * theDest, const byte * theSrc, size_t theNumElements)
		{ memcpy(theDest, theSrc, theNumElements * bytesize); }
};

//////////////////////////////////////////////////////////////////////////////////////////////

template <typename tByte>
class NoCrtMemMgr
{
public:
	typedef tByte byte;
	const static size_t bytesize = sizeof byte;

public:
	static byte * Alloc(size_t & theNumElements)
	{
		HANDLE aHeap = GetProcessHeap();

		LPVOID aPtr = HeapAlloc(aHeap, 0, theNumElements * bytesize);

		size_t aSize = HeapSize(aHeap, 0, aPtr);
		theNumElements = aSize / bytesize;

		return reinterpret_cast<byte *>(aPtr);
	}

	static byte * Realloc(void * theBuf, size_t & theNumElements)
	{
		HANDLE aHeap = GetProcessHeap();

		LPVOID aPtr = HeapReAlloc(aHeap, 0, theBuf, theNumElements * bytesize);

		size_t aSize = HeapSize(aHeap, 0, aPtr);
		theNumElements = aSize / bytesize;

		return reinterpret_cast<byte *>(aPtr);
	}

	static void Free(byte * theBuf)
		{ HeapFree(GetProcessHeap(), 0, theBuf); }

	static void Copy(byte * theDest, const byte * theSrc, size_t theNumElements)
	{
		for (size_t i = 0; i < theNumElements; i++)
			theDest[i] = theSrc[i];
	}
};

//////////////////////////////////////////////////////////////////////////////////////////////

template <typename tByte, typename tMemMgr = CrtMemMgr<tByte> >
class MemoryStream
{
public:
	typedef tByte byte;

private:
	byte * myBuf;
	size_t mySize;
	size_t myCapacity;
	size_t myReadIndex;

public:
	MemoryStream(size_t theInitialCapacity = 16)
		: myCapacity(theInitialCapacity), mySize(0), myReadIndex(0)
	{
		myBuf = tMemMgr::Alloc(myCapacity);
	}

	~MemoryStream()
	{
		tMemMgr::Free(myBuf);
	}

private:
	MemoryStream(const MemoryStream & theSrc);


public:
	const byte * GetBuffer() const
		{ return myBuf; }

	const size_t GetSize() const
		{ return mySize; }

	byte * EnsureWritableBuffer(size_t theCapacity)
	{
		if (myCapacity < theCapacity)
			Grow(theCapacity);

		mySize = theCapacity;
		return myBuf;
	}

	void Write(const byte * theSrc, size_t theOffset, size_t theSize)
	{
		size_t aNewSize = mySize + theSize;
		if (myCapacity < aNewSize)
		{
			size_t aCapacity = myCapacity * 2;
			while (aCapacity < aNewSize)
				aCapacity *= 2;

			Grow(aCapacity);
		}

		tMemMgr::Copy(&myBuf[mySize], &theSrc[theOffset], theSize);
		mySize += theSize;
	}

	size_t Read(byte * theDest, size_t theOffset, size_t theSize)
	{
		size_t aNumToCopy = mySize - myReadIndex;
		
		if (aNumToCopy > theSize)
			aNumToCopy = theSize;

		if (aNumToCopy != 0)
		{
			tMemMgr::Copy(&theDest[theOffset], &myBuf[myReadIndex], aNumToCopy);
			myReadIndex += aNumToCopy;
		}
		return aNumToCopy;
	}

	// TEMP
	void write(const byte * theSrc, size_t theSize)
	{
		Write(theSrc, 0, theSize);
	}

	size_t read(byte * theDest, size_t theSize)
	{
		return Read(theDest, 0, theSize);
	}

	void seekg(size_t thePos)
	{
		myReadIndex = thePos;
	}
	//end TEMP


private:
	void Grow()
	{
		Grow(myCapacity * 2);
	}

	void Grow(size_t theCapacity)
	{
		myBuf = tMemMgr::Realloc(myBuf, myCapacity = theCapacity);
	}
};

//////////////////////////////////////////////////////////////////////////////////////////////

template <typename tByte, typename tMemMgr = CrtMemMgr<tByte> >
class StreamWriter
{
private:
	MemoryStream<tByte, tMemMgr> & myStream;

public:
	StreamWriter(MemoryStream<tByte, tMemMgr> & theStream) : myStream(theStream)
	{ }

public:
	template<typename tBasic>
	void WriteBasic(const tBasic & theType)
	{
		myStream.Write( reinterpret_cast<const MemoryStream::byte *>(&theType), 0, sizeof tBasic);
	}

	void WriteString(const TCHAR * theString)
	{
		int aLength = lstrlen(theString);
		WriteBasic(aLength);

		myStream.Write( reinterpret_cast<const MemoryStream::byte *>(theString), 0, aLength * sizeof TCHAR);
	}
};

//////////////////////////////////////////////////////////////////////////////////////////////

template <typename tByte, typename tMemMgr = CrtMemMgr<tByte> >
class StreamReader
{
private:
	MemoryStream<tByte, tMemMgr> & myStream;

public:
	StreamReader(MemoryStream<tByte, tMemMgr> & theStream) : myStream(theStream)
	{ }

public:
	template<typename tBasic>
	void ReadBasic(tBasic & theType)
	{
		myStream.Read( reinterpret_cast<MemoryStream::byte *>(&theType), 0, sizeof tBasic);
	}

	void ReadString(TCHAR * theString)
	{
		int aLength;
		ReadBasic(aLength);

		if (aLength != 0)
			myStream.Read( reinterpret_cast<MemoryStream::byte *>(theString), 0, aLength * sizeof TCHAR);

		theString[aLength] = 0;
	}
};
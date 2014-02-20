#pragma once

class MemoryWriterInterface
{
public:
	void * AdvanceWrite(void * theDest, const void * theSrc, size_t theNumBytes)
	{
		// create your class with the same body
		return 0;
	}
};


class MemoryWriter_NoCrt
{
public:
	void * AdvanceWrite(void * theDest, const void * theSrc, size_t theNumBytes)
	{
		char * aDest = (char *)theDest;
		char * aSrc = (char *) theSrc;

		for (size_t i = 0; i < theNumBytes; i++)
			aDest[i] = aSrc[i];

		return aDest + theNumBytes;
	}
};


class MemoryWriter_Crt
{
public:
	void * AdvanceWrite(void * theDest, const void * theSrc, size_t theNumBytes)
	{
		memcpy(theDest, theSrc, theNumBytes);

		return (char *) theDest + theNumBytes;
	}
};



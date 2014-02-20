#pragma once


class StringUtil
{
public:
	static void ByteToCharsA(char theChar, char * theOut)
	{
		// KO: check this. because the char is signed, it might be unsafe
		// to bitwise-AND with 0xf0 (-128 in decimal)
		// KO: check successful
		

		char a = (theChar & 0xf0) >> 4;
		char * aOut = theOut;

		if (a > 9)
			* aOut = 'a' + a - 10;
		else
			* aOut = '0' + a;

		a = theChar & 0x0f;
		aOut++;

		if (a > 9)
			* aOut = 'a' + a - 10;
		else
			* aOut = '0' + a;
	}


	static void ByteToCharsW(char theChar, wchar_t * theOut)
	{
		char a = (theChar & 0xf0) >> 4;
		wchar_t * aOut = theOut;

		if (a > 9)
			* aOut = L'a' + a - 10;
		else
			* aOut = L'0' + a;

		a = theChar & 0x0f;
		aOut++;

		if (a > 9)
			* aOut = L'a' + a - 10;
		else
			* aOut = L'0' + a;
	}

	//Function to convert string of unsigned chars to string of chars
	static void ByteToStringA(const char * theByteArr, char * theOut, int theByteArrSize)
	{
		for (int i = 0; i < theByteArrSize; i++)
			ByteToCharsA( theByteArr[i], &theOut[2*i] );

		theOut[theByteArrSize*2] = 0;
	}

	static void ByteToStringW(const char * theByteArr, wchar_t * theOut, int theByteArrSize)
	{
		for (int i = 0; i < theByteArrSize; i++)
			ByteToCharsW( theByteArr[i], &theOut[2*i] );

		theOut[theByteArrSize*2] = 0;
	}

#ifdef _UNICODE

#  define ByteToString ByteToStringW
#  define ByteToChars  ByteToCharsW

#else

#  define ByteToString ByteToStringA
#  define ByteToChars  ByteToCharsA

#endif

	static bool StringToBytes(const char * theString, int theSize, char * theOut)
	{
		for(int i = 0; i < theSize/2; i++)
		{
			char a = theString[i*2];

			if(a >= '0' && a <= '9')
				theOut[i] = (a-'0')*16;

			else if(a >= 'a' && a <= 'f')
				theOut[i] = (a-'a'+10)*16;

			else if(a >= 'A' && a <= 'F')
				theOut[i] = (a-'A'+10)*16;

			else
				return false;


			a = theString[i*2+1];

			if(a >= '0' && a <= '9')
				theOut[i] += a-'0';

			else if(a >= 'a' && a <= 'f')
				theOut[i] += a-'a'+10;

			else if(a >= 'A' && a <= 'F')
				theOut[i] += a-'A'+10;

			else return false;
		}

		return true;
	}

	static DWORD BytesToDword(const char * theRawBytes)
	{
		DWORD a = theRawBytes[0];
		DWORD b = theRawBytes[1];
		DWORD c = theRawBytes[2];
		DWORD d = theRawBytes[3];

		return (a<<24) | (b<<16) | (c<<8) | (d<<0);
	}


	static bool GuidToString(const GUID * theGuid, TCHAR * theOut, int theOutSize)
	{
		if ( theOutSize < 37 )
			return false;

		TCHAR * aCur = theOut;

		ByteToChars( (char) ( (theGuid->Data1 & 0xff000000) >> 24 ), aCur);	aCur += 2;
		ByteToChars( (char) ( (theGuid->Data1 & 0x00ff0000) >> 16 ), aCur);	aCur += 2;
		ByteToChars( (char) ( (theGuid->Data1 & 0x0000ff00) >>  8 ), aCur);	aCur += 2;
		ByteToChars( (char) ( (theGuid->Data1 & 0x000000ff) >>  0 ), aCur);	aCur += 2;

		*aCur++ = _T('-');

		ByteToChars( (char) ( (theGuid->Data2 & 0xff00) >> 8 ), aCur);	aCur += 2;
		ByteToChars( (char) ( (theGuid->Data2 & 0x00ff) >> 0 ), aCur);	aCur += 2;

		*aCur++ = _T('-');

		ByteToChars( (char) ( (theGuid->Data3 & 0xff00) >> 8 ), aCur);	aCur += 2;
		ByteToChars( (char) ( (theGuid->Data3 & 0x00ff) >> 0 ), aCur);	aCur += 2;

		*aCur++ = _T('-');

		ByteToChars( theGuid->Data4[0], aCur);	aCur += 2;
		ByteToChars( theGuid->Data4[1], aCur);	aCur += 2;

		*aCur++ = _T('-');

		ByteToChars( theGuid->Data4[2], aCur);	aCur += 2;
		ByteToChars( theGuid->Data4[3], aCur);	aCur += 2;
		ByteToChars( theGuid->Data4[4], aCur);	aCur += 2;
		ByteToChars( theGuid->Data4[5], aCur);	aCur += 2;
		ByteToChars( theGuid->Data4[6], aCur);	aCur += 2;
		ByteToChars( theGuid->Data4[7], aCur);	aCur += 2;

		*aCur = 0;
		return true;
	}

	static int CompareCaseInsensitive(const TCHAR * theStr1, const TCHAR * theStr2)
	{
		return CompareStringW(LOCALE_INVARIANT, NORM_IGNORECASE, theStr1, -1, theStr2, -1);
	}

	template<typename tChar>
	static int ComputeHash(const tChar * theStr, int theLength = -1)
	{
		//taken from a sample
		const static int b = 378551;

		int a = 63689;
		int aRetVal = 0;

		size_t aLen = (size_t) theLength;

		for(size_t aInd = 0; aInd < aLen; aInd++)
		{
			wchar_t aChar = theStr[aInd];

			if (aChar == 0)
				break;

			aRetVal = aRetVal * a + (int) CharLowerW( (LPTSTR)aChar );
			a *= b;
		}

		aRetVal &= 0x7FFFFFFF;
		return aRetVal;
	}
};

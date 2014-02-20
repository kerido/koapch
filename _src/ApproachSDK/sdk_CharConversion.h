#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////

template<typename tTraits, int tSize>
struct CharConversionBuf
{
	const static int BufSize = tSize;
	tTraits Buf[BufSize];

	CharConversionBuf() { Buf[0] = 0; }

	operator const tTraits * () const { return Buf; }
	operator tTraits * ()             { return Buf; }
};

//////////////////////////////////////////////////////////////////////////////////////////////

template<int tSize>
class CharConversionTraitsA
{
public:
	typedef char          Char;
	typedef char *        AsciiTrans;
	typedef CharConversionBuf<wchar_t, tSize> WideTrans;

	static void CharToAscii(Char * theOrig, int, AsciiTrans & theOut, int)
	{
		theOut = theOrig;
	}

	static void CharToWide(Char * theOrig, int theLengthOrig, WideTrans & theOut, int theLengthOut)
	{
		if (theLengthOrig == -1)
			theLengthOrig = lstrlenA(theOrig);

		theLengthOrig++;

		theLengthOut = __min(theLengthOut, theLengthOrig);
		MultiByteToWideChar(CP_ACP, 0, theOrig, theLengthOrig, theOut, theLengthOut);
	}
};

//////////////////////////////////////////////////////////////////////////////////////////////

template<int tSize>
class CharConversionTraitsW
{
public:
	typedef wchar_t       Char;
	typedef wchar_t *     WideTrans;
	typedef CharConversionBuf<char, tSize> AsciiTrans;

	static void CharToAscii(Char * theOrig, int theLengthOrig, AsciiTrans & theOut, int theLengthOut)
	{
		if (theLengthOrig == -1)
			theLengthOrig = lstrlenW(theOrig);

		theLengthOrig++;

		theLengthOut = __min(theLengthOut, theLengthOrig);

		WideCharToMultiByte(CP_ACP, 0, theOrig, theLengthOrig, theOut, theLengthOut, NULL, NULL);
	}

	static void CharToWide(Char * theOrig, int, WideTrans & theOut, int)
	{
		theOut = theOrig;
	}
};

#ifdef _UNICODE

template<int tSize>
class CharConversionTraitsT : public CharConversionTraitsW<tSize>
{
public:
	typedef WideTrans TTrans;

	static void CharToT(Char * theOrig, int theLengthOrig, TTrans & theOut, int theLengthOut)
	{ return CharToWide(theOrig, theLengthOrig, theOut, theLengthOut); }
};

#else

template<int tSize>
class CharConversionTraitsT : public CharConversionTraitsA<tSize>
{
public:
	typedef AsciiTrans TTrans;

	static void CharToT(Char * theOrig, int theLengthOrig, TTrans & theOut, int theLengthOut)
	{ return CharToAscii(theOrig, theLengthOrig, theOut, theLengthOut); }
};

#endif

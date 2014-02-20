#pragma once

#include <Windows.h>


class DECLSPEC_NOVTABLE NSIS
{
public:
	struct Stack
	{
		Stack * Next;
		char    Text[1]; // this should be the length of string_size
	};

public:
	static DWORD DwordFromString_Dec(const char * theStr)
	{
		DWORD aRetVal = 0;

		for (const char * i = theStr; *i != 0 && i != theStr+8; i++)
		{
			aRetVal *= 10;
			aRetVal += (DWORD)( *i - '0');
		}

		return aRetVal;
	}


public:
	static void __stdcall Pop(Stack **& theStackTop)
	{
		if (theStackTop != 0 && *theStackTop != 0)
		{
			Stack * aStack = *theStackTop;
			*theStackTop = aStack->Next;

			GlobalFree( (HGLOBAL)aStack );
		}
	}

	static void __stdcall Push(Stack **& theStackTop, char * theText, int theStringSize)
	{
		if (theStackTop != 0)
		{
			Stack * aStack = (Stack *) GlobalAlloc(GPTR, sizeof (Stack) + theStringSize);
			lstrcpyn(aStack->Text, theText, theStringSize);

			aStack->Next = *theStackTop;
			*theStackTop = aStack;
		}
	}
};

#define NSISFUNCTYPE extern "C" void __declspec(dllexport) 

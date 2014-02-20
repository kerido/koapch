#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////

template<typename T>
class ProductVersion
{
public:
	enum VersionType
	{
		VERSION_CURRENT
	};

protected:
	T myW1, myW2, myW3, myW4;

public:
	ProductVersion(T theMaj = T(), T theMin = T(), T theRel = T(), T theRev = T())
		: myW1(theMaj), myW2(theMin), myW3(theRel), myW4(theRev) { }

	ProductVersion(const ProductVersion<T> & theV)
		: myW1(theV.myW1), myW2(theV.myW2), myW3(theV.myW3), myW4(theV.myW4) { }

	ProductVersion(VersionType theType, HMODULE theModule = 0)
	{
		Zero();

		if (theType == VERSION_CURRENT)
			GetProgramVersion(theModule);
	}


public:
	void Set(const ProductVersion<T> & theV)
	{
		myW1 = theV.myW1;
		myW2 = theV.myW2;
		myW3 = theV.myW3;
		myW4 = theV.myW4;
	}

	void Set(T theMaj, T theMin, T theRel = T(), T theRev = T() )
	{
		myW1 = theMaj;
		myW2 = theMin;
		myW3 = theRel;
		myW4 = theRev;
	}

	void Set(const TCHAR * theString, size_t theNumChars)
	{
		Zero();

		T * aTemp[4] = { &myW1, &myW2, &myW3, &myW4 };
		int aTempIndex = 0; // the index of the current position in aTemp;
		bool aIsSeparator = false;

		for(size_t i = 0; i < theNumChars; i++)
		{
			TCHAR aIt = theString[i];

			if (aIt >= '0' && aIt <= '9')
			{
				*(aTemp[aTempIndex]) *= 10;
				*(aTemp[aTempIndex]) += static_cast<T>(aIt - '0');

				aIsSeparator = false;
			}
			else if (!aIsSeparator)
			{
				aTempIndex++;

				aIsSeparator = true;
			}
		}
	}

	T GetMajor()    const { return myW1; }
	T GetMinor()    const { return myW2; }
	T GetRelease()  const { return myW3; }
	T GetRevision() const { return myW4; }

	void Zero()
	{
		Set( T(), T(), T(), T() );
	}

protected:
	bool GetProgramVersion(HMODULE theModule)
	{
		TCHAR aModuleFileName[MAX_PATH];
		int aLength = GetModuleFileName(theModule, aModuleFileName, MAX_PATH);

		DWORD aHandle = 0;
		DWORD aSize = GetFileVersionInfoSize(aModuleFileName, &aHandle);

		if (aSize == 0)
			return false;

		char * aVersionBuf = new char[aSize];

		BOOL aVerRes = GetFileVersionInfo(aModuleFileName, aHandle, aSize, aVersionBuf);

		if (aVerRes == 0)
		{
			delete [] aVersionBuf;
			return false;
		}

		VS_FIXEDFILEINFO * aVersionInfo = 0;

		aVerRes = VerQueryValue(aVersionBuf, _T("\\"), (void **) &aVersionInfo, (LPUINT) &aSize);

		if (aVerRes == 0)
		{
			delete [] aVersionBuf;
			return false;
		}

		myW1 = HIWORD(aVersionInfo->dwProductVersionMS);
		myW2 = LOWORD(aVersionInfo->dwProductVersionMS);
		myW3 = HIWORD(aVersionInfo->dwProductVersionLS);
		myW4 = LOWORD(aVersionInfo->dwProductVersionLS);

		delete [] aVersionBuf;

		return true;
	}


// Operators
public:

	bool operator == (const ProductVersion<T> & theV1) const
	{
		return
			myW1 == theV1.myW1 &&
			myW2 == theV1.myW2 &&
			myW3 == theV1.myW3 &&
			myW4 == theV1.myW4;
	}

	bool operator != (const ProductVersion<T> & theV1) const
	{
		return ! operator == (theV1);
	}

	bool operator > (const ProductVersion<T> & theV1) const
	{
		if (myW1 > theV1.myW1)
			return true;
		else if  (myW1 < theV1.myW1)
			return false;

		else if (myW2 > theV1.myW2)
			return true;
		else if  (myW2 < theV1.myW2)
			return false;

		else if (myW3 > theV1.myW3)
			return true;
		else if  (myW3 < theV1.myW3)
			return false;

		else return (myW4 > theV1.myW4);
	}

	bool operator >= (const ProductVersion<T> & theV1) const
	{
		if (myW1 > theV1.myW1)
			return true;
		else if  (myW1 < theV1.myW1)
			return false;

		else if (myW2 > theV1.myW2)
			return true;
		else if  (myW2 < theV1.myW2)
			return false;

		else if (myW3 > theV1.myW3)
			return true;
		else if  (myW3 < theV1.myW3)
			return false;

		else return (myW4 >= theV1.myW4);
	}

	bool operator < (const ProductVersion<T> & theV1) const
	{
		return ! operator >= (theV1);
	}

	bool operator<= (const ProductVersion<T> & theV1) const
	{
		return ! operator > (theV1);
	}

	ProductVersion & operator = (const ProductVersion<T> & theV)
	{
		Set(theV);
		return *this;
	}
};
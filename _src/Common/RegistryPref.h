#pragma once


template <typename tDataType = LONG, DWORD tRegType = REG_DWORD, DWORD tHive = HKLM>
class RegPref
{
public:
	RegPref() : myValue( tDataType () ) {}

	RegPref(const RegPref<tDataDype, tHive> & theVal)
		: myValue(theVal.myValue) {}

	RegPref(const tDataType & theVal)
		: myValue(theVal) {}

public:
	bool Load(HKEY theKey, const TCHAR * theValName)
	{
		DWORD aSize = sizeof(tDataType);
		DWORD aType = 0;

		typename tDataType aVal;

		LONG aRes = RegQueryValueEx(theKey, theValName, 0, &aType, (LPBYTE) &aVal, &aSize);

		if ( aRes != ERROR_SUCCESS )
			return false;

		if ( aSize != sizeof(tDataType) )
			return false;

		if ( aType != tRegType)
			return false;

		myValue = aVal;
		return true;

	}

	bool Save(HKEY theKey, const TCHAR * theKeyName) const
	{
		LONG aRes = RegSetValueEx( theKey, theKeyName, 0, tDataType, (const LPBYTE) &myValue, sizeof(tDataType) );

		return (aRes == ERROR_SUCCESS);
	}

public:
	static DWORD GetHive() { return tHive; }


	tDataType myValue;

	operator tDataType () { return myValue; }

};
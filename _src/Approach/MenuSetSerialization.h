#pragma once

#include "MenuSet.h"

//////////////////////////////////////////////////////////////////////////////////////////////

class MenuSetSerializer
{
public:
	template <typename tStream>
	static void Save(const MenuSet & theSet, tStream & theOut)
	{
		// 1. Version
		int aVersion = 1;
		WriteBasic(theOut, aVersion);

		// 2. Num Items
		int aNumEntries = theSet.GetEntryCount();
		WriteBasic(theOut, aNumEntries);

		for(int i = 0; i < aNumEntries; i++)
		{
			const MenuSetEntry & aIt = theSet.GetEntry(i);

			// 2.1 Type
			int aEntryType = (int) aIt.GetType();
			WriteBasic(theOut, aEntryType);

			if (aEntryType == MenuSetEntry::SEPARATOR)
				continue;

			// 2.2. Object ID
			WriteBasic(theOut, aIt.GetObjectID() );

			// 2.3. IconIndex
			WriteBasic(theOut, aIt.GetIconIndex() );

			// 2.4. Name
			WriteString(theOut, aIt.GetName() );

			// 2.5. Param Size
			int aParamSize = aIt.GetParamSize();
			WriteBasic(theOut, aParamSize);

			// 2.6. Param Data
			if (aParamSize != 0)
				theOut.write( reinterpret_cast<const char *>( aIt.GetParam() ), aParamSize );
		}
	}
	template <typename tStream>
	static void Load(MenuSet & theSet, tStream & theIn)
	{
		// 1. Version
		int aVersion;
		ReadBasic(theIn, aVersion);

		if (aVersion != 1)
			return;

		// 2. Num Items
		int aNumEntries;
		ReadBasic(theIn, aNumEntries);

		for(int i = 0; i < aNumEntries; i++)
		{
			MenuSetEntry aIt;

			// 2.1 Type
			int aEntryType;
			ReadBasic(theIn, aEntryType);
			aIt.SetType( (MenuSetEntry::EntryType) aEntryType);

			if (aEntryType != MenuSetEntry::SEPARATOR)
			{
				// 2.2. Object ID
				GUID aObjID;
				ReadBasic(theIn, aObjID);
				aIt.SetObjectID(aObjID);

				// 2.3. Icon Index
				int aIconIndex;
				ReadBasic(theIn, aIconIndex);
				aIt.SetIconIndex(aIconIndex);

				// 2.4. Name
				TCHAR aName[256];
				ReadString(theIn, aName);
				aIt.SetName(aName);

				// 2.5. Param Size
				int aParamSize;
				ReadBasic(theIn, aParamSize);

				// 2.6. Param Data
				if (aParamSize != 0)
				{
					BYTE * aParam = new BYTE[aParamSize];

					theIn.read( reinterpret_cast<char *>(aParam), aParamSize );
					aIt.SetParam(aParam, aParamSize);

					delete [] aParam;
				}
			}

			theSet.AddEntry(aIt);
		}
	}

private:
	template<typename tStream, typename tBasic>
	static void WriteBasic(tStream & theOut, const tBasic & theType)
	{
		theOut.write( reinterpret_cast<const char *>(&theType), sizeof tBasic);
	}

	template<typename tStream, typename tBasic>
	static void ReadBasic(tStream & theIn, tBasic & theType)
	{
		theIn.read( reinterpret_cast<char *>(&theType), sizeof tBasic);
	}

	template<typename tStream>
	static void WriteString(tStream & theOut, const TCHAR * theString)
	{
		int aLength = lstrlen(theString);
		WriteBasic(theOut, aLength);

		theOut.write( reinterpret_cast<const char *>(theString), aLength * sizeof TCHAR);
	}

	template<typename tStream>
	static void ReadString(tStream & theIn, TCHAR * theString)
	{
		int aLength;
		ReadBasic(theIn, aLength);

		if (aLength != 0)
			theIn.read( reinterpret_cast<char *>(theString), aLength * sizeof TCHAR);

		theString[aLength] = 0;
	}
};
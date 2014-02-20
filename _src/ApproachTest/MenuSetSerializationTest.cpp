#include "stdafx.h"

#include "sdk_BinaryStream.h"

#include "IpcGuids.h"
#include "MenuSet.h"
#include "MenuSetSerialization.h"

//////////////////////////////////////////////////////////////////////////////////////////////

using namespace System;

using namespace Microsoft::VisualStudio::TestTools;
using namespace Microsoft::VisualStudio::TestTools::UnitTesting;

//////////////////////////////////////////////////////////////////////////////////////////////

namespace ApproachTest
{
	[TestClass]
	public ref class MenuSetSerializationTest
	{
	public:

		[TestMethod]
		void Test_SerializeDeserialize()
		{
			MenuSet aOld;

			//////////////////////////////////////////////////////////////
			// 1. Approach Items from previous version
			MenuSetEntry aEntry1(MenuSetEntry::LIST, GUID_Hotspot_ApproachItems);
			aEntry1.SetName( _T("Entry 1") );
			aOld.AddEntry(aEntry1);


			//////////////////////////////////////////////////////////////
			// 2. Separator line
			MenuSetEntry aEntry2(MenuSetEntry::SEPARATOR);
			aOld.AddEntry(aEntry2);


			//////////////////////////////////////////////////////////////
			// 3. Run
			MenuSetEntry aEntry3(MenuSetEntry::ITEM, OBJ_RunItem);
			aEntry3.SetName( _T("Entry 3") );
			aOld.AddEntry(aEntry3);

			MemoryStream<char> aTemp;
			MenuSetSerializer::Save(aOld, aTemp);

			aTemp.seekg(0);

			MenuSet aNew;
			MenuSetSerializer::Load(aNew, aTemp);


			int aN = aOld.GetEntryCount();
			Assert::AreEqual( aN, aNew.GetEntryCount(), "The number of entries before and after serialization must be equal." );

			for(int i = 0; i < aN; i++)
			{
				bool aEntriesEq = ( aOld.GetEntry(i) == aNew.GetEntry(i) );

				Assert::IsTrue(aEntriesEq, String::Format("Entry data ({0}) before and after serialization must be equal.", i) );
			}
		}

		IMPLEMENT_TEST_CLASS;
	};
}

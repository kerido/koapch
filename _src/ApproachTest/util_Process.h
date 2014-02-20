#pragma once

using namespace System::Reflection;
using namespace System::Runtime::InteropServices;
using namespace System::Diagnostics;

class ProcessUtility
{
public:
	static HMODULE GetCurProcessModule()
	{
		Assembly^ aAs = Assembly::GetExecutingAssembly();
		Process ^ aP = Process::GetCurrentProcess();

		HMODULE aMod = 0;

		for each (ProcessModule ^ aIt in aP->Modules)
			if (aAs->Location == aIt->FileName)
			{
				aMod = (HMODULE) aIt->BaseAddress.ToInt64();
				break;
			}

		return aMod;
	}

};
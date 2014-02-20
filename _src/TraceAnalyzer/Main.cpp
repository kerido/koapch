#define _USE_RELEASE_TRACE

#include "Trace.h"

#include <iostream>
#include <fstream>
#include <map>

using namespace std;

typedef map<int, string> TraceCodeMap;
typedef TraceCodeMap::const_iterator TraceCodeIterC;

//////////////////////////////////////////////////////////////////////////////////////////////

#define FillTraceCode( _Code, _Collection) _Collection[_Code] = #_Code

int main(int theNumParams, char ** theCmdLine)
{
	//***PREPARE THE MAP***//
	TraceCodeMap aMap;

	FillTraceCode(PROGRAM_LAUNCHED, aMap);
	FillTraceCode(PROGRAM_QUIT, aMap);

	FillTraceCode(FRAMEWORK_INITIALIZE, aMap);
	FillTraceCode(FRAMEWORK_UNINITIALIZE, aMap);
	
	FillTraceCode(IPCMODULE_LOAD_SUCCESS, aMap);
	FillTraceCode(IPCMODULE_LOAD_FAIL, aMap);
	FillTraceCode(IPCMODULE_UNLOAD, aMap);

	FillTraceCode(CREATED_SHELLITEM_FACTORY, aMap);

	FillTraceCode(HOTSPOTMANAGER_REGISTERCLASS_SUCCESS, aMap);
	FillTraceCode(HOTSPOTMANAGER_REGISTERCLASS_FAIL, aMap);

	FillTraceCode(MENUMANAGER_INIT, aMap);
	FillTraceCode(MENUMANAGER_DISPOSE, aMap);

	FillTraceCode(MENUMANAGER_CREATEWND, aMap);
	FillTraceCode(MENUMANAGER_DESTROYWND, aMap);

	FillTraceCode(MENUWINDOW_CONSTRUCTOR, aMap);
	FillTraceCode(MENUWINDOW_DESTRUCTOR, aMap);

	FillTraceCode(PLUGIN_LOADED, aMap);
	FillTraceCode(PLUGIN_UNLOADED, aMap);

	FillTraceCode(REGISTER_KEYFILE_DOESNOTEXIST, aMap);
	FillTraceCode(REGISTER_KEYFILE_EXISTS, aMap);

	FillTraceCode(REGISTER_ACQUIRING_MACADDR, aMap);
	FillTraceCode(REGISTER_ACQUIRING_PROCID, aMap);
	FillTraceCode(REGISTER_ACQUIRING_MEMORY, aMap);
	FillTraceCode(REGISTER_ACQUIRING_HDRIVE, aMap);

	FillTraceCode(REGISTER_HARDWAREID_BEGIN, aMap);
	FillTraceCode(REGISTER_HARDWAREID_SUCCESS, aMap);
	FillTraceCode(REGISTER_HARDWAREID_FAIL, aMap);

	FillTraceCode(REGISTER_KEYIMPLCREATE_SUCCESS, aMap);
	FillTraceCode(REGISTER_KEYIMPLCREATE_FAIL, aMap);

	FillTraceCode(REGISTER_KEYDECRYPT_SUCCESS, aMap);
	FillTraceCode(REGISTER_KEYDECRYPT_FAIL, aMap);

	FillTraceCode(REGISTER_KEYEVALUATE_SUCCESS, aMap);
	FillTraceCode(REGISTER_KEYEVALUATE_FAIL, aMap);
	FillTraceCode(REGISTER_KEYEXECUTE_EXCEPTION, aMap);

	FillTraceCode(REGISTER_MD5HARDWAREID_BEGINPRINT, aMap);
	FillTraceCode(REGISTER_MD5HARDWAREID_ENDPRINT, aMap);

	FillTraceCode(REGISTER_KEYEXECUTE_SUCCESS, aMap);
	FillTraceCode(REGISTER_KEYEXECUTE_FAIL, aMap);

	FillTraceCode(REGVM_INSTRUCTION_ZERO_R, aMap);

	FillTraceCode(REGVM_INSTRUCTION_INCREMENT_1, aMap);
	FillTraceCode(REGVM_INSTRUCTION_INCREMENT_N, aMap);
	FillTraceCode(REGVM_INSTRUCTION_INCREMENT_R, aMap);

	FillTraceCode(REGVM_INSTRUCTION_DECREMENT_1, aMap);
	FillTraceCode(REGVM_INSTRUCTION_DECREMENT_N, aMap);
	FillTraceCode(REGVM_INSTRUCTION_DECREMENT_R, aMap);

	FillTraceCode(REGVM_INSTRUCTION_MULTIPLY_R, aMap);
	FillTraceCode(REGVM_INSTRUCTION_MULTIPLY_L, aMap);

	FillTraceCode(REGVM_INSTRUCTION_DIVIDE_REG, aMap);
	FillTraceCode(REGVM_INSTRUCTION_DIVIDE_LOC, aMap);

	FillTraceCode(REGVM_INSTRUCTION_LEFTSHIFT_N, aMap);
	FillTraceCode(REGVM_INSTRUCTION_LEFTSHIFT_R, aMap);

	FillTraceCode(REGVM_INSTRUCTION_RIGHTSHIFT_N, aMap);
	FillTraceCode(REGVM_INSTRUCTION_RIGHTSHIFT_R, aMap);

	FillTraceCode(REGVM_INSTRUCTION_LEFTROTATE_N, aMap);
	FillTraceCode(REGVM_INSTRUCTION_LEFTROTATE_R, aMap);

	FillTraceCode(REGVM_INSTRUCTION_RIGHTROTATE_N, aMap);
	FillTraceCode(REGVM_INSTRUCTION_RIGHTROTATE_R, aMap);

	FillTraceCode(REGVM_INSTRUCTION_MODULO_R, aMap);
	FillTraceCode(REGVM_INSTRUCTION_MODULO_N, aMap);

	FillTraceCode(REGVM_INSTRUCTION_AND_R, aMap);
	FillTraceCode(REGVM_INSTRUCTION_AND_N, aMap);

	FillTraceCode(REGVM_INSTRUCTION_XOR_R, aMap);
	FillTraceCode(REGVM_INSTRUCTION_XOR_N, aMap);

	FillTraceCode(REGVM_INSTRUCTION_OR_R, aMap);
	FillTraceCode(REGVM_INSTRUCTION_OR_N, aMap);

	FillTraceCode(REGVM_INSTRUCTION_NOT, aMap);

	FillTraceCode(REGVM_INSTRUCTION_SETREG_R, aMap);
	FillTraceCode(REGVM_INSTRUCTION_SETREG_N, aMap);
	FillTraceCode(REGVM_INSTRUCTION_SETREG_M, aMap);
	FillTraceCode(REGVM_INSTRUCTION_SETREG_L, aMap);

	FillTraceCode(REGVM_INSTRUCTION_SETMEM_R, aMap);
	FillTraceCode(REGVM_INSTRUCTION_SETMEM_R2, aMap);

	FillTraceCode(REGVM_INSTRUCTION_SETLOC_R, aMap);

	FillTraceCode(REGVM_INSTRUCTION_PUTLOCALS, aMap);
	FillTraceCode(REGVM_INSTRUCTION_DEREF_R, aMap);


	FillTraceCode(REGVM_INSTRUCTION_PUSHREG, aMap);
	FillTraceCode(REGVM_INSTRUCTION_POPREG, aMap);
	FillTraceCode(REGVM_INSTRUCTION_STDCALL, aMap);

	FillTraceCode(REGVM_INSTRUCTION_EQUALS, aMap);
	FillTraceCode(REGVM_INSTRUCTION_GREATER, aMap);
	FillTraceCode(REGVM_INSTRUCTION_LESS, aMap);

	FillTraceCode(REGVM_INSTRUCTION_FEATUREEXEC, aMap);

	FillTraceCode(REGVM_INSTRUCTION_RETURN, aMap);
	FillTraceCode(REGVM_INSTRUCTION_JUMP, aMap);

	FillTraceCode(FEATURE_HOTSPOTIMPL_LOCKED, aMap);
	FillTraceCode(FEATURE_HOTSPOTIMPL_UNLOCKED, aMap);

	FillTraceCode(FEATURE_MENUBUILDERS_LOCKED, aMap);
	FillTraceCode(FEATURE_MENUBUILDERS_UNLOCKED, aMap);

	FillTraceCode(FEATURE_PLUGINMANAGER_LOCKED, aMap);
	FillTraceCode(FEATURE_PLUGINMANAGER_UNLOCKED, aMap);


	//Added 2006-11-30
	FillTraceCode(PLUGIN_NOTLOADED, aMap);
	FillTraceCode(LOCALIZATION_INITIALIZED_BUILTIN, aMap);
	FillTraceCode(LOCALIZATION_INITIALIZED_XML, aMap);


	//Added 2006-12-04
	FillTraceCode(MENUWINDOW_POPULATEDATADUMP_BEFORE_START, aMap);
	FillTraceCode(MENUWINDOW_POPULATEDATADUMP_BEFORE_END, aMap);

	FillTraceCode(MENUWINDOW_POPULATEDATADUMP_AFTER_START, aMap);
	FillTraceCode(MENUWINDOW_POPULATEDATADUMP_AFTER_END, aMap);

	FillTraceCode(DYNAMIC_DISP_ITEM_GETTEXTEXTENT_SUCCESS, aMap);
	FillTraceCode(DYNAMIC_DISP_ITEM_GETTEXTEXTENT_FAILED, aMap);

	FillTraceCode(COMMON_DUMP_GETLASTERROR_BEGIN, aMap);
	FillTraceCode(COMMON_DUMP_GETLASTERROR_END, aMap);


	if ( theNumParams == 1 )
	{
		cerr << "You must specify TraceFile for analysis" << endl;
		cerr << "Call TraceAnalyzer.exe [YouFileName.txt] or TraceAnalyzer.exe -unittest";

		return 1;
	}

	char * aFileName = theCmdLine[1];

	if ( strcmp(aFileName, "-unittest") == 0)
	{
		for (int i = TRACECODE_MIN; i <= TRACECODE_MAX; i++)
		{
			TraceCodeIterC aIt = aMap.find(i);

			if (aIt == aMap.end() )
				cout << i << " :\t[NOT FOUND]" << endl;
			else
				cout << i << ":\t" << aIt->second.c_str() << endl;
		}
		return 0;
	}

	ifstream aF(aFileName, ios::in);

	if ( !aF.is_open() )
	{
		cerr << "Cannot find the Trace file." << endl;
		cerr << "Please verify that the path is correct.";
		return 1;
	}


	char aTemp[1000];
	bool aFail = false;
	
	while (true)
	{
		aF.getline(aTemp, sizeof aTemp);

		if ( aF.bad() || aF.eof() ) break;

		if ( aTemp[0] == '-' )	//output as raw bytes
			cout << "[B]:\t" << aTemp+1 << endl;

		else
		{
			int a;
			int aNum = sscanf(aTemp, "%d", &a);

			if (aNum != 1)
				break;

			TraceCodeIterC aIt = aMap.find(a);

			if (aIt == aMap.end() )
				cout << a << " :\t[NOT FOUND]" << endl;
			else
				cout << a << ":\t" << aIt->second.c_str() << endl;
		}
	}
}
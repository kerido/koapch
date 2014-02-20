#pragma once

#include <crtdbg.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

//////////////////////////////////////////////////////////////////////////////////////////////


#if defined  (_DEBUG) || defined (_FORCETRACE)
#define TRACEMAXSTRING	1024

static char szBuffer[TRACEMAXSTRING];

inline void Trace(const char* format, ...)
{
	va_list args;
	va_start(args,format);
	int nBuf;

	nBuf = _vsnprintf(szBuffer,
										TRACEMAXSTRING,
										format,
										args);
	va_end(args);

	_RPT0(_CRT_WARN,szBuffer);
}

#define TraceEx _snprintf(szBuffer,TRACEMAXSTRING,"%s(%d): ", \
                          &strrchr(__FILE__,'\\')[1],__LINE__); \
_RPT0(_CRT_WARN,szBuffer); \
Trace


void AssertionFailure(char *exp, char *file, int line);

#define Assert(exp) \
	if (exp) ; \
	else AssertionFailure( #exp, __FILE__, __LINE__ )

#else // not _DEBUG


//inline void Trace(...){}
#define Trace

#define Assert(exp) 


#endif //defined  (_DEBUG) || defined (_FORCETRACE)



//////////////////////////////////////////////////////////////////////////////////////////////

enum ReleaseTraceEvent_V1
{
	PROGRAM_LAUNCHED,
	PROGRAM_QUIT,

	FRAMEWORK_INITIALIZE,
	FRAMEWORK_UNINITIALIZE,

	IPCMODULE_LOAD_SUCCESS,
	IPCMODULE_UNLOAD,

	CREATED_SHELLITEM_FACTORY,

	HOTSPOTMANAGER_REGISTERCLASS_SUCCESS,
	HOTSPOTMANAGER_REGISTERCLASS_FAIL,

	MENUMANAGER_INIT,
	MENUMANAGER_DISPOSE,

	MENUMANAGER_CREATEWND,
	MENUMANAGER_DESTROYWND,

	MENUWINDOW_CONSTRUCTOR,
	MENUWINDOW_DESTRUCTOR,

	PLUGIN_LOADED,
	PLUGIN_UNLOADED,

	REGISTER_KEYFILE_DOESNOTEXIST,
	REGISTER_KEYFILE_EXISTS,

	REGISTER_ACQUIRING_MACADDR,
	REGISTER_ACQUIRING_PROCID,
	REGISTER_ACQUIRING_MEMORY,
	REGISTER_ACQUIRING_HDRIVE,

	REGISTER_HARDWAREID_BEGIN,
	REGISTER_HARDWAREID_SUCCESS,
	REGISTER_HARDWAREID_FAIL,

	REGISTER_KEYIMPLCREATE_SUCCESS,
	REGISTER_KEYIMPLCREATE_FAIL,

	REGISTER_KEYDECRYPT_SUCCESS,
	REGISTER_KEYDECRYPT_FAIL,

	REGISTER_KEYEVALUATE_SUCCESS,
	REGISTER_KEYEVALUATE_FAIL,

	REGISTER_KEYEXECUTE_SUCCESS,
	REGISTER_KEYEXECUTE_FAIL,
	REGISTER_KEYEXECUTE_EXCEPTION,

	REGISTER_MD5HARDWAREID_BEGINPRINT,
	REGISTER_MD5HARDWAREID_ENDPRINT,

	REGVM_INSTRUCTION_ZERO_R,

	REGVM_INSTRUCTION_INCREMENT_1,
	REGVM_INSTRUCTION_INCREMENT_N,
	REGVM_INSTRUCTION_INCREMENT_R,

	REGVM_INSTRUCTION_DECREMENT_1,
	REGVM_INSTRUCTION_DECREMENT_N,
	REGVM_INSTRUCTION_DECREMENT_R,

	REGVM_INSTRUCTION_MULTIPLY_R,
	REGVM_INSTRUCTION_MULTIPLY_L,

	REGVM_INSTRUCTION_DIVIDE_REG,
	REGVM_INSTRUCTION_DIVIDE_LOC,

	REGVM_INSTRUCTION_LEFTSHIFT_N,
	REGVM_INSTRUCTION_LEFTSHIFT_R,

	REGVM_INSTRUCTION_RIGHTSHIFT_N,
	REGVM_INSTRUCTION_RIGHTSHIFT_R,

	REGVM_INSTRUCTION_LEFTROTATE_N,
	REGVM_INSTRUCTION_LEFTROTATE_R,

	REGVM_INSTRUCTION_RIGHTROTATE_N,
	REGVM_INSTRUCTION_RIGHTROTATE_R,

	REGVM_INSTRUCTION_MODULO_R,
	REGVM_INSTRUCTION_MODULO_N,

	REGVM_INSTRUCTION_AND_R,
	REGVM_INSTRUCTION_AND_N,

	REGVM_INSTRUCTION_XOR_R,
	REGVM_INSTRUCTION_XOR_N,

	REGVM_INSTRUCTION_OR_R,
	REGVM_INSTRUCTION_OR_N,

	REGVM_INSTRUCTION_NOT,

	REGVM_INSTRUCTION_SETREG_R,
	REGVM_INSTRUCTION_SETREG_N,
	REGVM_INSTRUCTION_SETREG_M,
	REGVM_INSTRUCTION_SETREG_L,

	REGVM_INSTRUCTION_SETMEM_R,
	REGVM_INSTRUCTION_SETMEM_R2,

	REGVM_INSTRUCTION_SETLOC_R,

	REGVM_INSTRUCTION_PUTLOCALS,
	REGVM_INSTRUCTION_DEREF_R,

	REGVM_INSTRUCTION_PUSHREG,
	REGVM_INSTRUCTION_POPREG,
	REGVM_INSTRUCTION_STDCALL,

	REGVM_INSTRUCTION_EQUALS,
	REGVM_INSTRUCTION_GREATER,
	REGVM_INSTRUCTION_LESS,

	REGVM_INSTRUCTION_FEATUREEXEC,

	REGVM_INSTRUCTION_RETURN,
	REGVM_INSTRUCTION_JUMP,

	FEATURE_HOTSPOTIMPL_LOCKED,
	FEATURE_HOTSPOTIMPL_UNLOCKED,

	FEATURE_MENUBUILDERS_LOCKED,
	FEATURE_MENUBUILDERS_UNLOCKED,

	FEATURE_PLUGINMANAGER_LOCKED,
	FEATURE_PLUGINMANAGER_UNLOCKED,



	//Added 2006-11-30
	PLUGIN_NOTLOADED,

	LOCALIZATION_INITIALIZED_BUILTIN,
	LOCALIZATION_INITIALIZED_XML,


	//Added 2006-12-04
	MENUWINDOW_POPULATEDATADUMP_BEFORE_START,
	MENUWINDOW_POPULATEDATADUMP_BEFORE_END,

	MENUWINDOW_POPULATEDATADUMP_AFTER_START,
	MENUWINDOW_POPULATEDATADUMP_AFTER_END,

	DYNAMIC_DISP_ITEM_GETTEXTEXTENT_SUCCESS,
	DYNAMIC_DISP_ITEM_GETTEXTEXTENT_FAILED,

	COMMON_DUMP_GETLASTERROR_BEGIN,
	COMMON_DUMP_GETLASTERROR_END,

	PLUGIN_CANNOTOPENPLUGINPATH,

	IPCMODULE_LOAD_FAIL
};


enum ReleaseTraceEventMinMax
{
	TRACECODE_MIN = PROGRAM_LAUNCHED,
	TRACECODE_MAX = PLUGIN_CANNOTOPENPLUGINPATH
};


#ifdef _USE_RELEASE_TRACE

extern void ReleaseTraceCode(int theReleaseTraceCode);
extern void ReleaseTraceBytes(void * theBytes, int theNumBytes);

#else

#  define ReleaseTraceCode
#  define ReleaseTraceBytes

#endif	//_USE_RELEASE_TRACE
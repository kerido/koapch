#pragma once



/*********************************************************************************************
when defined, the compiler won't issue warnings related to non secure CRT functions
*********************************************************************************************/

#define _CRT_SECURE_NO_WARNINGS 1



/*********************************************************************************************
   when defined, the compiler will substitute non-secure CRT functions with more
   secure versions
*********************************************************************************************/

#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES 1


#include "WindowsConstants.h"



#include <windows.h>
#include <shlobj.h>
#include <commctrl.h>
#include <tchar.h>
#include <wininet.h>


#ifdef _DEBUG
#   define INLINE
#else
#   define INLINE inline
#endif

//////////////////////////////////////////////////////////////////////////////////////////////
//                                CONFIGURATION SYMBOLS                                     //
//////////////////////////////////////////////////////////////////////////////////////////////

/*********************************************************************************************
   use built-in Shell manipulation routines (bind to parent and get display name)
*********************************************************************************************/
#define _USE_APPROACH


/*********************************************************************************************
   when defined, icon will be drawn straight from the image list, otherwise, GetIcon
   is used (1) and then DrawIcon (2)
*********************************************************************************************/

//#define _DRAW_FROM_IMGLIST


/*********************************************************************************************
   when defined, display "Registration" tab in the main window.
   This tab displays computer hardware id (for testing purposes)
*********************************************************************************************/

#define _APPROACH_USE_HARDWAREID


/*********************************************************************************************
   when defined, Approach will search for the key file in the installation root and will try
   to execute it in order to unlock the functionality available to registered users only.
   Whenever there are problems with the key file (the computer hardware id does not correspond
   to that of the key file, instructions are wrong, etc.) the actual functionality is
   governed by other configuration symbols.
*********************************************************************************************/

#define _APPROACH_USE_REGISTRATION



#if defined(_APPROACH_USE_REGISTRATION) && !defined(_APPROACH_USE_HARDWAREID)
#  error _APPROACH_USE_HARDWAREID must be enabled in order to use _APPROACH_USE_REGISTRATION
#endif



/*********************************************************************************************
   When defined, the Debug builds will write each item's attributes into the Trace Output.
   This is done by Shell Item Factory methods when trying to create an actual ShellItem
   instance
*********************************************************************************************/

//#define _USE_ATTRIBUTE_CHECKING


/*********************************************************************************************
   When defined, Approach will try to extract a cached icon from Shell32's cache. Otherwise
   a stand-alone cache is used.
*********************************************************************************************/
//#define _USE_SHELL_ICON_CACHE



/*********************************************************************************************
   when defined, creates a background thread that monitors presence of a debugger
   and, if identified, terminates the program
*********************************************************************************************/
#ifndef _DEBUG
//#  define _ANTI_DEBUGGER_PROTECT
#endif



/*********************************************************************************************
   when defined, Approach extracts the Registered User from the Key
*********************************************************************************************/

#define _USE_EXTENDED_KEY


/*********************************************************************************************
   when defined, Approach analyzes the usage of FolderMenus.dll
*********************************************************************************************/

#define _USE_FOLDERMENUS_USAGE_TRACING


/*********************************************************************************************
   when defined, Approach uses localhost for web service communications
*********************************************************************************************/

//#define _APPROACH_LOCAL_TEST


/*********************************************************************************************
   when defined, Approach implements ReleaseTraceCode and ReleasTraceBytes to store status
   info while a Release (Free) build app is running. The data is dumped into a special Text
   file each time the program is exited
*********************************************************************************************/

#define _USE_RELEASE_TRACE



/*********************************************************************************************
when defined, will define the Trace macro irregardless of the configuration (debug/release)
*********************************************************************************************/

//#define _FORCETRACE



/*********************************************************************************************
when defined the preferences integration will use the IChangeableEntity interface
*********************************************************************************************/

//#define _USE_CHANGEABLEENTITY



/*********************************************************************************************
   when defined, Approach monitors the ./Locale directory of changes
*********************************************************************************************/

#define _USE_LOCALIZATION_MONITORING



/*********************************************************************************************
   when defined as nonzero, Approach generates an En_Us Reference file at startup
*********************************************************************************************/

#define _APPROACH_USE_REFERENCE_FILE 0


/*********************************************************************************************
   when defined as nonzero, Approach will display various localizable aspects
   every 5 seconds so that screenshots can be taken
*********************************************************************************************/

#define _USE_LOCALIZATION_TESTING 0

//////////////////////////////////////////////////////////////////////////////////////////////

#define DFN_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
	EXTERN_C const GUID DECLSPEC_SELECTANY name \
	= { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }


// {63B01035-B12E-47CA-B55B-1F377F2E4634}
DFN_GUID (GUID_Scope,              0x63B01035, 0xB12E, 0x47CA, 0xB5, 0x5B, 0x1F, 0x37, 0x7F, 0x2E, 0x46, 0x34);

// {EDA75809-0C83-4e04-BE3C-AF750471D374}
DFN_GUID (GUID_InstantWave,        0xEDA75809, 0x0C83, 0x4E04, 0xBE, 0x3C, 0xAF, 0x75, 0x04, 0x71, 0xD3, 0x74);

// {9528B21A-9059-42ED-80C9-DEF8A244135B}
DFN_GUID (GUID_ApproachVersion_V1, 0x9528B21A, 0x9059, 0x42ED, 0x80, 0xC9, 0xDE, 0xF8, 0xA2, 0x44, 0x13, 0x5B);


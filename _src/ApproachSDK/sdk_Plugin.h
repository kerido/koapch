#pragma once

#include "sdk_RuntimeObject.h"

//////////////////////////////////////////////////////////////////////////////////////////////

//! Represents a KO Approach plugin.
MIDL_INTERFACE("3DADED7D-BA62-4D3F-AD18-37B4F7C6DBA8") IApproachPlugin : public IRuntimeObject
{
	//! Initializes the plugin immediately after the plugin DLL is loaded.
	//! 
	//! \param [in] theFrameworkVersion
	//!     Specifies the version of the Approach core services framework. Currently
	//!     has the values of 2.
	//! 
	//! \return
	//!     S_OK if plugin loaded successfully; otherwise, a COM error value.
	//! 
	//!\remarks
	//!     If the plugin returns an COM error the corresponding DLL will be immediately unloaded.
	STDMETHOD (OnLoad) (int theFrameworkVersion) = 0;


	//! Performs finalization tasks and releases all resources allocated inside #OnLoad.
	//! 
	//! \return
	//!     S_OK if finalization completed successfully; otherwise, a COM error value.
	//! 
	//! \remarks
	//!     The plugin DLL is unloaded after this method completes irregardless of the return value.
	STDMETHOD (OnUnload) () = 0;
};

// {3DADED7D-BA62-4D3F-AD18-37B4F7C6DBA8}
DEFINE_GUID(IID_IApproachPlugin,
						0x3DADED7D, 0xBA62, 0x4D3F, 0xAD, 0x18, 0x37, 0xB4, 0xF7, 0xC6, 0xDB, 0xA8);


typedef IApproachPlugin * (* PluginRetFn) ();

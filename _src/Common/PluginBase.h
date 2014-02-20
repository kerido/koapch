#pragma once

#include "sdk_Plugin.h"
#include "sdk_ComObject.h"

//////////////////////////////////////////////////////////////////////////////////////////////

template<>
class DECLSPEC_NOVTABLE ComEntry<IApproachPlugin> :
	public ComEntryWithParentDiscovery<IApproachPlugin, IRuntimeObject>
{
};

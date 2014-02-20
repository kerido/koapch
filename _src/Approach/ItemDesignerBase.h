#pragma once

#include "sdk_ItemDesigner.h"
#include "sdk_ComObject.h"

//////////////////////////////////////////////////////////////////////////////////////////////

template<>
class DECLSPEC_NOVTABLE ComEntry<IItemDesigner> :
	public ComEntryWithParentDiscovery<IItemDesigner, IRuntimeObject>
{
};

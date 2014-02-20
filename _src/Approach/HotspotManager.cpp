#include "stdafx.h"

#include "sdk_Localization.h"

#include "Framework.h"
#include "HotspotNew.h"

//////////////////////////////////////////////////////////////////////////////////////////////

HotspotManager::HotspotManager()
{
	Trace("HotspotManager::HotspotManager.");
}

//////////////////////////////////////////////////////////////////////////////////////////////

HotspotManager::~HotspotManager()
{
	Trace("HotspotManager::~HotspotManager.");
}

//////////////////////////////////////////////////////////////////////////////////////////////

void HotspotManager::Add(Hotspot * theHS)
{
	GUID aGuid;
	theHS->GetGuid(aGuid);

	HotspotMapIter aIt = myHotspotMap.find(aGuid);

	if ( aIt == myHotspotMap.end() )
		aIt = myHotspotMap.insert( aIt, HotspotGuidPair(aGuid, theHS) );
}

//////////////////////////////////////////////////////////////////////////////////////////////

void HotspotManager::Remove(const GUID & theGuid)
{
	HotspotMapIter aIt = myHotspotMap.find(theGuid);

	if ( aIt != myHotspotMap.end() )
	{
		delete aIt->second;
		myHotspotMap.erase( aIt );
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////

bool HotspotManager::ToggleHotspot(const GUID & theGuid, bool theEnable)
{
	HotspotMapIterC aIt = myHotspotMap.find(theGuid);

	if ( aIt == myHotspotMap.end() )
		return false;

	IHotspotEnabler * aHSE = Framework::GetHotspotEnabler();
	bool aRes = aHSE->ToggleHotspot(aIt->second, theEnable);

	for (HotspotProcessorIter aIt_Pr = myHotspotProcessors.begin(); aIt_Pr != myHotspotProcessors.end(); aIt_Pr++)
		(*aIt_Pr)->OnHotspotToggle(theGuid, theEnable, aRes);

	return aRes;
}

//////////////////////////////////////////////////////////////////////////////////////////////

bool HotspotManager::ToggleHotspot(const GUID & theGuid)
{
	HotspotMapIterC aIt = myHotspotMap.find(theGuid);

	if ( aIt == myHotspotMap.end() )
		return false;

	bool aEnable = !aIt->second->IsEnabled();

	IHotspotEnabler * aHSE = Framework::GetHotspotEnabler();
	bool aRes = aHSE->ToggleHotspot(aIt->second, aEnable);

	for (HotspotProcessorIter aIt_Pr = myHotspotProcessors.begin(); aIt_Pr != myHotspotProcessors.end(); aIt_Pr++)
		(*aIt_Pr)->OnHotspotToggle(theGuid, aEnable, aRes);

	return aRes;
}

//////////////////////////////////////////////////////////////////////////////////////////////

bool HotspotManager::IsHotspotEnabled(const GUID & theGuid) const
{
	HotspotMapIterC aIt = myHotspotMap.find(theGuid);

	if ( aIt == myHotspotMap.end() )
		return false;
	else
		return aIt->second->IsEnabled();
}

//////////////////////////////////////////////////////////////////////////////////////////////

Hotspot * HotspotManager::GetHotspot(const GUID & theGuid)
{
	HotspotMapIter aIt = myHotspotMap.find(theGuid);

	if ( aIt == myHotspotMap.end() )
		return 0;
	else
		return aIt->second;
}

//////////////////////////////////////////////////////////////////////////////////////////////

void HotspotManager::RegisterProcessor(IHotspotToggleProcessor * thePr)
{
	myHotspotProcessors.push_back(thePr);
}

//////////////////////////////////////////////////////////////////////////////////////////////

void HotspotManager::UnregisterProcessor(IHotspotToggleProcessor * thePr)
{
	myHotspotProcessors.remove(thePr);
}

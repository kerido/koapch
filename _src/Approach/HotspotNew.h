#pragma once

#include "sdk_GuidComparer.h"

#include "Trace.h"

//////////////////////////////////////////////////////////////////////////////////////////////

//! Represents a program feature.
class Hotspot
{
	friend class HotspotManager;
	friend class HotspotEnablerImpl_Unregistered;
	friend class HotspotEnablerImpl_Registered;

protected:
	bool myEnabled;

public:
	Hotspot() : myEnabled(false) {}
	virtual ~Hotspot()           {}

protected:
	//! Called when the feature is being enabled/disabled.
	//! \param theEnable   True if the feature is being enabled; otherwise, false.
	virtual void OnToggle(bool theEnable) = 0
		{ myEnabled = theEnable; }

public:
	//! Returns whether a feature is currently enabled.
	//! \return  True if the feature is enabled; otherwise, false.
	bool IsEnabled() const { return myEnabled; }

	//! Returns the GUID representing the current object.
	//! \param [out] theOut    The reference that will receive the object's GUID/
	virtual void GetGuid(GUID & theOut) const = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////////

//! TODO
class DECLSPEC_NOVTABLE IHotspotToggleProcessor
{
public:
	virtual void OnHotspotToggle(const GUID & theGuid, bool theEnable, bool theResult) = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////////

//! Represents the KO Approach feature manager.
class HotspotManager
{
// Type definitions
private:
	typedef std::map<GUID, Hotspot *, GuidComparer>  HotspotMap;
	typedef HotspotMap::value_type                   HotspotGuidPair;
	typedef HotspotMap::iterator                     HotspotMapIter;
	typedef HotspotMap::const_iterator               HotspotMapIterC;
	typedef HotspotMap::reverse_iterator             HotspotMapRiter;
	typedef HotspotMap::const_reverse_iterator       HotspotMapRiterC;

	typedef std::list<IHotspotToggleProcessor *> HotspotProcessorList;
	typedef HotspotProcessorList::iterator       HotspotProcessorIter;
	typedef HotspotProcessorList::const_iterator HotspotProcessorIterC;


// Fields
private:
	WNDCLASS             myWndClass;           //!< The window class for the windowed feature.
	HotspotMap           myHotspotMap;         //!< The map of features that the object is monitoring.
	HotspotProcessorList myHotspotProcessors;  //!< The list of objects responding to feature events.


// Constructors, destructor
public:

	//! Creates an instance of the object and initializes the necessary resources.
	HotspotManager();


	//! Destroys the object and frees any resources used. Actual feature instances
	//! are also destroyed here.
	~HotspotManager();


// Interface
public:

	//! Adds a feature to the feature manager for monitoring.
	//! 
	//! \param theHS
	//!     Pointer to the feature to be added. The feature will be automatically
	//      destroyed in the feature manager's destructor.
	void Add(Hotspot * theHS);


	//! Removes a feature identified by a GUID.
	//! \param theGuid   The GUID of the feature to be removed.
	void Remove(const GUID & theGuid);


	//! Switches a feature on or off.
	//! \param theGuid   The GUID of the feature to be switched on or off.
	//! \param theEnable True if the feature must be switched on; otherwise, false.
	//! \return True if the action was really performed on the feature; otherwise, false.
	bool ToggleHotspot(const GUID & theGuid, bool theEnable);


	//! Switches on a feature if it is currently disabled and off
	//! if it is currently enabled.
	//! \param theGuid  The GUID of the feature to be switched on or off.
	//! \return True if the action was really performed on the feature; otherwise, false.
	bool ToggleHotspot(const GUID & theGuid);


	//! Determines if a feature identified by a GUID is currently enabled.
	//! \param theGuid  The GUID of the feature.
	//! \return  True if the feature is currently enabled; otherwise, false.
	bool IsHotspotEnabled(const GUID & theGuid) const;


	//! Retrieves a pointer to a feature identified by a GUID.
	//! \param theGuid  The GUID of the feature to be retrieved.
	//! \return  A non-null pointer if the feature was found; otherwise, a null pointer.
	Hotspot * GetHotspot(const GUID & theGuid);


	//! Adds a feature event processor.
	//! \param thePr   The object that will respond to feature events.
	void RegisterProcessor  (IHotspotToggleProcessor * thePr);


	//! Removes a feature event processor.
	//! \param thePr   The object previously added via a call to RegisterProcessor.
	void UnregisterProcessor(IHotspotToggleProcessor * thePr);


	void DestroyFeatures()
	{
		for(HotspotMapRiter aIt = myHotspotMap.rbegin(); aIt != myHotspotMap.rend(); aIt++)
			delete aIt->second;
	}
};


//////////////////////////////////////////////////////////////////////////////////////////////

class DECLSPEC_NOVTABLE IHotspotEnabler
{
public:
	virtual bool ToggleHotspot(Hotspot * theHS, bool theEnable) = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////////

class HotspotEnablerImpl_Unregistered : public IHotspotEnabler
{
	typedef bool (* ToggleHotspotFn) (Hotspot * theHS, bool theEnable, int & theCurNumActiveHotspots);

	ToggleHotspotFn myImpls[3];
	int myCurNumActiveHotspots;

public:
	HotspotEnablerImpl_Unregistered() : myCurNumActiveHotspots(0)
	{
		ReleaseTraceCode(FEATURE_HOTSPOTIMPL_LOCKED);

		myImpls[0] = myImpls[1] = ToggleHotspotImpl_Ok;
		myImpls[2] = ToggleHotspotImpl_No;
	}


protected:
	virtual bool ToggleHotspot(Hotspot * theHS, bool theEnable)
	{
		if (theHS->IsEnabled() == theEnable)
			return true;
		else
			return myImpls[ myCurNumActiveHotspots ] (theHS, theEnable, myCurNumActiveHotspots);
	}

private:
	static bool ToggleHotspotImpl_Ok (Hotspot * theHS, bool theEnable, int & theCurNumActiveHotspots)
	{
		theHS->OnToggle(theEnable);

		if (theEnable)
			theCurNumActiveHotspots++;
		else
			theCurNumActiveHotspots--;

		return true;
	}

	static bool ToggleHotspotImpl_No (Hotspot * theHS, bool theEnable, int & theCurNumActiveHotspots)
	{
		if (theEnable)
			return false;
		else
		{
			theHS->OnToggle(theEnable);
			theCurNumActiveHotspots--;

			return true;
		}
	}
};

//////////////////////////////////////////////////////////////////////////////////////////////

class HotspotEnablerImpl_Registered : public IHotspotEnabler
{
public:
	HotspotEnablerImpl_Registered()
	{
		ReleaseTraceCode(FEATURE_HOTSPOTIMPL_UNLOCKED);
	}

protected:
	virtual bool ToggleHotspot(Hotspot * theHS, bool theEnable)
	{
		theHS->OnToggle(theEnable);
		return true;
	}
};

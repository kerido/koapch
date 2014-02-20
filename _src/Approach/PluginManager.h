#pragma once

#include "sdk_Plugin.h"
#include "sdk_MenuBuilder.h"
#include "sdk_GuidComparer.h"

#include "LogicCommon.h"
#include "Application.h"
#include "Framework.h"
#include "typedefs.h"

//////////////////////////////////////////////////////////////////////////////////////////////

class PluginManagerCommon : public IPluginManager
{
private:
	bool myHasLoadedPlugins;


public:
	PluginManagerCommon() : myHasLoadedPlugins(false)
	{ }


protected:
	bool HasInstalledPlugins() const
	{
		return myHasLoadedPlugins;
	}

	bool LoadPlugins(const TCHAR * thePath, int theSize_Path)
	{
		bool aRes = false;

		HANDLE hDirValid = CreateFile(
			thePath,
			GENERIC_READ,                      //open for reading 
			FILE_SHARE_READ|FILE_SHARE_WRITE,
			NULL,                              //no security 
			OPEN_EXISTING,
			FILE_FLAG_BACKUP_SEMANTICS,        //trying to obtain a handle to a directory
			NULL );

		if (hDirValid == INVALID_HANDLE_VALUE)	//dir doesn't exist
		{
			ReleaseTraceCode(PLUGIN_CANNOTOPENPLUGINPATH);
			return aRes;
		}

		String aFilePattern = thePath;

		size_t aLength = aFilePattern.size();
		if ( aFilePattern.substr(aLength - 1, 1) != _T("\\") )
			aFilePattern += _T("\\");

		String aCorrectDir = aFilePattern;

		aFilePattern += _T("*.dll");

		//begin the enumeration process
		WIN32_FIND_DATA aWFD;

		HANDLE aSearchRes = FindFirstFile(aFilePattern.c_str(), &aWFD);

		if (aSearchRes != INVALID_HANDLE_VALUE)
		{
			const ApplicationSettings * aSt = Application::InstanceC().Prefs();
			do
			{
				myHasLoadedPlugins = true;

				if (aSt->IsPluginDisabled(aWFD.cFileName))
					continue;

				String aFullFileName = aCorrectDir + aWFD.cFileName;

				aRes |= OnPluginFound( aFullFileName.c_str(), aFullFileName.size() );
			} while ( FindNextFile(aSearchRes, & aWFD) );

			FindClose(aSearchRes);
		}

		CloseHandle(hDirValid);
		return aRes;
	}



protected:
	virtual bool OnPluginFound(const TCHAR * thePluginPath, size_t theSize_Path) = 0;


protected:
	void ReleaseSinglePlugin(const PluginInfo * theInfo)
	{
		Application & aApp = Application::Instance();

		aApp.ReleaseMenuBuilders(theInfo->myModule);
		aApp.ReleaseContextMenuExtensions(theInfo->myModule);

		ReleaseTraceCode(PLUGIN_UNLOADED);

		HRESULT aRes = theInfo->myPlugin->OnUnload();
		FreeLibrary(theInfo->myModule);
	}


	template <typename tImpl>
	static bool AddPlugin_Yes(tImpl * theThis, const TCHAR * thePluginPath, size_t theSize_Path)
	{
		if ( HMODULE aDLL = LoadLibrary(thePluginPath) )
		{
			if ( PluginRetFn aFn = (PluginRetFn)GetProcAddress(aDLL, "GetKOApproachPlugin") )
				if ( IApproachPlugin * aPlugin = aFn() )
					if ( SUCCEEDED( aPlugin->OnLoad(1) ) )
					{
						if ( theThis->ReallyAddPlugin(aPlugin, aDLL) )
						{
							theThis->ProcessPluginServices(aPlugin, aDLL);
							return true;
						}
						else
						{
							ReleaseTraceCode(PLUGIN_NOTLOADED);
							aPlugin->OnUnload();
						}
					}

					FreeLibrary(aDLL);
		}

		return false;
	}

private:
	void ProcessPluginServices(IApproachPlugin * thePlugin, HMODULE theDll)
	{
		CComQIPtr<IClassFactory> aCF(thePlugin);

		if (aCF == 0)
			return;

		Application & aApp = Application::Instance();

		//1. MenuBuilder
		IMenuBuilder * aMB = 0;
		HRESULT aRes = aCF->CreateInstance(NULL, IID_IMenuBuilder, (void**) &aMB);

		if (aMB != 0)
			aApp.RegisterMenuBuilder(aMB, theDll);


		//2. Context Menu Extension
		IContextMenuExtension * aCME = 0;
		aRes = aCF->CreateInstance(NULL, IID_IContextMenuExtension, (void **) &aCME);

		if (aCME != 0)
			aApp.RegisterContextMenuExtension(aCME, theDll);
	}
};

//////////////////////////////////////////////////////////////////////////////////////////////

class PluginManagerImpl_Limited : public PluginManagerCommon
{
	typedef bool (* AddPluginFn) (PluginManagerImpl_Limited * theThis, const TCHAR * thePath, size_t theSize_Path);

	AddPluginFn myAdditions[3];

	PluginInfo myPlugins[2];
	GUID myPluginGuids[2];

	int myNumLoadedPlugins;


public:
	PluginManagerImpl_Limited() : myNumLoadedPlugins(0)
	{
		ReleaseTraceCode(FEATURE_PLUGINMANAGER_LOCKED);

		myAdditions[0] = AddPlugin_Yes<PluginManagerImpl_Limited>;
		myAdditions[1] = AddPlugin_Yes<PluginManagerImpl_Limited>;
		myAdditions[2] = AddPlugin_No;
	}

	bool OnPluginFound(const TCHAR * thePluginPath, size_t theSize_Path)
	{
		return myAdditions[myNumLoadedPlugins] (this, thePluginPath, theSize_Path);
	}

	void ReleasePlugins()
	{
		for (int i = 0; i < myNumLoadedPlugins; i++)
			ReleaseSinglePlugin( &myPlugins[i] );

		myNumLoadedPlugins = 0;
	}

	int GetPluginCount() const
	{
		return myNumLoadedPlugins;
	}


	bool ReallyAddPlugin(IApproachPlugin * thePlugin, HMODULE theDll)
	{
		HRESULT aRes = thePlugin->GetTypeGuid( &myPluginGuids[myNumLoadedPlugins] );
		if ( FAILED(aRes) )
			return false;

		bool aAlreadyExists = false;

		for ( int i = 0; i < myNumLoadedPlugins; i++)
			if ( myPluginGuids[i] == myPluginGuids[myNumLoadedPlugins] )
				return false;

		//proceed to actual loading
		ReleaseTraceCode(PLUGIN_LOADED);
		Trace("Adding a plugin\n");

		myPlugins[myNumLoadedPlugins++] = PluginInfo(theDll, thePlugin);
		//the GUID is already loaded so there is no need to copy

		return true;
	}

	HRESULT GetPluginInfo(const TCHAR * theFullPath, PluginInfo * theOut)
	{
		if (theOut == NULL)
			return E_POINTER;

		HMODULE aMod = GetModuleHandle(theFullPath);

		if (aMod == NULL)
			return E_FAIL;

		for(int i = 0; i < myNumLoadedPlugins; i++)
			if (myPlugins[i].myModule == aMod)
			{
				*theOut = myPlugins[i];
				return S_OK;
			}

		return S_FALSE;
	}


protected:
	static bool AddPlugin_No(PluginManagerImpl_Limited *, const TCHAR *, size_t)
		{ return false; }
};

//////////////////////////////////////////////////////////////////////////////////////////////

class PluginManagerImpl_Unlimited : public PluginManagerCommon
{
protected:
	typedef std::map<GUID, PluginInfo, GuidComparer> PluginInfoMap;
	typedef PluginInfoMap::iterator                  PluginInfoIter;

protected:
	PluginInfoMap myPlugins;

public:
	PluginManagerImpl_Unlimited()
	{
		ReleaseTraceCode(FEATURE_PLUGINMANAGER_UNLOCKED);
	}

	void ReleasePlugins()
	{
		for (PluginInfoIter aIt = myPlugins.begin(); aIt != myPlugins.end(); aIt++)
			ReleaseSinglePlugin(&aIt->second);

		myPlugins.clear();
	}

	bool OnPluginFound(const TCHAR * thePluginPath, size_t theSize_Path)
	{
		return AddPlugin_Yes(this, thePluginPath, theSize_Path);
	}

	int GetPluginCount() const
	{
		return (int) myPlugins.size();
	}

	bool ReallyAddPlugin(IApproachPlugin * thePlugin, HMODULE theDll)
	{
		GUID aGuid;
		HRESULT aRes = thePlugin->GetTypeGuid(&aGuid);

		if ( FAILED(aRes) )
			return false;

		PluginInfoIter aIt = myPlugins.find(aGuid);

		if ( aIt != myPlugins.end() )
			return false;

		ReleaseTraceCode(PLUGIN_LOADED);
		Trace("Adding a plugin\n");

		myPlugins[aGuid] = PluginInfo(theDll, thePlugin);
		return true;
	}

	HRESULT GetPluginInfo(const TCHAR * theFullPath, PluginInfo * theOut)
	{
		if (theOut == NULL)
			return E_POINTER;

		HMODULE aMod = GetModuleHandle(theFullPath);

		if (aMod == NULL)
			return E_FAIL;

		for(PluginInfoIter aIt = myPlugins.begin(); aIt != myPlugins.end(); aIt++)
			if (aIt->second.myModule == aMod)
			{
				*theOut =aIt->second;
				return S_OK;
			}

			return S_FALSE;
	}
};
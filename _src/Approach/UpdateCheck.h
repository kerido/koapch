#pragma once

#include "sdk_Thread.h"

//////////////////////////////////////////////////////////////////////////////////////////////

class DECLSPEC_NOVTABLE IAsyncUpdateCheckResultHandler
{
public:
	virtual void HandleUpdateCheckResult(class UpdateCheckTask * theTask) = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////////

class UpdateCheckData
{
private:
	const static size_t Size_Url = 2000;
	const static size_t Size_Version = 500;

private:
	bool myHasUpdates;
	TCHAR myUrl[Size_Url];
	size_t mySize_Url;
	TCHAR myVersion[Size_Version];
	size_t mySize_Version;

public:
	UpdateCheckData() { Reset(); }

public:
	void Reset()
	{
		myHasUpdates = false;

		mySize_Url = Size_Url;
		mySize_Version = Size_Version;

		myUrl[0] = myVersion[0] = 0;
	}

	bool HasUpdates() const        { return myHasUpdates; }
	void HasUpdates(bool theValue) { myHasUpdates = theValue; }

	const TCHAR * Url() const      { return myUrl; }
	TCHAR * Url()                  { return myUrl; }
	size_t & UrlSize()             { return mySize_Url; }

	const TCHAR * Version() const  { return myVersion; }
	TCHAR * Version()              { return myVersion; }
	size_t & VersionSize()         { return mySize_Version; }
};

//////////////////////////////////////////////////////////////////////////////////////////////

class UpdateCheckTask : public Thread
{
private:
	volatile LONG myStatus;
	IAsyncUpdateCheckResultHandler * myHandler;
	UpdateCheckData * myData;


public:
	UpdateCheckTask();
	~UpdateCheckTask();

public:
	void HandleUpdates(IAsyncUpdateCheckResultHandler * theHandler, UpdateCheckData * theData);
	bool GetUpdateData(TCHAR * theVersion, int & theSizeVersion, TCHAR * theUrl, int & theSizeUrl) const;
	static bool ShouldPerformUpdateCheck();
	static void SaveUpdateCheckResult();

public:
	void Terminate();

protected:
	DWORD Run();
};

//////////////////////////////////////////////////////////////////////////////////////////////

class UpdateCheckPackage
{
private:
	UpdateCheckTask * myUpdateCheckTask;
	UpdateCheckData * myUpdateCheckData;

public:
	UpdateCheckPackage();
	~UpdateCheckPackage();

public:
	void HandleUpdates(IAsyncUpdateCheckResultHandler * theHandler);
	const UpdateCheckData & GetData() const;

private:
	void EnsureUpdateCheckTask();
	void EnsureUpdateCheckData();
	void DestroyUpdateCheckTask();
	void DestroyUpdateCheckData();
};

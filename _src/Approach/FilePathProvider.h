#pragma once

class FilePathProvider
{
public:
	enum PathType
	{
		Dir_Main,
		Dir_Locale,
		Dir_Plugins,
		File_MainModule,
		File_IpcModule,
		File_HtmlHelp,
		File_Key,
		File_TempKey,
		File_ReleaseTrace
	};


private:
	TCHAR myDirPath[1024];
	int myDirPathLength;

	TCHAR myMainModulePath[1024];
	int myMainModulePathLength;

public:
	FilePathProvider(HMODULE theModule)
	{
		// 1. Main Module Path
		myMainModulePathLength = GetModuleFileName(theModule, myMainModulePath, 1024);

		// 2. Main Directory Path
		TCHAR * aBSOccur = _tcsrchr(myMainModulePath, _T('\\') ); //find last occurrence of a backslash

		//Assert(aBSOccur != 0);

		myDirPathLength = (int) (aBSOccur - myMainModulePath);
		lstrcpyn(myDirPath, myMainModulePath, myDirPathLength + 1);
		myDirPath[myDirPathLength] = 0;
	}

public:
	HRESULT GetPath(PathType thePathType, TCHAR * theDestPath, int * theSize) const
	{
		if (theDestPath == 0 || theSize == 0)
			return E_INVALIDARG;

		if      (thePathType == Dir_Main)
		{
			if (*theSize < myDirPathLength + 1)
				return E_OUTOFMEMORY;

			lstrcpyn(theDestPath, myDirPath, myDirPathLength + 1);

			*theSize = myDirPathLength;
			return S_OK;
		}

		else if (thePathType == Dir_Locale)
		{
			if (*theSize < myDirPathLength + 7 + 1)
				return E_OUTOFMEMORY;

			lstrcpyn(theDestPath, myDirPath, myDirPathLength + 1);
			lstrcat(theDestPath, _T("\\Locale") );

			*theSize = myDirPathLength + 7;	//increment by the length of the '\Locale' string
			return S_OK;
		}

		else if (thePathType == Dir_Plugins)
		{
			if (*theSize < myDirPathLength + 8 + 1)
				return E_OUTOFMEMORY;

			lstrcpyn(theDestPath, myDirPath, myDirPathLength + 1);
			lstrcat(theDestPath, _T("\\Plugins") );

			*theSize = myDirPathLength + 8;	//increment by the length of the '\Plugins' string
			return S_OK;
		}

		else if (thePathType == File_MainModule)
		{
			if (*theSize < myMainModulePathLength + 1)
				return E_OUTOFMEMORY;

			lstrcpyn(theDestPath, myMainModulePath, myMainModulePathLength + 1);

			*theSize = myMainModulePathLength;
			return S_OK;
		}

		else if (thePathType == File_IpcModule)
		{
			if (*theSize < myDirPathLength + 16 + 1)
				return E_OUTOFMEMORY;

			lstrcpyn(theDestPath, myDirPath, myDirPathLength + 1);
			lstrcat(theDestPath, _T("\\ApproachIpc.dll") );

			*theSize = myDirPathLength + 16;	//increment by the length of the '\Plugins' string
			return S_OK;
		}

		else if (thePathType == File_HtmlHelp)
		{
			if (*theSize < myDirPathLength + 13 + 1)
				return E_OUTOFMEMORY;

			lstrcpyn(theDestPath, myDirPath, myDirPathLength + 1);
			lstrcat(theDestPath, _T("\\Approach.chm") );

			*theSize = myDirPathLength + 13;	//increment by the length of the '\Plugins' string
			return S_OK;
		}

		else if (thePathType == File_Key)
		{
			if (*theSize < myDirPathLength + 13 + 1)
				return E_OUTOFMEMORY;

			lstrcpyn(theDestPath, myDirPath, myDirPathLength + 1);
			lstrcat(theDestPath, _T("\\Approach.key") );

			*theSize = myDirPathLength + 13;	//increment by the length of the '\Approach.key' string
			return S_OK;
		}

		else if (thePathType == File_ReleaseTrace)
		{
			if (*theSize < myDirPathLength + 18 + 1)
				return E_OUTOFMEMORY;

			lstrcpyn(theDestPath, myDirPath, myDirPathLength + 1);
			lstrcat(theDestPath, _T("\\ApproachTrace.txt") );

			*theSize = myDirPathLength + 18;	//increment by the length of the '\Plugins' string
			return S_OK;
		}

		else if (thePathType == File_TempKey)
		{
			DWORD aLength = GetTempPath(*theSize, theDestPath);

			if (aLength == 0)
				return E_OUTOFMEMORY;

			if ( theDestPath[aLength-1] == _T('\\') )
				aLength--;

			lstrcpy(theDestPath + aLength, _T("\\Approach.key") );
			*theSize = aLength + 13;			//increment by the length of the '\Approach.key' string

			return S_OK;
		}
		else
			return E_FAIL;
	}
};
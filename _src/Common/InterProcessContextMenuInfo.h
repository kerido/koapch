#pragma once

#include <tchar.h>
#include <shlobj.h>

#include "IpcGuids.h"
#include "UtilPidlFunc.h"
#include "Trace.h"

//////////////////////////////////////////////////////////////////////////////////////////////

class InterProcessContextMenuInfo
{
private:
	const static UINT NumFields = 2;

protected:
	struct SizeDescriptor
	{
		UINT Sizes[NumFields];
		UINT FullBytes() const { return Sizes[0] + Sizes[1]; }
	};

	const static UINT SizeDescriptorSize = sizeof(SizeDescriptor);

	
protected:
	IShellFolder *  myShellFolder;
	LPITEMIDLIST    myChildPidl;
	LPITEMIDLIST    myFolderPidl;
	HWND            myActivatorWnd;


//Constructors, destructor
public:
	InterProcessContextMenuInfo
				(
					IShellFolder *  theSF,           //!< TODO
					LPITEMIDLIST    thePidl,         //!< TODO
					HWND            theActivatorWnd  //!< TODO
				)
	{
		SecureZeroMemory(this, sizeof InterProcessContextMenuInfo);

		myShellFolder = theSF;
		myChildPidl = thePidl;
		myActivatorWnd = theActivatorWnd;

		myShellFolder->AddRef();
	}

	InterProcessContextMenuInfo()
	{
		SecureZeroMemory(this, sizeof InterProcessContextMenuInfo);
	}

	~InterProcessContextMenuInfo()
	{
		if (myShellFolder != 0)
			myShellFolder->Release();
	}


//Serialization routines
public:
	template<typename T_Writer>
	static void WriteToSharedMemory(InterProcessContextMenuInfo * theObj)
	{
		char * aBuf_Temp = 0;
		HANDLE aHandle = GetSharedMemoryBuffer(&aBuf_Temp, false);

		if (aHandle == 0)
		{
			Trace("Unable to query shared memory buffer\n");
			return;
		}

		size_t aRet = theObj->WriteToMemoryBuffer<T_Writer>(aBuf_Temp);	//real operation
		
		Trace("WriteToMemoryBuffer returned %d bytes\n", aRet);

		ReleaseSharedMemoryBuffer(aHandle, aBuf_Temp);
	}


	template<typename T_Writer>
	static void ReadAndExecuteFromSharedMemory()
	{
		char * aBuf_Temp = 0;
		HANDLE aHandle = GetSharedMemoryBuffer(&aBuf_Temp, true);

		if (aHandle == 0)
		{
			Trace("Unable to query shared memory buffer\n");
			return;
		}

		InterProcessContextMenuInfo aObj;

		size_t aRet = aObj.ReadFromMemoryBuffer(aBuf_Temp);
		Trace("WriteToMemoryBuffer returned %d bytes\n", aRet);

		if (aRet != 0)
			aObj.ExecuteCM<T_Writer>();

		ReleaseSharedMemoryBuffer(aHandle, aBuf_Temp);
	}

	bool ConstructAbsoluteFolderPidl()
	{
		IPersistFolder2 * aPF = 0;
		HRESULT aRes = myShellFolder->QueryInterface(IID_IPersistFolder2, (void **) &aPF);

		if ( SUCCEEDED(aRes) && aPF != 0)
		{
			aRes = aPF->GetCurFolder(&myFolderPidl);

			aPF->Release();
		}

		return ( SUCCEEDED(aRes) && myFolderPidl != 0);
	}


// Implementation Details
private:

	// theSize of zero indicates default size usage
	static HANDLE GetSharedMemoryBuffer(char ** theOut, bool theForReading, size_t theSize = 0)
	{
		DWORD aDesiredAccess = theForReading ? FILE_MAP_READ : FILE_MAP_ALL_ACCESS;
		size_t aDesiredSize  = theSize == 0  ?      4096     : theSize;

		HANDLE aMapFile = OpenFileMapping(aDesiredAccess, FALSE, GetMemoryMapName() );

		if (aMapFile == NULL || aMapFile == INVALID_HANDLE_VALUE)
			return 0;

		char * aBuf = (char *)MapViewOfFile(aMapFile, aDesiredAccess, 0, 0, aDesiredSize);

		if (aBuf == NULL)
		{
			CloseHandle(aMapFile);
			return 0;
		}
		else
		{
			*theOut = aBuf;
			return aMapFile;
		}
	}

	static void ReleaseSharedMemoryBuffer(HANDLE theHandle, const char * theBuf)
	{
		UnmapViewOfFile( (void *) theBuf);
		CloseHandle(theHandle);
	}

	template<typename T_Writer>
	size_t WriteToMemoryBuffer(char * theBuf) const
	{
		SizeDescriptor aSz;
		CalculateSize(aSz);

		typename T_Writer aWr;
		void * aBuf = theBuf;

		/*0*/   aBuf = aWr.AdvanceWrite( aBuf, &aSz,            SizeDescriptorSize );

		/*1*/   aBuf = aWr.AdvanceWrite( aBuf, &myActivatorWnd, sizeof HWND  );

		/*2*/   aBuf = aWr.AdvanceWrite( aBuf,  myFolderPidl,   aSz.Sizes[0] );

		/*3*/   aBuf = aWr.AdvanceWrite( aBuf,  myChildPidl,    aSz.Sizes[1] );

		return  (size_t)( (char *)aBuf - theBuf);
	}

	size_t ReadFromMemoryBuffer(char * theBuf)
	{
		char * aBuf_Temp = theBuf;

		/*0*/  SizeDescriptor * aSz   = (SizeDescriptor *)aBuf_Temp;
		aBuf_Temp += SizeDescriptorSize;

		/*1*/  myActivatorWnd         =     * (HWND *)aBuf_Temp;
		aBuf_Temp += sizeof(HWND);

		/*2*/  myFolderPidl           = (LPITEMIDLIST)aBuf_Temp;
		aBuf_Temp += aSz->Sizes[0];

		/*3*/  myChildPidl            = (LPITEMIDLIST)aBuf_Temp;
		aBuf_Temp += aSz->Sizes[1];


		// Data ready

		IShellFolder * aSF_Desk = 0;
		HRESULT aRes = SHGetDesktopFolder(&aSF_Desk);

		if (myFolderPidl->mkid.cb == 0)
			myShellFolder = aSF_Desk;	// do not release
		else
		{
			aRes = aSF_Desk->BindToObject(myFolderPidl, NULL, IID_IShellFolder, (void **) &myShellFolder);
			aSF_Desk->Release();
		}


		if ( SUCCEEDED(aRes) )
			return (size_t)(aBuf_Temp - theBuf);
		else
			return 0;
	}

	void CalculateSize (SizeDescriptor & theSize) const
	{
		theSize.Sizes[0] = UtilPidlFunc::PidlSize(myFolderPidl);
		theSize.Sizes[1] = UtilPidlFunc::PidlSize(myChildPidl);
	}

	template<typename T_Writer>
	HRESULT ExecuteCM()
	{
		IShellBrowser * aSB = (IShellBrowser *) SendMessage(myActivatorWnd, WM_USER+7, 0, 0);

		if (aSB == 0)
			return E_FAIL;

		LPITEMIDLIST aAbsolute = UtilPidlFunc::PidlAppend<T_Writer>(myFolderPidl, myChildPidl);
		HRESULT aRes = aSB->BrowseObject(aAbsolute, SBSP_SAMEBROWSER|SBSP_ABSOLUTE);
		CoTaskMemFree(aAbsolute);

		return aRes;
	}

	static const TCHAR * GetMemoryMapName()
		{ return File_IpcExchange; }
};

#pragma once

#include "sdk_ItemProvider.h"

#include "ShellItem.h"

//////////////////////////////////////////////////////////////////////////////////////////////

class ShellFolder :
	public ShellItem,
	public IItemProvider
{
protected:
	IShellFolder * mySF;   //<! represents the CURRENT folder as opposed to ShellItem::myParentSF

// Constructors, destructor
public:

	//! Creates an object instance by the Shell Item ID List (Pidl) and
	//! an optional pointer to the Shell Folder that owns the Pidl.
	ShellFolder(LPITEMIDLIST theInitPIDL, IShellFolder * theParentSF = NULL);

	//! Creates an object instance representing a file system folder
	//! \param [in] thePath   The file system path to the folder.
	ShellFolder(const TCHAR * thePath);

  //! Creates an object instance representing a special shell folder,
	//! like the Control Panel and Start Menu folder.
	//! \param [in] theCSIDL   The code of the special shell folder.
	ShellFolder(int theCSIDL);

	//! Destroys the object instance.
	virtual ~ShellFolder();


protected:
	bool IsFolder() const { return true; }


// IItemProvider members
protected:
	STDMETHODIMP EnumItems(ULONG theOptions, IEnumItems ** theOutEnum);


// IUnknown members
protected:
	ULONG STDMETHODCALLTYPE AddRef();
	ULONG STDMETHODCALLTYPE Release();
	STDMETHODIMP QueryInterface(REFIID theIID,  void ** theOut);


// Implementation Details
private:
	void InitShellFolder();      //!< initializes mySF

/*
protected:
	class ShellEnumThread : public Thread
	{
		HRESULT myRetVal;
		IShellFolder * mySF;
		DWORD myFlags;
		HWND myWnd;
		IEnumIDList ** myOut;


	public:
		HRESULT BeginEnum(IShellFolder * theSF, HWND theWnd, DWORD theFlags, IEnumIDList ** theOut)
		{
			mySF = theSF;
			myWnd = theWnd;
			myFlags = theFlags;
			myOut = theOut;

			Thread::Run(this);
			WaitForSingleObject(myHandle, INFINITE);
			return myRetVal;
		}


	protected:
		virtual DWORD Run()
		{
			HRESULT aRes =  mySF->EnumObjects(myWnd, myFlags, myOut);
			InterlockedExchange(&myRetVal, aRes);

			return 0;
		}

		virtual void Terminate()
		{
		}
	};	//end class ShellEnumThread
	*/
};	//end class ShellFolder

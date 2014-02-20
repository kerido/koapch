#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////

#include <shlobj.h>

#include "sdk_ContextMenuExtension.h"
#include "sdk_Item.h"

#include "PluginBase.h"

//////////////////////////////////////////////////////////////////////////////////////////////

void * operator new(size_t theSize) throw();

void operator delete(void * thePtr) throw();

//////////////////////////////////////////////////////////////////////////////////////////////

class DECLSPEC_NOVTABLE ClipboardAssistantPlugin :
	public ComEntry3<IApproachPlugin, IContextMenuExtension, IClassFactory>
{
private:
	HINSTANCE myInstance;    //!< TODO
	WNDCLASS myClass;        //!< TODO
	HWND myWindow;           //!< TODO
	HGLOBAL myMemory;        //!< TODO
	size_t myCurSize;        //!< TODO


public:
	ClipboardAssistantPlugin(HINSTANCE theDllInstance);

	~ClipboardAssistantPlugin();


public:
	HINSTANCE GetInstance() const
		{ return myInstance; }
	
	HGLOBAL EnsureMemoryHandle(size_t theNewSize);

	HWND GetHandlerWindow() const;


// IApproachPlugin Members
protected:
	STDMETHODIMP OnLoad(int theFrameworkVersionCode);
	STDMETHODIMP OnUnload();


// IContextMenuExtension Members
protected:
	STDMETHODIMP QueryExecutor(Item * theItem, ULONG theExt1, ULONG theExt2, IContextMenuExecutor ** theOut);


// IClassFactory Members
protected:
	STDMETHODIMP CreateInstance(IUnknown * theOuter, REFIID theIID, void ** theOut);

	STDMETHODIMP LockServer(BOOL theLock);


// IRuntimeObject Members
protected:
	STDMETHODIMP GetTypeGuid(LPGUID theOutGuid);


// Implementation Details
private:
	void InitWindow();
	static LRESULT __stdcall WndProc(HWND theWnd, UINT theMsg, WPARAM theWParam, LPARAM theLParam);
};

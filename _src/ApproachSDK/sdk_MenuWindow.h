//(c) 2005 Kirill Osipov

#pragma once

#include "sdk_Theme.h"
#include "sdk_ComObject.h"
#include "Trace.h"

class MenuWindow;

//////////////////////////////////////////////////////////////////////////////////////////////

MIDL_INTERFACE("CC21334B-1D18-497F-9FFE-7AF7BF929849") IMenuManagerAgent : public IUnknown
{
	struct MenuData
	{
		WNDPROC   WndProc;
		LPCTSTR   WindowClassName;
		HINSTANCE ModuleInstance;
	};

	//! TODO:
	STDMETHOD (GetMenuData)        (MenuData * theOutData) = 0;

	//! TODO
	STDMETHOD (CreateMenuWindow) (MenuWindow * theObj, HWND * theOutHwnd) = 0;

	//! TODO:
	STDMETHOD (DestroyMenuChain) (MenuWindow * theStartFrom) = 0;
};

///////////////////////////////////////////////////////////////////////////////////////////////

MIDL_INTERFACE("5C49E217-D61F-4A06-B37A-D3AD81B003D0") IMenuKeyboardNav : public IUnknown
{
	enum NavKeyType
	{
		KEY_NEXT = 1,
		KEY_PREV
	};

	//! TODO:
	STDMETHOD (GetKeyCode) (int theNavKeyType, UINT * theOutCode) = 0;
};

///////////////////////////////////////////////////////////////////////////////////////////////


//! Represents a KO Approach menu that behaves similarly to a standard menu in Windows.
//! MenuWindow is implemented as a plain window, not a menu.
class MenuWindow : public IUnknown
{
// Fields
protected:
	HWND myHandle;            //!< Stores the Win32 window handle representing the current object.
	Theme * myTheme;          //!< Stores a pointer to the theme associated with the current object.
	ComRefCount<> myRefCount; //!< Stores the number of references to the current object.
	MenuWindow * myParent;    //!< Stores the parent window.
	MenuWindow * myChild;     //!< Stores the child window.
	int myDirection;          //!< Stores direction of window appearance relatively to its parent:
	                          //!  - A positive value corresponds to left-to-right direction.
	                          //!  - A positive value corresponds to right-to-left direction.
	                          //!  - If the window has no parent, zero value is used and positioning
	                          //!    is controlled by global layout.


// Constructors, Destructor
public:
	MenuWindow(IMenuManagerAgent * theAgent)
		: myTheme(0), myChild(0), myParent(0), myDirection(0)
	{
		theAgent->CreateMenuWindow(this, &myHandle);
		Trace("MenuWindow::MenuWindow. HWND = %x\n", GetHandle() );
	}

	virtual ~MenuWindow()                          { DestroyWindow(myHandle); }

//properties
public:
	HWND GetHandle() const                         { return myHandle; }

	virtual void SetTheme(Theme * theTheme)        { myTheme = theTheme; }
	Theme * GetTheme() const                       { return myTheme; }

	virtual void SetChild(MenuWindow * theWnd)     { myChild = theWnd; }
	MenuWindow * GetChild() const                  { return myChild; }

	virtual void SetParent(MenuWindow * theWnd)    { myParent = theWnd; }
	MenuWindow * GetParent() const                 { return myParent; }

	virtual void SetDirection(int theDir)          { myDirection = theDir; }
	int GetDirection() const                       { return myDirection; }

	//! The window procedure for MenuWindow class hierarchy is a virtual method.
	//! Every derived class typically overrides this method to implement a functionality
	virtual LRESULT WndProc(HWND theWnd, UINT theMsg, WPARAM theWParam, LPARAM theLParam)
		{ return DefWindowProc(theWnd, theMsg, theWParam, theLParam); }


//IUnknown members
public:
	virtual STDMETHODIMP QueryInterface(REFIID theIID, void ** theOut)
		{ return E_NOTIMPL; }

	virtual ULONG __stdcall Release()
		{ return myRefCount.Release(this); }

	virtual ULONG __stdcall AddRef()
		{ return myRefCount.AddRef(); }
};
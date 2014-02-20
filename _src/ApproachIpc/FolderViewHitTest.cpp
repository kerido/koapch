#include "Common.h"
#include "FolderViewHitTest.h"

//////////////////////////////////////////////////////////////////////////////////////////////

class ShellFolderViewHitTestImpl_ListView
{
	friend class ShellFolderViewHitTest;

private:
	struct LVHITTESTINFOEX
	{
		LVHITTESTINFO  i;
		DWORD          s[4];	//OSSPECIFIC: Win2000 dummy data. Without it ListView_HitTest crashes 
	};

private:
	static int HitTestItem(HWND theOriginator, int theX, int theY)
	{
		LVHITTESTINFOEX aInfo;
		aInfo.i.pt.x = theX;
		aInfo.i.pt.y = theY;

		MapWindowPoints(NULL, theOriginator, &aInfo.i.pt, 1);

		return ListView_HitTest(theOriginator, &aInfo.i);
	}
};

//////////////////////////////////////////////////////////////////////////////////////////////

class ShellFolderViewHitTestImpl_DirectUiView
{
	friend class ShellFolderViewHitTest;

private:
	MIDL_INTERFACE("DEE61571-6F83-4A7B-B8C0-CA2CA6F76BF8") IHitTestView : public IUnknown
	{
		STDMETHOD (HitTestItem) (POINT thePt, int * theOutItemIndex) = 0;

		//! Not used
		STDMETHOD (GetItemRect) (int theItemIndex, RECT * theOutItemRect) = 0;

		//! Not used
		STDMETHOD (MapRect)     (int, HWND *,RECT *) = 0;
	};


private:
	static int HitTestItem(HWND theOriginator, int theX, int theY)
	{
		// aParent has the SHELLDLL_DefView class
		HWND aParent = GetParent(theOriginator);

		IShellView * aSV = (IShellView *)GetWindowLongPtr(aParent, 0);

		if (aSV == 0)
			return - 1;

		IHitTestView * aHTV = 0;
		HRESULT aRes = aSV->QueryInterface( __uuidof(IHitTestView), (void **) &aHTV);

		if ( FAILED (aRes) || aHTV == 0)
			return -1;

		POINT aPt;
		aPt.x = theX;
		aPt.y = theY;

		// Map the point from screen coordinates to the coordinates of the DirectUI window
		// represented by the theOriginator handle. It is essential to map to the coordinates
		// of the view implementation (but not the Shell View window) because the DirectUI window
		// does not receive the WS_EX_LAYOUTRTL style when the system has Hebrew or Arabic
		// interface. The IHitTestView interface is aware of the situation and always performs
		// hit-testing based on LTR layout and non-mirrored coordinates.
		MapWindowPoints(NULL, theOriginator, &aPt, 1);

		int aItemIndex = -1;
		aRes = aHTV->HitTestItem(aPt, &aItemIndex);

		aHTV->Release();

		return aItemIndex;
	}
};

//////////////////////////////////////////////////////////////////////////////////////////////

ShellFolderViewHitTest::HitTestFunc ShellFolderViewHitTest::GetHitTestFunc(HWND theOriginator)
{
	TCHAR aTempWndClassName[256];
	GetClassName(theOriginator, aTempWndClassName, 256);

	if ( lstrcmp(aTempWndClassName, _T("SysListView32") ) == 0)
		return ShellFolderViewHitTestImpl_ListView::HitTestItem;

	else if ( lstrcmp(aTempWndClassName, _T("DirectUIHWND") ) == 0)
		return ShellFolderViewHitTestImpl_DirectUiView::HitTestItem;

	else
		return 0;
}
#pragma once

/////////////////////////////////////////////////////////////////////////////

#include "sdk_IconExtractor.h"
#include "sdk_Selectable.h"
#include "sdk_ComObject.h"

#include "DisplayItem.h"

interface IMenuBuilder;

/////////////////////////////////////////////////////////////////////////////

class DynamicDisplayItem :
	public DisplayItem,
	public ComEntry3<ISelectable, IIconAcceptor, IDisplayItemGetName>
{
private:
	const static DWORD DrawItemTextDefaultFlags = DT_EDITCONTROL|DT_NOPREFIX|DT_LEFT|DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS;

protected:
	ComRefCount<>  myNumRefs;
	Item         * myItem;
	IMenuBuilder * myMB;
	ItemIconData   myIconData;

//constructors, destructor
public:

	DynamicDisplayItem(Item * theItem = 0);

	virtual ~DynamicDisplayItem();


// DisplayItem members
public:
	virtual void Measure (HDC theDC, ItemMeasure & theMeasure, const Theme * theTheme);
	virtual void Draw(const ItemDrawData * theData, const class Theme * theTheme);

	virtual Item * GetLogicalItem() const { return myItem; }
	virtual void OnEvent(EventType theEvent, class MenuWindow * theHost);


// ISelectable members
protected:
	STDMETHODIMP OnSelect(ULONG theFlags);


// IDisplayItemGetName members
protected:
	STDMETHODIMP GetDisplayString(TCHAR * theOutBuf, int * theSize);


// IIconAcceptor members
protected:
	STDMETHODIMP RetrieveIconData(IconExtractParams * theParams, void ** theOutAsyncArg);
	STDMETHODIMP RetrieveIconDataAsync(IconExtractParams * theParams, void * theAsyncArg);
	STDMETHODIMP ReleaseAsyncArg(void * theAsyncArg);


// IUnknown members
public:

	ULONG STDMETHODCALLTYPE AddRef();

	ULONG STDMETHODCALLTYPE Release();

	STDMETHODIMP QueryInterface(REFIID theIID, void ** theOut);


// Implementation details
protected:
	void DrawFull(HDC theDC, bool theSelected, const RECT & theContainer, const Theme * theTheme);
	void DrawIconOnly(HDC theDC, bool theSelected, const RECT & theContainer, const Theme * theTheme);

	void DrawBackground(HDC theDC, bool theSelected, const RECT & theContainer, const Theme * theTheme);
	void DrawText(HDC theDC, bool theSelected, const RECT & theContainer, const Theme * theTheme);
	void DrawIcon(HDC theDC, bool theSelected, const RECT & theContainer, const Theme * theTheme);
	void DrawArrow(HDC theDC, bool theSelected, const RECT & theContainer, const Theme * theTheme);

private:
	HRESULT UpdateIconData(int theCode, const ItemIconData * theData);
};

#pragma once

#include "Preferences.h"
#include "Trace.h"

//////////////////////////////////////////////////////////////////////////////////////////////

class ApplicationSettingsRegistrySerializer
{
public:
	bool Load(ApplicationSettings & theSettings)
	{
		return theSettings.Load();
	}

	bool Save(const ApplicationSettings & theSettings)
	{
		return theSettings.Save();
	}
};

//////////////////////////////////////////////////////////////////////////////////////////////

/*
class PreferencesXmlSerializer
{
private:
	typedef std::list<IPrefsPersistenceHandler *>     PersistenceHandlerList;
	typedef PersistenceHandlerList::iterator          PersistenceHandlerIter;
	typedef PersistenceHandlerList::const_iterator    PersistenceHandlerIterC;

private:
	PersistenceHandlerList myConsumers;


public:
	~PreferencesXmlSerializer()
	{
		ATLASSERT(myConsumers.size() == 0);
	}

public:
	HRESULT Load(const wchar_t * theFilePath)
	{
		try
		{
			MSXML2::IXMLDOMDocumentPtr aDoc ( __uuidof(MSXML2::DOMDocument30) );

			if (aDoc == 0)
			{
				Trace("Failed to create XML DOM object\n");
				return E_POINTER;
			}

			Trace("XML DOM object created successfully\n");

			BOOL aRet = aDoc->load(theFilePath);

			if ( !aRet )
			{
				Trace("Failed to load the document\n");
				return E_ACCESSDENIED;
			}

			Trace("Document loaded successfully\n");

			for (PersistenceHandlerIter aIt = myConsumers.begin(); aIt != myConsumers.end(); aIt++)
			{
				IPrefsPersistenceHandler * aHandler = (*aIt);

				HRESULT aRes = aHandler->LoadPrefs(aDoc);

				if ( FAILED(aRes) )
				{
					aRes = aHandler->ResetPrefs();
					aRes = aHandler->SavePrefs(aDoc);
				}
			}

			Trace("Consumed preferences successfully\n");
		}
		catch (_com_error & theError)
		{
			return theError.Error();
		}
		catch(...)
		{
			return E_FAIL;
		}

		return S_OK;
	}

	HRESULT Save(const wchar_t * theFilePath)
	{
		MSXML2::IXMLDOMDocumentPtr aDoc( __uuidof(MSXML2::DOMDocument30) );

		if (aDoc == 0)
		{
			Trace("Failed to create XML DOM object\n");
			return E_POINTER;
		}
		aDoc->preserveWhiteSpace = VARIANT_TRUE;

		// Create a processing instruction targeted for xml
		MSXML2::IXMLDOMProcessingInstructionPtr aXml = aDoc->createProcessingInstruction("xml", "version='1.0'");
		aDoc->appendChild(aXml);

		// Create the root element
		MSXML2::IXMLDOMElementPtr aRoot = aDoc->createElement("Approach_Preferences");

		// Create a "version" attribute for the root element
		MSXML2::IXMLDOMAttributePtr aAttrVersion = aDoc->createAttribute("version");
		aAttrVersion->value = "1";
		aRoot->setAttributeNode(aAttrVersion);

		// Add the root element to the DOM instance.
		aDoc->appendChild(aRoot);

		for (PersistenceHandlerIter aIt = myConsumers.begin(); aIt != myConsumers.end(); aIt++)
		{
			IPrefsPersistenceHandler * aHandler = (*aIt);

			HRESULT aRes = aHandler->SavePrefs(aDoc);
		}
		
		aDoc->save(theFilePath);

		return 0;
	}

public:
	void AddHandler(IPrefsPersistenceHandler * theHandler)
	{
		myConsumers.push_back(theHandler);
	}

	void RemoveHandler(IPrefsPersistenceHandler * theHandler)
	{

	}

};
*/


/*
#import <msxml3.dll>	// use XML parser's classes
//NOTE: maybe this also must be a COM interface for the sake of standardization
class DECLSPEC_NOVTABLE IPrefsPersistenceHandler
{
public:
/// <summary>
///    Loads the preferences from the specified XML document
/// </summary>
/// <returns>
///    S_OK    - preferences were loaded successfully
///    E_*     - preferences could not be loaded due to an error
/// </returns>
virtual HRESULT LoadPrefs(MSXML2::IXMLDOMDocument * theDoc) = 0;


/// <summary>
///    Saves the preferences to the specified XML document
/// </summary>
/// <returns>
///    S_OK    - preferences were saved successfully
///    S_FALSE - preferences were not saved because they are identical
///    E_*     - preferences could not be saved due to an error
/// </returns>
virtual HRESULT SavePrefs(MSXML2::IXMLDOMDocument * theDoc) = 0;


/// <summary>
///    Resets the internal state of the object
/// </summary>
/// <returns>
///    S_OK    - preferences were reset successfully
///    E_*     - preferences could not be reset due to an error
/// </returns>
virtual HRESULT ResetPrefs() = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////////




// IPrefsPersistenceHandler members
protected:
virtual HRESULT LoadPrefs(MSXML2::IXMLDOMDocument * theDoc);

virtual HRESULT SavePrefs(MSXML2::IXMLDOMDocument * theDoc);

virtual HRESULT ResetPrefs();





//////////////////////////////////////////////////////////////////////////////////////////////

HRESULT PreferencesBase::LoadPrefs(MSXML2::IXMLDOMDocument * theDoc)
{
MSXML2::IXMLDOMNodePtr aGlobal = theDoc->selectSingleNode( _T("//Approach_Preferences/Global") ) ;

if (aGlobal == 0)
return E_FAIL;


// UILocale
{
_bstr_t aLocale = XmlUtility::GetAttributeValue(aGlobal, L"UiLocale", L"value");
if (aLocale.length() == 0)
return E_FAIL;

lstrcpynW(myLocale, aLocale, 6);
}


// ZipArchivesAreFolders
{
_bstr_t aZipsAreFolders = XmlUtility::GetAttributeValue(aGlobal, L"ZipArchivesAreFolders", L"value");
if (aZipsAreFolders.length() == 0)
return E_FAIL;

myZipsAreFolders = _wtoi(aZipsAreFolders);
}


// MaxMenuWidth
{
_bstr_t aMaxMenuWidth = XmlUtility::GetAttributeValue(aGlobal, L"MaxMenuWidth", L"value");
if (aMaxMenuWidth.length() == 0)
return E_FAIL;

const wchar_t * aPercent = wcsstr( (const wchar_t *)aMaxMenuWidth, L"%");

if (aPercent)
myMaxMenuWidth = -_wtoi(aMaxMenuWidth);
else
myMaxMenuWidth = _wtoi(aMaxMenuWidth);
}


// ItemOrder : sorting
{
_bstr_t aSorting = XmlUtility::GetAttributeValue(aGlobal, L"ItemOrder", L"sorting");
if (aSorting.length() == 0)
return E_FAIL;

if ( aSorting.operator == (L"1") )
myOrdering |= ORDER_ALPHABETICAL;
}


// ItemOrder : folders_on_top
{
_bstr_t aFoldersOnTop = XmlUtility::GetAttributeValue(aGlobal, L"ItemOrder", L"folders_on_top");
if (aFoldersOnTop.length() == 0)
return E_FAIL;

if ( aFoldersOnTop.operator == (L"1") )
myOrdering |= ORDER_FOLDERS_ONTOP;
}


// HiddenFilesDisplayMode
{
_bstr_t aHiddenFilesDisplayMode = XmlUtility::GetAttributeValue(aGlobal, L"HiddenFilesDisplayMode", L"value");
if (aHiddenFilesDisplayMode.length() == 0)
return E_FAIL;

myHiddenFilesMode = HiddenFilesMode_Binding::val( aHiddenFilesDisplayMode );
}


// BrowseFoldersMode
{
_bstr_t aBrowseFoldersMode = XmlUtility::GetAttributeValue(aGlobal, L"BrowseFoldersMode", L"value");
if (aBrowseFoldersMode.length() == 0)
return E_FAIL;

myBrowseInNewWindow = BrowseMode_Binding::val( aBrowseFoldersMode );
}


// Icons : type



// Icons : display_overlays



// ChildMenuInterval
{
_bstr_t aChildMenuInterval = XmlUtility::GetAttributeValue(aGlobal, L"ChildMenuInterval", L"value");
if (aChildMenuInterval.length() == 0)
return E_FAIL;

myTimeoutSec = _wtoi(aChildMenuInterval);
}

MSXML2::IXMLDOMNodePtr aScroll = aGlobal->selectSingleNode( _T("Scrolling") ) ;

if (aScroll == 0)
return E_FAIL;


// Scrolling : Interval
{
_bstr_t aScrollInterval = XmlUtility::GetAttributeValue(aScroll, L"Interval", L"value");
if (aScrollInterval.length() == 0)
return E_FAIL;

myTimeoutScroll = _wtoi(aScrollInterval);
}


// Scrolling : ScrollItemPositioning
{
_bstr_t aScrollItemPos = XmlUtility::GetAttributeValue(aScroll, L"ScrollItemPositioning", L"value");
if (aScrollItemPos.length() == 0)
return E_FAIL;

//TEMP: make a valid flag
PrefsSerz::ScrollItemPos aSer = ScrollItemPos_Binding::val( aScrollItemPos );
}


// Scrolling : ScrollAmount : mouse_wheel
{
_bstr_t aNumScrollWheel = XmlUtility::GetAttributeValue(aScroll, L"ScrollAmount", L"mouse_wheel");
if (aNumScrollWheel.length() == 0)
return E_FAIL;

myScrollWheel = _wtoi(aNumScrollWheel);
}


// Scrolling : ScrollAmount : page_keys
{
_bstr_t aNumScrollPage = XmlUtility::GetAttributeValue(aScroll, L"ScrollAmount", L"page_keys");
if (aNumScrollPage.length() == 0)
return E_FAIL;

myScrollPage = _wtoi(aNumScrollPage);
}



return S_OK;
}

//////////////////////////////////////////////////////////////////////////////////////////////

HRESULT PreferencesBase::SavePrefs(MSXML2::IXMLDOMDocument * theDoc)
{
MSXML2::IXMLDOMNodePtr aRoot = theDoc->selectSingleNode( _T("//Approach_Preferences") ) ;

// Global node
MSXML2::IXMLDOMElementPtr aGlobal = theDoc->createElement("Global");
aRoot->appendChild(aGlobal);


// UILocale
{
XmlUtility::CreateElementWithAttr(aGlobal, L"UILocale", L"value", myLocale);
}


// ZipArchivesAreFolders
{
wchar_t aBuf[32];
_itow(myZipsAreFolders, aBuf, 10);

XmlUtility::CreateElementWithAttr(aGlobal, L"ZipArchivesAreFolders", L"value", aBuf);
}


// MaxMenuWidth
{
wchar_t aBuf[32];
if ( myMaxMenuWidth < 0)
{
_itow(-myMaxMenuWidth, aBuf, 10);
lstrcatW(aBuf, L"%");
}
else
{
_itow(myMaxMenuWidth, aBuf, 10);
lstrcatW(aBuf, L"px");
}

XmlUtility::CreateElementWithAttr(aGlobal, L"MaxMenuWidth", L"value", aBuf);
}


// ItemOrder
{
MSXML2::IXMLDOMElementPtr aElem = XmlUtility::CreateElementWithAttr(aGlobal, L"ItemOrder", L"sorting",
(myOrdering & ORDER_ALPHABETICAL) == 0 ? L"0" : L"1");

XmlUtility::AddAttribute(aElem, L"folders_on_top",
(myOrdering & ORDER_FOLDERS_ONTOP) == 0 ? L"0" : L"1");
}


// HiddenFilesDisplayMode
{
_bstr_t aValue = HiddenFilesMode_Binding::sdisplay( (HiddenFilesMode) myHiddenFilesMode);
XmlUtility::CreateElementWithAttr(aGlobal, L"HiddenFilesDisplayMode", L"value", aValue);
}


// BrowseFoldersMode
{
_bstr_t aValue = BrowseMode_Binding::sdisplay( (BrowseMode) myBrowseInNewWindow);
XmlUtility::CreateElementWithAttr(aGlobal, L"HiddenFilesDisplayMode", L"value", aValue);
}


// Icons : type



// Icons : display_overlays



// ChildMenuInterval
{
wchar_t aBuf[32];
_itow(myTimeoutSec, aBuf, 10);

XmlUtility::CreateElementWithAttr(aGlobal, L"ChildMenuInterval", L"value", aBuf);
}


// Scrolling node
MSXML2::IXMLDOMElementPtr aScroll = theDoc->createElement("Scrolling");
aGlobal->appendChild(aScroll);

if (aScroll == 0)
return E_FAIL;


// Scrolling : Interval
{
wchar_t aBuf[32];
_itow(myTimeoutScroll, aBuf, 10);

XmlUtility::CreateElementWithAttr(aScroll, L"Interval", L"value", aBuf);
}


// Scrolling : ScrollItemPositioning
{
_bstr_t aValue = ScrollItemPos_Binding::sdisplay(myScrollItemPos);
XmlUtility::CreateElementWithAttr(aScroll, L"ScrollItemPositioning", L"value", aValue);
}


// Scrolling : ScrollAmount
{
wchar_t aBuf[32];
_itow(myScrollWheel, aBuf, 10);

MSXML2::IXMLDOMElementPtr aElem = XmlUtility::CreateElementWithAttr(aScroll, L"ScrollAmount", L"mouse_wheel", aBuf);


_itow(myScrollPage, aBuf, 10);

XmlUtility::AddAttribute(aElem, L"page_keys", aBuf);
}

return S_OK;
}

//////////////////////////////////////////////////////////////////////////////////////////////

HRESULT PreferencesBase::ResetPrefs()
{
return E_NOTIMPL;
}


//////////////////////////////////////////////////////////////////////////////////////////////

// enum bindings

BIND_ENUM(PrefsSerz::BrowseMode, 3, BrowseMode_Binding)
{
{ PrefsSerz::BROWSE_USESYSTEM,      _T("BROWSE_USESYSTEM")       },
{ PrefsSerz::BROWSE_EXISTINGWINDOW, _T("BROWSE_EXISTINGWINDOW")  },
{ PrefsSerz::BROWSE_NEWWINDOW,      _T("BROWSE_NEWWINDOW")       }
};

BIND_ENUM(PrefsSerz::HiddenFilesMode, 3, HiddenFilesMode_Binding)
{
{ PrefsSerz::HIDDEN_USESYSTEM,   _T("HIDDEN_USESYSTEM")  },
{ PrefsSerz::HIDDEN_SHOWALWAYS,  _T("HIDDEN_SHOWALWAYS") },
{ PrefsSerz::HIDDEN_SHOWNEVER,   _T("HIDDEN_SHOWNEVER")  }
};

BIND_ENUM(PrefsSerz::ScrollItemPos, 3, ScrollItemPos_Binding)
{
{ PrefsSerz::SCROLLITEMPOS_RESPECTIVE,  _T("SCROLLITEMPOS_RESPECTIVE") },
{ PrefsSerz::SCROLLITEMPOS_TOP,         _T("SCROLLITEMPOS_TOP") },
{ PrefsSerz::SCROLLITEMPOS_BOTTOM,      _T("SCROLLITEMPOS_BOTTOM") },
};

BIND_ENUM(PrefsSerz::DataType, 2, PrefsSerzDataType_Binding)
{
{ 0,  _T("0") },
{ 1,  _T("1") },
};


class XmlUtility
{
public:
static _bstr_t GetAttributeValue(MSXML2::IXMLDOMNode * theRoot, const BSTR theTag, const BSTR theAttr)
{
MSXML2::IXMLDOMNodePtr aRootPtr = theRoot;

// UILocale
MSXML2::IXMLDOMNodePtr aNode = aRootPtr->selectSingleNode(theTag);

if (aNode == 0)
return bstr_t();

MSXML2::IXMLDOMAttributePtr aAttr = aNode->attributes->getNamedItem(theAttr);

if (aAttr == 0)
return bstr_t();
else
return aAttr->value.bstrVal;
}

static MSXML2::IXMLDOMElementPtr CreateElementWithAttr( MSXML2::IXMLDOMNode * theRoot, const BSTR theElemName,
const BSTR theAttrName, const BSTR theAttrValue)
{
MSXML2::IXMLDOMElementPtr aElem = theRoot->ownerDocument->createElement(theElemName);
theRoot->appendChild(aElem);

AddAttribute(aElem, theAttrName, theAttrValue);

return aElem;
}

static void AddAttribute(MSXML2::IXMLDOMElement * theNode, const BSTR theAttrName, const BSTR theAttrValue)
{
MSXML2::IXMLDOMAttributePtr aAttr = theNode->ownerDocument->createAttribute(theAttrName);
aAttr->value = theAttrValue;
theNode->setAttributeNode(aAttr);
}
};



*/
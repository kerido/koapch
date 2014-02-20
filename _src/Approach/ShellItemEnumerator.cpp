#include "stdafx.h"

#include "ShellItemEnumerator.h"
#include "Framework.h"
#include "Application.h"
#include "ShellItemFactory.h"
#include "ShellItem.h"

#include "UtilShellFunc.h"

//////////////////////////////////////////////////////////////////////////////////////////////

//! \brief Enumerates Shell Items.
//! 
//! Instances of this class are created by ShellFolder and ShellItemList.
class ShellItemEnumerator : public ComEntry1<IEnumItems>
{
private:
	typedef std::vector<ShellItem *>  PidlVect;
	typedef PidlVect::iterator        PidlIter;
	typedef PidlVect::const_iterator  PidlIterC;


private:
	//! Compares Shell Items by their Pidls.
	class ShellItemComparer
	{
	private:
		IShellFolder * mySF;


	public:
		ShellItemComparer(IShellFolder * theSF) : mySF(theSF) { }


	public:
		//! Compares the Pidls of two ShellItem instances when sorting the item list.
		//! \param [in] theItem1   The first ShellItem instance.
		//! \param [in] theItem2   The second ShellItem instance.
		//! \return                True if the Pidl in \a theItem1 is less than the Pidl in \a theItem2; otherwise, false.
		//! \remarks               It is assumed that the two items belong to one folder.
		bool operator () (const ShellItem * theItem1, const ShellItem * theItem2) const
		{
			HRESULT aRes = mySF->CompareIDs(SHCIDS_CANONICALONLY, theItem1->GetPidl(), theItem2->GetPidl());

			ATLASSERT( SUCCEEDED(aRes) );

			if ( FAILED(aRes) )
				return false;

			short aCode = HRESULT_CODE(aRes);

			return aCode < 0;
		}
	};

// IUnknown Members
protected:
	ComRefCount<> myRefs;         //!< Stores the number of references to this object;
	IEnumIDList * myEnum;         //!< Pointer to the Pidl enumeration object used as back-end for item enumeration.
	IShellFolder * mySF;          //!< Pointer to the Shell Folder being enumerated.
	IShellItemFactory * myFact;   //!< Pointer to the active Shell Item factory.
	ULONG myFlags;                //!< Stores enumeration options.
	PidlVect myFldrs;             //!< Contains Pidls in order in which they will be enumerated.
	PidlVect myFiles;             //!< Contains a temporary list of Pidls of non-folder items when folders
	                              //!  are instructed to appear on top.
	PidlVect * myEffective;       //! Points to the actual list that will be enumerated.
	PidlIterC myCur;              //!< Points to an element in #myFldrs. Advances with each call to #Next.

public:
	ShellItemEnumerator(IShellFolder * theSF, IEnumIDList * theEnum, ULONG theFlags)
		: mySF(theSF), myEnum(theEnum), myFlags(theFlags)
	{
		myFact = Application::Instance().GetShellItemFactory();

		mySF->AddRef();
		myEnum->AddRef();


		if ( (myFlags & (ENUMOPTIONS_FOLDERS_ON_TOP|ENUMOPTIONS_FOLDERS_ON_BTM)) != 0)
			FillLists<true>();
		else
			FillLists<false>();


		if ( (myFlags & ENUMOPTIONS_SORT_ITEMS) != 0)
			SortLists();


		MergeLists( (myFlags & ENUMOPTIONS_FOLDERS_ON_BTM) == 0);
	}

	~ShellItemEnumerator()
	{
		myEnum->Release();
		mySF->Release();
	}

protected:
	//! Retrieves the desired number of Items and advances the iterator.
	//! \param [in]  theNumItems      The desired number of to be retrieved.
	//! \param [out] theOutItem       Pointer to the output array which will be filled with Items.
	//!                               Assumed to have minimum capacity of \a theNumItems.
	//! \param [out] theNumExtracted  Pointer to a value that will receive the actual number of Items retrieved.
	//! \return       S_OK if the exact desired number of Items was extracted.
	//!               S_FALSE if fewer items were retrieved than was desired and enumeration is finished.
	STDMETHODIMP Next(ULONG theNumItems, Item ** theOutItem, ULONG * theNumExtracted)
	{
		ULONG i = 0;

		for ( ; i < theNumItems && myCur != myEffective->end(); i++, myCur++)
			theOutItem[i] = *myCur;

		*theNumExtracted = i;
		return (i < theNumItems) ? S_FALSE : S_OK;
	}

	STDMETHODIMP Skip(ULONG theNumItems)
	{
		return myEnum->Skip(theNumItems);
	}

	STDMETHODIMP Reset()
	{
		return myEnum->Reset();
	}

	STDMETHODIMP Clone(IEnumItems ** theOut)
	{
		if (theOut == 0)
			return E_POINTER;

		IEnumIDList * aNew = 0;
		HRESULT aRes = myEnum->Clone(&aNew);

		if ( FAILED(aRes) )
			return aRes;

		*theOut = new ShellItemEnumerator(mySF, aNew, myFlags);
		aNew->Release();

		return S_OK;
	}

protected:
	ULONG STDMETHODCALLTYPE AddRef()
		{ return myRefs.AddRef(); }

	ULONG STDMETHODCALLTYPE Release()
		{ return myRefs.Release(this); }

	STDMETHODIMP QueryInterface(REFIID theIID, void ** theOut)
		{ return InternalQueryInterface(theIID, this, theOut); }


// Implementation Details
private:
	template<bool tPerformAttributeCheck>
	void FillLists()
	{
		while (true)
		{
			ULONG aNumFetched = 0;
			LPITEMIDLIST aChildPidl;

			HRESULT aRes = myEnum->Next(1, &aChildPidl, &aNumFetched);

			if (aChildPidl == 0 || aNumFetched == 0 || FAILED(aRes))
				break;

			ShellItem * aItem = myFact->CreateChild(mySF, aChildPidl);

			if (tPerformAttributeCheck)
			{
				if ( aItem->IsFolder() )
					myFldrs.push_back(aItem);
				else
					myFiles.push_back(aItem);
			}
			else
				myFldrs.push_back(aItem);
		}
	}

	void SortLists()
	{
		ShellItemComparer aC(mySF);

		if ( !myFldrs.empty() )
			std::sort(myFldrs.begin(), myFldrs.end(), aC);

		if ( !myFiles.empty() )
			std::sort(myFiles.begin(), myFiles.end(), aC);
	}

	void MergeLists(bool theFoldersOnTop)
	{
		if (theFoldersOnTop)
		{
			myFldrs.reserve( myFldrs.size() + myFiles.size() );

			for (PidlIterC aIt = myFiles.begin(); aIt != myFiles.end(); aIt++)
				myFldrs.push_back(*aIt);

			myFiles.clear();
			myEffective = &myFldrs;
		}
		else
		{
			myFiles.reserve( myFldrs.size() + myFiles.size() );

			for (PidlIterC aIt = myFldrs.begin(); aIt != myFldrs.end(); aIt++)
				myFiles.push_back(*aIt);

			myFldrs.clear();
			myEffective = &myFiles;
		}

		myCur = myEffective->begin();
	}
};

//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP ShellItemList::GetTypeGuid( GUID * theOut )
{
	if (theOut == 0)
		return E_POINTER;

	*theOut = OBJ_ShellItemList;
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP ShellItemList::EnumItems(const BYTE * theData, int theDataSize, ULONG theOptions, IEnumItems ** theOutEnum)
{
	if (theOutEnum == 0)
		return E_POINTER;

	if (theData == 0 || theDataSize != sizeof ShellItemProviderParam)
		return E_INVALIDARG;

	const ShellItemProviderParam * aPar = reinterpret_cast<const ShellItemProviderParam *>(theData);

	if(aPar->Magic != ShellItemProviderParam::SiPp)
		return E_INVALIDARG;

	if (aPar->ParamType == ShellItemProviderParam::FullPath)
	{
		CComPtr<IShellFolder> aSF;
		LPITEMIDLIST aPidl = 0;

		HRESULT aRes = UtilShellFunc::ParseDisplayName(aPar->Path, lstrlen(aPar->Path), &aSF, &aPidl);

		if ( FAILED(aRes) )
			return aRes;

		CComPtr<IShellFolder> aChildSF;
		aRes = aSF->BindToObject(aPidl, NULL, IID_IShellFolder, (void **) &aChildSF);

		CoTaskMemFree(aPidl);

		if ( FAILED(aRes) )
			return aRes;

		return Enum(aChildSF, theOptions, theOutEnum);
	}
	else if (aPar->ParamType == ShellItemProviderParam::FullPidl)
	{
		CComPtr<IShellFolder> aSF;
		HRESULT aRes = SHGetDesktopFolder(&aSF);

		if ( FAILED(aRes) )
			return aRes;

		CComPtr<IShellFolder> aChildSF;
		aRes = aSF->BindToObject(aPar->Pidl, NULL, IID_IShellFolder, (void **) &aChildSF);

		if ( FAILED(aRes) )
			return aRes;

		return Enum(aChildSF, theOptions, theOutEnum);
	}
	else
		return E_ABORT;
}

//////////////////////////////////////////////////////////////////////////////////////////////

DWORD ShellItemList::GetEnumFlags()
{
	DWORD aEnumFlags = SHCONTF_FOLDERS|SHCONTF_NONFOLDERS;

	if (Application::InstanceC().Prefs()->GetActualHiddenFilesMode() == PrefsSerz::HIDDEN_SHOWALWAYS )
		aEnumFlags |= SHCONTF_INCLUDEHIDDEN;

	return aEnumFlags;
}

//////////////////////////////////////////////////////////////////////////////////////////////

HRESULT ShellItemList::Enum(IShellFolder * theSF, ULONG theOptions, IEnumItems ** theOutEnum)
{
	if (theOutEnum == 0)
		return E_POINTER;

	IEnumIDList * aLst = 0;
	HRESULT aRes = theSF->EnumObjects(NULL, GetEnumFlags(), &aLst);

	if ( FAILED (aRes) )
		return aRes;

	else if (aLst == 0)
		return E_FAIL;

	*theOutEnum = new ShellItemEnumerator(theSF, aLst, theOptions);
	aLst->Release();

	return S_OK;
}
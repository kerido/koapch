#pragma once

#include <utility>

//////////////////////////////////////////////////////////////////////////////////////////////

//! \def SUPPORTED_CPP0X
//! When set to nonzero, C++0X code with variadic templates will be present.

#ifdef __GXX_EXPERIMENTAL_CXX0X__
#  define SUPPORTED_CPP0X 1
#else
#  define SUPPORTED_CPP0X 0
#endif //__GXX_EXPERIMENTAL_CXX0X__

//////////////////////////////////////////////////////////////////////////////////////////////

//! \def SUPPORTED_UUIDOF
//! When set to nonzero, assumes that the compiler can provide a built-in
//! __uuidof operator.

#ifdef _MSC_EXTENSIONS
#  define SUPPORTED_UUIDOF 1
#else
#  define SUPPORTED_UUIDOF 0
#endif //_MSC_EXTENSIONS

//////////////////////////////////////////////////////////////////////////////////////////////

#if (SUPPORTED_UUIDOF == 0)

template<typename t>
struct hold_uuidof { static GUID __IID; };

#  define __uuidof(Q) hold_uuidof<Q>::__IID
#  define DECLSPEC_NOVTABLE

#  define DEFINE_UUIDOF_ID(Q, IID) template<> GUID hold_uuidof<Q>::__IID = IID;
#  define DEFINE_UUIDOF(Q) DEFINE_UUIDOF_ID(Q, IID_##Q)

//TEMP
DEFINE_GUID(IID_IUnknown1,
            0x00000000, 0x0000, 0x0000, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46);

#endif	// (SUPPORTED_UUIDOF != 0)


//! Provides discovery information for a supported COM interface.
template<typename tInterface, LPCGUID tIID = &__uuidof(tInterface)>
class DECLSPEC_NOVTABLE ComEntryDiscoverable
{
public:
	typedef tInterface Interface;

protected:
	template<typename tObject>
	static HRESULT InternalQueryInterface(REFIID theIID, tObject * theObj, void ** theOut)
	{
		if (theIID == *tIID)
			*theOut = static_cast<tInterface *>(theObj);
		else
			return E_NOINTERFACE;

		return S_OK;
	}
};


//! Implements exactly one COM interface and provides discovery information for it.
template<typename tInterface, LPCGUID tIID = &__uuidof(tInterface)>
class DECLSPEC_NOVTABLE ComEntry : public tInterface, public ComEntryDiscoverable<tInterface, tIID>
{
};


//! Provides discovery information for a parent COM interface that is common
//! between two or more child COM interfaces being implemented.
//! 
//! \tparam tInterface     The type of the interface being discovered.
//! 
//! \tparam tIntermediate  The interface that is a child of \a tInterface that
//!                        can serve as a hint for resolving casting ambiguities.
//! 
//! \tparam tIID           The interface ID of \a tInterface
//! 
//! \remarks
//! This class is necessary for providing discovery information about IUnknown
//! since the latter is parent for all COM interfaces. In such cases where direct
//! cast to IUnknown is not possible, two-step casting is necessary.
template<typename tInterface, typename tIntermediate, LPCGUID tIID = &__uuidof(tInterface)>
class DECLSPEC_NOVTABLE ComEntry2StepDiscoverable
{
public:
	typedef tInterface Interface;

protected:
	template<typename tObject>
	static HRESULT InternalQueryInterface(REFIID theIID, tObject * theObj, void ** theOut)
	{
		if (theIID == *tIID)
			*theOut = static_cast<tInterface *>( static_cast<tIntermediate *>(theObj) );
		else
			return E_NOINTERFACE;

		return S_OK;
	}
};


//! Represents a terminating block for interface discovery, allowing for
//! discovery of IUnknown.
//! 
//! \tparam tIntermediate  Any interface that implements IUnknown.
template <typename tIntermediate>
class DECLSPEC_NOVTABLE ComEntryTerminator : public ComEntry2StepDiscoverable<IUnknown, tIntermediate>
{
};

//! Represents a compound COM entry that can implement n COM interfaces and
//! contain discovery information about (n+1) COM interfaces where the last interface
//! must always be IUnknown. In order to implement more than one COM interface,
//! this class can be daisy-chained in the \a tEntry2 template argument.
//! 
//! \tparam tEntry1        The first COM entry.
//! \tparam tEntry2        Second COM entry. If unneeded, a terminating entry
//!                        is used, providing discovery of IUnknown.
template <typename tEntry1, typename tEntry2 = ComEntryTerminator<typename tEntry1::Interface> >
class DECLSPEC_NOVTABLE ComEntryCompound : public tEntry1, public tEntry2
{
public:
	template<typename tObject>
	static HRESULT InternalQueryInterface(REFIID theIID, tObject * theObj, void ** theOut)
	{
		HRESULT aRes = tEntry1::InternalQueryInterface(theIID, theObj, theOut);

		if ( FAILED(aRes) )
			aRes = tEntry2::InternalQueryInterface(theIID, theObj, theOut);

		return aRes;
	}
};


//! TODO
template<typename tInterface, typename tParent, LPCGUID tIID = &__uuidof(tInterface)>
class DECLSPEC_NOVTABLE ComEntryWithParentDiscovery :
	public tInterface,
	public ComEntryCompound
		<
			ComEntryDiscoverable<tInterface>,
			ComEntry2StepDiscoverable<tParent, tInterface>
		>
{
public:
	typedef tInterface Interface;
};

//////////////////////////////////////////////////////////////////////////////////////////////


//! Implements exactly one COM interface and provides discovery information
//! about this interface, plus IUnknown.
template <typename tInterface1>
class DECLSPEC_NOVTABLE ComEntry1 :
	public ComEntryCompound
					<
						ComEntry<tInterface1>
					>
{ };


//! Implements exactly two COM interfaces and provides discovery information
//! about these interfaces, plus IUnknown.
template <typename tInterface1, typename tInterface2>
class DECLSPEC_NOVTABLE ComEntry2 :
	public ComEntryCompound
					<
						ComEntry<tInterface1>,
						ComEntry1<tInterface2>
					>
{ };


//! Implements exactly three COM interfaces and provides discovery information
//! about these interfaces, plus IUnknown.
template <typename tInterface1, typename tInterface2, typename tInterface3>
class DECLSPEC_NOVTABLE ComEntry3 :
	public ComEntryCompound
					<
						ComEntry<tInterface1>,
						ComEntry2<tInterface2, tInterface3>
					>
{ };


//! Implements exactly four COM interfaces and provides discovery information
//! about these interfaces, plus IUnknown.
template <typename tInterface1, typename tInterface2, typename tInterface3, typename tInterface4>
class DECLSPEC_NOVTABLE ComEntry4 :
	public ComEntryCompound
					<
						ComEntry<tInterface1>,
						ComEntry3<tInterface2, tInterface3, tInterface4>
					>
{ };


//! Implements exactly five COM interfaces and provides discovery information
//! about these interfaces, plus IUnknown.
template <typename tInterface1, typename tInterface2, typename tInterface3, typename tInterface4, typename tInterface5>
class DECLSPEC_NOVTABLE  ComEntry5 :
	public ComEntryCompound
					<
						ComEntry<tInterface1>,
						ComEntry4<tInterface2, tInterface3, tInterface4, tInterface5>
					>
{ };

//////////////////////////////////////////////////////////////////////////////////////////////

// Variadic-template versions

#if (SUPPORTED_CPP0X != 0)

//! Implements an arbitrary number of COM interfaces and provides discovery
//! information about these interfaces, plus IUnknown.
template<typename ... tInterfaces>
class DECLSPEC_NOVTABLE ComEntryV;

template<typename tInterface, typename ... tTail>
class DECLSPEC_NOVTABLE ComEntryV<tInterface, tTail...> :
	public ComEntryCompound
					<
						ComEntry<tInterface>,
						ComEntryV<tTail...>
					>
{ };

template <typename tInterface>
class DECLSPEC_NOVTABLE ComEntryV<tInterface> :
	public ComEntry1<tInterface>
{ };

#endif


//////////////////////////////////////////////////////////////////////////////////////////////

template<bool tThreadSafe = false>
struct ComRefCount
{
	LONG myNumRefs;

	ComRefCount() : myNumRefs(1L) { }

	ULONG STDMETHODCALLTYPE AddRef()
	{
		if (!tThreadSafe)
			return ++myNumRefs;
		else
		return InterlockedIncrement(&myNumRefs);
	}

	template<typename tObj>
	ULONG STDMETHODCALLTYPE Release(tObj * theObj)
	{
		ULONG aNumRefs;

		if (!tThreadSafe)
			aNumRefs = --myNumRefs;
		else
			aNumRefs = InterlockedDecrement(&myNumRefs);

		if (aNumRefs <= 0L)
			delete theObj;

		return aNumRefs;
	}
};

//////////////////////////////////////////////////////////////////////////////////////////////

template<typename tImpl, bool tThreadSafe = false>
class ComInstance : public tImpl
{
protected:
	ComRefCount<tThreadSafe> myRefs;


public:
	ComInstance()
	{ }

#if (SUPPORTED_CPP0X != 0)

	template<typename ... tParams>
	ComInstance(tParams && ... theParams)
		: tImpl( std::forward<tParams>(theParams)... )
	{ }
	
#else
	
	template<typename tParam1>
	ComInstance(tParam1 theParam1)
		: tImpl(theParam1)
	{ }

	template<typename tParam1, typename tParam2>
	ComInstance(tParam1 theParam1, tParam2 theParam2)
		: tImpl(theParam1, theParam2)
	{ }

	template<typename tParam1, typename tParam2, typename tParam3>
	ComInstance(tParam1 theParam1, tParam2 theParam2, tParam3 theParam3)
		: tImpl(theParam1, theParam2, theParam3)
	{ }

	template<typename tParam1, typename tParam2, typename tParam3, typename tParam4>
	ComInstance(tParam1 theParam1, tParam2 theParam2, tParam3 theParam3, tParam4 theParam4)
		: tImpl(theParam1, theParam2, theParam3, theParam4)
	{ }

	template<typename tParam1, typename tParam2, typename tParam3, typename tParam4, typename tParam5>
	ComInstance(tParam1 theParam1, tParam2 theParam2, tParam3 theParam3, tParam4 theParam4, tParam5 theParam5)
		: tImpl(theParam1, theParam2, theParam3, theParam4, theParam5)
	{ }
	
#endif	// (SUPPORTED_CPP0X != 0)


public:
	ULONG STDMETHODCALLTYPE AddRef()
	{
		return myRefs.AddRef();
	}

	ULONG STDMETHODCALLTYPE Release()
	{
		return myRefs.Release(this);
	}

	STDMETHODIMP QueryInterface(REFIID theIID,  void ** theOut)
	{
		HRESULT aRes = tImpl::InternalQueryInterface(theIID, this, theOut);

		if ( SUCCEEDED(aRes) )
			AddRef();

		return aRes;
	}
};

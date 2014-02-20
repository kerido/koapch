#pragma once

#include <tchar.h>

//////////////////////////////////////////////////////////////////////////////////////////////

//! Represents a single value of an enumeration together with
//! a string key meant for extracting its localized description.
//! \tparam T_Enum    An enumeration type for which localization is provided.
//! \tparam T_Lookup  The type of the lookup keys for extracting string data.
template <typename T_Enum, typename T_Lookup>
struct BindableEnumInfo
{
	T_Enum   Val;
	T_Lookup Lookup;

	typedef T_Lookup standard_lookup_type;

	// the default operation supports only one lookup dimension -- zero-based
	T_Lookup & operator() (size_t theIndex)
		{ return Lookup; }

	static bool Equals (const T_Lookup & theMyLookup, const T_Lookup & theLookup)
		{ return theMyLookup == theLookup; }
};

//////////////////////////////////////////////////////////////////////////////////////////////

//! Represents a specialization of BindableEnumInfo<T_Enum, TCHAR *>
//! where TCHAR* fields represent string lookup keys.
//! \tparam T_Enum    An enumeration type for which localization is provided.
template <typename T_Enum>
struct BindableEnumInfo<T_Enum, LPCTSTR>
{
	T_Enum  Val;
	LPCTSTR Lookup;

	typedef LPCTSTR standard_lookup_type;

	// the default operation supports only one lookup dimension -- zero-based
	const LPCTSTR & operator() (size_t theIndex) const
	{ return Lookup; }

	static bool Equals (LPCTSTR theMyLookup, LPCTSTR theLookup)
		{ return ( _tcscmp(theMyLookup, theLookup) == 0 ); }
};

//////////////////////////////////////////////////////////////////////////////////////////////

//! \brief Represents values of a single enumeration together with
//! string keys meant for extracting their localized description.
//! \tparam T_Enum        An enumeration type for which localization is provided.
//! \tparam T_InfoStruct  A data type of a single entry.
//! \tparam DefaultValue  The default value returned when parsing.
template
<
	typename T_Enum,
	typename T_InfoStruct = BindableEnumInfo<T_Enum, LPCTSTR>,
	T_Enum DefaultValue = (T_Enum) 0
>
struct BindableEnum
{
	typedef typename T_InfoStruct                       data_type;
	typedef typename T_InfoStruct::standard_lookup_type slookup;


	// data array and its size
	static data_type Properties[];
	const static size_t ArraySize;


	template <typename T_Lookup>
	static T_Enum val(const T_Lookup & theLookup, size_t theIndex = 0)
	{
		for (size_t i = 0; i < ArraySize; i++)
		{
			const T_Lookup & aLookup = Properties[i](theIndex);

			if ( T_InfoStruct::Equals(aLookup, theLookup) )
				return Properties[i].Val;
		}

		return DefaultValue;
	}


	template <typename T_Lookup>
	static const T_Lookup & display(T_Enum theVal, size_t theIndex = 0)
	{
		for (size_t i = 0; i < ArraySize; i++)
		{
			const data_type & aDt = Properties[i];

			if (aDt.Val == theVal )
			{
				const T_Lookup & aVal = aDt(theIndex);
				return aVal;
			}
		}

		return display<T_Lookup>(DefaultValue, theIndex);
	}

	static const slookup & sdisplay(T_Enum theVal, size_t theIndex = 0)
	{
		return display<slookup>(theVal, theIndex);
	}
};

//////////////////////////////////////////////////////////////////////////////////////////////

//! Provides a shorthand notation for creating localizable enumeration bindings
#define BIND_ENUM(originalEnumName, arraySize, newTypedefName) \
typedef BindableEnum<originalEnumName> newTypedefName; \
const size_t newTypedefName::ArraySize = arraySize; \
newTypedefName::data_type newTypedefName::Properties[] =

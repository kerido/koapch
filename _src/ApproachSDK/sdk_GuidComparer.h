#pragma once

#include <functional>

struct GuidComparer : public std::binary_function <GUID, GUID, bool> 
{
	bool operator() ( const GUID & theLeft, const GUID & theRight ) const
		{ return memcmp( &theLeft, &theRight, sizeof GUID ) < 0; }
};
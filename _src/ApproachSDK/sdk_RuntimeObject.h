#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////

//! When implemented, represents an object whose type can be identified by a GUID.
MIDL_INTERFACE("A8919E42-5B82-43DB-B225-C3989BEE5E19") IRuntimeObject : public IUnknown
{
public:
	//! Retrieves the type GUID of the object.
	//! \param [out] theOutGuid  The pointer to the output GUID.
	//! \return  S_OK if the GUID was successfully retrieved; otherwise, a COM error value.
	STDMETHOD (GetTypeGuid) (LPGUID theOutGuid) = 0;
};

DFN_GUID(IID_IRuntimeObject,
         0xA8919E42, 0x5B82, 0x43DB, 0xB2, 0x25, 0xC3, 0x98, 0x9B, 0xEE, 0x5E, 0x19);

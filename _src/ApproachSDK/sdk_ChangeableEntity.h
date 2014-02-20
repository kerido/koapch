#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////

//! Represents an object that can report when its state has changed.
class DECLSPEC_NOVTABLE IChangeableEntity : public IUnknown
{
public:

	//! Reports a change of state event.
	//! 
	//! \param [in] theChangeFlags
	//!     TODO
	//! 
	//! \return
	//!     S_OK if the operation completes successfully; otherwise, a COM error value.
	STDMETHOD (RaiseUpdateChanges) (ULONG theChangeFlags) = 0;
};

// {57DD1E2D-685E-4EB2-8E99-2AE7189A57B6}
DFN_GUID(IID_IChangeableEntity, 0x57DD1E2D, 0x685E, 0x4EB2, 0x8E, 0x99, 0x2A, 0xE7, 0x18, 0x9A, 0x57, 0xB6);

#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////

MIDL_INTERFACE("C92577BF-168C-4C87-AC44-C569F50A8F7A") IScrollable : public IUnknown
{
	enum ScrollFlags
	{
		SCROLLFLAGS_NONE                  = 0,  //!< perform default scrolling
		SCROLLFLAGS_DONT_CHANGE_SELECTION = 1   //!< not currently used
	};


	/// <summary>
	///   Scrolls the contents downwards or upwards the specified number of items
	/// </summary>
	/// <param name="theDeltaItems">
	///   Specifies the number of items to scroll. A positive number causes
	//    downwards scrolling and negative, upwards scrolling
	/// </param>
	/// <param name="theFlags">Specifies additional flags</param>
	STDMETHOD (Scroll) (int theDeltaItems, int theFlags) = 0;
};

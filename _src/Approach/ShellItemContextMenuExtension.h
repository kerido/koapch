#pragma once

#include "sdk_ComObject.h"

//////////////////////////////////////////////////////////////////////////////////////////////

class ShellItemContextMenuExtension : public ComEntry1<IContextMenuExtension>
{
protected:
	STDMETHODIMP QueryExecutor(Item * theItem, ULONG theExt1, ULONG theExt2, IContextMenuExecutor ** theOut);
};

//////////////////////////////////////////////////////////////////////////////////////////////

typedef ComInstance<ShellItemContextMenuExtension> ShellItemContextMenuExtensionImpl;

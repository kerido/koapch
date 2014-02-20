#include "Common.h"
#include "Trace.h"

#if defined  (_DEBUG) || defined (_FORCETRACE)

void AssertionFailure(char *exp, char *file, int line)
{
	Trace(exp);
	Trace(" failed at file: %s, line %d\n", file, line);
}

#endif
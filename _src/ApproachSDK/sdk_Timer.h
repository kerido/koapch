#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////

MIDL_INTERFACE("4ABE6417-4366-4019-ACE8-ADD804E8983D") ITimerTarget : public IUnknown
{
	//! TODO
	STDMETHOD (OnTimer) (int theID, int theElapsed, void * theArg) = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////////

MIDL_INTERFACE("A95716B4-21C5-429F-9983-D11F16B0E72F") ITimerHandler : public IUnknown
{
	struct TimerHandlerData
	{
		TimerHandlerData(ITimerTarget * theTarget = 0, void * theArg = 0)
			: Target(theTarget), Arg(theArg) {}

		ITimerTarget * Target;
		void * Arg;
	};

	//! TODO
	STDMETHOD (GetAvailableTimerID) (int * theOutTimerID) = 0;

	//! TODO
	STDMETHOD (SetTimer)            (int theID, int theTimeout, ITimerTarget * theParam, void * theArg) = 0;

	//! TODO
	STDMETHOD (KillTimer)           (int theID) = 0;
};
#pragma once

class CriticalSection
{
private:
	CRITICAL_SECTION myCS;

public:
	CriticalSection()
	{
		InitializeCriticalSection(&myCS);
	}

	~CriticalSection()
	{
		DeleteCriticalSection(&myCS);
	}

	void Enter()
	{
		EnterCriticalSection(&myCS);
	}

	void Leave()
	{
		LeaveCriticalSection(&myCS);
	}

	template<typename tFunctor>
	void Execute(tFunctor & theFunc)
	{
		Enter();

		try
		{
			theFunc();
		}
		catch(...)
		{
			Leave();
			throw;
		}

		Leave();
	}
};

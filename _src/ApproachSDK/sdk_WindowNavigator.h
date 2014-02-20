#pragma once

class TerminateNavigator
{
public:
	HWND find(HWND theCurWindow)
		{ return theCurWindow; }

	template<typename tVisitor>
	void iterate(HWND theCurWindow, tVisitor & theVisitor)
		{ }
};


class WindowNavigatorUtility
{
public:
	static BOOL WINAPI EnumDirectChildWindows(HWND theParent, WNDENUMPROC theCallback, LPARAM theParam)
	{
		//retrieve the first child
		HWND aChild = GetTopWindow(theParent);
		BOOL aRet = TRUE;

		while (aChild != 0)
		{
			aRet = theCallback(aChild, theParam);

			if (aRet == FALSE)
				break;

			aChild = GetNextWindow(aChild, GW_HWNDNEXT);
		}

		return aRet;
	}
};

template <typename tCondition, int tMaxDepth, typename tChildNav = TerminateNavigator>
class MaxDepthWindowNavigator
{
protected:
	class EnumContext
	{
	protected:
		typedef MaxDepthWindowNavigator<tCondition, tMaxDepth, tChildNav> ParentType;

	protected:
		ParentType & myOwner;
		int myCurDepth;
		HWND myFoundWindow;

	public:
		EnumContext(ParentType & theOwner) : myOwner(theOwner), myCurDepth(0), myFoundWindow(0)
		{ }

	public:
		ParentType & GetOwner() const { return myOwner; }

		HWND GetFoundWindow() const { return myFoundWindow; }
		void SetFoundWindow(HWND val) { myFoundWindow = val; }

		int GetDepth() const { return myCurDepth; }
		void IncreaseDepth() { myCurDepth++; }
		void DecreaseDepth() { myCurDepth--; }
	};

	template <typename tVisitor>
	class IterateContext : public EnumContext
	{
	protected:
		tVisitor & myVisitor;

	public:
		IterateContext(ParentType & theOwner, tVisitor & theVisitor)
			: EnumContext(theOwner), myVisitor(theVisitor)
		{ }

		tVisitor & GetVisitor() const { return myVisitor; }
	};

private:
	tChildNav myChild;
	tCondition myCondition;

public:
	HWND find(HWND theCurWindow)
	{
		EnumContext aCtx(*this);

		if (theCurWindow != 0)
			WindowNavigatorUtility::EnumDirectChildWindows(theCurWindow, Handler_OnFoundWindow, (LPARAM) &aCtx);
		else
			EnumWindows(Handler_OnFoundWindow, (LPARAM) &aCtx);


		if ( aCtx.GetFoundWindow() != 0)
			return myChild.find( aCtx.GetFoundWindow() );

		else
			return 0;
	}

	template<typename tVisitor>
	void iterate(HWND theCurWindow, tVisitor & theVisitor)
	{
		IterateContext<tVisitor> aCtx(*this, theVisitor);

		if (theCurWindow != 0)
			WindowNavigatorUtility::EnumDirectChildWindows(theCurWindow, Handler_OnFoundWindow_Iterate<tVisitor>, (LPARAM) &aCtx);
		else
			EnumWindows(Handler_OnFoundWindow_Iterate<tVisitor>, (LPARAM) &aCtx);
	}


protected:
	static BOOL __stdcall Handler_OnFoundWindow(HWND theWnd, LPARAM theLParam)
	{
		EnumContext * aCtx = reinterpret_cast<EnumContext *>(theLParam);

		return aCtx->GetOwner().Handler_OnFoundWindow(theWnd, aCtx);
	}

	BOOL Handler_OnFoundWindow(HWND theWnd, EnumContext * theCtx)
	{
		if ( myCondition(theWnd) )
		{
			theCtx->SetFoundWindow(theWnd);
			return FALSE;	// found
		}

		if (theCtx->GetDepth() < tMaxDepth)
		{
			theCtx->IncreaseDepth();
			WindowNavigatorUtility::EnumDirectChildWindows(theWnd, Handler_OnFoundWindow, (LPARAM) theCtx);
			theCtx->DecreaseDepth();

			if (theCtx->GetFoundWindow() != 0)
				return FALSE;
		}
		return TRUE;
	}


protected:
	template <typename tVisitor>
	static BOOL __stdcall Handler_OnFoundWindow_Iterate(HWND theWnd, LPARAM theLParam)
	{
		IterateContext<tVisitor> * aCtx = reinterpret_cast<IterateContext<tVisitor> *>(theLParam);

		return aCtx->GetOwner().Handler_OnFoundWindow_Iterate<tVisitor>(theWnd, aCtx);
	}

	template <typename tVisitor>
	BOOL Handler_OnFoundWindow_Iterate(HWND theWnd, IterateContext<tVisitor> * theCtx)
	{
		if ( myCondition(theWnd) )
		{
			tVisitor & aVis = theCtx->GetVisitor();

			bool aSearchFurther = aVis(theWnd);

			myChild.iterate(theWnd, aVis);

			return aSearchFurther;
		}

		if (theCtx->GetDepth() < tMaxDepth)
		{
			theCtx->IncreaseDepth();
			WindowNavigatorUtility::EnumDirectChildWindows(theWnd, Handler_OnFoundWindow_Iterate<tVisitor>, (LPARAM) theCtx);
			theCtx->DecreaseDepth();
		}

		return TRUE;
	}
};


template <typename tCondition, typename tChildNav = TerminateNavigator>
class UnlimitedDepthWindowNavigator
{
protected:
	class EnumContext
	{
	protected:
		typedef UnlimitedDepthWindowNavigator<tCondition, tChildNav> ParentType;

	protected:
		ParentType & myOwner;
		HWND myFoundWindow;

	public:
		EnumContext(ParentType & theOwner) : myOwner(theOwner), myFoundWindow(0)
		{ }

	public:
		ParentType & GetOwner() const { return myOwner; }

		HWND GetFoundWindow() const { return myFoundWindow; }
		void SetFoundWindow(HWND val) { myFoundWindow = val; }
	};

	template <typename tVisitor>
	class IterateContext : public EnumContext
	{
	protected:
		tVisitor & myVisitor;

	public:
		IterateContext(ParentType & theOwner, tVisitor & theVisitor)
			: EnumContext(theOwner), myVisitor(theVisitor)
		{ }

		tVisitor & GetVisitor() const { return myVisitor; }
	};

protected:
	tChildNav myChild;
	tCondition myCondition;

public:
	HWND find(HWND theCurWindow)
	{
		EnumContext aCtx(*this);

		if (theCurWindow != 0)
			EnumChildWindows(theCurWindow, Handler_OnFoundWindow_Children, (LPARAM) &aCtx);
		else
			EnumWindows(Handler_OnFoundWindow_TopMost, (LPARAM) &aCtx);


		if ( aCtx.GetFoundWindow() != 0)
			return myChild.find( aCtx.GetFoundWindow() );

		else
			return 0;
	}

	template<typename tVisitor>
	void iterate(HWND theCurWindow, tVisitor & theVisitor)
	{
		IterateContext<tVisitor> aCtx(*this, theVisitor);

		if (theCurWindow != 0)
			EnumChildWindows(theCurWindow, Handler_OnFoundWindow_Iterate_Children<tVisitor>, (LPARAM) &aCtx);
		else
			EnumWindows(Handler_OnFoundWindow_Iterate_TopMost<tVisitor>, (LPARAM) &aCtx);
	}


protected:
	static BOOL __stdcall Handler_OnFoundWindow_TopMost(HWND theWnd, LPARAM theLParam)
	{
		EnumContext * aCtx = reinterpret_cast<EnumContext *>(theLParam);

		return aCtx->GetOwner().Handler_OnFoundWindow_TopMost(theWnd, aCtx);
	}

	static BOOL __stdcall Handler_OnFoundWindow_Children(HWND theWnd, LPARAM theLParam)
	{
		EnumContext * aCtx = reinterpret_cast<EnumContext *>(theLParam);

		return aCtx->GetOwner().Handler_OnFoundWindow_Children(theWnd, aCtx);
	}

	BOOL Handler_OnFoundWindow_TopMost(HWND theWnd, EnumContext * theCtx)
	{
		if ( myCondition(theWnd) )
		{
			theCtx->SetFoundWindow(theWnd);
			return FALSE;	// found
		}

		EnumChildWindows(theWnd, Handler_OnFoundWindow_Children, (LPARAM) theCtx);

		if (theCtx->GetFoundWindow() != 0)
			return FALSE;
		else
			return TRUE;
	}

	BOOL Handler_OnFoundWindow_Children(HWND theWnd, EnumContext * theCtx)
	{
		if ( myCondition(theWnd) )
		{
			theCtx->SetFoundWindow(theWnd);
			return FALSE;	// found
		}
		else
			return TRUE;
	}


protected:
	template <typename tVisitor>
	static BOOL __stdcall Handler_OnFoundWindow_Iterate_TopMost(HWND theWnd, LPARAM theLParam)
	{
		IterateContext<tVisitor> * aCtx = reinterpret_cast<IterateContext<tVisitor> *>(theLParam);

		return aCtx->GetOwner().Handler_OnFoundWindow_Iterate_TopMost<tVisitor>(theWnd, aCtx);
	}

	template <typename tVisitor>
	static BOOL __stdcall Handler_OnFoundWindow_Iterate_Children(HWND theWnd, LPARAM theLParam)
	{
		IterateContext<tVisitor> * aCtx = reinterpret_cast<IterateContext<tVisitor> *>(theLParam);

		return aCtx->GetOwner().Handler_OnFoundWindow_Iterate_Children<tVisitor>(theWnd, aCtx);
	}

	template <typename tVisitor>
	BOOL Handler_OnFoundWindow_Iterate_TopMost(HWND theWnd, IterateContext<tVisitor> * theCtx)
	{
		if ( myCondition(theWnd) )
		{
			tVisitor & aVis = theCtx->GetVisitor();

			bool aSearchFurther = aVis(theWnd);

			myChild.iterate(theWnd, aVis);

			return aSearchFurther;
		}

		EnumChildWindows(theWnd, Handler_OnFoundWindow_Iterate_Children<tVisitor>, (LPARAM) theCtx);

		return TRUE;
	}

	template <typename tVisitor>
	BOOL Handler_OnFoundWindow_Iterate_Children(HWND theWnd, IterateContext<tVisitor> * theCtx)
	{
		if ( myCondition(theWnd) )
		{
			tVisitor & aVis = theCtx->GetVisitor();

			bool aSearchFurther = aVis(theWnd);

			myChild.iterate(theWnd, aVis);

			return aSearchFurther;
		}
		else
			return TRUE;
	}
};



template <typename tCondition, int tMaxDepth, typename tChildNav = TerminateNavigator>
class MaxDepthThreadWindowNavigator : public MaxDepthWindowNavigator<tCondition, tMaxDepth, tChildNav>
{
public:
	HWND find(DWORD theThreadID)
	{
		EnumContext aCtx(*this);
		EnumThreadWindows(theThreadID, Handler_OnFoundWindow, (LPARAM) &aCtx);

		if ( aCtx.GetFoundWindow() != 0)
			return myChild.find( aCtx.GetFoundWindow() );

		else
			return 0;
	}

	template<typename tVisitor>
	void iterate(DWORD theThreadID, tVisitor & theVisitor)
	{
		IterateContext<tVisitor> aCtx(*this, theVisitor);
		EnumWindows(theThreadID, Handler_OnFoundWindow_Iterate<tVisitor>, (LPARAM) &aCtx);
	}
};


template <typename tCondition, typename tChildNav = TerminateNavigator>
class UnlimitedDepthThreadWindowNavigator : public UnlimitedDepthWindowNavigator<tCondition, tChildNav>
{
public:
	HWND find(DWORD theThreadID)
	{
		EnumContext aCtx(*this);
		EnumThreadWindows(theThreadID, Handler_OnFoundWindow_TopMost, (LPARAM) &aCtx);

		if ( aCtx.GetFoundWindow() != 0)
			return myChild.find( aCtx.GetFoundWindow() );

		else
			return 0;
	}

	template<typename tVisitor>
	void iterate(DWORD theThreadID, tVisitor & theVisitor)
	{
		IterateContext<tVisitor> aCtx(*this, theVisitor);
		EnumWindows(theThreadID, Handler_OnFoundWindow_Iterate_TopMost<tVisitor>, (LPARAM) &aCtx);
	}
};

template <typename tCondition, typename tChildNav = TerminateNavigator>
class UnlimitedDepthParentWindowNavigator
{
protected:
	tChildNav myChild;
	tCondition myCondition;

public:
	HWND find(HWND theCurWindow)
	{
		for( HWND aWnd = GetParent(theCurWindow); aWnd != 0; aWnd = GetParent(aWnd) )
			if ( myCondition(aWnd) )
				return aWnd;

		return 0;
	}

	template<typename tVisitor>
	void iterate(HWND theCurWindow, tVisitor & theVisitor)
	{
		for( HWND aWnd = GetParent(theCurWindow); aWnd != 0; aWnd = GetParent(aWnd) )
			theVisitor(aWnd);
				return aWnd;
	}
};

template<const TCHAR * tCondition>
class ClassCondition
{
public:
	bool operator() (HWND theWnd)
	{
		TCHAR aClass [1000];
		GetClassName(theWnd, aClass, 1000);

		return lstrcmp(aClass, tCondition) == 0;
	}
};

template<const TCHAR * tCondition>
class TextCondition
{
public:
	bool operator() (HWND theWnd)
	{
		TCHAR aTxt [1000];
		GetWindowText(theWnd, aTxt, 1000);

		return lstrcmp(aTxt, tCondition) == 0;
	}
};


template<typename tCondition1, typename tCondition2, bool tOr = false>
class LogicalCondition
{
private:
	tCondition1 myCond1;
	tCondition2 myCond2;

public:
	bool operator() (HWND theWnd)
	{
		if (tOr)
			return myCond1(theWnd) || myCond2(theWnd);
		else
			return myCond1(theWnd) && myCond2(theWnd);
	}
};

template<LONG_PTR tVal, int tIndex, bool tMustHave = true>
class WindowLongCondition
{
public:
	bool operator() (HWND theWnd)
	{
		LONG_PTR aStyle = GetWindowLongPtr(theWnd, tIndex);

		if (tMustHave)
			return (aStyle & tVal) == tVal;
		else
			return (aStyle & tVal) == 0;
	}
};

template<LONG_PTR tStyle, bool tMustHave = true>
class WindowStyleCondition : public WindowLongCondition<tStyle, GWL_STYLE, tMustHave>
{
};

template<LONG_PTR tStyle, bool tMustHave = true>
class WindowExStyleCondition : public WindowLongCondition<tStyle, GWL_EXSTYLE, tMustHave>
{
};

typedef WindowStyleCondition<WS_VISIBLE> VisibleCondition;
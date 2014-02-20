#pragma once

//TODO: create three implementations of the ILayoutAlgorithm interface
class ILayoutAlgorithm
{
public:
	virtual void FitRect(RECT & theRect) = 0;
	virtual void LayoutViews(IDisplayItemHost ** theHost) = 0;
};

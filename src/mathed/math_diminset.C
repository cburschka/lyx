#include "math_diminset.h"


MathDimInset::MathDimInset()
	: MathInset(), width_(0), ascent_(0), descent_(0)
{}


int MathDimInset::ascent() const
{
	return ascent_;
}


int MathDimInset::descent() const
{
	return descent_;
}


int MathDimInset::width() const
{
	return width_;
}

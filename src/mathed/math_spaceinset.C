#ifdef __GNUG__
#pragma implementation
#endif

#include "math_spaceinset.h"
#include "math_support.h"
#include "LColor.h"
#include "Painter.h"
#include "math_mathmlstream.h"



MathSpaceInset::MathSpaceInset(int sp)
	: space_(sp)
{}


MathInset * MathSpaceInset::clone() const
{
	return new MathSpaceInset(*this);
}


void MathSpaceInset::metrics(MathMetricsInfo const &) const
{
	width_ = space_ ? space_ * 2 : 2;
	if (space_ > 3)
		width_ *= 2;
	if (space_ == 5)
		width_ *= 2;
	width_  += 4;
	ascent_  = 4;
	descent_ = 0;
}


void MathSpaceInset::draw(Painter & pain, int x, int y) const
{ 
	
// XPoint p[4] = {{++x, y-3}, {x, y}, {x+width-2, y}, {x+width-2, y-3}};
	
// Sadly, HP-UX CC can't handle that kind of initialization.
	
	int xp[4];
	int yp[4];
	
	xp[0] = ++x;             yp[0] = y - 3;
	xp[1] = x;	             yp[1] = y;
	xp[2] = x + width_ - 2;  yp[2] = y;
	xp[3] = x + width_ - 2;  yp[3] = y - 3;
	
	pain.lines(xp, yp, 4, space_ ? LColor::latex : LColor::math);
}


void MathSpaceInset::incSpace()
{
	space_ = (space_ + 1) % 6;
}


void MathSpaceInset::maplize(MapleStream & os) const
{
	os << ' ';
}


void MathSpaceInset::octavize(OctaveStream & os) const
{
	os << ' ';
}


void MathSpaceInset::normalize(NormalStream & os) const
{
	os << "[space " << space_ << "] ";
}


void MathSpaceInset::write(WriteStream & os) const
{
	if (space_ >= 0 && space_ < 6)
		os << '\\' << latex_mathspace[space_] << ' ';
}



#ifdef __GNUG__
#pragma implementation
#endif

#include "math_spaceinset.h"
#include "math_support.h"
#include "LColor.h"
#include "frontends/Painter.h"
#include "math_mathmlstream.h"



char const * latex_mathspace[] = {
	"!", ",", ";", ":", "quad", "qquad", "lyxnegspace", "lyxposspace"
};




MathSpaceInset::MathSpaceInset(int sp)
	: space_(sp)
{}


MathSpaceInset::MathSpaceInset(string const & name)
	: space_(1)
{
	for (int i = 0; i < 8; ++i) 
		if (latex_mathspace[i] == name) 
			space_ = i;
}



MathInset * MathSpaceInset::clone() const
{
	return new MathSpaceInset(*this);
}


void MathSpaceInset::metrics(MathMetricsInfo &) const
{
	switch (space_) {
		case 0: dim_.w = 6; break;
		case 1: dim_.w = 6; break;
		case 2: dim_.w = 8; break;
		case 3: dim_.w = 10; break;
		case 4: dim_.w = 20; break;
		case 5: dim_.w = 40; break;
		case 6: dim_.w = -2; break;
		case 7: dim_.w =  2; break;
		default: dim_.w = 6; break;
	}
	dim_.a  = 4;
	dim_.d = 0;
}


void MathSpaceInset::draw(MathPainterInfo & pain, int x, int y) const
{

// Sadly, HP-UX CC can't handle that kind of initialization.
// XPoint p[4] = {{++x, y-3}, {x, y}, {x+width-2, y}, {x+width-2, y-3}};
	if (space_ >= 6)
		return;

	int xp[4];
	int yp[4];

	xp[0] = ++x;              yp[0] = y - 3;
	xp[1] = x;	              yp[1] = y;
	xp[2] = x + width() - 2;  yp[2] = y;
	xp[3] = x + width() - 2;  yp[3] = y - 3;

	pain.pain.lines(xp, yp, 4, space_ ? LColor::latex : LColor::math);
}


void MathSpaceInset::incSpace()
{
	space_ = (space_ + 1) % 6;
}


void MathSpaceInset::maplize(MapleStream & os) const
{
	os << ' ';
}

void MathSpaceInset::mathematicize(MathematicaStream & os) const
{
	os << ' ';
}


void MathSpaceInset::octavize(OctaveStream & os) const
{
	os << ' ';
}


void MathSpaceInset::normalize(NormalStream & os) const
{
	os << "[space " << int(space_) << ' ' << latex_mathspace[space_] << "] ";
}


void MathSpaceInset::write(WriteStream & os) const
{
	if (space_ >= 0 && space_ < 8)
		os << '\\' << latex_mathspace[space_] << ' ';
}

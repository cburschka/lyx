#ifdef __GNUG__
#pragma implementation
#endif

#include "math_spaceinset.h"
#include "math_support.h"
#include "LColor.h"
#include "frontends/Painter.h"
#include "math_mathmlstream.h"
#include "debug.h"


char const * latex_mathspace[] = {
	"!", "negmedspace", "negthickspace",  // negative space
	",", ":", ";", "quad", "qquad",       // positive space
	"lyxnegspace", "lyxposspace"          // LyX special ("unvisible space")
};

int const nSpace = sizeof(latex_mathspace)/sizeof(char *);


MathSpaceInset::MathSpaceInset(int sp)
	: space_(sp)
{}


MathSpaceInset::MathSpaceInset(string const & name)
	: space_(1)
{
	for (int i = 0; i < nSpace; ++i) 
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
		case 1: dim_.w = 8; break;
		case 2: dim_.w = 10; break;
		case 3: dim_.w = 6; break;
		case 4: dim_.w = 8; break;
		case 5: dim_.w = 10; break;
		case 6: dim_.w = 20; break;
		case 7: dim_.w = 40; break;
		case 8: dim_.w = -2; break;
		case 9: dim_.w =  2; break;
		default: dim_.w = 6;
	}
	dim_.a = 4;
	dim_.d = 0;
}


void MathSpaceInset::draw(MathPainterInfo & pi, int x, int y) const
{

// Sadly, HP-UX CC can't handle that kind of initialization.
// XPoint p[4] = {{++x, y-3}, {x, y}, {x+width-2, y}, {x+width-2, y-3}};
	if (space_ >= nSpace - 2)
		return;

	int xp[4];
	int yp[4];

	xp[0] = ++x;              yp[0] = y - 3;
	xp[1] = x;                yp[1] = y;
	xp[2] = x + width() - 2;  yp[2] = y;
	xp[3] = x + width() - 2;  yp[3] = y - 3;

	pi.pain.lines(xp, yp, 4, (space_ < 3) ? LColor::latex : LColor::math);
}


void MathSpaceInset::incSpace()
{
	space_ = (space_ + 1) % (nSpace - 2);
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
	os << "[space " << int(space_) << "] ";
}


void MathSpaceInset::write(WriteStream & os) const
{
	if (space_ >= 0 && space_ < nSpace) {
		os << '\\' << latex_mathspace[space_];
		os.pendingSpace(true);
	}
}

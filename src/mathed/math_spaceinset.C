/**
 * \file math_spaceinset.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "math_spaceinset.h"
#include "math_data.h"
#include "math_mathmlstream.h"

#include "LaTeXFeatures.h"
#include "LColor.h"

#include "frontends/Painter.h"

using std::string;
using std::auto_ptr;


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


auto_ptr<InsetBase> MathSpaceInset::doClone() const
{
	return auto_ptr<InsetBase>(new MathSpaceInset(*this));
}


int MathSpaceInset::width() const
{
	switch (space_) {
		case 0: return 6;
		case 1: return 8;
		case 2: return 10;
		case 3: return 6;
		case 4: return 8;
		case 5: return 10;
		case 6: return 20;
		case 7: return 40;
		case 8: return -2;
		case 9: return  2;
		default: return 6;
	}
}


int MathSpaceInset::ascent() const
{
	return 4;
}


int MathSpaceInset::descent() const
{
	return 0;
}


void MathSpaceInset::metrics(MetricsInfo &, Dimension & dim) const
{
	dim.wid = width();
	dim.asc = ascent();
	dim.des = descent();
}


void MathSpaceInset::draw(PainterInfo & pi, int x, int y) const
{
	// Sadly, HP-UX CC can't handle that kind of initialization.
	// XPoint p[4] = {{++x, y-3}, {x, y}, {x+width-2, y}, {x+width-2, y-3}};
	if (space_ >= nSpace - 2)
		return;

	int xp[4];
	int yp[4];
	int w = width();

	xp[0] = ++x;        yp[0] = y - 3;
	xp[1] = x;          yp[1] = y;
	xp[2] = x + w - 2;  yp[2] = y;
	xp[3] = x + w - 2;  yp[3] = y - 3;

	pi.pain.lines(xp, yp, 4, (space_ < 3) ? LColor::latex : LColor::math);
}


void MathSpaceInset::incSpace()
{
	space_ = (space_ + 1) % (nSpace - 2);
}


void MathSpaceInset::validate(LaTeXFeatures & features) const
{
	if (space_ >= 0 && space_< nSpace) {
		if ((latex_mathspace[space_] == "negmedspace")
		 || (latex_mathspace[space_] == "negthickspace"))
			features.require("amsmath");
	}
}


void MathSpaceInset::maple(MapleStream & os) const
{
	os << ' ';
}

void MathSpaceInset::mathematica(MathematicaStream & os) const
{
	os << ' ';
}


void MathSpaceInset::octave(OctaveStream & os) const
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

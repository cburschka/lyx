/**
 * \file GraphicsParams.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GraphicsParams.h"

#include <cstdlib>
#include <sstream>

using namespace std;

namespace lyx {
namespace graphics {

Params::Params()
	: display(true),
	  scale(100),
	  pixel_ratio(1.0),
	  angle(0)
{}


bool operator==(Params const & a, Params const & b)
{
	return (a.filename == b.filename &&
		a.display == b.display &&
		a.bb == b.bb &&
		a.scale == b.scale &&
		a.pixel_ratio == b.pixel_ratio &&
		a.angle == b.angle);
}


bool operator!=(Params const & a, Params const & b)
{
	return !(a == b);
}


ostream & operator<<(ostream & os, BoundingBox const & bb)
{
	os << bb.xl.asString() << ' ' << bb.yb.asString() << ' '
	   << bb.xr.asString() << ' ' << bb.yt.asString();
	return os;
}


BoundingBox::BoundingBox()
{}


BoundingBox::BoundingBox(string const & bb)
{
	if (bb.empty())
		return;

	istringstream is(bb.c_str());
	string a, b, c, d;
	is >> a >> b >> c >> d;

	Length xl_tmp = Length(a);
	if (xl_tmp.value() < 0)
		xl_tmp = Length(-xl_tmp.value(), xl_tmp.unit());
	Length yb_tmp = Length(b);
	if (yb_tmp.value() < 0)
		yb_tmp = Length(-yb_tmp.value(), yb_tmp.unit());
	Length xr_tmp = Length(c);
	if (xr_tmp.value() < 0)
		xr_tmp = Length(-xr_tmp.value(), xr_tmp.unit());
	Length yt_tmp = Length(d);
	if (yt_tmp.value() < 0)
		yt_tmp = Length(-yt_tmp.value(), yt_tmp.unit());

	// inBP returns the length in Postscript points.
	// Note further that there are 72 Postscript pixels per inch.
	if (xr_tmp.inBP() <= xl_tmp.inBP() || yt_tmp.inBP() <= yb_tmp.inBP())
		return;

	xl = xl_tmp;
	yb = yb_tmp;
	xr = xr_tmp;
	yt = yt_tmp;
}


bool BoundingBox::empty() const
{
	return xl.zero() && yb.zero() && xr.zero() && yt.zero();
}


bool operator==(BoundingBox const & a, BoundingBox const & b)
{
	return (a.xl == b.xl &&
		a.yb == b.yb &&
		a.xr == b.xr &&
		a.yt == b.yt);
}


bool operator!=(BoundingBox const & a, BoundingBox const & b)
{
	return !(a == b);
}

} // namespace graphics
} // namespace lyx

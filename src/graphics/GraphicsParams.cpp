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

#include "Length.h"

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
	os << bb.xl << ' ' << bb.yb << ' ' << bb.xr << ' ' << bb.yt;
	return os;
}


BoundingBox::BoundingBox()
	: xl(0), yb(0), xr(0), yt(0)
{}


BoundingBox::BoundingBox(string const & bb)
	: xl(0), yb(0), xr(0), yt(0)
{
	if (bb.empty())
		return;

	istringstream is(bb.c_str());
	string a, b, c, d;
	is >> a >> b >> c >> d;

	// inBP returns the length in Postscript points.
	// Note further that there are 72 Postscript pixels per inch.
	unsigned int const xl_tmp = abs(Length(a).inBP());
	unsigned int const yb_tmp = abs(Length(b).inBP());
	unsigned int const xr_tmp = abs(Length(c).inBP());
	unsigned int const yt_tmp = abs(Length(d).inBP());

	if (xr_tmp <= xl_tmp || yt_tmp <= yb_tmp)
		return;

	xl = xl_tmp;
	yb = yb_tmp;
	xr = xr_tmp;
	yt = yt_tmp;
}


bool BoundingBox::empty() const
{
	return (!xl && !yb && !xr && !yt);
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

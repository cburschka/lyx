/**
 * \file GraphicsParams.C
 * Read the file COPYING
 *
 * \author Angus Leeming 
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include <cstdlib>

#include "GraphicsParams.h"
#include "Lsstream.h"
#include "lyxlength.h"

using std::abs;

namespace grfx {

Params::Params()
	: display(ColorDisplay),
	  scale(100),
	  angle(0)
{}


bool operator==(Params const & a, Params const & b)
{
	return (a.filename == b.filename &&
		a.display == b.display &&
		a.bb == b.bb &&
		a.scale == b.scale &&
		a.angle == b.angle);
}


bool operator!=(Params const & a, Params const & b)
{
	return !(a == b);
}


BoundingBox::BoundingBox()
	: xl(0), yb(0), xr(0), yt(0)
{}


BoundingBox::BoundingBox(string const & bb)
	: xl(0), yb(0), xr(0), yt(0)
{
	if (bb.empty())
		return;

	std::istringstream is(bb.c_str());
	string a, b, c, d;
	is >> a >> b >> c >> d;

	// inBP returns the length in Postscript points.
	// Note further that there are 72 Postscript pixels per inch.
	unsigned int const xl_tmp = abs(LyXLength(a).inBP());
	unsigned int const yb_tmp = abs(LyXLength(b).inBP());
	unsigned int const xr_tmp = abs(LyXLength(c).inBP());
	unsigned int const yt_tmp = abs(LyXLength(d).inBP());

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

} // namespace grfx

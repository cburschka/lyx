/*
 * \file GraphicsParams.C
 * Copyright 2002 the LyX Team
 * Read the file COPYING
 *
 * \author Angus Leeming <a.leeming@ic.ac.uk>
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "GraphicsParams.h"
#include "Lsstream.h"
#include "lyxlength.h"


namespace grfx {

Params::Params()
	: display(ColorDisplay),
	  width(0),
	  height(0),
	  scale(0),
	  angle(0)
{}


bool operator==(Params const & a, Params const & b)
{
	return (a.filename == b.filename &&
		a.display  == b.display &&
		a.bb       == b.bb &&
		a.width    == b.width &&
		a.height   == b.height &&
		a.scale    == b.scale &&
		a.angle    == b.angle);
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
	int const xl_tmp = LyXLength(a).inBP();
	int const yb_tmp = LyXLength(b).inBP();
	int const xr_tmp = LyXLength(c).inBP();
	int const yt_tmp = LyXLength(d).inBP();

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

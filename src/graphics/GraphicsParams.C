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
#include "insets/insetgraphicsParams.h"
#include "lyxrc.h"
#include "support/filetools.h"
#include "support/lstrings.h"
#include "support/LAssert.h"

namespace grfx {

GParams::GParams(InsetGraphicsParams const & iparams, string const & filepath)
	: width(0),
	  height(0),
	  scale(0),
	  angle(0)
{
	filename = iparams.filename;
	if (!filepath.empty()) {
		filename = MakeAbsPath(filename, filepath);
	}

	if (iparams.clip)
		bb = iparams.bb;

	if (iparams.rotate)
		angle = int(iparams.rotateAngle);

	if (iparams.display == InsetGraphicsParams::DEFAULT) {

		if (lyxrc.display_graphics == "mono")
			display = MONOCHROME;
		else if (lyxrc.display_graphics == "gray")
			display = GRAYSCALE;
		else if (lyxrc.display_graphics == "color")
			display = COLOR;
		else
			display = NONE;

	} else if (iparams.display == InsetGraphicsParams::NONE) {
		display = NONE;

	} else if (iparams.display == InsetGraphicsParams::MONOCHROME) {
		display = MONOCHROME;

	} else if (iparams.display == InsetGraphicsParams::GRAYSCALE) {
		display = GRAYSCALE;

	} else if (iparams.display == InsetGraphicsParams::COLOR) {
		display = COLOR;
	}

	// Override the above if we're not using a gui
	if (!lyxrc.use_gui) {
		display = NONE;
	}

	if (iparams.lyxsize_type == InsetGraphicsParams::SCALE) {
		scale = iparams.lyxscale;

	} else if (iparams.lyxsize_type == InsetGraphicsParams::WH) {
		if (!iparams.lyxwidth.zero())
			width  = iparams.lyxwidth.inPixels(1, 1);
		if (!iparams.lyxheight.zero())
			height = iparams.lyxheight.inPixels(1, 1);

		// inPixels returns a value scaled by lyxrc.zoom.
		// We want, therefore, to undo this.
		double const scaling_factor = 100.0 / double(lyxrc.zoom);
		width  = uint(scaling_factor * width);
		height = uint(scaling_factor * height);
	}
}


bool operator==(GParams const & a, GParams const & b)
{
	return (a.filename        == b.filename &&
		a.display         == b.display &&
		a.bb              == b.bb &&
		a.width           == b.width &&
		a.height          == b.height &&
		a.scale           == b.scale &&
		a.angle           == b.angle);
}


bool operator!=(GParams const & a, GParams const & b)
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

	string tmp1;
	string tmp2 = split(bb, tmp1, ' ');
	if (!isValidLength(tmp1))
		return;

	LyXLength const length_xl(tmp1);

	tmp2 = split(tmp2, tmp1, ' ');
	if (!isValidLength(tmp1))
		return;

	LyXLength const length_yb(tmp1);

	tmp2 = split(tmp2, tmp1, ' ');
	if (!isValidLength(tmp1) || !isValidLength(tmp2))
		return;

	LyXLength const length_xr(tmp1);
	LyXLength const length_yt(tmp2);

	// inPixels returns the length in inches, scaled by
	// lyxrc.dpi and lyxrc.zoom.
	// We want, therefore, to undo all this lyxrc nonsense because we
	// want the bounding box in Postscript pixels.
	// Note further that there are 72 Postscript pixels per inch.
	double const scaling_factor = 7200.0 / (lyxrc.dpi * lyxrc.zoom);
	unsigned int const xl_tmp =
		uint(scaling_factor * length_xl.inPixels(1, 1));
	unsigned int const yb_tmp =
		uint(scaling_factor * length_yb.inPixels(1, 1));
	unsigned int const xr_tmp =
		uint(scaling_factor * length_xr.inPixels(1, 1));
	unsigned int const yt_tmp =
		uint(scaling_factor * length_yt.inPixels(1, 1));

	if (xr <= xl || yt <= yb)
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

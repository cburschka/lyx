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
#include "debug.h"
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

	if (iparams.clip) {
		bb = iparams.bb;

		// Get the original Bounding Box from the file
		string const tmp = readBB_from_PSFile(filename);
		lyxerr[Debug::GRAPHICS] << "BB_from_File: " << tmp << std::endl;
		if (!tmp.empty()) {
			int const bb_orig_xl = strToInt(token(tmp, ' ', 0));
			int const bb_orig_yb = strToInt(token(tmp, ' ', 1));

			bb.xl -= bb_orig_xl;
			bb.xr -= bb_orig_xl;
			bb.yb -= bb_orig_yb;
			bb.yt -= bb_orig_yb;
		}

		bb.xl = std::max(0, bb.xl);
		bb.xr = std::max(0, bb.xr);
		bb.yb = std::max(0, bb.yb);
		bb.yt = std::max(0, bb.yt);

		// Paranoia check.
		int const width  = bb.xr - bb.xl;
		int const height = bb.yt - bb.yb;

		if (width  < 0 || height < 0) {
			bb.xl = 0;
			bb.xr = 0;
			bb.yb = 0;
			bb.yt = 0;
		}
	}
	
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
			width  = iparams.lyxwidth.inBP();
		if (!iparams.lyxheight.zero())
			height = iparams.lyxheight.inBP();
	}
}


bool operator==(GParams const & a, GParams const & b)
{
	return (a.filename == b.filename &&
		a.display  == b.display &&
		a.bb       == b.bb &&
		a.width    == b.width &&
		a.height   == b.height &&
		a.scale    == b.scale &&
		a.angle    == b.angle);
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

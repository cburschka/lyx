/**
 * \file InsetGraphicsParams.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Baruch Even
 * \author Herbert Voß
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetGraphicsParams.h"

#include "Buffer.h"
#include "LyX.h" // for use_gui
#include "Lexer.h"
#include "LyXRC.h"

#include "graphics/epstools.h"
#include "graphics/GraphicsParams.h"
#include "graphics/GraphicsTypes.h"

#include "support/convert.h"
#include "support/debug.h"
#include "support/lyxlib.h"
#include "support/lstrings.h"
#include "support/Translator.h"

#include <ostream>

using namespace std;
using namespace lyx::support;

namespace lyx {


InsetGraphicsParams::InsetGraphicsParams()
{
	init();
}


InsetGraphicsParams::InsetGraphicsParams(InsetGraphicsParams const & igp)
{
	// I decided to skip the initialization since the copy will overwrite
	// everything anyway.
	//    init();
	copy(igp);
}


void InsetGraphicsParams::operator=(InsetGraphicsParams const & params)
{
	// Are we assigning the object into itself?
	if (this == &params)
		return;
	copy(params);
}


void InsetGraphicsParams::init()
{
	filename.erase();
	lyxscale = 100;			// lyx scaling in percentage
	display = true;         // may be overriden by display mode in preferences
	scale = string("100");			// output scaling in percentage
	width = Length();
	height = Length();
	keepAspectRatio = false;	// for LaTeX output
	draft = false;			// draft mode
	scaleBeforeRotation = false;	// scale image before rotating

	bb = string();			// bounding box
	clip = false;			// clip image

	rotateAngle = "0";		// angle of rotation in degrees
	rotateOrigin.erase();		// Origin of rotation
	special.erase();		// additional userdefined stuff
	groupId.clear();
}


void InsetGraphicsParams::copy(InsetGraphicsParams const & igp)
{
	filename = igp.filename;
	lyxscale = igp.lyxscale;
	display = igp.display;
	scale = igp.scale;
	width = igp.width;
	height = igp.height;
	keepAspectRatio = igp.keepAspectRatio;
	draft = igp.draft;
	scaleBeforeRotation = igp.scaleBeforeRotation;

	bb = igp.bb;
	clip = igp.clip;

	rotateAngle = igp.rotateAngle;
	rotateOrigin = igp.rotateOrigin;
	special = igp.special;
	groupId = igp.groupId;
}


bool operator==(InsetGraphicsParams const & left,
		InsetGraphicsParams const & right)
{
	return left.filename == right.filename &&
	    left.lyxscale == right.lyxscale &&
	    left.display == right.display &&
	    left.scale == right.scale &&
	    left.width == right.width &&
	    left.height == right.height &&
	    left.keepAspectRatio == right.keepAspectRatio &&
	    left.draft == right.draft &&
	    left.scaleBeforeRotation == right.scaleBeforeRotation &&

	    left.bb == right.bb &&
	    left.clip == right.clip &&

	    left.rotateAngle == right.rotateAngle &&
	    left.rotateOrigin == right.rotateOrigin &&
	    left.special == right.special &&
	    left.groupId == right.groupId;
}


bool operator!=(InsetGraphicsParams const & left,
		InsetGraphicsParams const & right)
{
	return	!(left == right);
}


void InsetGraphicsParams::Write(ostream & os, Buffer const & buffer) const
{
	// Do not write the default values
	if (!filename.empty())
		os << "\tfilename " << filename.outputFileName(buffer.filePath()) << '\n';
	if (lyxscale != 100)
		os << "\tlyxscale " << lyxscale << '\n';
	if (!display)
		os << "\tdisplay false\n";
	if (!scale.empty() && !float_equal(convert<double>(scale), 0.0, 0.05)) {
		if (!float_equal(convert<double>(scale), 100.0, 0.05))
			os << "\tscale " << scale << '\n';
	} else {
		if (!width.zero())
			os << "\twidth " << width.asString() << '\n';
		if (!height.zero())
			os << "\theight " << height.asString() << '\n';
	}

	if (keepAspectRatio)
		os << "\tkeepAspectRatio\n";
	if (draft)			// draft mode
		os << "\tdraft\n";
	if (scaleBeforeRotation)
		os << "\tscaleBeforeRotation\n";

	if (!bb.empty())		// bounding box
		os << "\tBoundingBox " << bb << '\n';
	if (clip)			// clip image
		os << "\tclip\n";

	if (!rotateAngle.empty()
		&& !float_equal(convert<double>(rotateAngle), 0.0, 0.001))
		os << "\trotateAngle " << rotateAngle << '\n';
	if (!rotateOrigin.empty())
		os << "\trotateOrigin " << rotateOrigin << '\n';
	if (!special.empty())
		os << "\tspecial " << special << '\n';
	if (!groupId.empty())
		os << "\tgroupId "<< groupId << '\n';
}


bool InsetGraphicsParams::Read(Lexer & lex, string const & token, string const & bufpath)
{
	if (token == "filename") {
		lex.eatLine();
		filename.set(lex.getString(), bufpath);
	} else if (token == "lyxscale") {
		lex.next();
		lyxscale = lex.getInteger();
	} else if (token == "display") {
		lex.next();
		string const type = lex.getString();
		display = lex.getString() != "false";
	} else if (token == "scale") {
		lex.next();
		scale = lex.getString();
	} else if (token == "width") {
		lex.next();
		width = Length(lex.getString());
		scale = string();
	} else if (token == "height") {
		lex.next();
		height = Length(lex.getString());
		scale = string();
	} else if (token == "keepAspectRatio") {
		keepAspectRatio = true;
	} else if (token == "draft") {
		draft = true;
	} else if (token == "scaleBeforeRotation") {
		scaleBeforeRotation = true;
	} else if (token == "BoundingBox") {
		bb.erase();
		for (int i = 0; i < 4; ++i) {
			if (i != 0)
				bb += ' ';
			lex.next();
			bb += lex.getString();
		}
	} else if (token == "clip") {
		clip = true;
	} else if (token == "rotateAngle") {
		lex.next();
		rotateAngle = lex.getString();
	} else if (token == "rotateOrigin") {
		lex.next();
		rotateOrigin=lex.getString();
	} else if (token == "special") {
		lex.eatLine();
		special = lex.getString();
	} else if (token == "groupId") {
		lex.eatLine();
		groupId = lex.getString();

	// catch and ignore following two old-format tokens and their arguments.
	// e.g. "size_kind scale" clashes with the setting of the
	// "scale <value>" keyword.
	} else if (token == "size_kind" || token == "lyxsize_kind") {
		lex.next();
		lex.getString();

	} else {
		// If it's none of the above, it's not ours.
		return false;
	}
	return true;
}


graphics::Params InsetGraphicsParams::as_grfxParams() const
{
	graphics::Params pars;
	pars.filename = filename;
	pars.scale = lyxscale;
	pars.angle = convert<double>(rotateAngle);

	if (clip) {
		pars.bb = bb;

		// Get the original Bounding Box from the file
		string const tmp = graphics::readBB_from_PSFile(filename);
		LYXERR(Debug::GRAPHICS, "BB_from_File: " << tmp);
		if (!tmp.empty()) {
			unsigned int const bb_orig_xl = convert<unsigned int>(token(tmp, ' ', 0));
			unsigned int const bb_orig_yb = convert<unsigned int>(token(tmp, ' ', 1));

			// new pars.bb values must be >= zero
			if (pars.bb.xl > bb_orig_xl)
				pars.bb.xl -= bb_orig_xl;
			else
				pars.bb.xl = 0;

			if (pars.bb.xr > bb_orig_xl)
				pars.bb.xr -= bb_orig_xl;
			else
				pars.bb.xr = 0;

			if (pars.bb.yb > bb_orig_yb)
				pars.bb.yb -= bb_orig_yb;
			else
				pars.bb.yb = 0;

			if (pars.bb.yt > bb_orig_yb)
				pars.bb.yt -= bb_orig_yb;
			else
				pars.bb.yt = 0;
		}

		// Paranoia check.
		int const width  = pars.bb.xr - pars.bb.xl;
		int const height = pars.bb.yt - pars.bb.yb;

		if (width  < 0 || height < 0) {
			pars.bb.xl = 0;
			pars.bb.xr = 0;
			pars.bb.yb = 0;
			pars.bb.yt = 0;
		}
	}

	pars.display = display;

	// Override the above if we're not using a gui
	if (!use_gui)
		pars.display = false;

	return pars;
}


} // namespace lyx

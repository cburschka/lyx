/* This file is part of
 * =================================================
 *
 *          LyX, The Document Processor
 *          Copyright 1995 Matthias Ettrich.
 *          Copyright 1995-2001 The LyX Team.
 *
 * \author Baruch Even
 * \author Herbert Voss <voss@lyx.org>
 *
 * ================================================= */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "insetgraphicsParams.h"

#include "graphics/GraphicsParams.h"

#include "support/filetools.h"
#include "support/lyxlib.h"
#include "support/LOstream.h"
#include "support/LAssert.h"
#include "support/lstrings.h"
#include "lyxrc.h"
#include "debug.h"
#include "lyxlex.h"

using std::ostream;

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

InsetGraphicsParams &
InsetGraphicsParams::operator=(InsetGraphicsParams const & params)
{
	// Are we assigning the object into itself?
	if (this == &params)
		return * this;
	copy(params);
	return *this;
}

void InsetGraphicsParams::init()
{
	subcaptionText = filename = string();
	lyxscale = 100;			// lyx scaling in percentage
	display = grfx::DefaultDisplay;	// see pref
	scale = 100;			// output scaling in percentage
	width = LyXLength();
	height = LyXLength();
	keepAspectRatio = false;	// for latex
	draft = false;			// draft mode
	noUnzip = false;		// unzip files

	bb = string();			// bounding box
	clip = false;			// clip image

	rotateOrigin = "leftBaseline";	// Origin
	rotateAngle = 0.0;		// in degrees
	subcaption = false;		// subfigure
	subcaptionText = string();	// subfigure caption
	special = string();		// userdefined stuff
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
	noUnzip = igp.noUnzip;

	bb = igp.bb;
	clip = igp.clip;

	rotateAngle = igp.rotateAngle;
	rotateOrigin = igp.rotateOrigin;
	subcaption = igp.subcaption;
	subcaptionText = igp.subcaptionText;
	special = igp.special;
}

bool operator==(InsetGraphicsParams const & left,
		InsetGraphicsParams const & right)
{
	if (left.filename == right.filename &&
	    left.lyxscale == right.lyxscale &&
	    left.display == right.display &&
	    left.scale == right.scale &&
	    left.width == right.width &&
	    left.height == right.height &&
	    left.keepAspectRatio == right.keepAspectRatio &&
	    left.draft == right.draft &&
	    left.noUnzip == right.noUnzip &&


	    left.bb == right.bb &&
	    left.clip == right.clip &&

	    lyx::float_equal(left.rotateAngle, right.rotateAngle, 0.001) &&
	    left.rotateOrigin == right.rotateOrigin &&
	    left.subcaption == right.subcaption &&
	    left.subcaptionText == right.subcaptionText &&
	    left.special == right.special
	   )
		return true;

	return false;
}

bool operator!=(InsetGraphicsParams const & left,
		InsetGraphicsParams const & right)
{
	return	!(left == right);
}

void InsetGraphicsParams::Write(ostream & os) const
{
	// Do not write the default values

	if (!filename.empty()) {
		os << "\tfilename " << filename << '\n';
	}
	if (lyxscale != 100)
		os << "\tlyxscale " << lyxscale << '\n';
	if (display != grfx::DefaultDisplay)
		os << "\tdisplay " << grfx::displayTranslator.find(display) << '\n';
	if (scale) {
		if (scale != 100)
			os << "\tscale " << scale << '\n';
	} else {
		if (!width.zero())
			os << "\twidth " << width.asString() << '\n';
	}

	if (!height.zero())
		os << "\theight " << height.asString() << '\n';
	if (keepAspectRatio)
		os << "\tkeepAspectRatio\n";
	if (draft)			// draft mode
		os << "\tdraft\n";
	if (noUnzip)
		os << "\tnoUnzip\n";

	if (!bb.empty())		// bounding box
		os << "\tBoundingBox " << bb << '\n';
	if (clip)			// clip image
		os << "\tclip\n";

	if (rotateAngle != 0.0)
		os << "\trotateAngle " << rotateAngle << '\n';
	if (rotateOrigin != "leftBaseline")
		os << "\trotateOrigin " << rotateOrigin << '\n';
	if (subcaption)
		os << "\tsubcaption\n";
	if (!subcaptionText.empty())
		os << "\tsubcaptionText \"" << subcaptionText << '\"' << '\n';
	if (!special.empty())
		os << "\tspecial " << special << '\n';
}


bool InsetGraphicsParams::Read(LyXLex & lex, string const & token)
{
	if (token == "filename") {
		lex.eatLine();
		filename = lex.getString();
	} else if (token == "lyxscale") {
		lex.next();
		lyxscale = lex.getInteger();
	} else if (token == "display") {
		lex.next();
		string const type = lex.getString();
		display = grfx::displayTranslator.find(type);
	} else if (token == "scale") {
		lex.next();
		scale = lex.getInteger();
	} else if (token == "width") {
		lex.next();
		width = LyXLength(lex.getString());
		scale = 0;
	} else if (token == "height") {
		lex.next();
		height = LyXLength(lex.getString());
	} else if (token == "keepAspectRatio") {
		keepAspectRatio = true;
	} else if (token == "draft") {
		draft = true;
	} else if (token == "noUnzip") {
		noUnzip = true;
	} else if (token == "BoundingBox") {
		for (int i=0; i<4 ;i++) {
			lex.next();
			bb += (lex.getString()+" ");
		}
	} else if (token == "clip") {
		clip = true;
	} else if (token == "rotateAngle") {
		lex.next();
		rotateAngle = lex.getFloat();
	} else if (token == "rotateOrigin") {
		lex.next();
		rotateOrigin=lex.getString();
	} else if (token == "subcaption") {
		subcaption = true;
	} else if (token == "subcaptionText") {
		lex.next();
		subcaptionText = lex.getString();
	} else if (token == "special") {
		lex.eatLine();
		special = lex.getString();

	// catch and ignore following two old-format tokens and their arguments.
	// e.g. "size_kind scale" clashes with the setting of the "scale" keyword.
	} else if (token == "size_kind" || token == "lyxsize_kind") {
		lex.next();
		lex.getString();
	} else {	// If it's none of the above, it's not ours.
		return false;
	}
	return true;
}


grfx::Params InsetGraphicsParams::as_grfxParams(string const & filepath) const
{
	grfx::Params pars;
	pars.filename = filename;
	pars.scale = lyxscale;
	pars.angle = rotateAngle;

	if (!filepath.empty())
		pars.filename = MakeAbsPath(pars.filename, filepath);

	if (clip) {
		pars.bb = bb;

		// Get the original Bounding Box from the file
		string const tmp = readBB_from_PSFile(filename);
		lyxerr[Debug::GRAPHICS] << "BB_from_File: " << tmp << std::endl;
		if (!tmp.empty()) {
			unsigned int const bb_orig_xl = strToInt(token(tmp, ' ', 0));
			unsigned int const bb_orig_yb = strToInt(token(tmp, ' ', 1));

			// new pars.bb values must be >= zero 
			if  (pars.bb.xl > bb_orig_xl)
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
	
	if (display == grfx::DefaultDisplay) {
		pars.display = lyxrc.display_graphics;
	} else {
		pars.display = display;
	}
	
	// Override the above if we're not using a gui
	if (!lyxrc.use_gui) {
		pars.display = grfx::NoDisplay;
	}

	return pars;
}

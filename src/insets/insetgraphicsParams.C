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

#include "support/translator.h"
#include "support/filetools.h"
#include "support/lyxlib.h"
#include "support/LOstream.h"
#include "support/LAssert.h"
#include "support/lstrings.h"
#include "lyxrc.h"
#include "debug.h"
#include "lyxlex.h"

using std::ostream;


namespace {

/// This variable keeps a tab on whether the translator was set with the
/// translations.
bool translatorsSet = false;

/// This is the translator between the Display enum and corresponding lyx
/// file strings.
Translator< InsetGraphicsParams::DisplayType, string >
displayTranslator(InsetGraphicsParams::DEFAULT, "default");

} // namespace anon


InsetGraphicsParams::InsetGraphicsParams()
{
	init();
	// Set translators
	if (! translatorsSet) {
		translatorsSet = true;
		// Fill the display translator
		displayTranslator.addPair(DEFAULT, "default");
		displayTranslator.addPair(MONOCHROME, "monochrome");
		displayTranslator.addPair(GRAYSCALE, "grayscale");
		displayTranslator.addPair(COLOR, "color");
		displayTranslator.addPair(NONE, "none");
	}
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
	bb = string();			// bounding box
	draft = false;			// draft mode
	clip = false;			// clip image
	display = DEFAULT;		// see pref
	subcaption = false;		// subfigure
	noUnzip = false;		// unzip files
	width = LyXLength();		// set to 0pt
	height = LyXLength();
	lyxwidth = LyXLength();		// for the view in lyx
	lyxheight = LyXLength();	// also set to 0pt
	scale = 0;			// unit is %
	lyxscale = 0;			// same for lyxview
	size_kind = DEFAULT_SIZE;	// do nothing
	lyxsize_kind = DEFAULT_SIZE;	// do nothing
	keepAspectRatio = false;	// for latex
	keepLyXAspectRatio = false;	// for lyx
	rotate = false;			// Rotating
	rotateOrigin = "leftBaseline";	// Origin
	rotateAngle = 0.0;		// in degrees
	special = string();		// userdefined stuff
}

void InsetGraphicsParams::copy(InsetGraphicsParams const & igp)
{
	filename = igp.filename;
	bb = igp.bb;
	draft = igp.draft;
	clip = igp.clip;
	display = igp.display;
	subcaption = igp.subcaption;
	subcaptionText = igp.subcaptionText;
	noUnzip = igp.noUnzip;
	keepAspectRatio = igp.keepAspectRatio;
	width = igp.width;
	height = igp.height;
	scale = igp.scale;
	size_kind = igp.size_kind;
	lyxsize_kind = igp.lyxsize_kind;
	lyxwidth = igp.lyxwidth;
	lyxheight = igp.lyxheight;
	keepLyXAspectRatio = igp.keepLyXAspectRatio;
	lyxscale = igp.lyxscale;
	keepLyXAspectRatio = igp.keepLyXAspectRatio;
	rotate = igp.rotate;
	rotateOrigin = igp.rotateOrigin;
	rotateAngle = igp.rotateAngle;
	special = igp.special;
}

bool operator==(InsetGraphicsParams const & left,
		InsetGraphicsParams const & right)
{
	if (left.filename == right.filename &&
	    left.bb == right.bb &&
	    left.draft == right.draft &&
	    left.clip == right.clip &&
	    left.display == right.display &&
	    left.subcaption == right.subcaption &&
	    left.noUnzip == right.noUnzip &&
	    left.subcaptionText == right.subcaptionText &&
	    left.keepAspectRatio == right.keepAspectRatio &&
	    left.width == right.width &&
	    left.height == right.height &&
	    left.scale == right.scale &&
	    left.size_kind == right.size_kind &&
	    left.lyxsize_kind == right.lyxsize_kind &&
	    left.lyxwidth == right.lyxwidth &&
	    left.lyxheight == right.lyxheight &&
	    left.keepLyXAspectRatio == right.keepLyXAspectRatio &&
	    left.lyxscale == right.lyxscale &&
	    left.keepLyXAspectRatio == right.keepLyXAspectRatio &&
	    left.rotate == right.rotate &&
	    left.rotateOrigin == right.rotateOrigin &&
	    lyx::float_equal(left.rotateAngle, right.rotateAngle, 0.001 &&
			     left.special == right.special)
		)
		return true;

	return false;
}

bool operator!=(InsetGraphicsParams const & left,
		InsetGraphicsParams const & right)
{
	return	!(left == right);
}


namespace {

InsetGraphicsParams::sizeKind getSizeKind(string const & str_in)
{
	if (str_in == "width_height")
		return InsetGraphicsParams::WH;
	if (str_in == "scale")
		return InsetGraphicsParams::SCALE;

	// all other like "original"
	return InsetGraphicsParams::DEFAULT_SIZE;
}


string const getSizeKindStr(InsetGraphicsParams::sizeKind sK_in)
{
	if (sK_in == InsetGraphicsParams::SCALE)
		return "scale";
	if (sK_in == InsetGraphicsParams::WH)
		return "width_height";

	// all other like DEFAULT_SIZE"
	return "original";
}	

} //anon


void InsetGraphicsParams::Write(ostream & os) const
{
	// If there is no filename, write nothing for it.
	if (!filename.empty()) {
		os << "\tfilename " << filename << '\n';
	}
	if (!bb.empty())		// bounding box
		os << "\tBoundingBox " << bb << '\n';
	if (clip)			// clip image
		os << "\tclip\n";
	if (draft)			// draft mode
		os << "\tdraft\n";
	// Save the display type for the view inside lyx
	os << "\tdisplay " << displayTranslator.find(display) << '\n';
	// Save the subcaption status
	if (subcaption)
		os << "\tsubcaption\n";
	if (!subcaptionText.empty())
		os << "\tsubcaptionText \"" << subcaptionText << '\"' << '\n';
	if (noUnzip)
		os << "\tnoUnzip\n";
	os << "\tsize_kind " << getSizeKindStr(size_kind) << '\n';
	if (!width.zero())
		os << "\twidth " << width.asString() << '\n';
	if (!height.zero())
		os << "\theight " << height.asString() << '\n';
	if (scale != 0)
		os << "\tscale " << scale << '\n';
	if (keepAspectRatio)
		os << "\tkeepAspectRatio\n";
	if (rotate)
		os << "\trotate\n";
	if (rotateAngle != 0.0)
		os << "\trotateAngle " << rotateAngle << '\n';
	if (!rotateOrigin.empty())
		os << "\trotateOrigin " << rotateOrigin << '\n';
	if (!special.empty())
		os << "\tspecial " << special << '\n';
	// the values for the view in lyx
	os << "\tlyxsize_kind " << getSizeKindStr(lyxsize_kind) << '\n';
	if (!lyxwidth.zero())		// the lyx-viewsize
		os << "\tlyxwidth " << lyxwidth.asString() << '\n';
	if (!lyxheight.zero())
		os << "\tlyxheight " << lyxheight.asString();
	if (keepLyXAspectRatio)
		os << "\tkeepLyXAspectRatio\n";
	if (lyxscale != 0)
		os << "\tlyxscale " << lyxscale << '\n';
}


bool InsetGraphicsParams::Read(LyXLex & lex, string const & token)
{
	if (token == "filename") {
		lex.eatLine();
		filename = lex.getString();
	} else if (token == "BoundingBox") {
		for (int i=0; i<4 ;i++) {
			lex.next();
			bb += (lex.getString()+" ");
		}
	} else if (token == "clip") {
		clip = true;
	} else if (token == "draft") {
		draft = true;
	} else if (token == "display") {
		lex.next();
		string const type = lex.getString();
		display = displayTranslator.find(type);
	} else if (token == "subcaption") {
		subcaption = true;
	} else if (token == "subcaptionText") {
		lex.next();
		subcaptionText = lex.getString();
	} else if (token == "noUnzip") {
		noUnzip = true;
	} else if (token == "size_kind") {
		lex.next();
		size_kind = getSizeKind(lex.getString());
	} else if (token == "width") {
		lex.next();
		width = LyXLength(lex.getString());
	} else if (token == "height") {
		lex.next();
		height = LyXLength(lex.getString());
	} else if (token == "keepAspectRatio") {
		keepAspectRatio = true;
	} else if (token == "scale") {
		lex.next();
		scale = lex.getInteger();
	} else if (token == "rotate") {
		rotate = true;
	} else if (token == "rotateAngle") {
		lex.next();
		rotateAngle = lex.getFloat();
	} else if (token == "rotateOrigin") {
		lex.next();
		rotateOrigin=lex.getString();
	} else if (token == "lyxsize_kind") {
		lex.next();
		lyxsize_kind = getSizeKind(lex.getString());
	} else if (token == "lyxwidth") {
		lex.next();
		lyxwidth = LyXLength(lex.getString());
	} else if (token == "lyxheight") {
		lex.next();
		lyxheight = LyXLength(lex.getString());
	} else if (token == "keepLyXAspectRatio") {
		keepLyXAspectRatio = true;
	} else if (token == "lyxscale") {
		lex.next();
		lyxscale = lex.getInteger();
	} else if (token == "special") {
		lex.eatLine();
		special = lex.getString();
	} else {	// If it's none of the above, its not ours.
		return false;
	}
	return true;
}


grfx::Params InsetGraphicsParams::as_grfxParams(string const & filepath) const
{
	grfx::Params pars;
	pars.width    = 0;
	pars.height   = 0;
	pars.scale    = 0;
	pars.keepLyXAspectRatio = false;
	pars.angle    = 0;
	pars.filename = filename;

	if (!filepath.empty()) {
		pars.filename = MakeAbsPath(pars.filename, filepath);
	}

	if (clip) {
		pars.bb = bb;

		// Get the original Bounding Box from the file
		string const tmp = readBB_from_PSFile(filename);
		lyxerr[Debug::GRAPHICS] << "BB_from_File: " << tmp << std::endl;
		if (!tmp.empty()) {
			int const bb_orig_xl = strToInt(token(tmp, ' ', 0));
			int const bb_orig_yb = strToInt(token(tmp, ' ', 1));

			pars.bb.xl -= bb_orig_xl;
			pars.bb.xr -= bb_orig_xl;
			pars.bb.yb -= bb_orig_yb;
			pars.bb.yt -= bb_orig_yb;
		}

		pars.bb.xl = std::max(0, pars.bb.xl);
		pars.bb.xr = std::max(0, pars.bb.xr);
		pars.bb.yb = std::max(0, pars.bb.yb);
		pars.bb.yt = std::max(0, pars.bb.yt);

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
	
	if (rotate)
		pars.angle = int(rotateAngle);

	switch (display) {
		case InsetGraphicsParams::NONE:
			pars.display = grfx::NoDisplay;
		break;

		case InsetGraphicsParams::MONOCHROME:
			pars.display = grfx::MonochromeDisplay;
		break;

		case InsetGraphicsParams::GRAYSCALE: 
			pars.display = grfx::GrayscaleDisplay;

		case InsetGraphicsParams::COLOR:
			pars.display = grfx::ColorDisplay;
		break;

		default: {
			if (lyxrc.display_graphics == "mono")
				pars.display = grfx::MonochromeDisplay;
			else if (lyxrc.display_graphics == "gray")
				pars.display = grfx::GrayscaleDisplay;
			else if (lyxrc.display_graphics == "color")
				pars.display = grfx::ColorDisplay;
			else
				pars.display = grfx::NoDisplay;
		}
	}
	
	// Override the above if we're not using a gui
	if (!lyxrc.use_gui) {
		pars.display = grfx::NoDisplay;
	}

	if (lyxsize_kind == InsetGraphicsParams::SCALE) {
		pars.scale = lyxscale;

	} else if (lyxsize_kind == InsetGraphicsParams::WH) {
		pars.width = lyxwidth.inBP();
		pars.height = lyxheight.inBP();
		pars.keepLyXAspectRatio = keepLyXAspectRatio;
	}
	
	return pars;
}

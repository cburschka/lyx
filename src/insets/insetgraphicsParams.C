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

#include "support/translator.h"
#include "support/filetools.h"
#include "support/lyxlib.h"
#include "support/LOstream.h"
#include "support/LAssert.h"


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
	size_type = DEFAULT_SIZE;	// do nothing
	lyxsize_type = DEFAULT_SIZE;	// do nothing
	keepLyXAspectRatio = false;	// only for LyXview
	keepAspectRatio = false;	// only for latex
	rotate = false;			// Rotating
	rotateOrigin = "center";	// Origin
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
	size_type = igp.size_type;
	lyxsize_type = igp.lyxsize_type;
	lyxwidth = igp.lyxwidth;
	lyxheight = igp.lyxheight;
	keepLyXAspectRatio = igp.keepLyXAspectRatio;
	lyxscale = igp.lyxscale;
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
	    left.size_type == right.size_type &&
	    left.lyxsize_type == right.lyxsize_type &&
	    left.lyxwidth == right.lyxwidth &&
	    left.lyxheight == right.lyxheight &&
	    left.keepLyXAspectRatio == right.keepLyXAspectRatio &&
	    left.lyxscale == right.lyxscale &&
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
	// we always need the size type
	// 0: no special
	// 1: width/height combination
	// 2: scale
	os << "\tsize_type " <<  size_type << '\n';
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
	os << "\tlyxsize_type " <<  lyxsize_type << '\n';
	if (!lyxwidth.zero())		// the lyx-viewsize
		os << "\tlyxwidth " << lyxwidth.asString() << '\n';
	if (!lyxheight.zero())
		os << "\tlyxheight " << lyxheight.asString();
	if (keepLyXAspectRatio)
		os << "\tkeepLyXAspectRatio\n";
	if (lyxscale != 0)
		os << "\tlyxscale " << lyxscale << '\n';
}


bool InsetGraphicsParams::Read(LyXLex & lex, string const& token)
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
	} else if (token == "size_type") {
		lex.next();
		switch (lex.getInteger()) {
		case 0:
			size_type = DEFAULT_SIZE;
			break;
		case 1:
			size_type = WH;
			break;
		case 2:
			size_type = SCALE;
			break;
		}
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
	} else if (token == "lyxsize_type") {
		lex.next();
		switch (lex.getInteger()) {
		case 0:
			lyxsize_type = DEFAULT_SIZE;
			break;
		case 1:
			lyxsize_type = WH;
			break;
		case 2:
			lyxsize_type = SCALE;
			break;
		}
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

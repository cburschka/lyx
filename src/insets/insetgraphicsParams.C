// -*- C++ -*-
/* This file is part of
 * =================================================
 * 
 *          LyX, The Document Processor
 *          Copyright 1995 Matthias Ettrich.
 *          Copyright 1995-2000 The LyX Team.
 *
 *          This file Copyright 2000 Baruch Even
 * ================================================= */

#include <config.h> 

#ifdef __GNUG__
#pragma implementation
#endif 

#include "insetgraphicsParams.h"

#include "support/translator.h"
#include "support/filetools.h"

#include "support/LAssert.h"

namespace {

/// This variable keeps a tab on whether the translator was set with the
/// translations.
bool translatorsSet = false;

/// This is the translator between the Resize enum and corresponding lyx
/// file strings.
Translator< InsetGraphicsParams::Resize, string >
resizeTranslator(InsetGraphicsParams::DEFAULT_SIZE, "default");

/// This is the translator between the Origin enum and corresponding lyx
/// file strings.
Translator< InsetGraphicsParams::Origin, string >
originTranslator(InsetGraphicsParams::DEFAULT, "default");

/// This is the translator between the Display enum and corresponding lyx
/// file strings.
Translator< InsetGraphicsParams::DisplayType, string >
displayTranslator(InsetGraphicsParams::MONOCHROME, "monochrome");

} // namespace anon


InsetGraphicsParams::InsetGraphicsParams()
{
	init();

	// Set translators
	if (! translatorsSet) {
		translatorsSet = true;

		// Fill the resize translator
		resizeTranslator.addPair(DEFAULT_SIZE, "default");
		resizeTranslator.addPair(CM, "cm");
		resizeTranslator.addPair(INCH, "inch");
		resizeTranslator.addPair(PERCENT_PAGE, "percentOfPage");
		resizeTranslator.addPair(PERCENT_COLUMN, "percentOfColumn");

		// Fill the origin translator
		originTranslator.addPair(DEFAULT, "default");
		originTranslator.addPair(LEFTTOP, "leftTop");
		originTranslator.addPair(LEFTCENTER, "leftCenter");
		originTranslator.addPair(LEFTBASELINE, "leftBaseLine");
		originTranslator.addPair(LEFTBOTTOM, "leftBottom");
		originTranslator.addPair(CENTERTOP, "centerTop");
		originTranslator.addPair(CENTER, "center");
		originTranslator.addPair(CENTERBASELINE, "centerBaseLine");
		originTranslator.addPair(CENTERBOTTOM, "centerBottom");
		originTranslator.addPair(RIGHTTOP, "rightTop");
		originTranslator.addPair(RIGHTCENTER, "rightCenter");
		originTranslator.addPair(RIGHTBASELINE, "rightBaseLine");
		originTranslator.addPair(RIGHTBOTTOM, "rightBottom");
		originTranslator.addPair(REFERENCE_POINT, "referencePoint");

		// Fill the display translator
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
	display = MONOCHROME;
	inlineFigure = false;
	subcaption = false;
	keepAspectRatio = true;
	widthResize = DEFAULT_SIZE;
	widthSize = 0.0;
	heightResize = DEFAULT_SIZE;
	heightSize = 0.0;
	rotateOrigin = DEFAULT;
	rotateAngle = 0;

	testInvariant();
}

void InsetGraphicsParams::copy(InsetGraphicsParams const & igp)
{
	filename = igp.filename;
	display = igp.display;
	inlineFigure = igp.inlineFigure;
	subcaption = igp.subcaption;
	subcaptionText = igp.subcaptionText;
	keepAspectRatio = igp.keepAspectRatio;
	widthResize = igp.widthResize;
	widthSize = igp.widthSize;
	heightResize = igp.heightResize;
	heightSize = igp.heightSize;
	rotateOrigin = igp.rotateOrigin;
	rotateAngle = igp.rotateAngle;

	testInvariant();
}

void InsetGraphicsParams::testInvariant() const
{
	// Filename might be empty (when the dialog is first created).
	// Assert(!filename.empty());

	Assert(display == COLOR ||
	       display == MONOCHROME ||
	       display == GRAYSCALE ||
	       display == NONE
	      );

	Assert(widthResize == DEFAULT_SIZE ||
	       widthResize == CM ||
	       widthResize == INCH ||
	       widthResize == PERCENT_PAGE ||
	       widthResize == PERCENT_COLUMN
	      );

	Assert(heightResize == DEFAULT_SIZE ||
	       heightResize == CM ||
	       heightResize == INCH ||
	       heightResize == PERCENT_PAGE
	      );

	Assert(widthSize >= 0.0);
	Assert(heightSize >= 0.0);

	// Angle is in degrees and ranges -360 < angle < 360
	// The reason for this is that in latex there is a meaning for the
	// different angles and they are not necessarliy interchangeable,
	// it depends on the rotation origin.
	Assert(rotateAngle < 360);
	Assert(rotateAngle > -360);

}

bool operator==(InsetGraphicsParams const & left,
                InsetGraphicsParams const & right)
{
	if (left.filename == right.filename &&
	        left.display == right.display &&
	        left.inlineFigure == right.inlineFigure &&
	        left.subcaption == right.subcaption &&
	        left.subcaptionText == right.subcaptionText &&
	        left.keepAspectRatio == right.keepAspectRatio &&
	        left.widthResize == right.widthResize &&
	        left.widthSize == right.widthSize &&
	        left.heightResize == right.heightResize &&
	        left.heightSize == right.heightSize &&
	        left.rotateOrigin == right.rotateOrigin &&
	        left.rotateAngle == right.rotateAngle
	   )
		return true;

	return false;
}

bool operator!=(InsetGraphicsParams const & left,
                InsetGraphicsParams const & right)
{
	return 	!(left == right);
}


namespace {

void writeResize(ostream & os, string const & key,
                        InsetGraphicsParams::Resize resize, double size)
{
	os << ' ' << key << "Resize ";

	os << resizeTranslator.find(resize);
	os << ' ' << key << ' ' << size << '\n';
}

void writeOrigin(ostream & os,
                        InsetGraphicsParams::Origin origin)
{
	os << " rotateOrigin " << originTranslator.find(origin);
	os << '\n';
}

} // namespace anon


void InsetGraphicsParams::Write(Buffer const * buf, ostream & os) const
{
	// If there is no filename, write nothing for it.
	if (! filename.empty()) {
		os << "filename "
		<< MakeRelPath(filename, OnlyPath(buf->fileName()))
		<< '\n';
	}

	// Save the display type
	os << " display " << displayTranslator.find(display) << '\n';

	// Save the inline status
	if (inlineFigure)
		os << " inline";

	// Save the subcaption status
	if (subcaption)
		os << " subcaption";

	if (! subcaptionText.empty())
		os << " subcaptionText \"" << subcaptionText << '\"' << '\n';

	writeResize(os, "width", widthResize, widthSize);
	writeResize(os, "height", heightResize, heightSize);

	writeOrigin(os, rotateOrigin);
	if (rotateAngle != 0)
		os << " rotateAngle " << rotateAngle << '\n';
}


namespace {

void readResize(InsetGraphicsParams * igp, bool height,
                       string const & token)
{
	InsetGraphicsParams::Resize resize = InsetGraphicsParams::DEFAULT_SIZE;

	resize = resizeTranslator.find(token);

	if (height)
		igp->heightResize = resize;
	else
		igp->widthResize = resize;
}


void readOrigin(InsetGraphicsParams * igp, string const & token)
{
	// TODO: complete this function.
	igp->rotateOrigin = originTranslator.find(token);
}

} // namespace anon


bool InsetGraphicsParams::Read(Buffer const * buf, LyXLex & lex,
                               string const& token)
{
	if (token == "filename") {
		lex.next();
		filename = lex.GetString();

		if (!filename.empty()) {
			// Make the filename with absolute directory.
			filename = MakeAbsPath(filename, OnlyPath(buf->fileName()));
		}
	} else if (token == "display") {
		lex.next();
		string const type = lex.GetString();

		display = displayTranslator.find(type);
	} else if (token == "inline") {
		inlineFigure = true;
	} else if (token == "subcaption") {
		subcaption = true;
	} else if (token == "subcaptionText") {
		lex.next();
		subcaptionText = lex.GetString();
	} else if (token == "widthResize") {
		lex.next();
		string const token = lex.GetString();

		readResize(this, false, token);
	} else if (token == "width") {
		lex.next();
		widthSize = lex.GetFloat();
	} else if (token == "heightResize") {
		lex.next();
		string const token = lex.GetString();

		readResize(this, true, token);
	} else if (token == "height") {
		lex.next();
		heightSize = lex.GetFloat();
	} else if (token == "rotateOrigin") {
		lex.next();
		string const token = lex.GetString();

		readOrigin(this, token);
	} else if (token == "rotateAngle") {
		lex.next();
		rotateAngle = lex.GetInteger();
	} else {
		// If it's none of the above, its not ours.
		return false;
	}

	return true;
}

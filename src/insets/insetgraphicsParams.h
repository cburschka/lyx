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

#ifndef INSETGRAPHICSPARAMS_H
#define INSETGRAPHICSPARAMS_H

#ifdef __GNUG__
#pragma interface
#endif 

#include "LString.h"

#include "buffer.h"
#include "lyxlex.h"

using std::ostream;

/// This struct holds all the parameters needed by insetGraphics.
struct InsetGraphicsParams
{
	/// Image filename.
	string filename;

	/// How do we display the image?
	enum DisplayType {
	    /// In full color range (if it's not in color we show it as it is)
	    COLOR,
	    /// In Grayscale (256 shades of gray).
	    GRAYSCALE,
	    /// In black and white.
	    MONOCHROME,
	    /// Don't display it on screen, only keep a frame in place.
	    NONE
	};

	/// How to display the image
	DisplayType display;

	/// Is the figure inlined? (not in a paragraph of its own).
	bool inlineFigure;

	/// Do we have a subcaption?
	bool subcaption;

	/// The text of the subcaption.
	string subcaptionText;

	/// This is the different origins that the graphicx package support.
	enum Origin {
	    DEFAULT,
	    LEFTTOP,
	    LEFTCENTER,
	    LEFTBASELINE,
	    LEFTBOTTOM,
	    CENTERTOP,
	    CENTER,
	    CENTERBASELINE,
	    CENTERBOTTOM,
	    RIGHTTOP,
	    RIGHTCENTER,
	    RIGHTBASELINE,
	    RIGHTBOTTOM,
	    REFERENCE_POINT = LEFTBASELINE
	};

	/** The resize of the image, is it the default size, in cm, inch or
	    percentage of the page/column width/height */
	enum Resize {
	    DEFAULT_SIZE,
	    CM,
	    INCH,
	    PERCENT_PAGE,
	    PERCENT_COLUMN
	};


	/// Keep the ratio between height and width when resizing.
	bool keepAspectRatio;

	/// What width resize to do?
	Resize widthResize;
	/// Value of width resize
	float widthSize;
	/// What height resize to do?
	Resize heightResize;
	/// Value of height resize
	float heightSize;

	/// Origin point of rotation
	Origin rotateOrigin;
	/// Rotation angle.
	int rotateAngle;
	///
	InsetGraphicsParams();
	///
	InsetGraphicsParams(InsetGraphicsParams const &);
	///
	InsetGraphicsParams & operator=(InsetGraphicsParams const &);

	/// Save the parameters in the LyX format stream.
	void Write(Buffer const * buf, ostream & os) const;

	/// If the token belongs to our parameters, read it.
	bool Read(Buffer const * buf, LyXLex & lex, string const & token);

	/// Test the struct to make sure that all the options have legal values.
	void testInvariant() const;

private:
	/// Initialize the object to a default status.
	void init();

	/// Copy the other objects content to us, used in copy c-tor and assignment
	void copy(InsetGraphicsParams const & params);
};

///
bool operator==(InsetGraphicsParams const &, InsetGraphicsParams const &);

#endif 

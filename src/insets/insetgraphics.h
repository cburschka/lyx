// -*- C++ -*-
/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 * 	 
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2000 the LyX Team.
 *
 * ====================================================== */

#ifndef INSET_GRAPHICS_H
#define INSET_GRAPHICS_H

#ifdef __GNUG__
#pragma interface
#endif

#include "insets/lyxinset.h"
#include "LaTeXFeatures.h"
#include "vspace.h"
#include "insets/BoundingBox.h"

///
class InsetGraphics : public Inset {
public:
	///
	int ascent(Painter &, LyXFont const &) const;
	///
	int descent(Painter &, LyXFont const &) const;
	///
	int width(Painter &, LyXFont const &) const;
	///
	void draw(Painter &, LyXFont const &,
			  int baseline, float & x) const;
	///
	void Edit(BufferView *, int, int, unsigned int);
	///
	EDITABLE Editable() const;
	///
	void Write(ostream &) const;
	///
	void Read(LyXLex & lex);
	/** returns the number of rows (\n's) of generated tex code.
	 fragile != 0 means, that the inset should take care about
	 fragile commands by adding a \protect before.
	 */
	int Latex(ostream &, signed char fragile) const;
#ifndef USE_OSTREAM_ONLY
	///
	int Latex(string & file, signed char fragile) const;
	///
	int Linuxdoc(string & /*file*/) const;
	///
	int DocBook(string & /*file*/) const;
#else
	///
	int Linuxdoc(ostream &) const;
	///
	int DocBook(ostream &) const;
#endif
	/// Updates needed features for this inset.
	void Validate(LaTeXFeatures & features) const;

	/// returns LyX code associated with the inset. Used for TOC, ...)
	Inset::Code LyxCode() const { return Inset::GRAPHICS_CODE; }
  
	///
	Inset * Clone() const;
private:
	///
	string graphicsfile;
	///
	Pixmap pixmap;
	
	// We need variables to store the size of the boundingbox and
	// to store a pointer to the pixmap in.. The question is if
	// these should be in a "pixmap" class.
	// We also need to have variables about rotation and scaling,
	// width and height. in effect all the paramters that
	// \includegrapichs can handle. (the graphix.sty package)

	/** The "bounding box" of the graphics image. Its value field
	    must contain four dimensions, separated by spaces.	*/
	BoundingBox bb;
	/** The bounding box above is used for display and file both.
	    But we need this variable below to know if we shall use the
	    bounding box in the LaTex command or not. (i.e. if the user
	    has manually set the bb.) */
	bool use_bb;
	
	/** When a graphics file is parsed we first check for BoundingBox
	    and then for HiResBoundingBox, and set hiresbb depending on this.
	    This key makes LaTeX search for %%HiResBoundingBox comments
	    instead of the normal %%BoundingBox. Some applications use
	    this to specify more precise bounding boxes, becase the cumbers
	    can normally only have integer values. It is a Boolean, either
	    "true" or "false". */
	bool hiresbb;
	/** This key takes four arguments (like bb), but in this case the
	    origin is taken with respect to the bounding box specified in
	    the file. So to view a 20 bp square at the lower left-hand corner
	    of the picture, specify viewport=0 0 20 20. */
	BoundingBox viewport;
	/** Similar to the viewport key, but the four dimensions correspond
	    to the amount of space to be trimmed (cut off) at the left-hand
	    side, bottom, right-hand side and top of the included graphics. */
	BoundingBox trim;
	/// Natural height of figure
	LyXLength natheight;
	/// Natural width of figure.
	LyXLength natwidth;
	/// Rotation angle (in degrees, counterclockwise).
	float angle;
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
	/** Origin for rotation, similar to the origin parameter of
	    the \rotatebox command described on p.46 and Fig. 2.2 on p.46. */
	Origin origin;
	/// Required width (the width of the image is scaled to that value).
	LyXLength g_width;
	/// Required height (the height of the image is scaled to that value).
	LyXLength g_height;
	/** Required total height (the total height of the image is scaled
	    to that value). This key should be used instead of height if
	    images are rotated over 90 degrees, since the height can
	    disappear (and become the depth) and LaTeX will have difficulties
	    satisfying the user's request. */
	LyXLength totalheight;
	/** Boolean variable that can have the values "true" and "false"
	    (se above for defaults). When true, specifying both width and
	    height parameters does not distort the picture, but the image
	    is scaled so that neither of the width of height exceeds the
	    given dimensions. */
	bool keepaspectratio;
	/// Scale factor
	float scale;
	/** Clip the graphic to the bounding box. It is a Boolean, either
	    "true" or "false". */
	bool clip;
	/// Locally switch to draft mode. A Boolean valued key, like clip.
	bool draft;
	/// The graphics type.
	string type;
	///x The file extension of the file containing the image data.
	string ext;
	/// The file extension of the file "read" by LaTeX.
	string read;
	/// Any command to be applied to the file.
	string command;
};

#endif

// -*- C++ -*-
/* This file is part of
 * =================================================
 *
 *          LyX, The Document Processor
 *          Copyright 1995 Matthias Ettrich.
 *          Copyright 1995-2002 The LyX Team.
 *
 * \author Baruch Even
 * \author Herbert Voss <voss@lyx.org>
 * ================================================= */

#ifndef INSETGRAPHICSPARAMS_H
#define INSETGRAPHICSPARAMS_H

#ifdef __GNUG__
#pragma interface
#endif

#include "LString.h"

#include "buffer.h"
#include "lyxlex.h"

#include "graphics/GraphicsParams.h"


/// This struct holds all the parameters needed by insetGraphics.
struct InsetGraphicsParams
{
	/// How do we display the image?
	enum DisplayType {
	    DEFAULT,		// whatever is in lyxrc.display_graphics
	    COLOR,		// full color range
	    GRAYSCALE,		// 256 shades of gray
	    MONOCHROME,		// In black and white.
	    NONE		// only keep a frame in place.
	};
	///
	enum sizeType {		// for latex and/or lyx
	    DEFAULT_SIZE,	// like none
	    WH,			// width/height values
	    SCALE		// percentage value
	};
	/// Image filename.
	string filename;
	/// Do we have a subcaption?
	bool subcaption;
	/// The text of the subcaption.
	string subcaptionText;
	/// Do we rotate?
	bool rotate;
	/// Origin point of rotation
	string rotateOrigin;
	/// Rotation angle.
	float rotateAngle;
	/// clip image
	bool clip;
	/// draft mode
	bool draft;
	/// what to do with zipped files
	bool noUnzip;
	/// The bounding box with "xLB yLB yRT yRT ", divided by a space!
	string bb;
	/// Type of rescaling
	sizeType size_type;
	/// three possible values for rescaling (latex)
	LyXLength width;
	///
	LyXLength height;
	///
	int scale;
	/// Keep the ratio between height and width when resizing.
	bool keepAspectRatio;
	/// any userdefined special command
	string special;
	/// How to display the image inside lyx
	DisplayType display;
	/// Typ of the LyXView, same as for latex
	sizeType lyxsize_type;
	/// the size for the view inside lyx
	LyXLength lyxwidth;
	///
	LyXLength lyxheight;
	/// Typ of rescaling the Screen inside lyx
	int lyxscale;
	///
	InsetGraphicsParams();
	///
	InsetGraphicsParams(InsetGraphicsParams const &);
	///
	InsetGraphicsParams & operator=(InsetGraphicsParams const &);
	/// Save the parameters in the LyX format stream.
	void Write(std::ostream & os) const;
	/// If the token belongs to our parameters, read it.
	bool Read(LyXLex & lex, string const & token);
	/// convert
  // Only a subset of InsetGraphicsParams is needed for display purposes.
  // This function also interrogates lyxrc to ascertain whether
  // to display or not.
	grfx::GParams asGParams(string const & filepath) const;

private:
	/// Initialize the object to a default status.
	void init();
	/// Copy the other objects content to us, used in copy c-tor and assignment
	void copy(InsetGraphicsParams const & params);
};

///
bool operator==(InsetGraphicsParams const &, InsetGraphicsParams const &);
///
bool operator!=(InsetGraphicsParams const &, InsetGraphicsParams const &);

#endif

// -*- C++ -*-
/**
 * \file insetgraphicsParams.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Baruch Even
 * \author Herbert Voss
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef INSETGRAPHICSPARAMS_H
#define INSETGRAPHICSPARAMS_H

#ifdef __GNUG__
#pragma interface
#endif

#include "graphics/GraphicsTypes.h"
#include "LString.h"
#include "lyxlength.h"

class LyXLex;

namespace grfx {
	class Params;
}

/// This struct holds all the parameters needed by insetGraphics.
struct InsetGraphicsParams
{
	/// Image filename.
	string filename;
	/// Scaling the Screen inside Lyx
	unsigned int lyxscale;
	/// How to display the image inside LyX
	grfx::DisplayType display;
	/// Scaling for output (LaTeX)
	float scale;
	/// sizes for output (LaTeX)
	LyXLength width;
	///
	LyXLength height;
	/// Keep the ratio between height and width when resizing.
	bool keepAspectRatio;
	/// draft mode
	bool draft;
	/// what to do with zipped files
	bool noUnzip;

	/// The bounding box with "xLB yLB yRT yRT ", divided by a space!
	string bb;
	/// clip image
	bool clip;

	/// Rotation angle.
	float rotateAngle;
	/// Origin point of rotation
	string rotateOrigin;
	/// Do we have a subcaption?
	bool subcaption;
	/// The text of the subcaption.
	string subcaptionText;
	/// any userdefined special command
	string special;

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
	grfx::Params as_grfxParams(string const & filepath) const;

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

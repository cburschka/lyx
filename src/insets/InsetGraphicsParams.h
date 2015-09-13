// -*- C++ -*-
/**
 * \file InsetGraphicsParams.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Baruch Even
 * \author Herbert Voß
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef INSETGRAPHICSPARAMS_H
#define INSETGRAPHICSPARAMS_H


#include "graphics/GraphicsParams.h"

#include "support/FileName.h"

#include <string>

namespace lyx {

namespace graphics { class Params; }

class Lexer;
class Buffer;


/// This class holds all the parameters needed by insetGraphics.
class InsetGraphicsParams
{
public:
	/// Image filename.
	support::DocFileName filename;
	/// Scaling the Screen inside Lyx
	unsigned int lyxscale;
	/// If to display the image inside LyX
	bool display;
	/// Scaling for output (LaTeX)
	std::string scale;
	/// sizes for output (LaTeX)
	Length width;
	///
	Length height;
	/// Keep the ratio between height and width when resizing.
	bool keepAspectRatio;
	/// draft mode
	bool draft;
	/// scale image before rotating
	bool scaleBeforeRotation;

	/// The bounding box
	graphics::BoundingBox bbox;
	/// clip image
	bool clip;

	/// Rotation angle.
	std::string rotateAngle;
	/// Origin point of rotation
	std::string rotateOrigin;
	/// any userdefined special command
	std::string special;

	///
	InsetGraphicsParams();
	///
	InsetGraphicsParams(InsetGraphicsParams const &);
	///
	InsetGraphicsParams & operator=(InsetGraphicsParams const &);
	/// Save the parameters in the LyX format stream.
	/// Buffer is needed to figure out if a figure is embedded.
	void Write(std::ostream & os, Buffer const & buf) const;
	/// If the token belongs to our parameters, read it.
	bool Read(Lexer & lex, std::string const & token, std::string const & bufpath);
	/// convert
	// Only a subset of InsetGraphicsParams is needed for display purposes.
	// This function also interrogates lyxrc to ascertain whether
	// to display or not.
	graphics::Params as_grfxParams() const;

	// FIXME UNICODE. Write functions need to use odostream instead of ostream firstly.
	/// Identification of the graphics template. No template equals empty string.
	std::string groupId;
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

} // namespace lyx

#endif

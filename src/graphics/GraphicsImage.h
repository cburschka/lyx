// -*- C++ -*-
/**
 *  \file GraphicsImage.h
 *  Copyright 2002 the LyX Team
 *  Read the file COPYING
 *
 *  \author Baruch Even <baruch.even@writeme.com>
 *  \author Angus Leeming <a.leeming@ic.ac.uk>
 *
 *  An abstract base class for the images themselves.
 *  Allows the user to retrieve the pixmap, once loaded and to issue commands
 *  to modify it.
 *
 *  The signals newImage and loadableFormats are connected to the approriate
 *  derived classes elsewhere, allowing the graphics cache to access them
 *  without knowing anything about their instantiation.
 *
 *  The loading process can be asynchronous, but cropping, rotating and
 *  scaling block execution.
 */

#ifndef GRAPHICSIMAGE_H
#define GRAPHICSIMAGE_H

#include "GraphicsTypes.h"
#include "LString.h"

#include <boost/shared_ptr.hpp>

#include <sigc++/signal_system.h>

#include <X11/X.h>

#include <vector>
#include <utility>

#ifdef __GNUG__
#pragma interface
#endif

namespace grfx {

class GParams;

class GImage
{
public:
	/// A list of supported formats.
	typedef std::vector<string> FormatList;
	/** This will be connected to a function that will return whichever
	 *  derived class we desire.
	 */
	static SigC::Signal0<ImagePtr> newImage;

	/// Return the list of loadable formats.
	static SigC::Signal0<FormatList> loadableFormats;

	///
	virtual ~GImage() {}

	/// Create a copy
	virtual GImage * clone() const = 0;

	///
	virtual Pixmap getPixmap() const = 0;

	/// Get the image width
	virtual unsigned int getWidth() const = 0;

	/// Get the image height
	virtual unsigned int getHeight() const = 0;

	/** At the end of the loading or modification process, return the new
	 *  image by emitting this signal */
	typedef SigC::Signal1<void, bool> SignalType;
	///
	typedef boost::shared_ptr<SignalType> SignalTypePtr;

	/// Start loading the image file.
	virtual void load(string const & filename, SignalTypePtr) = 0;

	/** Generate the pixmap.
	 *  Uses the params to decide on color, grayscale etc.
	 *  Returns true if the pixmap is created.
	 */
	virtual bool setPixmap(GParams const & params) = 0;

	/// Clip the image using params.
	virtual void clip(GParams const & params) = 0;

	/// Rotate the image using params.
	virtual void rotate(GParams const & params) = 0;

	/// Scale the image using params.
	virtual void scale(GParams const & params) = 0;

protected:
	/** Uses the params to ascertain the dimensions of the scaled image.
	 *  Returned as make_pair(width, height).
	 *  If something geso wrong, returns make_pair(getWidth(), getHeight())
	 */
	std::pair<unsigned int, unsigned int>
	getScaledDimensions(GParams const & params) const;
};

} // namespace grfx

#endif // GRAPHICSIMAGE_H

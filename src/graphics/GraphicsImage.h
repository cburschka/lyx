// -*- C++ -*-
/**
 *  \file GraphicsImage.h
 *  Read the file COPYING
 *
 *  \author Baruch Even 
 *  \author Angus Leeming 
 *
 * Full author contact details available in file CREDITS
 *
 *  An abstract base class for the images themselves.
 *  Allows the user to retrieve the pixmap, once loaded and to issue commands
 *  to modify it.
 *
 *  The boost::functions newImage and loadableFormats are connected to the
 *  appropriate derived classes elsewhere, allowing the graphics cache to
 *  access them without knowing anything about their instantiation.
 *
 *  The loading process can be asynchronous, but cropping, rotating and
 *  scaling block execution.
 */

#ifndef GRAPHICSIMAGE_H
#define GRAPHICSIMAGE_H

#include "LString.h"

#include <boost/shared_ptr.hpp>
#include <boost/function/function0.hpp>
#include <boost/signals/signal1.hpp>

#include <vector>
#include <utility>

#ifdef __GNUG__
#pragma interface
#endif

namespace grfx {

class Params;

class Image {
public:
	/** This is to be connected to a function that will return a new
	 *  instance of a viable derived class.
	 */
	typedef boost::shared_ptr<Image> ImagePtr;
	///
	static boost::function0<ImagePtr> newImage;

	/// Return the list of loadable formats.
	typedef std::vector<string> FormatList;
	///
	static boost::function0<FormatList> loadableFormats;

	/// Must define default c-tor explicitly as we define a copy c-tor.
	Image() {}
	/// Don't copy the signal finishedLoading
	Image(Image const &) {}
	///
	virtual ~Image() {}

	/// Create a copy
	virtual Image * clone() const = 0;

	/// Get the image width
	virtual unsigned int getWidth() const = 0;

	/// Get the image height
	virtual unsigned int getHeight() const = 0;

	/// is the image drawable ?
	virtual bool isDrawable() const = 0;

	/** At the end of the loading process inform the outside world
	 *  by emitting a signal.
	 */
	typedef boost::signal1<void, bool> SignalType;
	///
	SignalType finishedLoading;

	/** Start loading the image file.
	 *  The caller should expect this process to be asynchronous and
	 *  so should connect to the "finished" signal above.
	 */
	virtual void load(string const & filename) = 0;

	/** Generate the pixmap.
	 *  Uses the params to decide on color, grayscale etc.
	 *  Returns true if the pixmap is created.
	 */
	virtual bool setPixmap(Params const & params) = 0;

	/// Clip the image using params.
	virtual void clip(Params const & params) = 0;

	/// Rotate the image using params.
	virtual void rotate(Params const & params) = 0;

	/// Scale the image using params.
	virtual void scale(Params const & params) = 0;

protected:
	/** Uses the params to ascertain the dimensions of the scaled image.
	 *  Returned as make_pair(width, height).
	 *  If something goes wrong, returns make_pair(getWidth(), getHeight())
	 */
	std::pair<unsigned int, unsigned int>
	getScaledDimensions(Params const & params) const;
};

} // namespace grfx

#endif // GRAPHICSIMAGE_H

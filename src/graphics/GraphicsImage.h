// -*- C++ -*-
/**
 * \file GraphicsImage.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Baruch Even
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 *
 * An abstract base class for the images themselves.
 * Allows the user to retrieve the pixmap, once loaded and to issue commands
 * to modify it.
 *
 * The boost::functions newImage and loadableFormats are connected to the
 * appropriate derived classes elsewhere, allowing the graphics cache to
 * access them without knowing anything about their instantiation.
 *
 * The loading process can be asynchronous, but cropping, rotating and
 * scaling block execution.
 */

#ifndef GRAPHICSIMAGE_H
#define GRAPHICSIMAGE_H

#include "Dimension.h"

#include <boost/function.hpp>
#include <boost/signal.hpp>

#include <vector>

namespace lyx {

namespace support { class FileName; }

namespace graphics {

class Params;

class Image {
public:
	/** This is to be connected to a function that will return a new
	 *  instance of a viable derived class.
	 */
	static boost::function<Image *()> newImage;

	///
	virtual ~Image() {}

	/// Create a copy
	virtual Image * clone() const = 0;

	/// Get the image width
	virtual unsigned int width() const = 0;

	/// Get the image height
	virtual unsigned int height() const = 0;

	/// Is the image drawable ?
	virtual bool isDrawable() const = 0;

	/** At the end of the loading process inform the outside world
	 *  by emitting a signal
	 */
	typedef boost::signal<void(bool)> SignalType;
	///
	SignalType finishedLoading;

	/** Start loading the image file.
	 *  The caller should expect this process to be asynchronous and
	 *  so should connect to the "finished" signal above.
	 */
	virtual bool load(support::FileName const & filename) = 0;

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
	/// Must define default c-tor explicitly as we define a copy c-tor.
	Image() {}
	/// Don't copy the signal finishedLoading
	Image(Image const &) {}

	/** Uses the params to ascertain the dimensions of the scaled image.
	 *  Returned as Dimension(width, height, 0 descend).
	 *  If something goes wrong, returns make_pair(getWidth(), getHeight(), 0)
	 */
	Dimension scaledDimension(Params const & params) const;
};


} // namespace graphics
} // namespace lyx

#endif // GRAPHICSIMAGE_H

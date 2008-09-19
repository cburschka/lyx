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

namespace lyx {

namespace support { class FileName; }

namespace graphics {

class Params;

class Image {
public:
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
};

/// Only way to create a new Image.
/// Implemented in the frontend.
Image * newImage();

} // namespace graphics
} // namespace lyx

#endif // GRAPHICSIMAGE_H

// -*- C++ -*-
/**
 * \file QLImage.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QLIMAGE_H
#define QLIMAGE_H


#include "graphics/GraphicsImage.h"

#include <qpixmap.h>

namespace lyx {
namespace graphics {

class QLImage : public Image {
public:
	/// Access to this class is through this static method.
	static ImagePtr newImage();

	/// Return the list of loadable formats.
	static FormatList loadableFormats();

	~QLImage();
	QPixmap const & qpixmap() const { return xformed_pixmap_; }

private:
	/// Create a copy
	virtual Image * clone_impl() const;
	/// Get the image width
	virtual unsigned int getWidth_impl() const;
	/// Get the image height
	virtual unsigned int getHeight_impl() const;
	// FIXME Is the image drawable ?
	virtual bool isDrawable_impl() const { return true; }
	/**
	 * Load the image file into memory.
	 * The process is asynchronous, so this method starts the loading.
	 * When finished, the Image::finishedLoading signal is emitted.
	 */
	virtual void load_impl(string const & filename);
	/**
	 * Generate the pixmap, based on the current state of
	 * image_ (clipped, rotated, scaled etc).
	 * Uses the params to decide on color, grayscale etc.
	 * Returns true if the pixmap is created.
	 */
	virtual bool setPixmap_impl(Params const & params);
	/// Clip the image using params.
	virtual void clip_impl(Params const & params);
	/// Rotate the image using params.
	virtual void rotate_impl(Params const & params);
	/// Scale the image using params.
	virtual void scale_impl(Params const & params);

	/// Access to the class is through newImage() and clone.
	QLImage();
	///
	QLImage(QLImage const &);

	/// the original loaded image
	QPixmap pixmap_;

	/// the transformed pixmap for display
	QPixmap xformed_pixmap_;
};

} // namespace graphics
} // namespace lyx

#endif // QLIMAGE_H

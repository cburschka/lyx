// -*- C++ -*-
/**
 * \file GuiImage.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUIIMAGE_H
#define GUIIMAGE_H

#include "graphics/GraphicsImage.h"

#include <QImage>
#include <QPixmap>

namespace lyx {
namespace graphics {

class GuiImage : public Image {
public:
	/// Access to this class is through this static method.
	static ImagePtr newImage();

	/// Return the list of loadable formats.
	static FormatList loadableFormats();

	/// Retrieve the buffered pixmap.
	QPixmap const & qpixmap() const { return transformed_pixmap_; }

	/// Retrieve the buffered pixmap.
	QImage const & qimage() const { return transformed_; }

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
	virtual void load_impl(support::FileName const & filename);
	/**
	 * Finishes the process of modifying transformed_, using
	 * \c params to decide on color, grayscale etc.
	 * \returns true if successful.
	 */
	virtual bool setPixmap_impl(Params const & params);
	/// Clip the image using params.
	virtual void clip_impl(Params const & params);
	/// Rotate the image using params.
	virtual void rotate_impl(Params const & params);
	/// Scale the image using params.
	virtual void scale_impl(Params const & params);

	/// Access to the class is through newImage() and clone.
	GuiImage() {}
	///
	GuiImage(GuiImage const &);

	/// The original loaded image.
	QImage original_;

	/// The transformed image for display.
	QImage transformed_;
	/// Buffer the pixmap itself
	QPixmap transformed_pixmap_;
};

} // namespace graphics
} // namespace lyx

#endif // GUIIMAGE_H

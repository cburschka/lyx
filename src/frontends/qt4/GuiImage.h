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
#include <QString>

namespace lyx {
namespace graphics {

class GuiImage : public Image
{
public:
	/// Access to this class is through this static method.
	static Image * newImage();

	/// Retrieve the rendered image.
	QImage const & image() const;

private:
	/// Create a copy
	Image * clone() const;
	/// Get the image width
	unsigned int width() const;
	/// Get the image height
	unsigned int height() const;
	// FIXME Is the image drawable ?
	bool isDrawable() const { return true; }
	/**
	 * Load the image file into memory.
	 */
	bool load(support::FileName const & filename);
	bool load();
	/**
	 * Finishes the process of modifying transformed_, using
	 * \c params to decide on color, grayscale etc.
	 * \returns true if successful.
	 */
	bool setPixmap(Params const & params);

	/// Clip the image using params.
	bool clip(Params const & params);
	/// Rotate the image using params.
	bool rotate(Params const & params);
	/// Scale the image using params.
	bool scale(Params const & params);

	/// Access to the class is through newImage() and clone.
	GuiImage();
	///
	GuiImage(GuiImage const &);

	/// The original loaded image.
	QImage original_;

	/// The transformed image for display.
	QImage transformed_;

	///
	bool is_transformed_;
	///
	QString fname_;
};

} // namespace graphics
} // namespace lyx

#endif // GUIIMAGE_H

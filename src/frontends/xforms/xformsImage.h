// -*- C++ -*-
/**
 * \file xformsImage.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

/* An instantiation of Image that makes use of the xforms lirary routines
 * to load and store the image in memory.
 */

#ifndef XFORMSIMAGE_H
#define XFORMSIMAGE_H

#include "graphics/GraphicsImage.h"

#include <X11/X.h>

struct flimage_;
typedef flimage_ FL_IMAGE;

namespace lyx {
namespace graphics {

class xformsImage : public Image
{
public:
	/// Access to this class is through this static method.
	static ImagePtr newImage();

	/// Return the list of loadable formats.
	static FormatList loadableFormats();

	///
	~xformsImage();
	///
	Pixmap getPixmap() const;

	/// Internal callbacks.
	void statusCB(std::string const &);
	void errorCB(std::string const &);

private:
	/// Create a copy
	virtual Image * clone_impl() const;
	/// Get the image width
	virtual unsigned int getWidth_impl() const;
	/// Get the image height
	virtual unsigned int getHeight_impl() const;
	/// Is the image drawable ?
	virtual bool isDrawable_impl() const;
	/** Load the image file into memory.
	 *  The process is asynchronous, so this method starts the loading.
	 *  When finished, the Image::finishedLoading signal is emitted.
	 */
	virtual void load_impl(std::string const & filename);
	/** Generate the pixmap, based on the current state of
	 *  image_ (clipped, rotated, scaled etc).
	 *  Uses the params to decide on color, grayscale etc.
	 *  Returns true if the pixmap is created.
	 */
	bool setPixmap_impl(Params const & params);
	/// Clip the image using params.
	virtual void clip_impl(Params const & params);

	/// Rotate the image using params.
	virtual void rotate_impl(Params const & params);

	/// Scale the image using params.
	virtual void scale_impl(Params const & params);

	/// Access to the class is through newImage() and clone.
	xformsImage();
	///
	xformsImage(xformsImage const &);

	/// The xforms container.
	FL_IMAGE * image_;

	/// The pixmap itself.
	Pixmap pixmap_;

	/// Is the pixmap initialized?
	enum PixmapStatus {
		///
		PIXMAP_UNINITIALISED,
		///
		PIXMAP_FAILED,
		///
		PIXMAP_SUCCESS
	};

	PixmapStatus pixmap_status_;
};

} // namespace graphics
} // namespace lyx

#endif // XFORMSIMAGE_H

// -*- C++ -*-
/**
 *  \file xformsImage.h
 *  Copyright 2002 the LyX Team
 *  Read the file COPYING
 *
 *  \author Angus Leeming, a.leeming@ic.ac.uk
 */

/* An instantiation of Image that makes use of the xforms lirary routines
 * to load and store the image in memory.
 */

#ifndef XFORMSIMAGE_H
#define XFORMSIMAGE_H

#ifdef __GNUG__
#pragma interface
#endif

#include "graphics/GraphicsImage.h"

struct flimage_;
typedef flimage_ FL_IMAGE;

namespace grfx {

class xformsImage : public Image
{
public:
	/// Access to this class is through this static method.
	static ImagePtr newImage();

	/// Return the list of loadable formats.
	static FormatList loadableFormats();

	///
	~xformsImage();

	/// Create a copy
	Image * clone() const;

	///
	Pixmap getPixmap() const;

	/// Get the image width
	unsigned int getWidth() const;

	/// Get the image height
	unsigned int getHeight() const;

	/** Load the image file into memory.
	 *  The process is asynchronous, so this method starts the loading.
	 *  When finished, the Image::finishedLoading signal is emitted.
	 */
	void load(string const & filename);

	/** Generate the pixmap, based on the current state of
	 *  image_ (clipped, rotated, scaled etc).
	 *  Uses the params to decide on color, grayscale etc.
	 *  Returns true if the pixmap is created.
	 */
	bool setPixmap(Params const & params);

	/// Clip the image using params.
	void clip(Params const & params);

	/// Rotate the image using params.
	void rotate(Params const & params);

	/// Scale the image using params.
	void scale(Params const & params);

	/// Internal callbacks.
	void statusCB(string const &);
	///
	void errorCB(string const &);

private:
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

} // namespace grfx

#endif // XFORMSIMAGE_H

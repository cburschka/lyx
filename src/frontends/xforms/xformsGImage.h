// -*- C++ -*-
/**
 *  \file xformsGImage.h
 *  Copyright 2002 the LyX Team
 *  Read the file COPYING
 *
 *  \author Angus Leeming, a.leeming@ic.ac.uk
 */

/* An instantiation of GImage that makes use of the xforms lirary routines
 * to load and store the image in memory.
 */

#ifndef XFORMS_GRAPHICSIMAGE_H
#define XFORMS_GRAPHICSIMAGE_H

#ifdef __GNUG__
#pragma interface
#endif

#include "graphics/GraphicsImage.h"

struct flimage_;
typedef flimage_ FL_IMAGE;

namespace grfx {

class xformsGImage : public GImage
{
public:
	/// Access to this class is through this static method.
	static ImagePtr newImage();

	/// Return the list of loadable formats.
	static FormatList loadableFormats();

	///
	~xformsGImage();

	/// Create a copy
	GImage * clone() const;

	///
	Pixmap getPixmap() const;

	/// Get the image width
	unsigned int getWidth() const;

	/// Get the image height
	unsigned int getHeight() const;

	/** Load the image file into memory.
	 *  The process is asynchronous, so this method starts the loading
	 *  and saves the signal. It is emitted once loading is finished.
	 */
	void load(string const & filename, SignalTypePtr);

	/** Generate the pixmap, based on the current state of
	 *  image_ (clipped, rotated, scaled etc).
	 *  Uses the params to decide on color, grayscale etc.
	 *  Returns true if the pixmap is created.
	 */
	bool setPixmap(GParams const & params);

	/// Clip the image using params.
	void clip(GParams const & params);

	/// Rotate the image using params.
	void rotate(GParams const & params);

	/// Scale the image using params.
	void scale(GParams const & params);

	/// Internal callbacks.
	void statusCB(string const &);
	///
	void errorCB(string const &);

private:
	/// Access to the class is through newImage() and clone.
	xformsGImage();
	///
	xformsGImage(xformsGImage const &);

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

	/// Emit this signal when the loading process is finished.
	GImage::SignalTypePtr on_finish_;
};

} // namespace grfx

#endif // XFORMS_GRAPHICSIMAGE_H

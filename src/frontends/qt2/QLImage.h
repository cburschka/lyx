// -*- C++ -*-
/**
 * \file QLImage.h
 * Copyright 2002 the LyX Team
 * Read the file COPYING
 *
 * \author Angus Leeming, a.leeming@ic.ac.uk
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

#ifndef QLIMAGE_H
#define QLIMAGE_H

#ifdef __GNUG__
#pragma interface
#endif

#include "graphics/GraphicsImage.h"
 
#include <qpixmap.h>
 
namespace grfx {

class QLImage : public Image
{
public:
	/// Access to this class is through this static method.
	static ImagePtr newImage();

	/// Return the list of loadable formats.
	static FormatList loadableFormats();

	~QLImage();

	/// Create a copy
	virtual Image * clone() const;

	QPixmap const & qpixmap() const { return xformed_pixmap_; }

	/// Get the image width
	virtual unsigned int getWidth() const;

	/// Get the image height
	virtual unsigned int getHeight() const;

	// FIXME 
	virtual bool isDrawable() const { return true; } 
 
	/** 
	 * Load the image file into memory.
	 * The process is asynchronous, so this method starts the loading.
	 * When finished, the Image::finishedLoading signal is emitted.
	 */
	virtual void load(string const & filename);

	/**
	 * Generate the pixmap, based on the current state of
	 * image_ (clipped, rotated, scaled etc).
	 * Uses the params to decide on color, grayscale etc.
	 * Returns true if the pixmap is created.
	 */
	virtual bool setPixmap(Params const & params);

	/// Clip the image using params.
	virtual void clip(Params const & params);

	/// Rotate the image using params.
	virtual void rotate(Params const & params);

	/// Scale the image using params.
	virtual void scale(Params const & params);

private:
	/// Access to the class is through newImage() and clone.
	QLImage();
	///
	QLImage(QLImage const &);

	/// the original loaded image
	QPixmap pixmap_;

	/// the transformed pixmap for display
	QPixmap xformed_pixmap_;
};

} // namespace grfx

#endif // QLIMAGE_H

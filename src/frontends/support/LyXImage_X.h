// -*- C++ -*-
/* This file is part of
 * =================================================
 * 
 *          LyX, The Document Processor
 *          Copyright 1995 Matthias Ettrich.
 *          Copyright 1995-2000 The LyX Team.
 *
 *          This file Copyright 2000 Baruch Even
 * ================================================= */

#ifndef LYXIMAGE_X_H
#define LYXIMAGE_X_H

#include "frontends/support/LyXImage.h"
#include <X11/Xlib.h>

// This class actually acts as a base class when X-Windows is used.

class LyXImage {
public:
	///
	LyXImage();
	///
	LyXImage(Pixmap pixmap, unsigned int width, unsigned int height);
	///
	~LyXImage();

	///
	Pixmap getPixmap() const;
	
	/// Get the image width
	unsigned int getWidth() const { return width_; }
	
	/// Get the image height
	unsigned int getHeight() const { return height_; }
	
private:
	/// The pixmap itself.
	Pixmap pixmap_;
	/// Is the pixmap initialized?
	bool pixmapInitialized;
	/// Width of the image
	unsigned int width_;
	/// Height of the image
	unsigned int height_;
};

#endif

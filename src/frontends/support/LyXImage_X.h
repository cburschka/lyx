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

#ifdef __GNUG__
#pragma interface
#endif

#include "frontends/support/LyXImage.h"
#include <X11/Xlib.h>

// This class actually acts as a base class when X-Windows is used.

class LyXImage {
public:
	///
	LyXImage();
	///
	LyXImage(Pixmap pixmap);
	///
	~LyXImage();

	///
	Pixmap getPixmap() const;

private:
	/// The pixmap itself.
	Pixmap pixmap_;
	/// Is the pixmap initialized?
	bool pixmapInitialized;
};

#endif

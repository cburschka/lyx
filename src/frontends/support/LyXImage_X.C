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

#include <config.h>
#include "LyXImage.h"

#include FORMS_H_LOCATION

#include "support/LAssert.h"

LyXImage::LyXImage()
	: pixmap_(0), pixmapInitialized(false), width_(0), height_(0)
{}

LyXImage::LyXImage(Pixmap pixmap, unsigned int width, unsigned int height)
	: pixmap_(pixmap), pixmapInitialized(true), width_(width), height_(height)
{}

LyXImage::~LyXImage()
{
	if (pixmapInitialized)
		XFreePixmap(fl_get_display(), pixmap_);
}

Pixmap
LyXImage::getPixmap() const
{
	lyx::Assert(pixmapInitialized);
		
	return pixmap_;
}

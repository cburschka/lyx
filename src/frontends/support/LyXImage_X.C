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

#ifdef __GNUG__
#pragma implementation
#endif

#include <config.h>
#include "LyXImage.h"

#include FORMS_H_LOCATION

#include "support/LAssert.h"

LyXImage::LyXImage()
	: pixmap_(0), pixmapInitialized(false)
{}

LyXImage::LyXImage(Pixmap pixmap)
	: pixmap_(pixmap), pixmapInitialized(true)
{}

LyXImage::~LyXImage()
{
	if (pixmapInitialized)
		XFreePixmap(fl_get_display(), pixmap_);
}

Pixmap
LyXImage::getPixmap() const
{
	Assert(pixmapInitialized);
		
	return pixmap_;
}

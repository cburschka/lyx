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

#ifdef __GNUG__
#pragma implementation
#endif

#include "graphics/GraphicsCache.h"
#include "graphics/GraphicsCacheItem.h"
#include "graphics/GraphicsCacheItem_pimpl.h"


GraphicsCacheItem::GraphicsCacheItem()
	: pimpl(new GraphicsCacheItem_pimpl)
{
	pimpl->refCount = 1;
}


GraphicsCacheItem::~GraphicsCacheItem()
{
	destroy();
}


bool
GraphicsCacheItem::setFilename(string const & filename)
{
	filename_ = filename;
	return pimpl->setFilename(filename);
}


GraphicsCacheItem::GraphicsCacheItem(GraphicsCacheItem const & gci)
{
	pimpl = 0;
	copy(gci);
}

GraphicsCacheItem &
GraphicsCacheItem::operator=(GraphicsCacheItem const & gci)
{
	// Are we trying to copy the object onto itself.
	if (this == &gci)
		return *this;

	// Destroy old copy 
	destroy();

	// And then copy new object.
	copy(gci);

	return *this;
}


void
GraphicsCacheItem::copy(GraphicsCacheItem const & gci)
{
	pimpl = gci.pimpl;
	++(pimpl->refCount);
}


void
GraphicsCacheItem::destroy()
{
	if (pimpl) {
		--(pimpl->refCount);
		if (pimpl->refCount == 0) {
			delete pimpl;
			GraphicsCache * gc = GraphicsCache::getInstance();
			gc->removeFile(filename_);
		}
	}
}


GraphicsCacheItem::ImageStatus 
GraphicsCacheItem::getImageStatus() const { return pimpl->imageStatus_; }


int 
GraphicsCacheItem::getHeight() const { return pimpl->height_; }	


int 
GraphicsCacheItem::getWidth() const { return pimpl->width_; }


Pixmap 
GraphicsCacheItem::getImage() const { return pimpl->pixmap_; }

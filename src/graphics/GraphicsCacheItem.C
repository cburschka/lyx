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
#include "frontends/support/LyXImage.h"


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
	: pimpl(0)
{
	// copy will set the actual value of the pimpl.
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

GraphicsCacheItem *
GraphicsCacheItem::Clone() const
{
	return new GraphicsCacheItem(*this);
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
	if (!pimpl) 
		return;

	--(pimpl->refCount);
	if (pimpl->refCount == 0) {
		delete pimpl;
		pimpl = 0;

		GraphicsCache * gc = GraphicsCache::getInstance();
		gc->removeFile(filename_);
	}
}


GraphicsCacheItem::ImageStatus 
GraphicsCacheItem::getImageStatus() const { return pimpl->imageStatus_; }

LyXImage * 
GraphicsCacheItem::getImage() const { return pimpl->getImage(); }

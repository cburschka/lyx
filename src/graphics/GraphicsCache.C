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

#include "GraphicsCache.h"

#include "support/LAssert.h"

GraphicsCache * GraphicsCache::singleton = 0;


GraphicsCache * 
GraphicsCache::getInstance()
{
	if (!singleton) {
		singleton = new GraphicsCache;
		Assert(singleton != 0);
	}

	return singleton;
}


GraphicsCache::~GraphicsCache()
{
	// The map elements should have already been eliminated.
	Assert(cache.empty());
	
	delete singleton;
}


GraphicsCacheItem *
GraphicsCache::addFile(string const & filename)
{
	CacheType::iterator it = cache.find(filename);
	
	if (it != cache.end()) {
		return new GraphicsCacheItem( *((*it).second) );
	}
	
	GraphicsCacheItem * cacheItem = new GraphicsCacheItem();
	if (cacheItem == 0)
		return 0;
	
	cacheItem->setFilename(filename);
	
	cache[filename] = cacheItem;
	
	// We do not want to return the main cache object, otherwise when the
	// will destroy their copy they will destroy the main copy.
	return new GraphicsCacheItem( *cacheItem );
}


void
GraphicsCache::removeFile(string const & filename)
{
	// We do not destroy the GraphicsCacheItem since we are here because
	// the last copy of it is being erased.

	CacheType::iterator it = cache.find(filename);
	if (it != cache.end())
		cache.erase(it);
}

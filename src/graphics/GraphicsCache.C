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
#include "GraphicsCacheItem.h"

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


GraphicsCache::shared_ptr_item
GraphicsCache::addFile(string const & filename)
{
	CacheType::iterator it = cache.find(filename);
	
	if (it != cache.end()) {
		return (*it).second;
	}
	
	shared_ptr_item cacheItem(new GraphicsCacheItem(filename));
	if (cacheItem.get() == 0)
		return cacheItem;
	
	cache[filename] = cacheItem;

	// GraphicsCacheItem_ptr is a shared_ptr and thus reference counted,
	// it is safe to return it directly.
	return cacheItem;
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

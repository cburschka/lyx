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
    if (! singleton) {
        singleton = new GraphicsCache;
		Assert(singleton != 0);
    }

    return singleton;
}


GraphicsCache::~GraphicsCache()
{
	// Free the map.
	//std::foreach(map.begin(), map.end(), ...);
	// This is not really needed, it will only happen on program close and in
	// any case the OS will release those resources (not doing it may have 
	// a good effect on closing time).

    delete singleton;
}


GraphicsCacheItem * 
GraphicsCache::addFile(string const & filename)
{
    CacheType::const_iterator it = cache.find(filename);
    
    if (it != cache.end()) {
        return (*it).second;
    }
	
	GraphicsCacheItem * cacheItem = new GraphicsCacheItem();
	if (cacheItem == 0) {
		return 0;
	}

	bool result = cacheItem->setFilename(filename);
	if (!result) 
		return 0;

	cache[filename] = cacheItem;
	
    return cacheItem;
}


void
GraphicsCache::removeFile(string const & filename)
{
    CacheType::const_iterator it = cache.find(filename);
    
    if (it != cache.end()) {
        // INCOMPLETE!
//        cache.erase(it);
    }
}

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


GraphicsCache * GraphicsCache::singleton = 0;


GraphicsCache * 
GraphicsCache::getInstance()
{
    if (! singleton) {
        singleton = new GraphicsCache;
    }

    return singleton;
}


GraphicsCache::~GraphicsCache()
{
        delete singleton;
}


GraphicsCacheItem * 
GraphicsCache::addFile(string const & filename)
{
    CacheType::const_iterator it = cache.find(filename);
    
    if (it != cache.end()) {
        return (*it).second;
    }
    // INCOMPLETE!
    return 0;
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

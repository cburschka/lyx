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
#include "GraphicsCache.h"

GraphicsCache * 
GraphicsCache::getInstance()
{
    if (! singleton) {
        singleton = new GraphicsCache;
    }

    return singleton;
}


GraphicsCacheItem * 
GraphicsCache::addFile(string filename)
{
    CacheType::const_iterator it = cache.find(filename);
    
    if (it != cache.end()) {
        return (*it).second;
    }
    // INCOMPLETE!
    return 0;
}

void
GraphicsCache::removeFile(string filename)
{
    CacheType::const_iterator it = cache.find(filename);
    
    if (it != cache.end()) {
        // INCOMPLETE!
//        cache.erase(it);
    }
}

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

#ifndef GRAPHICSCACHE_H
#define GRAPHICSCACHE_H

#ifdef __GNUG__
#pragma interface
#endif

#include <map>

#include "LString.h"
#include "GraphicsCacheItem.h"
#include "support/utility.hpp"

/** GraphicsCache is the manager of the image cache, it is responsible to
    create the GraphicsCacheItem's and maintain them.
    
    GraphicsCache is a singleton class, there should be only one instance of
    it at any moment.
*/
class GraphicsCache : public noncopyable {
public:
    /// Get the instance of the class.
    static GraphicsCache * getInstance();

    /// Add a file to the cache.
    GraphicsCacheItem * addFile(string const & filename);

private:
    /// Remove a cache item if it's count has gone to zero.
    void removeFile(string const & filename);
    
    /// Private c-tor so we can control how many objects are instantiated.
    GraphicsCache() {}

    /// Private d-tor so that no-one will destroy us.
    ~GraphicsCache();

    /// Holder of the single instance of the class.
    static GraphicsCache * singleton;
    ///
    typedef std::map<string, GraphicsCacheItem *> CacheType;
    ///
    CacheType cache;

	// We need this so that an Item can tell the cache that it should be
	// deleted. (to call removeFile).
	// It also helps removing a warning gcc emits.
	friend GraphicsCacheItem;
};
#endif

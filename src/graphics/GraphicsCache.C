/*
 * \file GraphicsCache.C
 * Copyright 2002 the LyX Team
 * Read the file COPYING
 *
 * \author Baruch Even <baruch.even@writeme.com>
 * \author Angus Leeming <a.leeming@ic.ac.uk>
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "GraphicsCache.h"
#include "GraphicsCacheItem.h"
#include "GraphicsImage.h"

#include "debug.h"

#include "support/filetools.h"

#include "frontends/lyx_gui.h"

namespace grfx {

GCache & GCache::get()
{
	static bool start = true;
	if (start) {
		start = false;
		lyx_gui::init_graphics();
	}

	// Now return the cache
	static GCache singleton;
	return singleton;
}


GCache::GCache()
{
	cache = new CacheType;
}


// all elements are destroyed by the shared_ptr's in the map.
GCache::~GCache()
{
	delete cache;
}


std::vector<string> GCache::loadableFormats() const
{
	return GImage::loadableFormats();
}


void GCache::add(string const & file)
{
	if (!AbsolutePath(file)) {
		lyxerr << "GCacheItem::add(" << file << "):\n"
		       << "The file must be have an absolute path."
		       << std::endl;
		return;
	}
	
	// Is the file in the cache already?
	if (inCache(file)) {
		lyxerr[Debug::GRAPHICS] << "GCache::add(" << file << "):\n"
					<< "The file is already in the cache."
					<< std::endl;
		return;
	}

	
	(*cache)[file] = GraphicPtr(new GCacheItem(file));
}


void GCache::remove(string const & file)
{
	CacheType::iterator it = cache->find(file);
	if (it == cache->end())
		return;

	GraphicPtr item = it->second;
	
	if (item.use_count() == 1) {
		// The graphics file is in the cache, but nothing else
		// references it.
		cache->erase(it);
	}
}


bool GCache::inCache(string const & file) const
{
	return cache->find(file) != cache->end();
}


GraphicPtr const GCache::graphic(string const & file) const
{
	CacheType::const_iterator it = cache->find(file);
	if (it == cache->end())
		return GraphicPtr();

	return it->second;
}


ImagePtr const GCache::image(string const & file) const
{
	CacheType::const_iterator it = cache->find(file);
	if (it == cache->end())
		return ImagePtr();

	return it->second->image();
}


} // namespace grfx

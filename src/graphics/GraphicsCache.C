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
#include "GraphicsParams.h"
#include "insets/insetgraphics.h"
#include "frontends/GUIRunTime.h"


namespace grfx {

GCache & GCache::get()
{
	static bool start = true;
	if (start) {
		start = false;
		GUIRunTime::initialiseGraphics();
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


void GCache::update(InsetGraphics const & inset, string const & filepath)
{
	// A subset only of InsetGraphicsParams is needed for display purposes.
	// The GraphicsParams c-tor also interrogates lyxrc to ascertain whether
	// to display or not.
	GParams params(inset.params(), filepath);

	// Each inset can reference only one file, so check the cache for any
	// graphics files referenced by inset. If the name of this file is
	// different from that in params, then remove the reference.
	CacheType::iterator it = find(inset);

	if (it != cache->end()) {
		CacheItemType item = it->second;
		if (item->filename() != params.filename) {
			item->remove(inset);
			if (item->empty())
				cache->erase(it);
		}
	}

	// Are we adding a new file or modifying the display of an existing one?
	it = cache->find(params.filename);

	if (it != cache->end()) {
		it->second->modify(inset, params);
		return;
	}

	CacheItemType item(new GCacheItem(inset, params));
	if (item.get() != 0)
		(*cache)[params.filename] = item;
}


void GCache::remove(InsetGraphics const & inset)
{
	CacheType::iterator it = find(inset);
	if (it == cache->end())
		return;

	CacheItemType item = it->second;
	item->remove(inset);
	if (item->empty()) {
		cache->erase(it);
	}
}


void GCache::startLoading(InsetGraphics const & inset)
{
	CacheType::iterator it = find(inset);
	if (it == cache->end())
		return;

	it->second->startLoading(inset);
}


ImagePtr const GCache::image(InsetGraphics const & inset) const
{
	CacheType::const_iterator it = find(inset);
	if (it == cache->end())
		return ImagePtr();

	return it->second->image(inset);
}


ImageStatus GCache::status(InsetGraphics const & inset) const
{
	CacheType::const_iterator it = find(inset);
	if (it == cache->end())
		return ErrorUnknown;

	return it->second->status(inset);
}


void GCache::changeDisplay(bool changed_background)
{
	CacheType::iterator it = cache->begin();
	CacheType::iterator end = cache->end();
	for(; it != end; ++it)
		it->second->changeDisplay(changed_background);
}


GCache::CacheType::iterator
GCache::find(InsetGraphics const & inset)
{
	CacheType::iterator it  = cache->begin();
	CacheType::iterator end = cache->end();
	for (; it != end; ++it) {
		if (it->second->referencedBy(inset))
			return it;
	}

	return cache->end();
}


GCache::CacheType::const_iterator
GCache::find(InsetGraphics const & inset) const
{
	CacheType::const_iterator it  = cache->begin();
	CacheType::const_iterator end = cache->end();
	for (; it != end; ++it) {
		if (it->second->referencedBy(inset))
			return it;
	}

	return cache->end();
}


unsigned int GCache::raw_width(string const & filename) const
{
	CacheType::const_iterator it = cache->find(filename);
	if (it == cache->end())
		return 0;

	return it->second->raw_width();
}


unsigned int GCache::raw_height(string const & filename) const
{
	CacheType::const_iterator it = cache->find(filename);
	if (it == cache->end())
		return 0;

	return it->second->raw_height();
}


std::vector<string> GCache::loadableFormats() const {
	return GImage::loadableFormats();
}

} // namespace grfx


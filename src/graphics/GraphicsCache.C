/*
 * \file GraphicsCache.C
 * Copyright 2002 the LyX Team
 * Read the file COPYING
 *
 * \author Baruch Even <baruch.even@writeme.com>
 * \author Angus Leeming <leeming@lyx.org>
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

#include <map>

namespace grfx {

/** The cache contains one item per file, so use a map to find the
 *  cache item quickly by filename.
 */
typedef std::map<string, Cache::ItemPtr> CacheType;

struct Cache::Impl {
	///
	CacheType cache;
};


Cache & Cache::get()
{
	// Now return the cache
	static Cache singleton;
	return singleton;
}


Cache::Cache()
	: pimpl_(new Impl())
{}


Cache::~Cache()
{}


std::vector<string> Cache::loadableFormats() const
{
	return Image::loadableFormats();
}


void Cache::add(string const & file) const
{
	if (!AbsolutePath(file)) {
		lyxerr << "Cache::add(" << file << "):\n"
		       << "The file must be have an absolute path."
		       << std::endl;
		return;
	}

	// Is the file in the cache already?
	if (inCache(file)) {
		lyxerr[Debug::GRAPHICS] << "Cache::add(" << file << "):\n"
					<< "The file is already in the cache."
					<< std::endl;
		return;
	}

	pimpl_->cache[file] = ItemPtr(new CacheItem(file));
}


void Cache::remove(string const & file) const
{
	CacheType::iterator it = pimpl_->cache.find(file);
	if (it == pimpl_->cache.end())
		return;

	ItemPtr & item = it->second;

	if (item.use_count() == 1) {
		// The graphics file is in the cache, but nothing else
		// references it.
		pimpl_->cache.erase(it);
	}
}


bool Cache::inCache(string const & file) const
{
	return pimpl_->cache.find(file) != pimpl_->cache.end();
}


Cache::ItemPtr const Cache::item(string const & file) const
{
	CacheType::const_iterator it = pimpl_->cache.find(file);
	if (it == pimpl_->cache.end())
		return ItemPtr();

	return it->second;
}

} // namespace grfx

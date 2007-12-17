/**
 * \file GraphicsCache.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Baruch Even
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GraphicsCache.h"
#include "GraphicsCacheItem.h"
#include "GraphicsImage.h"

#include "support/debug.h"
#include "support/FileName.h"
#include "support/filetools.h"

#include <map>

using namespace std;
using namespace lyx::support;

namespace lyx {

namespace graphics {

/** The cache contains one item per file, so use a map to find the
 *  cache item quickly by filename.
 */
typedef map<FileName, Cache::ItemPtr> CacheType;

class Cache::Impl {
public:
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
	: pimpl_(new Impl)
{}


Cache::~Cache()
{
	delete pimpl_;
}


vector<string> Cache::loadableFormats() const
{
	return Image::loadableFormats();
}


void Cache::add(FileName const & file) const
{
	// Is the file in the cache already?
	if (inCache(file)) {
		LYXERR(Debug::GRAPHICS, "Cache::add(" << file << "):\n"
					<< "The file is already in the cache.");
		return;
	}

	pimpl_->cache[file] = ItemPtr(new CacheItem(file));
}


void Cache::remove(FileName const & file) const
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


bool Cache::inCache(FileName const & file) const
{
	return pimpl_->cache.find(file) != pimpl_->cache.end();
}


Cache::ItemPtr const Cache::item(FileName const & file) const
{
	CacheType::const_iterator it = pimpl_->cache.find(file);
	if (it == pimpl_->cache.end())
		return ItemPtr();

	return it->second;
}

} // namespace graphics
} // namespace lyx

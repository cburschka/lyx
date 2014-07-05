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

#include "Format.h"

#include "frontends/Application.h"

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


// FIXME THREAD
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


vector<string> const & Cache::loadableFormats() const
{
	static vector<string> fmts;

	if (!fmts.empty())
		return fmts;

	// The formats recognised by LyX
	Formats::const_iterator begin = formats.begin();
	Formats::const_iterator end   = formats.end();

	// The formats natively loadable.
	vector<string> nformat = frontend::loadableImageFormats();
	
	vector<string>::const_iterator it = nformat.begin();
	for (; it != nformat.end(); ++it) {
		for (Formats::const_iterator fit = begin; fit != end; ++fit) {
			if (fit->extension() == *it) {
				fmts.push_back(fit->name());
				break;
			}
		}
	}

	if (lyxerr.debugging()) {
		LYXERR(Debug::GRAPHICS, "LyX recognises the following image formats:");

		vector<string>::const_iterator fbegin = fmts.begin();
		vector<string>::const_iterator fend = fmts.end();
		for (vector<string>::const_iterator fit = fbegin; fit != fend; ++fit) {
			if (fit != fbegin)
				LYXERR(Debug::GRAPHICS, ", ");
			LYXERR(Debug::GRAPHICS, *fit);
		}
		LYXERR(Debug::GRAPHICS, '\n');
	}

	return fmts;
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

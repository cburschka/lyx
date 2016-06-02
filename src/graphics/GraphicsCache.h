// -*- C++ -*-
/**
 * \file GraphicsCache.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Baruch Even
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 *
 * lyx::graphics::Cache is the manager of the image cache.
 * It is responsible for creating the lyx::graphics::CacheItem's
 * and maintaining them.
 *
 * lyx::graphics::Cache is a singleton class. It is possible to have only one
 * instance of it at any moment.
 */

#ifndef GRAPHICSCACHE_H
#define GRAPHICSCACHE_H

#include <memory>
#include <string>
#include <vector>


namespace lyx {

namespace support { class FileName; }

namespace graphics {

class CacheItem;

class Cache {
public:

	/// This is a singleton class. Get the instance.
	static Cache & get();

	/** Which graphics formats can be loaded directly by the image loader.
	 *  Other formats can be loaded if a converter to a loadable format
	 *  can be defined.
	 */
	std::vector<std::string> const & loadableFormats() const;

	/// Add a graphics file to the cache.
	void add(support::FileName const & file) const;

	/// Remove a file from the cache.
	void remove(support::FileName const & file) const;

	/// Returns \c true if the file is in the cache.
	bool inCache(support::FileName const & file) const;

	/** Get the cache item associated with file.
	 *  Returns an empty container if there is no such item.
	 *
	 *  IMPORTANT: whatever uses an image must make a local copy of this
	 *  ItemPtr. The shared_ptr<>::use_count() function is
	 *  used to ascertain whether or not to remove the item from the cache
	 *  when remove(file) is called.
	 *
	 *  You have been warned!
	 */
	typedef std::shared_ptr<CacheItem> ItemPtr;
	///
	ItemPtr const item(support::FileName const & file) const;

private:
	/// noncopyable
	Cache(Cache const &);
	void operator=(Cache const &);

	/** Make the c-tor, d-tor private so we can control how many objects
	 *  are instantiated.
	 */
	Cache();
	///
	~Cache();

	/// Use the Pimpl idiom to hide the internals.
	class Impl;
	/// The pointer never changes although *pimpl_'s contents may.
	Impl * const pimpl_;
};

} // namespace graphics
} // namespace lyx

#endif // GRAPHICSCACHE_H

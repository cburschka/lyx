// -*- C++ -*-
/**
 * \file ConverterCache.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Baruch Even
 * \author Angus Leeming
 * \author Georg Baum
 *
 * Full author contact details are available in file CREDITS.
 *
 * ConverterCache is the manager of the file cache.
 * It is responsible for creating the ConverterCacheItem's
 * and maintaining them.
 *
 * ConverterCache is a singleton class. It is possible to have
 * only one instance of it at any moment.
 */

#ifndef CONVERTERCACHE_H
#define CONVERTERCACHE_H

#include "support/strfwd.h"


namespace lyx {

namespace support { class FileName; }

/**
 * Cache for converted files. The cache works as follows:
 *
 * The key for a cache item consists of the absolute name of the original
 * file and the format name of the target format.  The original file in the
 * user directory is named \c orig_from in the code, the format name is named
 * \c to_format. Example:
 * \c orig_from = "/home/me/myfigure.fig"
 * \c to_format = "eps"
 * A cache item is considered up to date (inCache() returns \c true) if
 * - The cache contains an item with key (\c orig_to, \c to_format)
 * - The stored timestamp of the item is identical with the actual timestamp
 *   of \c orig_from, or, if that is not the case, the stored checksum is
 *   identical with the actual checksum of \c orig_from.
 * Otherwise the item is not considered up to date, and add() will refresh it.
 *
 * There is no cache maintenance yet (max size, max age etc.)
 */
class ConverterCache {
public:

	/// This is a singleton class. Get the instance.
	static ConverterCache & get();

	/// Init the cache. This must be done after package initialization.
	static void init();

	/**
	 * Add \c converted_file (\c orig_from converted to \c to_format) to
	 * the cache if it is not already in or not up to date.
	 */
	void add(support::FileName const & orig_from, std::string const & to_format,
		 support::FileName const & converted_file) const;

	/// Remove a file from the cache.
	void remove(support::FileName const & orig_from,
		    std::string const & to_format) const;

	/// Remove all cached \p from_format -> \p to_format conversions
	void remove_all(std::string const & from_format,
			std::string const & to_format) const;

	/**
	 * Returns \c true if \c orig_from converted to \c to_format is in
	 * the cache and up to date.
	 */
	bool inCache(support::FileName const & orig_from,
		     std::string const & to_format) const;

	/// Get the name of the cached file
	support::FileName const & cacheName(support::FileName const & orig_from,
					    std::string const & to_format) const;

	/// Copy the file from the cache to \p dest
	bool copy(support::FileName const & orig_from, std::string const & to_format,
		  support::FileName const & dest) const;

private:
	/// noncopyable
	ConverterCache(ConverterCache const &);
	void operator=(ConverterCache const &);

	/** Make the c-tor, d-tor private so we can control how many objects
	 *  are instantiated.
	 */
	ConverterCache();
	///
	~ConverterCache();

	/// Use the Pimpl idiom to hide the internals.
	class Impl;
	/// The pointer never changes although *pimpl_'s contents may.
	Impl * const pimpl_;
};

} // namespace lyx

#endif

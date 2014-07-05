/**
 * \file ConverterCache.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Baruch Even
 * \author Angus Leeming
 * \author Georg Baum
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "ConverterCache.h"

#include "Format.h"
#include "Lexer.h"
#include "LyXRC.h"
#include "Mover.h"

#include "support/convert.h"
#include "support/debug.h"
#include "support/filetools.h"
#include "support/lyxtime.h"
#include "support/Package.h"

#include "support/lassert.h"
#include <boost/crc.hpp>

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <map>
#include <sstream>

using namespace std;
using namespace lyx::support;

namespace lyx {

namespace {

unsigned long do_crc(string const & s)
{
	boost::crc_32_type crc;
	crc = for_each(s.begin(), s.end(), crc);
	return crc.checksum();
}


// FIXME THREAD
// This should be OK because it is only assigned during init()
static FileName cache_dir;


class CacheItem {
public:
	CacheItem() {}
	CacheItem(FileName const & orig_from, string const & to_format,
		  time_t t, unsigned long c)
		: timestamp(t), checksum(c)
	{
		ostringstream os;
		os << setw(10) << setfill('0') << do_crc(orig_from.absFileName())
		   << '-' << to_format;
		cache_name = FileName(addName(cache_dir.absFileName(), os.str()));
		LYXERR(Debug::FILES, "Add file cache item " << orig_from
				     << ' ' << to_format << ' ' << cache_name
				     << ' ' << long(timestamp) << ' ' << checksum << '.');
	}
	~CacheItem()
	{}
	FileName cache_name;
	time_t timestamp;
	unsigned long checksum;
};

}


/** The cache contains one item per orig file and target format, so use a
 *  nested map to find the cache item quickly by filename and format.
 */
typedef map<string, CacheItem> FormatCacheType;
class FormatCache {
public:
	/// Format of the source file
	string from_format;
	/// Cache target format -> item to quickly find the item by format
	FormatCacheType cache;
};
typedef map<FileName, FormatCache> CacheType;


class ConverterCache::Impl {
public:
	///
	void readIndex();
	///
	void writeIndex();
	///
	CacheItem * find(FileName const & from, string const & format);
	CacheType cache;
};


void ConverterCache::Impl::readIndex()
{
	time_t const now = current_time();
	FileName const index(addName(cache_dir.absFileName(), "index"));
	ifstream is(index.toFilesystemEncoding().c_str());
	Lexer lex;
	lex.setStream(is);
	while (lex.isOK()) {
		if (!lex.next(true))
			break;
		string const orig_from = lex.getString();
		if (!lex.next())
			break;
		string const to_format = lex.getString();
		if (!lex.next())
			break;
		time_t const timestamp =
			convert<unsigned long>(lex.getString());
		if (!lex.next())
			break;
		unsigned long const checksum =
			convert<unsigned long>(lex.getString());
		FileName const orig_from_name(orig_from);
		CacheItem item(orig_from_name, to_format, timestamp, checksum);

		// Don't cache files that do not exist anymore
		if (!orig_from_name.exists()) {
			LYXERR(Debug::FILES, "Not caching file `"
				<< orig_from << "' (does not exist anymore).");
			item.cache_name.removeFile();
			continue;
		}

		// Don't add items that are not in the cache anymore
		// This can happen if two instances of LyX are running
		// at the same time and update the index file independantly.
		if (!item.cache_name.exists()) {
			LYXERR(Debug::FILES, "Not caching file `" << orig_from
				<< "' (cached copy does not exist anymore).");
			continue;
		}

		// Delete the cached file if it is too old
		if (difftime(now, item.cache_name.lastModified())
				> lyxrc.converter_cache_maxage) {
			LYXERR(Debug::FILES, "Not caching file `"
				<< orig_from << "' (too old).");
			item.cache_name.removeFile();
			continue;
		}

		FormatCache & format_cache = cache[orig_from_name];
		if (format_cache.from_format.empty())
			format_cache.from_format =
				formats.getFormatFromFile(orig_from_name);
		format_cache.cache[to_format] = item;
	}
	is.close();
}


void ConverterCache::Impl::writeIndex()
{
	FileName const index(addName(cache_dir.absFileName(), "index"));
	ofstream os(index.toFilesystemEncoding().c_str());
	os.close();
	if (!index.changePermission(0600))
		return;
	os.open(index.toFilesystemEncoding().c_str());
	CacheType::iterator it1 = cache.begin();
	CacheType::iterator const end1 = cache.end();
	for (; it1 != end1; ++it1) {
		FormatCacheType const & format_cache = it1->second.cache;
		FormatCacheType::const_iterator it2 = format_cache.begin();
		FormatCacheType::const_iterator const end2 = format_cache.end();
		for (; it2 != end2; ++it2)
			os << Lexer::quoteString(it1->first.absFileName())
			   << ' ' << it2->first << ' '
			   << it2->second.timestamp << ' '
			   << it2->second.checksum << '\n';
	}
	os.close();
}


CacheItem * ConverterCache::Impl::find(FileName const & from,
		string const & format)
{
	if (!lyxrc.use_converter_cache)
		return 0;
	CacheType::iterator const it1 = cache.find(from);
	if (it1 == cache.end())
		return 0;
	FormatCacheType & format_cache = it1->second.cache;
	FormatCacheType::iterator const it2 = format_cache.find(format);
	if (it2 == format_cache.end())
		return 0;
	return &(it2->second);
}


/////////////////////////////////////////////////////////////////////
//
// ConverterCache
//
/////////////////////////////////////////////////////////////////////

ConverterCache::ConverterCache()
	: pimpl_(new Impl)
{}


ConverterCache::~ConverterCache()
{
	delete pimpl_;
}


// FIXME THREAD
ConverterCache & ConverterCache::get()
{
	// Now return the cache
	static ConverterCache singleton;
	return singleton;
}


void ConverterCache::init()
{
	if (!lyxrc.use_converter_cache)
		return;
	// We do this here and not in the constructor because package() gets
	// initialized after all static variables.
	cache_dir = FileName(addName(package().user_support().absFileName(), "cache"));
	if (!cache_dir.exists())
		if (!cache_dir.createDirectory(0700)) {
			lyxerr << "Could not create cache directory `"
			       << cache_dir << "'." << endl;
			exit(EXIT_FAILURE);
		}
	get().pimpl_->readIndex();
}


void ConverterCache::writeIndex() const
{
	if (!lyxrc.use_converter_cache 
		  || cache_dir.empty())
		return;
	pimpl_->writeIndex();
}


void ConverterCache::add(FileName const & orig_from, string const & to_format,
		FileName const & converted_file) const
{
	if (!lyxrc.use_converter_cache || orig_from.empty() ||
	    converted_file.empty())
		return;
	LYXERR(Debug::FILES, ' ' << orig_from
			     << ' ' << to_format << ' ' << converted_file);

	// FIXME: Should not hardcode this (see bug 3819 for details)
	if (to_format == "pstex") {
		FileName const converted_eps(changeExtension(converted_file.absFileName(), "eps"));
		add(orig_from, "eps", converted_eps);
	} else if (to_format == "pdftex") {
		FileName const converted_pdf(changeExtension(converted_file.absFileName(), "pdf"));
		add(orig_from, "pdf6", converted_pdf);
	}

	// Is the file in the cache already?
	CacheItem * item = pimpl_->find(orig_from, to_format);

	time_t const timestamp = orig_from.lastModified();
	Mover const & mover = getMover(to_format);
	if (item) {
		LYXERR(Debug::FILES, "ConverterCache::add(" << orig_from << "):\n"
					"The file is already in the cache.");
		// First test for timestamp
		if (timestamp == item->timestamp) {
			LYXERR(Debug::FILES, "Same timestamp.");
			return;
		}
		// Maybe the contents is still the same?
		item->timestamp = timestamp;
		unsigned long const checksum = orig_from.checksum();
		if (checksum == item->checksum) {
			LYXERR(Debug::FILES, "Same checksum.");
			return;
		}
		item->checksum = checksum;
		if (!mover.copy(converted_file, item->cache_name,
		              onlyFileName(item->cache_name.absFileName()))) {
			LYXERR(Debug::FILES, "Could not copy file " << orig_from << " to "
				<< item->cache_name);
		} else if (!item->cache_name.changePermission(0600)) {
			LYXERR(Debug::FILES, "Could not change file mode"
				<< item->cache_name);
		}
	} else {
		CacheItem new_item(orig_from, to_format, timestamp,
				orig_from.checksum());
		if (mover.copy(converted_file, new_item.cache_name,
		              onlyFileName(new_item.cache_name.absFileName()))) {
			if (!new_item.cache_name.changePermission(0600)) {
				LYXERR(Debug::FILES, "Could not change file mode"
					<< new_item.cache_name);
			}
			FormatCache & format_cache = pimpl_->cache[orig_from];
			if (format_cache.from_format.empty())
				format_cache.from_format =
					formats.getFormatFromFile(orig_from);
			format_cache.cache[to_format] = new_item;
		} else
			LYXERR(Debug::FILES, "ConverterCache::add(" << orig_from << "):\n"
						"Could not copy file.");
	}
}


void ConverterCache::remove(FileName const & orig_from,
		string const & to_format) const
{
	if (!lyxrc.use_converter_cache || orig_from.empty())
		return;
	LYXERR(Debug::FILES, orig_from << ' ' << to_format);

	CacheType::iterator const it1 = pimpl_->cache.find(orig_from);
	if (it1 == pimpl_->cache.end())
		return;
	FormatCacheType & format_cache = it1->second.cache;
	FormatCacheType::iterator const it2 = format_cache.find(to_format);
	if (it2 == format_cache.end())
		return;

	format_cache.erase(it2);
	if (format_cache.empty())
		pimpl_->cache.erase(it1);
}


void ConverterCache::remove_all(string const & from_format,
		string const & to_format) const
{
	if (!lyxrc.use_converter_cache)
		return;
	CacheType::iterator it1 = pimpl_->cache.begin();
	while (it1 != pimpl_->cache.end()) {
		if (it1->second.from_format != from_format) {
			++it1;
			continue;
		}
		FormatCacheType & format_cache = it1->second.cache;
		FormatCacheType::iterator it2 = format_cache.begin();
		while (it2 != format_cache.end()) {
			if (it2->first == to_format) {
				LYXERR(Debug::FILES, "Removing file cache item "
					<< it1->first << ' ' << to_format);
				it2->second.cache_name.removeFile();
				format_cache.erase(it2);
				// Have to start over again since items in a
				// map are not ordered
				it2 = format_cache.begin();
			} else {
				++it2;
			}
		}
		if (format_cache.empty()) {
			pimpl_->cache.erase(it1);
			// Have to start over again since items in a map are
			// not ordered
			it1 = pimpl_->cache.begin();
		} else {
			++it1;
		}
	}
	pimpl_->writeIndex();
}


bool ConverterCache::inCache(FileName const & orig_from,
		string const & to_format) const
{
	if (!lyxrc.use_converter_cache || orig_from.empty())
		return false;
	LYXERR(Debug::FILES, orig_from << ' ' << to_format);

	CacheItem * const item = pimpl_->find(orig_from, to_format);
	if (!item) {
		LYXERR(Debug::FILES, "not in cache.");
		return false;
	}

	// Special handling of pstex and pdftex formats: These are only
	// considered to be in the cache if the corresponding graphics
	// fiels are there as well. Otherwise copy() of the graphics below
	// would fail.
	// FIXME: Should not hardcode this (see bug 3819 for details)
	if (to_format == "pstex") {
		if (!inCache(orig_from, "eps"))
			return false;
	} else if (to_format == "pdftex") {
		if (!inCache(orig_from, "pdf6"))
			return false;
	}

	time_t const timestamp = orig_from.lastModified();
	if (item->timestamp == timestamp) {
		LYXERR(Debug::FILES, "identical timestamp.");
		return true;
	}
	if (item->checksum == orig_from.checksum()) {
		item->timestamp = timestamp;
		LYXERR(Debug::FILES, "identical checksum.");
		return true;
	}
	LYXERR(Debug::FILES, "in cache, but too old.");
	return false;
}


FileName const & ConverterCache::cacheName(FileName const & orig_from,
		string const & to_format) const
{
	LYXERR(Debug::FILES, orig_from << ' ' << to_format);

	CacheItem * const item = pimpl_->find(orig_from, to_format);
	LASSERT(item, { static const FileName fn; return fn; });
	return item->cache_name;
}


bool ConverterCache::copy(FileName const & orig_from, string const & to_format,
		FileName const & dest) const
{
	if (!lyxrc.use_converter_cache || orig_from.empty() || dest.empty())
		return false;
	LYXERR(Debug::FILES, orig_from << ' ' << to_format << ' ' << dest);

	// FIXME: Should not hardcode this (see bug 3819 for details)
	if (to_format == "pstex") {
		FileName const dest_eps(changeExtension(dest.absFileName(), "eps"));
		if (!copy(orig_from, "eps", dest_eps))
			return false;
	} else if (to_format == "pdftex") {
		FileName const dest_pdf(changeExtension(dest.absFileName(), "pdf"));
		if (!copy(orig_from, "pdf6", dest_pdf))
			return false;
	}

	CacheItem * const item = pimpl_->find(orig_from, to_format);
	LASSERT(item, return false);
	Mover const & mover = getMover(to_format);
	return mover.copy(item->cache_name, dest,
	                  onlyFileName(dest.absFileName()));
}

} // namespace lyx

/**
 * \file ConverterCache.C
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

#include "debug.h"
#include "lyxrc.h"
#include "mover.h"

#include "support/filetools.h"
#include "support/lyxlib.h"
#include "support/lyxtime.h"
#include "support/package.h"

#include <boost/crc.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/current_function.hpp>

#include <fstream>
#include <iomanip>
#include <map>
#include <sstream>

using lyx::support::absolutePath;
using lyx::support::addName;

using std::string;

namespace fs = boost::filesystem;

namespace lyx {

namespace {

unsigned long do_crc(string const & s)
{
	boost::crc_32_type crc;
	crc = std::for_each(s.begin(), s.end(), crc);
	return crc.checksum();
}


static string cache_dir;


class CacheItem {
public:
	CacheItem() {}
	CacheItem(string const & orig_from, string const & to_format,
	          time_t t, unsigned long c)
		: timestamp(t), checksum(c)
	{
		BOOST_ASSERT(absolutePath(orig_from));
		std::ostringstream os;
		os << std::setw(10) << std::setfill('0') << do_crc(orig_from)
		   << '-' << to_format;
		cache_name = addName(cache_dir, os.str());
		lyxerr[Debug::FILES] << "Add file cache item " << orig_from
		                     << ' ' << to_format << ' ' << cache_name
		                     << ' ' << timestamp << ' ' << checksum
		                     << '.' << std::endl;
	}
	~CacheItem() {}
	string cache_name;
	time_t timestamp;
	unsigned long checksum;
};

}


/** The cache contains one item per orig file and target format, so use a
 *  nested map to find the cache item quickly by filename and format.
 */
typedef std::map<string, CacheItem> FormatCacheType;
typedef std::map<string, FormatCacheType> CacheType;


class ConverterCache::Impl {
public:
	///
	void readIndex();
	///
	void writeIndex();
	///
	CacheItem * find(string const & from, string const & format);
	CacheType cache;
};


void ConverterCache::Impl::readIndex()
{
	time_t const now = current_time();
	string const index = addName(cache_dir, "index");
	std::ifstream is(index.c_str());
	while (is.good()) {
		string orig_from;
		string to_format;
		time_t timestamp;
		unsigned long checksum;
		if (!(is >> orig_from >> to_format >> timestamp >> checksum))
			return;
		CacheItem item(orig_from, to_format, timestamp, checksum);

		// Don't cache files that do not exist anymore
		if (!fs::exists(orig_from)) {
			lyxerr[Debug::FILES] << "Not caching file `"
				<< orig_from << "' (does not exist anymore)."
				<< std::endl;
			support::unlink(item.cache_name);
			continue;
		}

		// Delete the cached file if it is too old
		if (difftime(now, fs::last_write_time(item.cache_name)) >
		    lyxrc.converter_cache_maxage) {
			lyxerr[Debug::FILES] << "Not caching file `"
				<< orig_from << "' (too old)." << std::endl;
			support::unlink(item.cache_name);
			continue;
		}

		cache[orig_from][to_format] = item;
	}
	is.close();
}


void ConverterCache::Impl::writeIndex()
{
	string const index = addName(cache_dir, "index");
	std::ofstream os(index.c_str());
	os.close();
	if (!lyx::support::chmod(index.c_str(), 0600))
		return;
	os.open(index.c_str());
	CacheType::iterator it1 = cache.begin();
	CacheType::iterator const end1 = cache.end();
	for (; it1 != end1; ++it1) {
		FormatCacheType::iterator it2 = it1->second.begin();
		FormatCacheType::iterator const end2 = it1->second.end();
		for (; it2 != end2; ++it2)
			os << it1->first << ' ' << it2->first << ' '
			   << it2->second.timestamp << ' '
			   << it2->second.checksum << '\n';
	}
	os.close();
}


CacheItem * ConverterCache::Impl::find(string const & from,
		string const & format)
{
	if (!lyxrc.use_converter_cache)
		return 0;
	CacheType::iterator const it1 = cache.find(from);
	if (it1 == cache.end())
		return 0;
	FormatCacheType::iterator const it2 = it1->second.find(format);
	if (it2 == it1->second.end())
		return 0;
	return &(it2->second);
}


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
	cache_dir = addName(support::package().user_support(), "cache");
	if (!fs::exists(cache_dir))
		if (support::mkdir(cache_dir, 0700) != 0) {
			lyxerr << "Could not create cache directory `"
			       << cache_dir << "'." << std::endl;
			exit(EXIT_FAILURE);
		}
	get().pimpl_->readIndex();
}


ConverterCache::ConverterCache()
	: pimpl_(new Impl)
{}


ConverterCache::~ConverterCache()
{
	if (!lyxrc.use_converter_cache)
		return;
	pimpl_->writeIndex();
}


void ConverterCache::add(string const & orig_from, string const & to_format,
		string const & converted_file) const
{
	if (!lyxrc.use_converter_cache)
		return;
	lyxerr[Debug::FILES] << BOOST_CURRENT_FUNCTION << ' ' << orig_from
	                     << ' ' << to_format << ' ' << converted_file
	                     << std::endl;
	BOOST_ASSERT(absolutePath(orig_from));
	BOOST_ASSERT(absolutePath(converted_file));

	// Is the file in the cache already?
	CacheItem * item = pimpl_->find(orig_from, to_format);

	time_t const timestamp = fs::last_write_time(orig_from);
	Mover const & mover = movers(to_format);
	if (item) {
		lyxerr[Debug::FILES] << "ConverterCache::add(" << orig_from << "):\n"
		                        "The file is already in the cache."
		                     << std::endl;
		// First test for timestamp
		if (timestamp == item->timestamp) {
			lyxerr[Debug::FILES] << "Same timestamp."
			                     << std::endl;
			return;
		} else {
			// Maybe the contents is still the same?
			item->timestamp = timestamp;
			unsigned long const checksum = support::sum(orig_from);
			if (checksum == item->checksum) {
				lyxerr[Debug::FILES] << "Same checksum."
				                     << std::endl;
				return;
			}
			item->checksum = checksum;
		}
		if (!mover.copy(converted_file, item->cache_name, 0600))
			lyxerr[Debug::FILES] << "ConverterCache::add("
			                     << orig_from << "):\n"
		        	                "Could not copy file."
		        	             << std::endl;
	} else {
		CacheItem new_item = CacheItem(orig_from, to_format, timestamp,
				support::sum(orig_from));
		if (mover.copy(converted_file, new_item.cache_name, 0600))
			pimpl_->cache[orig_from][to_format] = new_item;
		else
			lyxerr[Debug::FILES] << "ConverterCache::add("
			                     << orig_from << "):\n"
		        	                "Could not copy file."
		                	     << std::endl;
	}
}


void ConverterCache::remove(string const & orig_from,
		string const & to_format) const
{
	if (!lyxrc.use_converter_cache)
		return;
	lyxerr[Debug::FILES] << BOOST_CURRENT_FUNCTION << ' ' << orig_from
	                     << ' ' << to_format << std::endl;
	BOOST_ASSERT(absolutePath(orig_from));

	CacheType::iterator const it1 = pimpl_->cache.find(orig_from);
	if (it1 == pimpl_->cache.end())
		return;
	FormatCacheType::iterator const it2 = it1->second.find(to_format);
	if (it2 == it1->second.end())
		return;

	it1->second.erase(it2);
	if (it1->second.empty())
		pimpl_->cache.erase(it1);
}


bool ConverterCache::inCache(string const & orig_from,
		string const & to_format) const
{
	if (!lyxrc.use_converter_cache)
		return false;
	lyxerr[Debug::FILES] << BOOST_CURRENT_FUNCTION << ' ' << orig_from
	                     << ' ' << to_format << std::endl;
	BOOST_ASSERT(absolutePath(orig_from));

	CacheItem * const item = pimpl_->find(orig_from, to_format);
	if (!item) {
		lyxerr[Debug::FILES] << "not in cache." << std::endl;
		return false;
	}
	time_t const timestamp = fs::last_write_time(orig_from);
	if (item->timestamp == timestamp) {
		lyxerr[Debug::FILES] << "identical timestamp." << std::endl;
		return true;
	}
	if (item->checksum == support::sum(orig_from)) {
		item->timestamp = timestamp;
		lyxerr[Debug::FILES] << "identical checksum." << std::endl;
		return true;
	}
	lyxerr[Debug::FILES] << "in cache, but too old." << std::endl;
	return false;
}


string const ConverterCache::cacheName(string const & orig_from,
		string const & to_format) const
{
	lyxerr[Debug::FILES] << BOOST_CURRENT_FUNCTION << ' ' << orig_from
	                     << ' ' << to_format << std::endl;
	BOOST_ASSERT(absolutePath(orig_from));

	CacheItem * const item = pimpl_->find(orig_from, to_format);
	BOOST_ASSERT(item);
	return item->cache_name;
}


bool ConverterCache::copy(string const & orig_from, string const & to_format,
		string const & dest) const
{
	if (!lyxrc.use_converter_cache)
		return false;
	lyxerr[Debug::FILES] << BOOST_CURRENT_FUNCTION << ' ' << orig_from
	                     << ' ' << to_format << ' ' << dest << std::endl;
	BOOST_ASSERT(absolutePath(orig_from));
	BOOST_ASSERT(absolutePath(dest));

	CacheItem * const item = pimpl_->find(orig_from, to_format);
	BOOST_ASSERT(item);
	Mover const & mover = movers(to_format);
	return mover.copy(item->cache_name, dest);
}

} // namespace lyx

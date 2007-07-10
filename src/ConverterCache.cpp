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
#include "debug.h"

#include "support/convert.h"
#include "support/filetools.h"
#include "support/lyxlib.h"
#include "support/lyxtime.h"
#include "support/Package.h"

#include <boost/crc.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/current_function.hpp>

#include <fstream>
#include <iomanip>
#include <map>
#include <sstream>

using lyx::support::addName;

using std::string;

namespace fs = boost::filesystem;

namespace lyx {

using support::FileName;

namespace {

unsigned long do_crc(string const & s)
{
	boost::crc_32_type crc;
	crc = std::for_each(s.begin(), s.end(), crc);
	return crc.checksum();
}


static FileName cache_dir;


class CacheItem {
public:
	CacheItem() {}
	CacheItem(FileName const & orig_from, string const & to_format,
		  time_t t, unsigned long c)
		: timestamp(t), checksum(c)
	{
		std::ostringstream os;
		os << std::setw(10) << std::setfill('0') << do_crc(orig_from.absFilename())
		   << '-' << to_format;
		cache_name = FileName(addName(cache_dir.absFilename(), os.str()));
		LYXERR(Debug::FILES) << "Add file cache item " << orig_from
				     << ' ' << to_format << ' ' << cache_name
				     << ' ' << timestamp << ' ' << checksum
				     << '.' << std::endl;
	}
	~CacheItem() {}
	FileName cache_name;
	time_t timestamp;
	unsigned long checksum;
};

}


/** The cache contains one item per orig file and target format, so use a
 *  nested map to find the cache item quickly by filename and format.
 */
typedef std::map<string, CacheItem> FormatCacheType;
class FormatCache {
public:
	/// Format of the source file
	string from_format;
	/// Cache target format -> item to quickly find the item by format
	FormatCacheType cache;
};
typedef std::map<FileName, FormatCache> CacheType;


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
	FileName const index(addName(cache_dir.absFilename(), "index"));
	std::ifstream is(index.toFilesystemEncoding().c_str());
	Lexer lex(0, 0);
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
		if (!fs::exists(orig_from_name.toFilesystemEncoding())) {
			LYXERR(Debug::FILES) << "Not caching file `"
				<< orig_from << "' (does not exist anymore)."
				<< std::endl;
			support::unlink(item.cache_name);
			continue;
		}

		// Don't add items that are not in the cache anymore
		// This can happen if two instances of LyX are running
		// at the same time and update the index file independantly.
		if (!fs::exists(item.cache_name.toFilesystemEncoding())) {
			LYXERR(Debug::FILES) << "Not caching file `"
				<< orig_from
				<< "' (cached copy does not exist anymore)."
				<< std::endl;
			continue;
		}

		// Delete the cached file if it is too old
		if (difftime(now, fs::last_write_time(item.cache_name.toFilesystemEncoding())) >
		    lyxrc.converter_cache_maxage) {
			LYXERR(Debug::FILES) << "Not caching file `"
				<< orig_from << "' (too old)." << std::endl;
			support::unlink(item.cache_name);
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
	FileName const index(addName(cache_dir.absFilename(), "index"));
	std::ofstream os(index.toFilesystemEncoding().c_str());
	os.close();
	if (!lyx::support::chmod(index, 0600))
		return;
	os.open(index.toFilesystemEncoding().c_str());
	CacheType::iterator it1 = cache.begin();
	CacheType::iterator const end1 = cache.end();
	for (; it1 != end1; ++it1) {
		FormatCacheType const & format_cache = it1->second.cache;
		FormatCacheType::const_iterator it2 = format_cache.begin();
		FormatCacheType::const_iterator const end2 = format_cache.end();
		for (; it2 != end2; ++it2)
			os << Lexer::quoteString(it1->first.absFilename())
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
	cache_dir = FileName(addName(support::package().user_support().absFilename(), "cache"));
	if (!fs::exists(cache_dir.toFilesystemEncoding()))
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


void ConverterCache::add(FileName const & orig_from, string const & to_format,
		FileName const & converted_file) const
{
	if (!lyxrc.use_converter_cache || orig_from.empty() ||
	    converted_file.empty())
		return;
	LYXERR(Debug::FILES) << BOOST_CURRENT_FUNCTION << ' ' << orig_from
			     << ' ' << to_format << ' ' << converted_file
			     << std::endl;

	// FIXME: Should not hardcode this (see bug 3819 for details)
	if (to_format == "pstex") {
		FileName const converted_eps(support::changeExtension(converted_file.absFilename(), "eps"));
		add(orig_from, "eps", converted_eps);
	} else if (to_format == "pdftex") {
		FileName const converted_pdf(support::changeExtension(converted_file.absFilename(), "pdf"));
		add(orig_from, "pdf", converted_pdf);
	}

	// Is the file in the cache already?
	CacheItem * item = pimpl_->find(orig_from, to_format);

	time_t const timestamp = fs::last_write_time(orig_from.toFilesystemEncoding());
	Mover const & mover = getMover(to_format);
	if (item) {
		LYXERR(Debug::FILES) << "ConverterCache::add(" << orig_from << "):\n"
					"The file is already in the cache."
				     << std::endl;
		// First test for timestamp
		if (timestamp == item->timestamp) {
			LYXERR(Debug::FILES) << "Same timestamp."
					     << std::endl;
			return;
		} else {
			// Maybe the contents is still the same?
			item->timestamp = timestamp;
			unsigned long const checksum = support::sum(orig_from);
			if (checksum == item->checksum) {
				LYXERR(Debug::FILES) << "Same checksum."
						     << std::endl;
				return;
			}
			item->checksum = checksum;
		}
		if (!mover.copy(converted_file, item->cache_name,
		                support::onlyFilename(item->cache_name.absFilename()), 0600))
			LYXERR(Debug::FILES) << "ConverterCache::add("
					     << orig_from << "):\n"
						"Could not copy file."
					     << std::endl;
	} else {
		CacheItem new_item(orig_from, to_format, timestamp,
				support::sum(orig_from));
		if (mover.copy(converted_file, new_item.cache_name,
		               support::onlyFilename(new_item.cache_name.absFilename()), 0600)) {
			FormatCache & format_cache = pimpl_->cache[orig_from];
			if (format_cache.from_format.empty())
				format_cache.from_format =
					formats.getFormatFromFile(orig_from);
			format_cache.cache[to_format] = new_item;
		} else
			LYXERR(Debug::FILES) << "ConverterCache::add("
					     << orig_from << "):\n"
						"Could not copy file."
				     << std::endl;
	}
}


void ConverterCache::remove(FileName const & orig_from,
		string const & to_format) const
{
	if (!lyxrc.use_converter_cache || orig_from.empty())
		return;
	LYXERR(Debug::FILES) << BOOST_CURRENT_FUNCTION << ' ' << orig_from
			     << ' ' << to_format << std::endl;

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
				LYXERR(Debug::FILES)
					<< "Removing file cache item "
					<< it1->first
					<< ' ' << to_format << std::endl;
				support::unlink(it2->second.cache_name);
				format_cache.erase(it2);
				// Have to start over again since items in a
				// map are not ordered
				it2 = format_cache.begin();
			} else
				++it2;
		}
		if (format_cache.empty()) {
			pimpl_->cache.erase(it1);
			// Have to start over again since items in a map are
			// not ordered
			it1 = pimpl_->cache.begin();
		} else
			++it1;
	}
	pimpl_->writeIndex();
}


bool ConverterCache::inCache(FileName const & orig_from,
		string const & to_format) const
{
	if (!lyxrc.use_converter_cache || orig_from.empty())
		return false;
	LYXERR(Debug::FILES) << BOOST_CURRENT_FUNCTION << ' ' << orig_from
			     << ' ' << to_format << std::endl;

	CacheItem * const item = pimpl_->find(orig_from, to_format);
	if (!item) {
		LYXERR(Debug::FILES) << "not in cache." << std::endl;
		return false;
	}
	time_t const timestamp = fs::last_write_time(orig_from.toFilesystemEncoding());
	if (item->timestamp == timestamp) {
		LYXERR(Debug::FILES) << "identical timestamp." << std::endl;
		return true;
	}
	if (item->checksum == support::sum(orig_from)) {
		item->timestamp = timestamp;
		LYXERR(Debug::FILES) << "identical checksum." << std::endl;
		return true;
	}
	LYXERR(Debug::FILES) << "in cache, but too old." << std::endl;
	return false;
}


FileName const & ConverterCache::cacheName(FileName const & orig_from,
		string const & to_format) const
{
	LYXERR(Debug::FILES) << BOOST_CURRENT_FUNCTION << ' ' << orig_from
			     << ' ' << to_format << std::endl;

	CacheItem * const item = pimpl_->find(orig_from, to_format);
	BOOST_ASSERT(item);
	return item->cache_name;
}


bool ConverterCache::copy(FileName const & orig_from, string const & to_format,
		FileName const & dest) const
{
	if (!lyxrc.use_converter_cache || orig_from.empty() || dest.empty())
		return false;
	LYXERR(Debug::FILES) << BOOST_CURRENT_FUNCTION << ' ' << orig_from
			     << ' ' << to_format << ' ' << dest << std::endl;

	// FIXME: Should not hardcode this (see bug 3819 for details)
	if (to_format == "pstex") {
		FileName const dest_eps(support::changeExtension(dest.absFilename(), "eps"));
		if (!copy(orig_from, "eps", dest_eps))
			return false;
	} else if (to_format == "pdftex") {
		FileName const dest_pdf(support::changeExtension(dest.absFilename(), "pdf"));
		if (!copy(orig_from, "pdf", dest_pdf))
			return false;
	}

	CacheItem * const item = pimpl_->find(orig_from, to_format);
	BOOST_ASSERT(item);
	Mover const & mover = getMover(to_format);
	return mover.copy(item->cache_name, dest,
	                  support::onlyFilename(dest.absFilename()));
}

} // namespace lyx

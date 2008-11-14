/**
 * \file DepTable.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Jean-Marc Lasgouttes
 * \author Ben Stanley
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "DepTable.h"

#include "support/debug.h"
#include "support/filetools.h"
#include "support/lstrings.h"
#include "support/lyxtime.h"

#include <sys/stat.h>

#include <fstream>

using namespace std;
using namespace lyx::support;

namespace lyx {


inline
bool DepTable::dep_info::changed() const
{
	return crc_prev != crc_cur && crc_cur != 0;
}


void DepTable::insert(FileName const & f, bool upd)
{
	if (deplist.find(f) == deplist.end()) {
		dep_info di;
		di.crc_prev = 0;
		if (upd) {
			LYXERR(Debug::DEPEND, " CRC...");
			di.crc_cur = f.checksum();
			LYXERR(Debug::DEPEND, "done.");
			struct stat f_info;
			stat(f.toFilesystemEncoding().c_str(), &f_info);
			di.mtime_cur = long(f_info.st_mtime);
		} else {
			di.crc_cur = 0;
			di.mtime_cur = 0;
		}
		deplist[f] = di;
	} else {
		LYXERR(Debug::DEPEND, " Already in DepTable");
	}
}


void DepTable::update()
{
	LYXERR(Debug::DEPEND, "Updating DepTable...");
	time_t const start_time = current_time();

	DepList::iterator itr = deplist.begin();
	while (itr != deplist.end()) {
		dep_info &di = itr->second;

		struct stat f_info;
		if (stat(itr->first.toFilesystemEncoding().c_str(), &f_info) == 0) {
			if (di.mtime_cur == f_info.st_mtime) {
				di.crc_prev = di.crc_cur;
				LYXERR(Debug::DEPEND, itr->first << " same mtime");
			} else {
				di.crc_prev = di.crc_cur;
				LYXERR(Debug::DEPEND, itr->first << " CRC... ");
				di.crc_cur = itr->first.checksum();
				LYXERR(Debug::DEPEND, "done");
			}
		} else {
			// file doesn't exist
			// remove stale files - if it's re-created, it
			// will be re-inserted by deplog.
			LYXERR(Debug::DEPEND, itr->first
				<< " doesn't exist. removing from DepTable.");
			DepList::iterator doomed = itr++;
			deplist.erase(doomed);
			continue;
		}

		if (lyxerr.debugging(Debug::DEPEND)) {
			if (di.changed())
				lyxerr << " +";
			lyxerr << endl;
		}
		++itr;
	}
	time_t const time_sec = current_time() - start_time;
	LYXERR(Debug::DEPEND, "Finished updating DepTable ("
		<< long(time_sec) << " sec).");
}


bool DepTable::sumchange() const
{
	DepList::const_iterator cit = deplist.begin();
	DepList::const_iterator end = deplist.end();
	for (; cit != end; ++cit) {
		if (cit->second.changed()) return true;
	}
	return false;
}


bool DepTable::haschanged(FileName const & fil) const
{
	DepList::const_iterator cit = deplist.find(fil);
	if (cit != deplist.end()) {
		if (cit->second.changed())
			return true;
	}
	return false;
}


bool DepTable::extchanged(string const & ext) const
{
	DepList::const_iterator cit = deplist.begin();
	DepList::const_iterator end = deplist.end();
	for (; cit != end; ++cit) {
		if (suffixIs(cit->first.absFilename(), ext)) {
			if (cit->second.changed())
				return true;
		}
	}
	return false;
}


bool DepTable::ext_exist(string const & ext) const
{
	DepList::const_iterator cit = deplist.begin();
	DepList::const_iterator end = deplist.end();
	for (; cit != end; ++cit) {
		if (suffixIs(cit->first.absFilename(), ext)) {
			return true;
		}
	}
	return false;
}


bool DepTable::exist(FileName const & fil) const
{
	return deplist.find(fil) != deplist.end();
}


void DepTable::remove_files_with_extension(string const & suf)
{
	DepList::iterator cit = deplist.begin();
	DepList::iterator end = deplist.end();
	while (cit != end) {
		if (suffixIs(cit->first.absFilename(), suf)) {
			// Can't erase the current iterator, but we
			// can increment and then erase.
			// Deplist is a map so only the erased
			// iterator is invalidated.
			DepList::iterator doomed = cit++;
			deplist.erase(doomed);
			continue;
		}
		++cit;
	}
}


void DepTable::remove_file(FileName const & filename)
{
	DepList::iterator cit = deplist.begin();
	DepList::iterator end = deplist.end();
	while (cit != end) {
		if (cit->first == filename) {
			// Can't erase the current iterator, but we
			// can increment and then erase.
			// deplist is a map so only the erased
			// iterator is invalidated.
			DepList::iterator doomed = cit++;
			deplist.erase(doomed);
			continue;
		}
		++cit;
	}
}


void DepTable::write(FileName const & f) const
{
	ofstream ofs(f.toFilesystemEncoding().c_str());
	DepList::const_iterator cit = deplist.begin();
	DepList::const_iterator end = deplist.end();
	for (; cit != end; ++cit) {
		// Store the second (most recently calculated)
		// CRC value.
		// The older one is effectively set to 0 upon re-load.
		LYXERR(Debug::DEPEND, "Write dep: "
		       << cit->second.crc_cur << ' '
		       << cit->second.mtime_cur << ' '
		       << cit->first);

		ofs << cit->second.crc_cur << ' '
		    << cit->second.mtime_cur << ' '
		    << cit->first << endl;
	}
}


bool DepTable::read(FileName const & f)
{
	ifstream ifs(f.toFilesystemEncoding().c_str());
	string nome;
	dep_info di;
	// This doesn't change through the loop.
	di.crc_prev = 0;

	while (ifs >> di.crc_cur >> di.mtime_cur && getline(ifs, nome)) {
		nome = ltrim(nome);

		LYXERR(Debug::DEPEND, "Read dep: "
		       << di.crc_cur << ' ' << di.mtime_cur << ' ' << nome);

		deplist[FileName(nome)] = di;
	}
	return deplist.size();
}


} // namespace lyx

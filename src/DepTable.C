/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 *	     Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2001 The LyX Team.
 *
 *           This file is Copyright 1996-2001
 *           Lars Gullik Bjønnes
 *           Ben Stanley
 *
 * ====================================================== 
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "DepTable.h"
#include "debug.h"

#include "support/lyxlib.h"
#include "support/filetools.h"
#include "support/lstrings.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <fstream>

using std::make_pair;
using std::ofstream;
using std::ifstream;
using std::endl;

inline bool DepTable::dep_info::changed() const
{
	return crc_prev != crc_cur && crc_cur != 0;
}

void DepTable::insert(string const & fi,
		      bool upd)
{
	// not quite sure if this is the correct place for MakeAbsPath
	string f = MakeAbsPath(fi);
	if (deplist.find(f) == deplist.end()) {
		dep_info di;
		di.crc_prev = 0;
		if (upd) {
			lyxerr[Debug::DEPEND] << " CRC..." << std::flush;
			di.crc_cur = lyx::sum(f);
			lyxerr[Debug::DEPEND] << "done." << endl;
			struct stat f_info;
			stat(fi.c_str(), &f_info);
			di.mtime_cur = f_info.st_mtime;
		} else {
			di.crc_cur = 0;
			di.mtime_cur = 0;
		}
		deplist[f] = di;
	} else {
		lyxerr[Debug::DEPEND] << " Already in DepTable" << endl;
	}
}
		

void DepTable::update()
{
	lyxerr[Debug::DEPEND] << "Updating DepTable..." << endl;
	time_t start_time = time(0);

	DepList::iterator itr = deplist.begin();
	while (itr != deplist.end()) {
		dep_info &di = itr->second;

		struct stat f_info;
		if (0 == stat(itr->first.c_str(), &f_info) ) {
			if (di.mtime_cur == f_info.st_mtime) {
				di.crc_prev = di.crc_cur;
				lyxerr[Debug::DEPEND] << itr->first << " same mtime";
			} else {
				di.crc_prev = di.crc_cur;
				lyxerr[Debug::DEPEND] << itr->first << " CRC... ";
				di.crc_cur = lyx::sum(itr->first);
				lyxerr[Debug::DEPEND] << "done";
			}
		} else {
			// file doesn't exist
			// remove stale files - if it's re-created, it
			// will be re-inserted by deplog.
			lyxerr[Debug::DEPEND] << itr->first 
				<< " doesn't exist. removing from DepTable." << endl;
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
	time_t time_sec = time(0) - start_time;
	lyxerr[Debug::DEPEND] << "Finished updating DepTable ("
		<< time_sec << " sec)." << endl;
}


bool DepTable::sumchange() const
{
	for (DepList::const_iterator cit = deplist.begin();
	     cit != deplist.end();
	     ++cit) {
		if ((*cit).second.changed()) return true;
	}
	return false;
}


bool DepTable::haschanged(string const & f) const
{
	// not quite sure if this is the correct place for MakeAbsPath
	string fil = MakeAbsPath(f);
	DepList::const_iterator cit = deplist.find(fil);
	if (cit != deplist.end()) {
		if (cit->second.changed())
			return true;
	}
	return false;
}


bool DepTable::extchanged(string const & ext) const
{
	for (DepList::const_iterator cit = deplist.begin();
	     cit != deplist.end();
	     ++cit) {
		if (suffixIs(cit->first, ext)) {
			if (cit->second.changed())
				return true;
		}
	}
	return false;
}


bool DepTable::ext_exist(const string& ext ) const
{
	for (DepList::const_iterator cit = deplist.begin();
		cit != deplist.end(); ++cit )  {
		
		if ( suffixIs(cit->first, ext) ) {
			return true;
		}
	}
	return false;
}

bool DepTable::exist(string const & fil) const
{
	return deplist.find(fil) != deplist.end();
}


void DepTable::remove_files_with_extension(string const & suf)
{
	DepList::iterator cit = deplist.begin();
	while (cit != deplist.end()) {
		if (suffixIs(cit->first, suf)) {
			// Can't erase the current iterator, but we can increment and then erase.
			// deplist is a map so only the erased iterator is invalidated.
			DepList::iterator doomed = cit++;
			deplist.erase(doomed);
			continue;
		}
		cit++;
	}
}


void DepTable::remove_file(string const & filename)
{
	DepList::iterator cit = deplist.begin();
	while (cit != deplist.end()) {
		if (OnlyFilename(cit->first) == filename) {
			// Can't erase the current iterator, but we can increment and then erase.
			// deplist is a map so only the erased iterator is invalidated.
			DepList::iterator doomed = cit++;
			deplist.erase(doomed);
			continue;
		}
		cit++;
	}
}


void DepTable::write(string const & f) const
{
	ofstream ofs(f.c_str());
	for (DepList::const_iterator cit = deplist.begin();
	     cit != deplist.end(); ++cit) {
		if (lyxerr.debugging(Debug::DEPEND)) {
			// Store the second (most recently calculated) CRC value.
			// The older one is effectively set to 0 upon re-load.
			lyxerr << "Write dep: "
			       << cit->first << " "
			       << cit->second.crc_cur << " "
			       << cit->second.mtime_cur << endl;
		}
		ofs << cit->first << " "
		    << cit->second.crc_cur << " "
		    << cit->second.mtime_cur << endl;
	}
}


void DepTable::read(string const & f)
{
	ifstream ifs(f.c_str());
	string nome;
	dep_info di;
	// This doesn't change through the loop.
	di.crc_prev = 0;
	
	while (ifs >> nome >> di.crc_cur >> di.mtime_cur) {
		if (lyxerr.debugging(Debug::DEPEND)) {
			lyxerr << "Read dep: "
			       << nome << " "
			       << di.crc_cur << " "
			       << di.mtime_cur << endl;
		}
		deplist[nome] = di;
	}
}


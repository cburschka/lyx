/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 *	     Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2001 The LyX Team.
 *
 *           This file is Copyright 1996-2001
 *           Lars Gullik Bjønnes
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

void DepTable::insert(string const & fi,
		      bool upd,
		      unsigned long one,
		      unsigned long two)
{
	// not quite sure if this is the correct place for MakeAbsPath
	string f = MakeAbsPath(fi);
	if (deplist.find(f) == deplist.end()) {
		long mtime = 0;
		if (upd) {
			one = two;
			two = lyx::sum(f);
			struct stat f_info;
			stat(fi.c_str(), &f_info);
			mtime = f_info.st_mtime;
		}
		dep_info di;
		di.first = one;
		di.second = two;
		di.mtime = mtime;
#if 0		
		deplist[f] = make_pair(one, two);
#else
		deplist[f] = di;
#endif
	}
}
		

void DepTable::update()
{
	for (DepList::iterator itr = deplist.begin();
	    itr != deplist.end();
	    ++itr) {
		unsigned long const one = itr->second.second;
		unsigned long two = one;
		long mtime = itr->second.mtime;
		struct stat f_info;
		stat(itr->first.c_str(), &f_info);

		if (mtime != f_info.st_mtime) {
			two = lyx::sum(itr->first);
			mtime = f_info.st_mtime;
		}
		
#if 0
		itr->second = make_pair(one, two);
#else
		dep_info di;
		di.first = one;
		di.second = two;
		di.mtime = mtime;
		
		itr->second = di;
#endif
		if (lyxerr.debugging(Debug::DEPEND)) {
			lyxerr << "Update dep: " << itr->first << " "
			       << one << " " << two;
			if (one != two)
				lyxerr << " +";
			lyxerr << endl;
		}
	}
}


bool DepTable::sumchange() const
{
	for (DepList::const_iterator cit = deplist.begin();
	     cit != deplist.end();
	     ++cit) {
		if ((*cit).second.first != cit->second.second) return true;
	}
	return false;
}


bool DepTable::haschanged(string const & f) const
{
	// not quite sure if this is the correct place for MakeAbsPath
	string fil = MakeAbsPath(f);
	DepList::const_iterator cit = deplist.find(fil);
	if (cit != deplist.end()) {
		if (cit->second.first != cit->second.second
		    && cit->second.second != 0)
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
			if (cit->second.first != cit->second.second)
				return true;
		}
	}
	return false;
}


bool DepTable::exist(string const & fil) const
{
	DepList::const_iterator cit = deplist.find(fil);
	if (cit != deplist.end()) return true;
	return false;
}


void DepTable::remove_files_with_extension(string const & suf)
{
	DepList tmp;
	// we want const_iterator (Lgb)
	for (DepList::iterator cit = deplist.begin();
	     cit != deplist.end(); ++cit) {
		if (!suffixIs(cit->first, suf))
			tmp[cit->first] = cit->second;
	}
	deplist.swap(tmp);
}


void DepTable::write(string const & f) const
{
	ofstream ofs(f.c_str());
	for (DepList::const_iterator cit = deplist.begin();
	     cit != deplist.end(); ++cit) {
		if (lyxerr.debugging(Debug::DEPEND)) {
			lyxerr << "Write dep: "
			       << cit->first << " "
			       << cit->second.first << " "
			       << cit->second.second << " "
			       << cit->second.mtime << endl;
		}
		ofs << cit->first << " "
		    << cit->second.first << " "
		    << cit->second.second << " "
		    << cit->second.mtime << endl;
	}
}


void DepTable::read(string const & f)
{
	ifstream ifs(f.c_str());
	string nome;
	unsigned long one = 0;
	unsigned long two = 0;
	unsigned long mtime = 0;
	
	while(ifs >> nome >> one >> two >> mtime) {
		if (lyxerr.debugging(Debug::DEPEND)) {
			lyxerr << "Read dep: "
			       << nome << " "
			       << one << " "
			       << two << " "
			       << mtime << endl;
		}
		dep_info di;
		di.first = one;
		di.second = two;
		di.mtime = mtime;
#if 0		
		deplist[nome] = make_pair(one, two);
#else
		deplist[nome] = di;
#endif
	}
}

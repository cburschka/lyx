/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 *	     Copyright (C) 1995 Matthias Ettrich
 *           Copyright (C) 1995-1998 The LyX Team.
 *
 *           This file is Copyright (C) 1996-1998
 *           Lars Gullik Bjønnes
 *
 * ====================================================== 
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "DepTable.h"
#include "support/lyxlib.h"
#include "support/filetools.h"
#include <fstream>
using std::make_pair;
using std::ofstream;
using std::ifstream;

void DepTable::insert(string const & fi,
		      bool upd,
		      unsigned long one,
		      unsigned long two)
{
	// not quite sure if this is the correct place for MakeAbsPath
	string f = MakeAbsPath(fi);
	if (deplist.find(f) == deplist.end()) {
		if (upd) {
			one = two;
			two = lyxsum(f.c_str());
		}
		deplist[f] = make_pair(one, two);
	}
}
		

void DepTable::update()
{
	for(DepList::iterator itr = deplist.begin();
	    itr != deplist.end();
	    ++itr) {
		unsigned long one = (*itr).second.second;
		unsigned long two = lyxsum((*itr).first.c_str());
		(*itr).second = make_pair(one, two);
		if (lyxerr.debugging()) {
			lyxerr << "update: " << (*itr).first << " "
			       << one << " " << two << endl;
		}
	}
}


bool DepTable::sumchange()
{
	for (DepList::const_iterator cit = deplist.begin();
	     cit != deplist.end();
	     ++cit) {
		if ((*cit).second.first != (*cit).second.second) return true;
	}
	return false;
}


bool DepTable::haschanged(string const & f)
{
	// not quite sure if this is the correct place for MakeAbsPath
	string fil = MakeAbsPath(f);
	DepList::const_iterator cit = deplist.find(fil);
	if (cit != deplist.end()) {
		if ((*cit).second.first != (*cit).second.second
		    && (*cit).second.second != 0)
			return true;
	}
	return false;
}


bool DepTable::extchanged(string const & ext)
{
	for (DepList::const_iterator cit = deplist.begin();
	     cit != deplist.end();
	     ++cit) {
		if (suffixIs((*cit).first, ext.c_str())) {
			if ((*cit).second.first != (*cit).second.second)
				return true;
		}
	}
		     
	return false;
}


void DepTable::write(string const & f)
{
	ofstream ofs(f.c_str());
	for (DepList::const_iterator cit = deplist.begin();
	     cit != deplist.end();
	     ++cit) {
		if (lyxerr.debugging()) {
			lyxerr << "Write dep: "
			       << (*cit).first << " "
			       << (*cit).second.first << " "
			       << (*cit).second.second << endl;
		}
		ofs << (*cit).first << " "
		    << (*cit).second.first << " "
		    << (*cit).second.second << endl;
	}
}

void DepTable::read(string const &f)
{
	ifstream ifs(f.c_str());
	string nome;
	unsigned long one = 0;
	unsigned long two = 0;
	while(ifs >> nome >> one >> two) {
		if (lyxerr.debugging()) {
			lyxerr << "read dep: "
			       << nome << " "
			       << one << " "
			       << two << endl;
		}
		deplist[nome] = make_pair(one, two);
	}
}

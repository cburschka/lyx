#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include <fstream>

#include "chset.h"
#include "support/filetools.h"
#include "support/LRegex.h"
#include "support/LSubstring.h"
#include "debug.h"

bool CharacterSet::loadFile(string const & fname)
{
	map_.clear();
	name_.clear();
	
	if (fname.empty() || fname == "ascii") 
		return true;	// ascii 7-bit
	
	// open definition file
	lyxerr[Debug::KBMAP]
		<< "Reading keymap file " << fname << ".cdef" << endl;
	string filename = LibFileSearch("kbd", fname.c_str(), "cdef");
	ifstream ifs(filename.c_str());
	if (!ifs) {
		lyxerr << "Unable to open keymap file" << endl;
		return true;		// no definition, use 7-bit ascii
	}
	name_ = fname;
	
	string str;
	int n;
	string line;
	LRegex reg("^([12][0-9][0-9])[ \t]+\"([^ ]+)\".*");
	while(getline(ifs, line)) {
		if (reg.exact_match(line)) {
			LRegex::SubMatches const & sub = reg.exec(line);
			n = atoi(line.substr(sub[1].first,
					     sub[1].second).c_str());
			str = LSubstring(line, sub[2].first, sub[2].second);
			if (lyxerr.debugging(Debug::KBMAP))
				lyxerr << "Chardef: " << n
				       << " to [" << str << "]" << endl;
			map_[str] = n;
		}
	}
	lyxerr[Debug::KBMAP] << "End of parsing of .cdef file." << endl;
	return false;
}


bool CharacterSet::encodeString(string & str) const
{
	Cdef::const_iterator cit = map_.find(str);
	if (cit != map_.end()) return true;
	return false;
}


string const & CharacterSet::getName() const
{
	return name_;
}

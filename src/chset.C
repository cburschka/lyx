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

using std::ifstream;
using std::getline;
using std::pair;
using std::make_pair;
using std::endl;

bool CharacterSet::loadFile(string const & fname)
{
	map_.clear();
	name_.erase();
	
	if (fname.empty() || fname == "ascii") 
		return true;	// ascii 7-bit
	
	// open definition file
	lyxerr[Debug::KBMAP]
		<< "Reading character set file " << fname << ".cdef" << endl;
	string filename = LibFileSearch("kbd", fname.c_str(), "cdef");
	ifstream ifs(filename.c_str());
	if (!ifs) {
		lyxerr << "Unable to open character set file" << endl;
		return true;		// no definition, use 7-bit ascii
	}
	name_ = fname;
	
	string str;
	int n;
	string line;
	// Ok, I'll be the first to admit that this is probably not
	// the fastest way to parse the cdef files, but I though it
	// was a bit neat. Anyway it is wrong to use the lyxlex parse
	// without the use of a keyword table.     
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


pair<bool, int> CharacterSet::encodeString(string & const str) const
{
	lyxerr[Debug::KBMAP] << "Checking if we know [" << str << "]" << endl;
	bool ret = false;
	int val = 0;
	Cdef::const_iterator cit = map_.find(str);
	if (cit != map_.end()) {
		ret =  true;
		val = (*cit).second;
	}
	lyxerr[Debug::KBMAP] << "   "
			     << (ret ? "yes we" : "no we don't")
			     <<  " know [" << str << "]" << endl;
	return make_pair(ret, val);
}


string const & CharacterSet::getName() const
{
	return name_;
}

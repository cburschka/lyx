#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "chset.h"
#include "support/filetools.h"
#include "support/lyxlib.h"
#include "debug.h"

#include <boost/regex.hpp>

#include <fstream>

using std::ifstream;
using std::getline;
using std::pair;
using std::make_pair;
using std::endl;
using boost::regex;
using boost::regex_match;
using boost::smatch;

bool CharacterSet::loadFile(string const & fname)
{
	map_.clear();
	name_.erase();

	// ascii 7-bit
	if (fname.empty() || fname == "ascii")
		return true;

	// open definition file
	lyxerr[Debug::KBMAP]
		<< "Reading character set file " << fname << ".cdef" << endl;
	string const filename = LibFileSearch("kbd", fname, "cdef");
	ifstream ifs(filename.c_str());
	if (!ifs) {
		lyxerr << "Unable to open character set file" << endl;
		return true;		// no definition, use 7-bit ascii
	}
	name_ = fname;

	string line;
	// Ok, I'll be the first to admit that this is probably not
	// the fastest way to parse the cdef files, but I though it
	// was a bit neat. Anyway it is wrong to use the lyxlex parse
	// without the use of a keyword table.
	regex reg("^([12][0-9][0-9])[ \t]+\"([^ ]+)\".*");
	while (getline(ifs, line)) {
		smatch sub;
		if (regex_match(line, sub, reg)) {
			int const n = lyx::atoi(sub[1]);
			string const str = sub[2];
			if (lyxerr.debugging(Debug::KBMAP))
				lyxerr << "Chardef: " << n
				       << " to [" << str << "]" << endl;
			map_[str] = n;
		}
	}
	lyxerr[Debug::KBMAP] << "End of parsing of .cdef file." << endl;
	return false;
}


pair<bool, int> const CharacterSet::encodeString(string const & str) const
{
	lyxerr[Debug::KBMAP] << "Checking if we know [" << str << "]" << endl;
	bool ret = false;
	int val = 0;
	Cdef::const_iterator cit = map_.find(str);
	if (cit != map_.end()) {
		ret =  true;
		val = cit->second;
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

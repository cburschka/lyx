/**
 * \file chset.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Jean-Marc Lasgouttes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "chset.h"

#include "debug.h"

#include "support/filetools.h"
#include "support/lyxlib.h"

#include <boost/regex.hpp>

#include <fstream>

using lyx::support::atoi;
using lyx::support::LibFileSearch;

using boost::regex;
using boost::smatch;

using std::endl;
using std::getline;
using std::make_pair;

using std::ifstream;
using std::pair;



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
			int const n = atoi(sub.str(1));
			string const str = sub.str(2);
			if (lyxerr.debugging(Debug::KBMAP))
				lyxerr << "Chardef: " << n
				       << " to [" << str << ']' << endl;
			map_[str] = n;
		}
	}
	lyxerr[Debug::KBMAP] << "End of parsing of .cdef file." << endl;
	return false;
}


pair<bool, int> const CharacterSet::encodeString(string const & str) const
{
	lyxerr[Debug::KBMAP] << "Checking if we know [" << str << ']' << endl;
	bool ret = false;
	int val = 0;
	Cdef::const_iterator cit = map_.find(str);
	if (cit != map_.end()) {
		ret =  true;
		val = cit->second;
	}
	lyxerr[Debug::KBMAP] << "   "
			     << (ret ? "yes we" : "no we don't")
			     <<  " know [" << str << ']' << endl;
	return make_pair(ret, val);
}


string const & CharacterSet::getName() const
{
	return name_;
}

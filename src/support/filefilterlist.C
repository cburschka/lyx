/**
 * \file filefilterlist.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "support/filefilterlist.h"
#include "support/globbing.h"
#include "support/lstrings.h"

// FIXME Interface violation
#include "gettext.h"

#include <boost/regex.hpp>

#include </usr/include/glob.h>

#include <sstream>

using std::distance;
using std::ostringstream;
using std::string;
using std::vector;


namespace lyx {
namespace support {

FileFilterList::FileFilterList(string const & qt_style_filter)
{
	string const filter = qt_style_filter
		+ (qt_style_filter.empty() ? string() : ";;")
		+ _("All files (*)");

	// Split data such as "TeX documents (*.tex);;LyX Documents (*.lyx)"
	// into individual filters.
	static boost::regex const separator_re(";;");

	string::const_iterator it = filter.begin();
	string::const_iterator const end = filter.end();
	while (true) {
		boost::match_results<string::const_iterator> what;

		if (!boost::regex_search(it, end, what, separator_re)) {
			parse_filter(string(it, end));
			break;
		}

		// Everything from the start of the input to
		// the start of the match.
		parse_filter(string(what[-1].first, what[-1].second));

		// Increment the iterator to the end of the match.
		it += distance(it, what[0].second);
	}
}


void FileFilterList::parse_filter(string const & filter)
{
	// Matches "TeX documents (*.tex)",
	// storing "TeX documents " as group 1 and "*.tex" as group 2.
	static boost::regex const filter_re("([^(]*)\\(([^)]+)\\) *$");

	boost::match_results<string::const_iterator> what;
	if (!boost::regex_search(filter, what, filter_re)) {
		// Just a glob, no description.
		filters_.push_back(Filter(string(), trim(filter)));
	} else {
		string const desc = string(what[1].first, what[1].second);
		string const globs = string(what[2].first, what[2].second);
		filters_.push_back(Filter(trim(desc), trim(globs)));
	}
}


string const FileFilterList::str(bool expand) const
{
	ostringstream ss;

	vector<Filter>::const_iterator const begin = filters_.begin();
	vector<Filter>::const_iterator const end = filters_.end();
	vector<Filter>::const_iterator it = begin;
	for (; it != end; ++it) {
		string const globs = expand ?
			convert_brace_glob(it->globs()) : it->globs();
		if (it != begin)
			ss << ";;";
		bool const has_description = !it->description().empty();
		if (has_description)
			ss << it->description() << " (";
		ss << globs;
		if (has_description)
			ss << ')';
	}

	return ss.str();
}

} // namespace support
} // namespace lyx

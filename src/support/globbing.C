/**
 * \file globbing.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "support/globbing.h"
#include "support/lstrings.h"
#include "support/path.h"

// FIXME Interface violation
#include "gettext.h"

#include <boost/regex.hpp>
#include <boost/tokenizer.hpp>

#include <glob.h>

#include <sstream>

using std::distance;
using std::ostringstream;
using std::string;
using std::vector;


namespace lyx {
namespace support {

string const convert_brace_glob(string const & glob)
{
	// Matches " *.{abc,def,ghi}", storing "*." as group 1 and
	// "abc,def,ghi" as group 2.
	static boost::regex const glob_re(" *([^ {]*)\\{([^ }]+)\\}");
	// Matches "abc" and "abc,", storing "abc" as group 1.
	static boost::regex const block_re("([^,}]+),?");

	string pattern;

	string::const_iterator it = glob.begin();
	string::const_iterator const end = glob.end();
	while (true) {
		boost::match_results<string::const_iterator> what;
		if (!boost::regex_search(it, end, what, glob_re)) {
			// Ensure that no information is lost.
			pattern += string(it, end);
			break;
		}

		// Everything from the start of the input to
		// the start of the match.
		pattern += string(what[-1].first, what[-1].second);

		// Given " *.{abc,def}", head == "*." and tail == "abc,def".
		string const head = string(what[1].first, what[1].second);
		string const tail = string(what[2].first, what[2].second);

		// Split the ','-separated chunks of tail so that
		// $head{$chunk1,$chunk2} becomes "$head$chunk1 $head$chunk2".
		string const fmt = " " + head + "$1";
		pattern += boost::regex_merge(tail, block_re, fmt);

		// Increment the iterator to the end of the match.
		it += distance(it, what[0].second);
	}

	return pattern;
}


vector<string> const glob(string const & pattern, int flags)
{
	glob_t glob_buffer;
	glob_buffer.gl_offs = 0;
	glob(pattern.c_str(), flags, 0, &glob_buffer);
	vector<string> const matches(glob_buffer.gl_pathv,
				     glob_buffer.gl_pathv +
				     glob_buffer.gl_pathc);
	globfree(&glob_buffer);
	return matches;
}


vector<string> const expand_globs(string const & mask,
				  string const & directory)
{
	typedef boost::tokenizer<boost::char_separator<char> > Tokenizer;
	boost::char_separator<char> const separator(" ");

	// Given "<glob> <glob> ... *.{abc,def} <glob>", expand to
	//       "<glob> <glob> ... *.abc *.def <glob>"
	string const converted_glob = convert_brace_glob(mask);

	Path p(directory);

	// Split into individual globs and then call 'glob' on each one.
	vector<string> matches;
	Tokenizer const tokens(converted_glob, separator);
	Tokenizer::const_iterator it = tokens.begin();
	Tokenizer::const_iterator const end = tokens.end();
	for (; it != end; ++it) {
		vector<string> const tmp = glob(*it);
		matches.insert(matches.end(), tmp.begin(), tmp.end());
	}
	return matches;
}


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

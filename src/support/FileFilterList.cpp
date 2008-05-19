/**
 * \file FileFilterList.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "support/FileFilterList.h"
#include "support/lstrings.h"

// FIXME Interface violation
#include "gettext.h"

#include <boost/regex.hpp>
#include <boost/tokenizer.hpp>

#include <sstream>

using lyx::docstring;

using std::distance;
using std::ostringstream;
using std::string;
using std::vector;


namespace {

/** Given a string such as
 *      "<glob> <glob> ... *.{abc,def} <glob>",
 *  convert the csh-style brace expresions:
 *      "<glob> <glob> ... *.abc *.def <glob>".
 *  Requires no system support, so should work equally on Unix, Mac, Win32.
 */
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

} // namespace anon


namespace lyx {
namespace support {

FileFilterList::Filter::Filter(lyx::docstring const & description,
			       std::string const & globs)
	: desc_(description)
{
	typedef boost::tokenizer<boost::char_separator<char> > Tokenizer;
	boost::char_separator<char> const separator(" ");

	// Given "<glob> <glob> ... *.{abc,def} <glob>", expand to
	//       "<glob> <glob> ... *.abc *.def <glob>"
	string const expanded_globs = convert_brace_glob(globs);

	// Split into individual globs.
	vector<string> matches;
	Tokenizer const tokens(expanded_globs, separator);
	globs_ = vector<string>(tokens.begin(), tokens.end());
}


FileFilterList::FileFilterList(docstring const & qt_style_filter)
{
	// FIXME UNICODE
	string const filter = lyx::to_utf8(qt_style_filter)
		+ (qt_style_filter.empty() ? string() : ";;")
		+ lyx::to_utf8(_("All Files "))
#if defined(_WIN32)
		+ ("(*.*)");
#else
		+ ("(*)");
#endif

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
	// Matches "LaTeX (plain) (*.tex)",
	// storing "LaTeX (plain) " as group 1 and "*.tex" as group 2.
	static boost::regex const filter_re("(.*)\\(([^(^)]+)\\) *$");

	boost::match_results<string::const_iterator> what;
	if (!boost::regex_search(filter, what, filter_re)) {
		// Just a glob, no description.
		filters_.push_back(Filter(docstring(), trim(filter)));
	} else {
		// FIXME UNICODE
		docstring const desc = lyx::from_utf8(string(what[1].first, what[1].second));
		string const globs = string(what[2].first, what[2].second);
		filters_.push_back(Filter(trim(desc), trim(globs)));
	}
}


docstring const FileFilterList::as_string() const
{
	// FIXME UNICODE
	ostringstream ss;

	vector<Filter>::const_iterator fit = filters_.begin();
	vector<Filter>::const_iterator const fend = filters_.end();
	for (; fit != fend; ++fit) {
		Filter::glob_iterator const gbegin = fit->begin();
		Filter::glob_iterator const gend = fit->end();
		if (gbegin == gend)
			continue;

		if (ss.tellp() > 0)
			ss << ";;";

		bool const has_description = !fit->description().empty();
		if (has_description)
			ss << lyx::to_utf8(fit->description()) << " (";

		for (Filter::glob_iterator git = gbegin; git != gend; ++git) {
			if (git != gbegin)
				ss << ' ';
			ss << *git;
		}

		if (has_description)
			ss << ')';
	}

	return lyx::from_utf8(ss.str());
}

} // namespace support
} // namespace lyx

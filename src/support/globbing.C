/**
 * \file globbing.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#include "globbing.h"

#include "path.h"

#include <boost/regex.hpp>
#include <boost/tokenizer.hpp>

#include "glob.h"

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
		it += std::distance(it, what[0].second);
	}

	return pattern;
}


vector<string> const glob(string const & pattern, int flags)
{
	glob_t glob_buffer = {0, 0, 0, 0, 0, 0, 0, 0, 0};
	glob(pattern.c_str(), flags, 0, &glob_buffer);
	vector<string> const matches(glob_buffer.gl_pathv,
				     glob_buffer.gl_pathv +
				     glob_buffer.gl_pathc);
	globfree(&glob_buffer);
	return matches;
}


vector<string> const expand_globs(string const & mask,
				  std::string const & directory)
{
	typedef boost::tokenizer<boost::char_separator<char> > Tokenizer;
	boost::char_separator<char> const separator(" ");

	// Given "<glob> <glob> ... *.{abc,def} <glob>", expand to
	//       "<glob> <glob> ... *.abc *.def <glob>"
	string const expanded_glob = convert_brace_glob(mask);

	Path p(directory);

	// Split into individual globs and then call 'glob' on each one.
	vector<string> matches;
	Tokenizer const tokens(expanded_glob, separator);
	Tokenizer::const_iterator it = tokens.begin();
	Tokenizer::const_iterator const end = tokens.end();
	for (; it != end; ++it) {
		vector<string> const tmp = glob(*it);
		matches.insert(matches.end(), tmp.begin(), tmp.end());
	}
	return matches;
}

} // namespace support
} // namespace lyx

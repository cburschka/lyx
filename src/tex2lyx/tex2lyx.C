/**
 * \file tex2lyx.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

// {[(

#include <config.h>

#include "tex2lyx.h"
#include "context.h"

#include "debug.h"
#include "lyxtextclass.h"
#include "support/path_defines.h"
#include "support/os.h"

#include <cctype>
#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>

using std::endl;
using std::cout;
using std::cerr;
using std::getline;

using std::ifstream;
using std::istringstream;
using std::ostringstream;
using std::stringstream;
using std::string;
using std::vector;

// Hacks to allow the thing to link in the lyxlayout stuff
LyXErr lyxerr(std::cerr.rdbuf());

void handle_comment(Parser & p)
{
	string s;
	while (p.good()) {
		Token const & t = p.get_token();
		if (t.cat() == catNewline)
			break;
		s += t.asString();
	}
	//cerr << "comment: " << s << "\n";
	p.skip_spaces();
}


string const trim(string const & a, char const * p)
{
	// BOOST_ASSERT(p);

	if (a.empty() || !*p)
		return a;

	string::size_type r = a.find_last_not_of(p);
	string::size_type l = a.find_first_not_of(p);

	// Is this the minimal test? (lgb)
	if (r == string::npos && l == string::npos)
		return string();

	return a.substr(l, r - l + 1);
}


void split(string const & s, vector<string> & result, char delim)
{
	//cerr << "split 1: '" << s << "'\n";
	istringstream is(s);
	string t;
	while (getline(is, t, delim))
		result.push_back(t);
	//cerr << "split 2\n";
}


string join(vector<string> const & input, char const * delim)
{
	ostringstream os;
	for (size_t i = 0; i < input.size(); ++i) {
		if (i)
			os << delim;
		os << input[i];
	}
	return os.str();
}


char const ** is_known(string const & str, char const ** what)
{
	for ( ; *what; ++what)
		if (str == *what)
			return what;
	return 0;
}



// current stack of nested environments
vector<string> active_environments;


string active_environment()
{
	return active_environments.empty() ? string() : active_environments.back();
}


int main(int argc, char * argv[])
{
	if (argc <= 1) {
		cerr << "Usage: " << argv[0] << " <infile.tex>" << endl;
		return 2;
	}

	lyx::support::os::init(&argc, &argv);
	lyx::support::setLyxPaths();

	ifstream is(argv[1]);
	Parser p(is);
	//p.dump();

	stringstream ss;
	LyXTextClass textclass = parse_preamble(p, ss);
	active_environments.push_back("document");
	Context context(true, textclass);
	parse_text(p, ss, FLAG_END, true, context);
	context.check_end_layout(ss);
	ss << "\n\\end_document\n";

	ss.seekg(0);
	cout << ss.str();
	return 0;
}

// }])

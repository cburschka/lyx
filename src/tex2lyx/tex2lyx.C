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
#include "support/filetools.h"
#include "support/os.h"

#include <boost/function.hpp>

#include <cctype>
#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <map>

using std::endl;
using std::cout;
using std::cerr;
using std::getline;

using std::ifstream;
using std::ofstream;
using std::istringstream;
using std::ostringstream;
using std::stringstream;
using std::string;
using std::vector;

using lyx::support::system_lyxdir;
using lyx::support::user_lyxdir;
using lyx::support::IsFileReadable;
using lyx::support::IsFileWriteable;

// Hacks to allow the thing to link in the lyxlayout stuff
LyXErr lyxerr(std::cerr.rdbuf());


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


string documentclass;
bool overwrite_files = false;


/// return the number of arguments consumed
typedef boost::function<int(string const &, string const &)> cmd_helper;


int parse_help(string const &, string const &)
{
	cerr << "Usage: tex2lyx [ command line switches ] <infile.tex>\n"
	        "Command line switches (case sensitive):\n"
	        "\t-help              summarize tex2lyx usage\n"
	        "\t-f                 Force creation of .lyx files even if they exist already\n"
		"\t-userdir dir       try to set user directory to dir\n"
		"\t-sysdir dir        try to set system directory to dir\n"
	        "\t-c textclass       declare the textclass" << endl;
	exit(0);
}


int parse_class(string const & arg, string const &)
{
	if (arg.empty()) {
		cerr << "Missing textclass string after -c switch" << endl;
		exit(1);
	}
	documentclass = arg;
	return 1;
}


int parse_sysdir(string const & arg, string const &)
{
	if (arg.empty()) {
		cerr << "Missing directory for -sysdir switch" << endl;
		exit(1);
	}
	system_lyxdir(arg);
	return 1;
}


int parse_userdir(string const & arg, string const &)
{
	if (arg.empty()) {
		cerr << "Missing directory for -userdir switch" << endl;
		exit(1);
	}
	user_lyxdir(arg);
	return 1;
}


int parse_force(string const &, string const &)
{
	overwrite_files = true;
	return 0;
}


void easyParse(int & argc, char * argv[])
{
	std::map<string, cmd_helper> cmdmap;

	cmdmap["-c"] = parse_class;
	cmdmap["-f"] = parse_force;
	cmdmap["-help"] = parse_help;
	cmdmap["--help"] = parse_help;
	cmdmap["-sysdir"] = parse_sysdir;
	cmdmap["-userdir"] = parse_userdir;

	for (int i = 1; i < argc; ++i) {
		std::map<string, cmd_helper>::const_iterator it
			= cmdmap.find(argv[i]);

		// don't complain if not found - may be parsed later
		if (it == cmdmap.end())
			continue;

		string arg((i + 1 < argc) ? argv[i + 1] : "");
		string arg2((i + 2 < argc) ? argv[i + 2] : "");

		int const remove = 1 + it->second(arg, arg2);

		// Now, remove used arguments by shifting
		// the following ones remove places down.
		argc -= remove;
		for (int j = i; j < argc; ++j)
			argv[j] = argv[j + remove];
		--i;
	}
}


void tex2lyx(std::istream &is, std::ostream &os)
{
	Parser p(is);
	//p.dump();

	stringstream ss;
	LyXTextClass textclass = parse_preamble(p, ss, documentclass);

	active_environments.push_back("document");
	Context context(true, textclass);
	parse_text(p, ss, FLAG_END, true, context);
	context.check_end_layout(ss);
	ss << "\n\\end_document\n";
	active_environments.pop_back();
	ss.seekg(0);
	os << ss.str();
#ifdef TEST_PARSER
	p.reset();
	ofstream parsertest("parsertest.tex");
	while (p.good())
		parsertest << p.get_token().asInput();
	// <origfile> and parsertest.tex should now have identical content
#endif
}


bool tex2lyx(string const &infilename, string const &outfilename)
{
	if (!(IsFileReadable(infilename) && IsFileWriteable(outfilename))) {
		return false;
	}
	if (!overwrite_files && IsFileReadable(outfilename)) {
		cerr << "Not overwriting existing file " << outfilename << "\n";
		return false;
	}
	ifstream is(infilename.c_str());
	ofstream os(outfilename.c_str());
#ifdef FILEDEBUG
	cerr << "File: " << infilename << "\n";
#endif
	tex2lyx(is, os);
	return true;
}


int main(int argc, char * argv[])
{
	easyParse(argc, argv);

	if (argc <= 1) {
		cerr << "Usage: tex2lyx [ command line switches ] <infile.tex>\n"
		          "See tex2lyx -help." << endl;
		return 2;
	}

	lyx::support::os::init(&argc, &argv);
	lyx::support::setLyxPaths();

	if (!IsFileReadable(argv[1])) {
		cerr << "Could not open input file \"" << argv[1]
		     << "\" for reading." << endl;
		return 2;
	}
	ifstream is(argv[1]);
	tex2lyx(is, cout);

	return 0;
}

// }])

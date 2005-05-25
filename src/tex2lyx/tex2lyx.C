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

#include "support/convert.h"
#include "support/filetools.h"
#include "support/fs_extras.h"
#include "support/lstrings.h"
#include "support/lyxlib.h"
#include "support/os.h"
#include "support/package.h"

#include <boost/function.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>

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
using std::map;

using lyx::support::isStrUnsignedInt;
using lyx::support::ltrim;
using lyx::support::MakeAbsPath;
using lyx::support::OnlyPath;
using lyx::support::rtrim;
using lyx::support::IsFileReadable;

namespace fs = boost::filesystem;


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


char const * const * is_known(string const & str, char const * const * what)
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


map<string, vector<ArgumentType> > known_commands;


void add_known_command(string const & command, string const & o1,
                       bool o2)
{
	// We have to handle the following cases:
	// definition                      o1    o2    invocation result
	// \newcommand{\foo}{bar}          ""    false \foo       bar
	// \newcommand{\foo}[1]{bar #1}    "[1]" false \foo{x}    bar x
	// \newcommand{\foo}[1][]{bar #1}  "[1]" true  \foo       bar
	// \newcommand{\foo}[1][]{bar #1}  "[1]" true  \foo[x]    bar x
	// \newcommand{\foo}[1][x]{bar #1} "[1]" true  \foo[x]    bar x
	unsigned int nargs = 0;
	vector<ArgumentType> arguments;
	string const opt1 = rtrim(ltrim(o1, "["), "]");
	if (isStrUnsignedInt(opt1)) {
		// The command has arguments
		nargs = convert<unsigned int>(opt1);
		if (nargs > 0 && o2) {
			// The first argument is optional
			arguments.push_back(optional);
			--nargs;
		}
	}
	for (unsigned int i = 0; i < nargs; ++i)
		arguments.push_back(required);
	known_commands[command] = arguments;
}


namespace {


/*!
 * Read a list of TeX commands from a reLyX compatible syntax file.
 * Since this list is used after all commands that have a LyX counterpart
 * are handled, it does not matter that the "syntax.default" file from reLyX
 * has almost all of them listed. For the same reason the reLyX-specific
 * reLyXre environment is ignored.
 */
void read_syntaxfile(string const & file_name)
{
	ifstream is(file_name.c_str());
	if (!is.good()) {
		cerr << "Could not open syntax file \"" << file_name
		     << "\" for reading." << endl;
		exit(2);
	}
	// We can use our TeX parser, since the syntax of the layout file is
	// modeled after TeX.
	// Unknown tokens are just silently ignored, this helps us to skip some
	// reLyX specific things.
	Parser p(is);
	while (p.good()) {
		Token const & t = p.get_token();
		if (t.cat() == catEscape) {
			string command = t.asInput();
			if (p.next_token().asInput() == "*") {
				p.get_token();
				command += '*';
			}
			p.skip_spaces();
			vector<ArgumentType> arguments;
			while (p.next_token().cat() == catBegin ||
			       p.next_token().asInput() == "[") {
				if (p.next_token().cat() == catBegin) {
					string const arg = p.getArg('{', '}');
					if (arg == "translate")
						arguments.push_back(required);
					else
						arguments.push_back(verbatim);
				} else {
					p.getArg('[', ']');
					arguments.push_back(optional);
				}
			}
			known_commands[command] = arguments;
		}
	}
}


string documentclass;
string syntaxfile;
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
	        "\t-c textclass       declare the textclass\n"
	        "\t-s syntaxfile      read additional syntax file" << endl;
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


int parse_syntaxfile(string const & arg, string const &)
{
	if (arg.empty()) {
		cerr << "Missing syntaxfile string after -s switch" << endl;
		exit(1);
	}
	syntaxfile = arg;
	return 1;
}


// Filled with the command line arguments "foo" of "-sysdir foo" or
// "-userdir foo".
string cl_system_support;
string cl_user_support;


int parse_sysdir(string const & arg, string const &)
{
	if (arg.empty()) {
		cerr << "Missing directory for -sysdir switch" << endl;
		exit(1);
	}
	cl_system_support = arg;
	return 1;
}


int parse_userdir(string const & arg, string const &)
{
	if (arg.empty()) {
		cerr << "Missing directory for -userdir switch" << endl;
		exit(1);
	}
	cl_user_support = arg;
	return 1;
}


int parse_force(string const &, string const &)
{
	overwrite_files = true;
	return 0;
}


void easyParse(int & argc, char * argv[])
{
	map<string, cmd_helper> cmdmap;

	cmdmap["-c"] = parse_class;
	cmdmap["-f"] = parse_force;
	cmdmap["-s"] = parse_syntaxfile;
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


// path of the first parsed file
string masterFilePath;
// path of the currently parsed file
string parentFilePath;

} // anonymous namespace


string getMasterFilePath()
{
	return masterFilePath;
}

string getParentFilePath()
{
	return parentFilePath;
}


namespace {

/*!
 *  Reads tex input from \a is and writes lyx output to \a os.
 *  Uses some common settings for the preamble, so this should only
 *  be used more than once for included documents.
 *  Caution: Overwrites the existing preamble settings if the new document
 *  contains a preamble.
 *  You must ensure that \p parentFilePath is properly set before calling
 *  this function!
 */
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
	ss << "\n\\end_body\n\\end_document\n";
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


/// convert TeX from \p infilename to LyX and write it to \p os
bool tex2lyx(string const &infilename, std::ostream &os)
{
	BOOST_ASSERT(lyx::support::AbsolutePath(infilename));
	ifstream is(infilename.c_str());
	if (!is.good()) {
		cerr << "Could not open input file \"" << infilename
		     << "\" for reading." << endl;
		return false;
	}
	string const oldParentFilePath = parentFilePath;
	parentFilePath = OnlyPath(infilename);
	tex2lyx(is, os);
	parentFilePath = oldParentFilePath;
	return true;
}

} // anonymous namespace


bool tex2lyx(string const &infilename, string const &outfilename)
{
	if (!overwrite_files && IsFileReadable(outfilename)) {
		cerr << "Not overwriting existing file " << outfilename << "\n";
		return false;
	}
	ofstream os(outfilename.c_str());
	if (!os.good()) {
		cerr << "Could not open output file \"" << outfilename
		     << "\" for writing." << endl;
		return false;
	}
#ifdef FILEDEBUG
	cerr << "Input file: " << infilename << "\n";
	cerr << "Output file: " << outfilename << "\n";
#endif
	return tex2lyx(infilename, os);
}


int main(int argc, char * argv[])
{
	fs::path::default_name_check(fs::no_check);

	easyParse(argc, argv);

	if (argc <= 1) {
		cerr << "Usage: tex2lyx [ command line switches ] <infile.tex>\n"
		          "See tex2lyx -help." << endl;
		return 2;
	}

	lyx::support::os::init(argc, argv);
	lyx::support::init_package(argv[0], cl_system_support, cl_user_support,
				   lyx::support::top_build_dir_is_two_levels_up);

	string const system_syntaxfile = lyx::support::LibFileSearch("reLyX", "syntax.default");
	if (system_syntaxfile.empty()) {
		cerr << "Error: Could not find syntax file \"syntax.default\"." << endl;
		exit(1);
	}
	read_syntaxfile(system_syntaxfile);
	if (!syntaxfile.empty())
		read_syntaxfile(syntaxfile);

	string const infilename = MakeAbsPath(argv[1]);
	masterFilePath = OnlyPath(infilename);
	parentFilePath = masterFilePath;

	if (tex2lyx(infilename, cout))
		return EXIT_SUCCESS;
	else
		return EXIT_FAILURE;
}

// }])

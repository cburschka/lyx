/**
 * \file tex2lyx.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

// {[(

#include <config.h>
#include <version.h>

#include "tex2lyx.h"

#include "Context.h"
#include "Encoding.h"
#include "Layout.h"
#include "LayoutFile.h"
#include "LayoutModuleList.h"
#include "ModuleList.h"
#include "Preamble.h"
#include "TextClass.h"

#include "support/convert.h"
#include "support/ExceptionMessage.h"
#include "support/filetools.h"
#include "support/lassert.h"
#include "support/lstrings.h"
#include "support/os.h"
#include "support/Package.h"
#include "support/Systemcall.h"

#include <cstdlib>
#include <algorithm>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <map>

using namespace std;
using namespace lyx::support;
using namespace lyx::support::os;

namespace lyx {

string const trimSpaceAndEol(string const & a)
{
	return trim(a, " \t\n\r");
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
	for (size_t i = 0; i != input.size(); ++i) {
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


TeX2LyXDocClass textclass;
CommandMap known_commands;
CommandMap known_environments;
CommandMap known_math_environments;
FullCommandMap possible_textclass_commands;
FullEnvironmentMap possible_textclass_environments;
FullCommandMap possible_textclass_theorems;
int const LYX_FORMAT = LYX_FORMAT_TEX2LYX;

/// used modules
LayoutModuleList used_modules;
vector<string> preloaded_modules;


void convertArgs(string const & o1, bool o2, vector<ArgumentType> & arguments)
{
	// We have to handle the following cases:
	// definition                      o1    o2    invocation result
	// \newcommand{\foo}{bar}          ""    false \foo       bar
	// \newcommand{\foo}[1]{bar #1}    "[1]" false \foo{x}    bar x
	// \newcommand{\foo}[1][]{bar #1}  "[1]" true  \foo       bar
	// \newcommand{\foo}[1][]{bar #1}  "[1]" true  \foo[x]    bar x
	// \newcommand{\foo}[1][x]{bar #1} "[1]" true  \foo[x]    bar x
	unsigned int nargs = 0;
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
}


void add_known_command(string const & command, string const & o1,
                       bool o2, docstring const & definition)
{
	vector<ArgumentType> arguments;
	convertArgs(o1, o2, arguments);
	known_commands[command] = arguments;
	if (!definition.empty())
		possible_textclass_commands[command] =
			FullCommand(arguments, definition);
}


void add_known_environment(string const & environment, string const & o1,
                           bool o2, docstring const & beg, docstring const &end)
{
	vector<ArgumentType> arguments;
	convertArgs(o1, o2, arguments);
	known_environments[environment] = arguments;
	if (!beg.empty() || ! end.empty())
		possible_textclass_environments[environment] =
			FullEnvironment(arguments, beg, end);
}


void add_known_theorem(string const & theorem, string const & o1,
                       bool o2, docstring const & definition)
{
	vector<ArgumentType> arguments;
	convertArgs(o1, o2, arguments);
	if (!definition.empty())
		possible_textclass_theorems[theorem] =
			FullCommand(arguments, definition);
}


Layout const * findLayoutWithoutModule(TextClass const & textclass,
                                       string const & name, bool command)
{
	DocumentClass::const_iterator it = textclass.begin();
	DocumentClass::const_iterator en = textclass.end();
	for (; it != en; ++it) {
		if (it->latexname() == name &&
		    ((command && it->isCommand()) || (!command && it->isEnvironment())))
			return &*it;
	}
	return 0;
}


InsetLayout const * findInsetLayoutWithoutModule(TextClass const & textclass,
                                                 string const & name, bool command)
{
	DocumentClass::InsetLayouts::const_iterator it = textclass.insetLayouts().begin();
	DocumentClass::InsetLayouts::const_iterator en = textclass.insetLayouts().end();
	for (; it != en; ++it) {
		if (it->second.latexname() == name &&
		    ((command && it->second.latextype() == InsetLayout::COMMAND) ||
		     (!command && it->second.latextype() == InsetLayout::ENVIRONMENT)))
			return &(it->second);
	}
	return 0;
}


namespace {

typedef map<string, DocumentClassPtr> ModuleMap;
ModuleMap modules;


bool addModule(string const & module, LayoutFile const & baseClass, LayoutModuleList & m, vector<string> & visited)
{
	// avoid endless loop for circular dependency
	vector<string>::const_iterator const vb = visited.begin();
	vector<string>::const_iterator const ve = visited.end();
	if (find(vb, ve, module) != ve) {
		cerr << "Circular dependency detected for module " << module << '\n';
		return false;
	}
	LyXModule const * const lm = theModuleList[module];
	if (!lm) {
		cerr << "Could not find module " << module << " in module list.\n";
		return false;
	}
	bool foundone = false;
	LayoutModuleList::const_iterator const exclmodstart = baseClass.excludedModules().begin();
	LayoutModuleList::const_iterator const exclmodend = baseClass.excludedModules().end();
	LayoutModuleList::const_iterator const provmodstart = baseClass.providedModules().begin();
	LayoutModuleList::const_iterator const provmodend = baseClass.providedModules().end();
	vector<string> const reqs = lm->getRequiredModules();
	if (reqs.empty())
		foundone = true;
	else {
		LayoutModuleList::const_iterator mit = m.begin();
		LayoutModuleList::const_iterator men = m.end();
		vector<string>::const_iterator rit = reqs.begin();
		vector<string>::const_iterator ren = reqs.end();
		for (; rit != ren; ++rit) {
			if (find(mit, men, *rit) != men) {
				foundone = true;
				break;
			}
			if (find(provmodstart, provmodend, *rit) != provmodend) {
				foundone = true;
				break;
			}
		}
		if (!foundone) {
			visited.push_back(module);
			for (rit = reqs.begin(); rit != ren; ++rit) {
				if (find(exclmodstart, exclmodend, *rit) == exclmodend) {
					if (addModule(*rit, baseClass, m, visited)) {
						foundone = true;
						break;
					}
				}
			}
			visited.pop_back();
		}
	}
	if (!foundone) {
		cerr << "Could not add required modules for " << module << ".\n";
		return false;
	}
	if (!m.moduleCanBeAdded(module, &baseClass))
		return false;
	m.push_back(module);
	return true;
}


void initModules()
{
	// Create list of dummy document classes if not already done.
	// This is needed since a module cannot be read on its own, only as
	// part of a document class.
	LayoutFile const & baseClass = LayoutFileList::get()[textclass.name()];
	static bool init = true;
	if (init) {
		baseClass.load();
		LyXModuleList::const_iterator const end = theModuleList.end();
		LyXModuleList::const_iterator it = theModuleList.begin();
		for (; it != end; ++it) {
			string const module = it->getID();
			LayoutModuleList m;
			vector<string> v;
			if (!addModule(module, baseClass, m, v))
				continue;
			modules[module] = getDocumentClass(baseClass, m);
		}
		init = false;
	}
}


bool addModule(string const & module)
{
	initModules();
	LayoutFile const & baseClass = LayoutFileList::get()[textclass.name()];
	if (!used_modules.moduleCanBeAdded(module, &baseClass))
		return false;
	FileName layout_file = libFileSearch("layouts", module, "module");
	if (textclass.read(layout_file, TextClass::MODULE)) {
		used_modules.push_back(module);
		// speed up further searches:
		// the module does not need to be checked anymore.
		ModuleMap::iterator const it = modules.find(module);
		if (it != modules.end())
			modules.erase(it);
		return true;
	}
	return false;
}

}


bool checkModule(string const & name, bool command)
{
	// Cache to avoid slowdown by repated searches
	static set<string> failed[2];

	// Only add the module if the command was actually defined in the LyX preamble
	bool theorem = false;
	if (command) {
		if (possible_textclass_commands.find('\\' + name) == possible_textclass_commands.end())
			return false;
	} else {
		if (possible_textclass_environments.find(name) == possible_textclass_environments.end()) {
			if (possible_textclass_theorems.find(name) != possible_textclass_theorems.end())
				theorem = true;
			else
				return false;
		}
	}
	if (failed[command].find(name) != failed[command].end())
		return false;

	initModules();
	LayoutFile const & baseClass = LayoutFileList::get()[textclass.name()];

	// Try to find a module that defines the command.
	// Only add it if the definition can be found in the preamble of the
	// style that corresponds to the command. This is a heuristic and
	// different from the way how we parse the builtin commands of the
	// text class (in that case we only compare the name), but it is
	// needed since it is not unlikely that two different modules define a
	// command with the same name.
	ModuleMap::iterator const end = modules.end();
	for (ModuleMap::iterator it = modules.begin(); it != end; ++it) {
		string const module = it->first;
		if (used_modules.moduleConflicts(module, &baseClass))
			continue;
		if (findLayoutWithoutModule(textclass, name, command))
			continue;
		if (findInsetLayoutWithoutModule(textclass, name, command))
			continue;
		DocumentClassConstPtr c = it->second;
		Layout const * layout = findLayoutWithoutModule(*c, name, command);
		InsetLayout const * insetlayout = layout ? 0 :
			findInsetLayoutWithoutModule(*c, name, command);
		docstring preamble;
		if (layout)
			preamble = layout->preamble();
		else if (insetlayout)
			preamble = insetlayout->preamble();
		if (preamble.empty())
			continue;
		bool add = false;
		if (command) {
			FullCommand const & cmd =
				possible_textclass_commands['\\' + name];
			if (preamble.find(cmd.def) != docstring::npos)
				add = true;
		} else if (theorem) {
			FullCommand const & thm =
				possible_textclass_theorems[name];
			if (preamble.find(thm.def) != docstring::npos)
				add = true;
		} else {
			FullEnvironment const & env =
				possible_textclass_environments[name];
			if (preamble.find(env.beg) != docstring::npos &&
			    preamble.find(env.end) != docstring::npos)
				add = true;
		}
		if (add) {
			FileName layout_file = libFileSearch("layouts", module, "module");
			if (textclass.read(layout_file, TextClass::MODULE)) {
				used_modules.push_back(module);
				// speed up further searches:
				// the module does not need to be checked anymore.
				modules.erase(it);
				return true;
			}
		}
	}
	failed[command].insert(name);
	return false;
}


bool isProvided(string const & name)
{
	// This works only for features that are named like the LaTeX packages
	return textclass.provides(name) || preamble.isPackageUsed(name);
}


bool noweb_mode = false;
bool pdflatex = false;
bool xetex = false;
bool is_nonCJKJapanese = false;
bool roundtrip = false;


namespace {


/*!
 * Read one command definition from the syntax file
 */
void read_command(Parser & p, string command, CommandMap & commands)
{
	if (p.next_token().asInput() == "*") {
		p.get_token();
		command += '*';
	}
	vector<ArgumentType> arguments;
	while (p.next_token().cat() == catBegin ||
	       p.next_token().asInput() == "[") {
		if (p.next_token().cat() == catBegin) {
			string const arg = p.getArg('{', '}');
			if (arg == "translate")
				arguments.push_back(required);
			else if (arg == "group")
				arguments.push_back(req_group);
			else if (arg == "item")
				arguments.push_back(item);
			else if (arg == "displaymath")
				arguments.push_back(displaymath);
			else
				arguments.push_back(verbatim);
		} else {
			string const arg = p.getArg('[', ']');
			if (arg == "group")
				arguments.push_back(opt_group);
			else
				arguments.push_back(optional);
		}
	}
	commands[command] = arguments;
}


/*!
 * Read a class of environments from the syntax file
 */
void read_environment(Parser & p, string const & begin,
		      CommandMap & environments)
{
	string environment;
	while (p.good()) {
		Token const & t = p.get_token();
		if (t.cat() == catLetter)
			environment += t.asInput();
		else if (!environment.empty()) {
			p.putback();
			read_command(p, environment, environments);
			environment.erase();
		}
		if (t.cat() == catEscape && t.asInput() == "\\end") {
			string const end = p.getArg('{', '}');
			if (end == begin)
				return;
		}
	}
}


/*!
 * Read a list of TeX commands from a reLyX compatible syntax file.
 * Since this list is used after all commands that have a LyX counterpart
 * are handled, it does not matter that the "syntax.default" file
 * has almost all of them listed. For the same reason the reLyX-specific
 * reLyXre environment is ignored.
 */
void read_syntaxfile(FileName const & file_name)
{
	ifdocstream is(file_name.toFilesystemEncoding().c_str());
	if (!is.good()) {
		cerr << "Could not open syntax file \"" << file_name
		     << "\" for reading." << endl;
		exit(2);
	}
	// We can use our TeX parser, since the syntax of the layout file is
	// modeled after TeX.
	// Unknown tokens are just silently ignored, this helps us to skip some
	// reLyX specific things.
	Parser p(is, string());
	while (p.good()) {
		Token const & t = p.get_token();
		if (t.cat() == catEscape) {
			string const command = t.asInput();
			if (command == "\\begin") {
				string const name = p.getArg('{', '}');
				if (name == "environments" || name == "reLyXre")
					// We understand "reLyXre", but it is
					// not as powerful as "environments".
					read_environment(p, name,
						known_environments);
				else if (name == "mathenvironments")
					read_environment(p, name,
						known_math_environments);
			} else {
				read_command(p, command, known_commands);
			}
		}
	}
}


string documentclass;
string default_encoding;
bool fixed_encoding = false;
string syntaxfile;
bool copy_files = false;
bool overwrite_files = false;
bool skip_children = false;
int error_code = 0;

/// return the number of arguments consumed
typedef int (*cmd_helper)(string const &, string const &);


int parse_help(string const &, string const &)
{
	cout << "Usage: tex2lyx [options] infile.tex [outfile.lyx]\n"
		"Options:\n"
		"\t-c textclass       Declare the textclass.\n"
		"\t-m mod1[,mod2...]  Load the given modules.\n"
		"\t-copyfiles         Copy all included files to the directory of outfile.lyx.\n"
		"\t-e encoding        Set the default encoding (latex name).\n"
		"\t-fixedenc encoding Like -e, but ignore encoding changing commands while parsing.\n"
		"\t-f                 Force overwrite of .lyx files.\n"
		"\t-help              Print this message and quit.\n"
		"\t-n                 translate literate programming (noweb, sweave,... ) file.\n"
		"\t-skipchildren      Do not translate included child documents.\n"
		"\t-roundtrip         re-export created .lyx file infile.lyx.lyx to infile.lyx.tex.\n"
		"\t-s syntaxfile      read additional syntax file.\n"
		"\t-sysdir SYSDIR     Set system directory to SYSDIR.\n"
		"\t                   Default: " << package().system_support() << "\n"
		"\t-userdir USERDIR   Set user directory to USERDIR.\n"
		"\t                   Default: " << package().user_support() << "\n"
		"\t-version           Summarize version and build info.\n"
		"Paths:\n"
		"\tThe program searches for the files \"encodings\", \"lyxmodules.lst\",\n"
		"\t\"textclass.lst\", \"syntax.default\", and \"unicodesymbols\", first in\n"
		"\t\"USERDIR\", then in \"SYSDIR\". The subdirectories \"USERDIR/layouts\"\n"
		"\tand \"SYSDIR/layouts\" are searched for layout and module files.\n"
		"Check the tex2lyx man page for more details."
	     << endl;
	exit(error_code);
}


int parse_version(string const &, string const &)
{
	cout << "tex2lyx " << lyx_version
	     << " (" << lyx_release_date << ")" << endl;
	cout << "Built on " << lyx_build_date << ", " << lyx_build_time
	     << endl;

	cout << lyx_version_info << endl;
	exit(error_code);
}


void error_message(string const & message)
{
	cerr << "tex2lyx: " << message << "\n\n";
	error_code = 1;
	parse_help(string(), string());
}


int parse_class(string const & arg, string const &)
{
	if (arg.empty())
		error_message("Missing textclass string after -c switch");
	documentclass = arg;
	return 1;
}


int parse_module(string const & arg, string const &)
{
	if (arg.empty())
		error_message("Missing modules string after -m switch");
	split(arg, preloaded_modules, ',');
	return 1;
}


int parse_encoding(string const & arg, string const &)
{
	if (arg.empty())
		error_message("Missing encoding string after -e switch");
	default_encoding = arg;
	return 1;
}


int parse_fixed_encoding(string const & arg, string const &)
{
	if (arg.empty())
		error_message("Missing encoding string after -fixedenc switch");
	default_encoding = arg;
	fixed_encoding = true;
	return 1;
}


int parse_syntaxfile(string const & arg, string const &)
{
	if (arg.empty())
		error_message("Missing syntaxfile string after -s switch");
	syntaxfile = internal_path(arg);
	return 1;
}


// Filled with the command line arguments "foo" of "-sysdir foo" or
// "-userdir foo".
string cl_system_support;
string cl_user_support;


int parse_sysdir(string const & arg, string const &)
{
	if (arg.empty())
		error_message("Missing directory for -sysdir switch");
	cl_system_support = internal_path(arg);
	return 1;
}


int parse_userdir(string const & arg, string const &)
{
	if (arg.empty())
		error_message("Missing directory for -userdir switch");
	cl_user_support = internal_path(arg);
	return 1;
}


int parse_force(string const &, string const &)
{
	overwrite_files = true;
	return 0;
}


int parse_noweb(string const &, string const &)
{
	noweb_mode = true;
	return 0;
}


int parse_skipchildren(string const &, string const &)
{
	skip_children = true;
	return 0;
}


int parse_roundtrip(string const &, string const &)
{
	roundtrip = true;
	return 0;
}


int parse_copyfiles(string const &, string const &)
{
	copy_files = true;
	return 0;
}


void easyParse(int & argc, char * argv[])
{
	map<string, cmd_helper> cmdmap;

	cmdmap["-h"] = parse_help;
	cmdmap["-help"] = parse_help;
	cmdmap["--help"] = parse_help;
	cmdmap["-v"] = parse_version;
	cmdmap["-version"] = parse_version;
	cmdmap["--version"] = parse_version;
	cmdmap["-c"] = parse_class;
	cmdmap["-m"] = parse_module;
	cmdmap["-e"] = parse_encoding;
	cmdmap["-fixedenc"] = parse_fixed_encoding;
	cmdmap["-f"] = parse_force;
	cmdmap["-s"] = parse_syntaxfile;
	cmdmap["-n"] = parse_noweb;
	cmdmap["-skipchildren"] = parse_skipchildren;
	cmdmap["-sysdir"] = parse_sysdir;
	cmdmap["-userdir"] = parse_userdir;
	cmdmap["-roundtrip"] = parse_roundtrip;
	cmdmap["-copyfiles"] = parse_copyfiles;

	for (int i = 1; i < argc; ++i) {
		map<string, cmd_helper>::const_iterator it
			= cmdmap.find(argv[i]);

		// don't complain if not found - may be parsed later
		if (it == cmdmap.end()) {
			if (argv[i][0] == '-')
				error_message(string("Unknown option `") + argv[i] + "'.");
			else
				continue;
		}

		string arg = (i + 1 < argc) ? os::utf8_argv(i + 1) : string();
		string arg2 = (i + 2 < argc) ? os::utf8_argv(i + 2) : string();

		int const remove = 1 + it->second(arg, arg2);

		// Now, remove used arguments by shifting
		// the following ones remove places down.
		os::remove_internal_args(i, remove);
		argc -= remove;
		for (int j = i; j < argc; ++j)
			argv[j] = argv[j + remove];
		--i;
	}
}


// path of the first parsed file
string masterFilePathLyX;
string masterFilePathTeX;
// path of the currently parsed file
string parentFilePathTeX;

} // anonymous namespace


string getMasterFilePath(bool input)
{
	return input ? masterFilePathTeX : masterFilePathLyX;
}

string getParentFilePath(bool input)
{
	if (input)
		return parentFilePathTeX;
	string const rel = to_utf8(makeRelPath(from_utf8(masterFilePathTeX),
	                                       from_utf8(parentFilePathTeX)));
	if (rel.substr(0, 3) == "../") {
		// The parent is not below the master - keep the path
		return parentFilePathTeX;
	}
	return makeAbsPath(rel, masterFilePathLyX).absFileName();
}


bool copyFiles()
{
	return copy_files;
}


bool overwriteFiles()
{
	return overwrite_files;
}


bool skipChildren()
{
	return skip_children;
}


bool roundtripMode()
{
	return roundtrip;
}


string fixedEncoding()
{
	if (fixed_encoding)
		return default_encoding;
	return "";
}


namespace {

/*!
 *  Reads tex input from \a is and writes lyx output to \a os.
 *  Uses some common settings for the preamble, so this should only
 *  be used more than once for included documents.
 *  Caution: Overwrites the existing preamble settings if the new document
 *  contains a preamble.
 *  You must ensure that \p parentFilePathTeX is properly set before calling
 *  this function!
 */
bool tex2lyx(idocstream & is, ostream & os, string encoding)
{
	// Set a sensible default encoding.
	// This is used until an encoding command is found.
	// For child documents use the encoding of the master, else ISO8859-1,
	// (formerly known by its latex name latin1), since ISO8859-1 does not
	// cause an iconv error if the actual encoding is different (bug 7509).
	if (encoding.empty()) {
		if (preamble.inputencoding() == "auto")
			encoding = "ISO8859-1";
		else {
			Encoding const * const enc = encodings.fromLyXName(
				preamble.inputencoding(), true);
			encoding = enc->iconvName();
		}
	}

	Parser p(is, fixed_encoding ? default_encoding : string());
	p.setEncoding(encoding);
	//p.dump();

	preamble.parse(p, documentclass, textclass);
	list<string> removed_modules;
	LayoutFile const & baseClass = LayoutFileList::get()[textclass.name()];
	if (!used_modules.adaptToBaseClass(&baseClass, removed_modules)) {
		cerr << "Could not load default modules for text class." << endl;
		return false;
	}

	// Load preloaded modules.
	// This needs to be done after the preamble is parsed, since the text
	// class may not be known before. It neds to be done before parsing
	// body, since otherwise the commands/environments provided by the
	// modules would be parsed as ERT.
	for (size_t i = 0; i < preloaded_modules.size(); ++i) {
		if (!addModule(preloaded_modules[i])) {
			cerr << "Error: Could not load module \""
			     << preloaded_modules[i] << "\"." << endl;
			return false;
		}
	}
	// Ensure that the modules are not loaded again for included files
	preloaded_modules.clear();

	active_environments.push_back("document");
	Context context(true, textclass);
	stringstream ss;
	// store the document language in the context to be able to handle the
	// commands like \foreignlanguage and \textenglish etc.
	context.font.language = preamble.defaultLanguage();
	// parse the main text
	parse_text(p, ss, FLAG_END, true, context);
	if (Context::empty)
		// Empty document body. LyX needs at least one paragraph.
		context.check_layout(ss);
	context.check_end_layout(ss);
	ss << "\n\\end_body\n\\end_document\n";
	active_environments.pop_back();

	// We know the used modules only after parsing the full text
	if (!used_modules.empty()) {
		LayoutModuleList::const_iterator const end = used_modules.end();
		LayoutModuleList::const_iterator it = used_modules.begin();
		for (; it != end; ++it)
			preamble.addModule(*it);
	}
	if (!preamble.writeLyXHeader(os, !active_environments.empty())) {
		cerr << "Could not write LyX file header." << endl;
		return false;
	}

	ss.seekg(0);
	os << ss.str();
#ifdef TEST_PARSER
	p.reset();
	ofdocstream parsertest("parsertest.tex");
	while (p.good())
		parsertest << p.get_token().asInput();
	// <origfile> and parsertest.tex should now have identical content
#endif
	return true;
}


/// convert TeX from \p infilename to LyX and write it to \p os
bool tex2lyx(FileName const & infilename, ostream & os, string const & encoding)
{
	ifdocstream is;
	// forbid buffering on this stream
	is.rdbuf()->pubsetbuf(0,0);
	is.open(infilename.toFilesystemEncoding().c_str());
	if (!is.good()) {
		cerr << "Could not open input file \"" << infilename
		     << "\" for reading." << endl;
		return false;
	}
	string const oldParentFilePath = parentFilePathTeX;
	parentFilePathTeX = onlyPath(infilename.absFileName());
	bool retval = tex2lyx(is, os, encoding);
	parentFilePathTeX = oldParentFilePath;
	return retval;
}

} // anonymous namespace


bool tex2lyx(string const & infilename, FileName const & outfilename,
	     string const & encoding)
{
	if (outfilename.isReadableFile()) {
		if (overwrite_files) {
			cerr << "Overwriting existing file "
			     << outfilename << endl;
		} else {
			cerr << "Not overwriting existing file "
			     << outfilename << endl;
			return false;
		}
	} else {
		cerr << "Creating file " << outfilename << endl;
	}
	ofstream os(outfilename.toFilesystemEncoding().c_str());
	if (!os.good()) {
		cerr << "Could not open output file \"" << outfilename
		     << "\" for writing." << endl;
		return false;
	}
#ifdef FILEDEBUG
	cerr << "Input file: " << infilename << "\n";
	cerr << "Output file: " << outfilename << "\n";
#endif
	return tex2lyx(FileName(infilename), os, encoding);
}


bool tex2tex(string const & infilename, FileName const & outfilename,
             string const & encoding)
{
	if (!tex2lyx(infilename, outfilename, encoding))
		return false;
	string command = quoteName(package().lyx_binary().toFilesystemEncoding());
	if (overwrite_files)
		command += " -f main";
	else
		command += " -f none";
	if (pdflatex)
		command += " -e pdflatex ";
	else if (xetex)
		command += " -e xetex ";
	else
		command += " -e latex ";
	command += quoteName(outfilename.toFilesystemEncoding());
	Systemcall one;
	if (one.startscript(Systemcall::Wait, command) == 0)
		return true;
	cerr << "Error: Running '" << command << "' failed." << endl;
	return false;
}

} // namespace lyx


int main(int argc, char * argv[])
{
	using namespace lyx;

	//setlocale(LC_CTYPE, "");

	lyxerr.setStream(cerr);

	os::init(argc, argv);

	try {
		init_package(internal_path(os::utf8_argv(0)), string(), string());
	} catch (ExceptionMessage const & message) {
		cerr << to_utf8(message.title_) << ":\n"
		     << to_utf8(message.details_) << endl;
		if (message.type_ == ErrorException)
			return EXIT_FAILURE;
	}

	easyParse(argc, argv);

	if (argc <= 1)
		error_message("Not enough arguments.");

	try {
		init_package(internal_path(os::utf8_argv(0)),
			     cl_system_support, cl_user_support);
	} catch (ExceptionMessage const & message) {
		cerr << to_utf8(message.title_) << ":\n"
		     << to_utf8(message.details_) << endl;
		if (message.type_ == ErrorException)
			return EXIT_FAILURE;
	}

	// Now every known option is parsed. Look for input and output
	// file name (the latter is optional).
	string infilename = internal_path(os::utf8_argv(1));
	infilename = makeAbsPath(infilename).absFileName();

	string outfilename;
	if (argc > 2) {
		outfilename = internal_path(os::utf8_argv(2));
		if (outfilename != "-")
			outfilename = makeAbsPath(outfilename).absFileName();
		if (roundtrip) {
			if (outfilename == "-") {
				cerr << "Error: Writing to standard output is "
				        "not supported in roundtrip mode."
				     << endl;
				return EXIT_FAILURE;
			}
			string texfilename = changeExtension(outfilename, ".tex");
			if (equivalent(FileName(infilename), FileName(texfilename))) {
				cerr << "Error: The input file `" << infilename
				     << "´ would be overwritten by the TeX file exported from `"
				     << outfilename << "´ in roundtrip mode." << endl;
				return EXIT_FAILURE;
			}
		}
	} else if (roundtrip) {
		// avoid overwriting the input file
		outfilename = changeExtension(infilename, ".lyx.lyx");
	} else
		outfilename = changeExtension(infilename, ".lyx");

	// Read the syntax tables
	FileName const system_syntaxfile = libFileSearch("", "syntax.default");
	if (system_syntaxfile.empty()) {
		cerr << "Error: Could not find syntax file \"syntax.default\"." << endl;
		return EXIT_FAILURE;
	}
	read_syntaxfile(system_syntaxfile);
	if (!syntaxfile.empty())
		read_syntaxfile(makeAbsPath(syntaxfile));

	// Read the encodings table.
	FileName const symbols_path = libFileSearch(string(), "unicodesymbols");
	if (symbols_path.empty()) {
		cerr << "Error: Could not find file \"unicodesymbols\"."
		     << endl;
		return EXIT_FAILURE;
	}
	FileName const enc_path = libFileSearch(string(), "encodings");
	if (enc_path.empty()) {
		cerr << "Error: Could not find file \"encodings\"."
		     << endl;
		return EXIT_FAILURE;
	}
	encodings.read(enc_path, symbols_path);
	if (!default_encoding.empty()) {
		Encoding const * const enc = encodings.fromLaTeXName(
			default_encoding, Encoding::any, true);
		if (!enc)
			error_message("Unknown LaTeX encoding `" + default_encoding + "'");
		default_encoding = enc->iconvName();
	}

	// Load the layouts
	LayoutFileList::get().read();
	//...and the modules
	theModuleList.read();

	// The real work now.
	masterFilePathTeX = onlyPath(infilename);
	parentFilePathTeX = masterFilePathTeX;
	if (outfilename == "-") {
		// assume same directory as input file
		masterFilePathLyX = masterFilePathTeX;
		if (tex2lyx(FileName(infilename), cout, default_encoding))
			return EXIT_SUCCESS;
	} else {
		masterFilePathLyX = onlyPath(outfilename);
		if (copy_files) {
			FileName const path(masterFilePathLyX);
			if (!path.isDirectory()) {
				if (!path.createPath()) {
					cerr << "Warning: Could not create directory for file `"
					     << masterFilePathLyX << "´." << endl;
					return EXIT_FAILURE;
				}
			}
		}
		if (roundtrip) {
			if (tex2tex(infilename, FileName(outfilename), default_encoding))
				return EXIT_SUCCESS;
		} else {
			if (tex2lyx(infilename, FileName(outfilename), default_encoding))
				return EXIT_SUCCESS;
		}
	}
	return EXIT_FAILURE;
}

// }])

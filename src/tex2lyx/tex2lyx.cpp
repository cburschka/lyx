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
#include "support/debug.h"
#include "support/ExceptionMessage.h"
#include "support/filetools.h"
#include "support/lassert.h"
#include "support/lstrings.h"
#include "support/Messages.h"
#include "support/os.h"
#include "support/Package.h"
#include "support/Systemcall.h"

#include <cstdlib>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <map>

using namespace std;
using namespace lyx::support;
using namespace lyx::support::os;

namespace lyx {

namespace frontend {
namespace Alert {
	void warning(docstring const & title, docstring const & message,
				 bool const &)
	{
		LYXERR0(title);
		LYXERR0(message);
	}
}
}


// Dummy texrow support
void TexRow::newline()
{}


void TexRow::newlines(int)
{}


// Dummy LyXRC support
class LyXRC {
public:
	string icon_set;
} lyxrc;


// Dummy translation support
Messages messages_;
Messages const & getMessages(std::string const &)
{
	return messages_;
}


Messages const & getGuiMessages()
{
	return messages_;
}


// Keep the linker happy on Windows
void lyx_exit(int)
{}


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
int const LYX_FORMAT = LYX_FORMAT_TEX2LYX;

/// used modules
LayoutModuleList used_modules;


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


bool checkModule(string const & name, bool command)
{
	// Cache to avoid slowdown by repated searches
	static set<string> failed[2];

	// Only add the module if the command was actually defined in the LyX preamble
	if (command) {
		if (possible_textclass_commands.find('\\' + name) == possible_textclass_commands.end())
			return false;
	} else {
		if (possible_textclass_environments.find(name) == possible_textclass_environments.end())
			return false;
	}
	if (failed[command].find(name) != failed[command].end())
		return false;

	// Create list of dummy document classes if not already done.
	// This is needed since a module cannot be read on its own, only as
	// part of a document class.
	LayoutFile const & baseClass = LayoutFileList::get()[textclass.name()];
	typedef map<string, DocumentClass *> ModuleMap;
	static ModuleMap modules;
	static bool init = true;
	if (init) {
		baseClass.load();
		DocumentClassBundle & bundle = DocumentClassBundle::get();
		LyXModuleList::const_iterator const end = theModuleList.end();
		LyXModuleList::const_iterator it = theModuleList.begin();
		for (; it != end; it++) {
			string const module = it->getID();
			LayoutModuleList m;
			// FIXME this excludes all modules that depend on another one
			if (!m.moduleCanBeAdded(module, &baseClass))
				continue;
			m.push_back(module);
			modules[module] = &bundle.makeDocumentClass(baseClass, m);
		}
		init = false;
	}

	// Try to find a module that defines the command.
	// Only add it if the definition can be found in the preamble of the
	// style that corresponds to the command. This is a heuristic and
	// different from the way how we parse the builtin commands of the
	// text class (in that case we only compare the name), but it is
	// needed since it is not unlikely that two different modules define a
	// command with the same name.
	ModuleMap::iterator const end = modules.end();
	for (ModuleMap::iterator it = modules.begin(); it != end; it++) {
		string const module = it->first;
		if (!used_modules.moduleCanBeAdded(module, &baseClass))
			continue;
		if (findLayoutWithoutModule(textclass, name, command))
			continue;
		if (findInsetLayoutWithoutModule(textclass, name, command))
			continue;
		DocumentClass const * c = it->second;
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


bool noweb_mode = false;
bool pdflatex = false;
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
			else if (arg == "item")
				arguments.push_back(item);
			else
				arguments.push_back(verbatim);
		} else {
			p.getArg('[', ']');
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
	Parser p(is);
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
string syntaxfile;
bool overwrite_files = false;
int error_code = 0;

/// return the number of arguments consumed
typedef int (*cmd_helper)(string const &, string const &);


int parse_help(string const &, string const &)
{
	cerr << "Usage: tex2lyx [options] infile.tex [outfile.lyx]\n"
		"Options:\n"
		"\t-c textclass       Declare the textclass.\n"
		"\t-e encoding        Set the default encoding (latex name).\n"
		"\t-f                 Force overwrite of .lyx files.\n"
		"\t-help              Print this message and quit.\n"
		"\t-n                 translate a noweb (aka literate programming) file.\n"
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
	lyxerr << "tex2lyx " << lyx_version
	       << " (" << lyx_release_date << ")" << endl;
	lyxerr << "Built on " << __DATE__ << ", " << __TIME__ << endl;

	lyxerr << lyx_version_info << endl;
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


int parse_encoding(string const & arg, string const &)
{
	if (arg.empty())
		error_message("Missing encoding string after -e switch");
	default_encoding = arg;
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


int parse_roundtrip(string const &, string const &)
{
	roundtrip = true;
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
	cmdmap["-e"] = parse_encoding;
	cmdmap["-f"] = parse_force;
	cmdmap["-s"] = parse_syntaxfile;
	cmdmap["-n"] = parse_noweb;
	cmdmap["-sysdir"] = parse_sysdir;
	cmdmap["-userdir"] = parse_userdir;
	cmdmap["-roundtrip"] = parse_roundtrip;

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
void tex2lyx(idocstream & is, ostream & os, string encoding)
{
	// Set a sensible default encoding.
	// This is used until an encoding command is found.
	// For child documents use the encoding of the master, else latin1,
	// since latin1 does not cause an iconv error if the actual encoding
	// is different (bug 7509).
	if (encoding.empty()) {
		if (preamble.inputencoding() == "auto")
			encoding = "latin1";
		else
			encoding = preamble.inputencoding();
	}

	Parser p(is);
	p.setEncoding(encoding);
	//p.dump();

	preamble.parse(p, documentclass, textclass);

	active_environments.push_back("document");
	Context context(true, textclass);
	stringstream ss;
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
		for (; it != end; it++)
			preamble.addModule(*it);
	}
	preamble.writeLyXHeader(os);

	ss.seekg(0);
	os << ss.str();
#ifdef TEST_PARSER
	p.reset();
	ofdocstream parsertest("parsertest.tex");
	while (p.good())
		parsertest << p.get_token().asInput();
	// <origfile> and parsertest.tex should now have identical content
#endif
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
	string const oldParentFilePath = parentFilePath;
	parentFilePath = onlyPath(infilename.absFileName());
	tex2lyx(is, os, encoding);
	parentFilePath = oldParentFilePath;
	return true;
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
	if (roundtrip) {
		if (argc > 2) {
			// Do not allow a user supplied output filename
			// (otherwise it could easily happen that LyX would
			// overwrite the original .tex file)
			cerr << "Error: output filename must not be given in roundtrip mode."
			     << endl;
			return EXIT_FAILURE;
		}
		outfilename = changeExtension(infilename, ".lyx.lyx");
	} else if (argc > 2) {
		outfilename = internal_path(os::utf8_argv(2));
		if (outfilename != "-")
			outfilename = makeAbsPath(outfilename).absFileName();
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
	if (!default_encoding.empty() && !encodings.fromLaTeXName(default_encoding))
		error_message("Unknown LaTeX encoding `" + default_encoding + "'");

	// Load the layouts
	LayoutFileList::get().read();
	//...and the modules
	theModuleList.read();

	// The real work now.
	masterFilePath = onlyPath(infilename);
	parentFilePath = masterFilePath;
	if (outfilename == "-") {
		if (tex2lyx(FileName(infilename), cout, default_encoding))
			return EXIT_SUCCESS;
	} else if (roundtrip) {
		if (tex2tex(infilename, FileName(outfilename), default_encoding))
			return EXIT_SUCCESS;
	} else {
		if (tex2lyx(infilename, FileName(outfilename), default_encoding))
			return EXIT_SUCCESS;
	}
	return EXIT_FAILURE;
}

// }])

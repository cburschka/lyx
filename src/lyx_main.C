/**
 * \file lyx_main.C
 * Copyright 1995-2002 the LyX Team
 * Read the file COPYING
 *
 * \author Lars Gullik Bjønnes
 */

#include <config.h>
#include <version.h>

#include "lyx_main.h"

#include "support/filetools.h"
#include "support/lyxlib.h"
#include "support/FileInfo.h"
#include "support/path.h"
#include "support/path_defines.h"
#include "debug.h"
#include "gettext.h"
#include "lyxlex.h"

#include "graphics/GraphicsTypes.h"

#include "bufferlist.h"
#include "buffer.h"
#include "buffer_funcs.h"
#include "lyxserver.h"
#include "kbmap.h"
#include "lyxfunc.h"
#include "ToolbarBackend.h"
#include "MenuBackend.h"
#include "language.h"
#include "lastfiles.h"
#include "encoding.h"
#include "converter.h"
#include "format.h"
#include "lyxtextclasslist.h"

#include "frontends/Alert.h"
#include "frontends/lyx_gui.h"

#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/signals/signal1.hpp>

#include <cstdlib>
#include <csignal>
#include <iostream>

using namespace lyx::support;

using std::vector;
using std::endl;

#ifndef CXX_GLOBAL_CSTD
using std::exit;
using std::signal;
using std::system;
#endif

extern void LoadLyXFile(string const &);
extern void QuitLyX();

extern LyXServer * lyxserver;

extern string system_lyxdir;
extern string user_lyxdir;

DebugStream lyxerr;

boost::scoped_ptr<LastFiles> lastfiles;

// This is the global bufferlist object
BufferList bufferlist;

// convenient to have it here.
boost::scoped_ptr<kb_keymap> toplevel_keymap;

namespace {

void showFileError(string const & error)
{
	Alert::warning(_("Could not read configuration file"),
		   bformat(_("Error while reading the configuration file\n%1$s.\n"
		     "Please check your installation."), error));
	exit(EXIT_FAILURE);
}

}

LyX::LyX(int & argc, char * argv[])
{
	// Here we need to parse the command line. At least
	// we need to parse for "-dbg" and "-help"
	bool const want_gui = easyParse(argc, argv);

	// set the DisplayTranslator only once; should that be done here??
	// if this should not be in this file, please also remove
	// #include "graphics/GraphicsTypes.h" at the top -- Rob Lahaye.
	lyx::graphics::setDisplayTranslator();

	if (want_gui) {
		lyx_gui::parse_init(argc, argv);
	}

	// check for any spurious extra arguments
	// other than documents
	for (int argi = 1; argi < argc ; ++argi) {
		if (argv[argi][0] == '-') {
			lyxerr << bformat(_("Wrong command line option `%1$s'. Exiting."),
				argv[argi]) << endl;
			exit(1);
		}
	}

	// Initialization of LyX (reads lyxrc and more)
	lyxerr[Debug::INIT] << "Initializing LyX::init..." << endl;
	init(want_gui);
	lyxerr[Debug::INIT] << "Initializing LyX::init...done" << endl;

	if (want_gui) {
		lyx_gui::parse_lyxrc();
	}

	vector<string> files;

	for (int argi = argc - 1; argi >= 1; --argi) {
		files.push_back(argv[argi]);
	}

	if (first_start)
		files.push_back(i18nLibFileSearch("examples", "splash.lyx"));

	// Execute batch commands if available
	if (!batch_command.empty()) {

		lyxerr[Debug::INIT] << "About to handle -x '"
		       << batch_command << '\'' << endl;

		Buffer * last_loaded = 0;

		vector<string>::const_iterator it = files.begin();
		vector<string>::const_iterator end = files.end();

		for (; it != end; ++it) {
			// get absolute path of file and add ".lyx" to
			// the filename if necessary
			string s = FileSearch(string(), *it, "lyx");
			if (s.empty()) {
				last_loaded = newFile(*it, "");
			} else {
				last_loaded = bufferlist.newBuffer(s, false);
				last_loaded->error.connect(boost::bind(&LyX::printError, this, _1));
				if (!loadLyXFile(last_loaded, s)) {
					bufferlist.release(last_loaded);
					last_loaded = newFile(*it, string());
				}
			}
		}

		// try to dispatch to last loaded buffer first
		if (last_loaded) {
			bool success = false;
			if (last_loaded->dispatch(batch_command, &success)) {
				QuitLyX();
				exit(!success);
			}
		} 
		files.clear(); // the files are already loaded
	}

	lyx_gui::start(batch_command, files);
}


extern "C" {

static void error_handler(int err_sig)
{
	switch (err_sig) {
	case SIGHUP:
		lyxerr << "\nlyx: SIGHUP signal caught" << endl;
		break;
	case SIGINT:
		// no comments
		break;
	case SIGFPE:
		lyxerr << "\nlyx: SIGFPE signal caught" << endl;
		break;
	case SIGSEGV:
		lyxerr << "\nlyx: SIGSEGV signal caught" << endl;
		lyxerr <<
			"Sorry, you have found a bug in LyX. "
			"Please read the bug-reporting instructions "
			"in Help->Introduction and send us a bug report, "
			"if necessary. Thanks !" << endl;
		break;
	case SIGTERM:
		// no comments
		break;
	}

	// Deinstall the signal handlers
	signal(SIGHUP, SIG_DFL);
	signal(SIGINT, SIG_DFL);
	signal(SIGFPE, SIG_DFL);
	signal(SIGSEGV, SIG_DFL);
	signal(SIGTERM, SIG_DFL);

	LyX::emergencyCleanup();

	lyxerr << "Bye." << endl;
	if (err_sig!= SIGHUP &&
	   (!GetEnv("LYXDEBUG").empty() || err_sig == SIGSEGV))
		lyx::support::abort();
	exit(0);
}

}


void LyX::printError(ErrorItem const & ei)
{
	std::cerr << _("LyX: ") << ei.error
		  << ':' << ei.description << std::endl;

}


void LyX::init(bool gui)
{
	signal(SIGHUP, error_handler);
	signal(SIGFPE, error_handler);
	signal(SIGSEGV, error_handler);
	signal(SIGINT, error_handler);
	signal(SIGTERM, error_handler);

	bool const explicit_userdir = setLyxPaths();

	// Check that user LyX directory is ok. We don't do that if
	// running in batch mode.
	if (gui) {
		queryUserLyXDir(explicit_userdir);
	} else {
		first_start = false;
	}

	// Disable gui when easyparse says so
	lyx_gui::use_gui = gui;

	if (lyxrc.template_path.empty()) {
		lyxrc.template_path = AddPath(system_lyxdir, "templates");
	}

	if (lyxrc.lastfiles.empty()) {
		lyxrc.lastfiles = AddName(user_lyxdir, "lastfiles");
	}

	if (lyxrc.roman_font_name.empty())
		lyxrc.roman_font_name = lyx_gui::roman_font_name();
	if (lyxrc.sans_font_name.empty())
		lyxrc.sans_font_name = lyx_gui::sans_font_name();
	if (lyxrc.typewriter_font_name.empty())
		lyxrc.typewriter_font_name = lyx_gui::typewriter_font_name();

	//
	// Read configuration files
	//

	readRcFile("lyxrc.defaults");
	system_lyxrc = lyxrc;
	system_formats = formats;
	system_converters = converters;
	system_lcolor = lcolor;

	string prefsfile = "preferences";
	// back compatibility to lyxs < 1.1.6
	if (LibFileSearch(string(), prefsfile).empty())
		prefsfile = "lyxrc";
	if (!LibFileSearch(string(), prefsfile).empty())
		readRcFile(prefsfile);

	readEncodingsFile("encodings");
	readLanguagesFile("languages");

	// Load the layouts
	lyxerr[Debug::INIT] << "Reading layouts..." << endl;
	LyXSetStyle();

	if (gui) {
		// Set up bindings
		toplevel_keymap.reset(new kb_keymap);
		defaultKeyBindings(toplevel_keymap.get());
		toplevel_keymap->read(lyxrc.bind_file);

		// Read menus
		readUIFile(lyxrc.ui_file);
	}

	if (lyxerr.debugging(Debug::LYXRC))
		lyxrc.print();

	os::setTmpDir(CreateLyXTmpDir(lyxrc.tempdir_path));
	if (lyxerr.debugging(Debug::INIT)) {
		lyxerr << "LyX tmp dir: `" << os::getTmpDir() << '\'' << endl;
	}

	lyxerr[Debug::INIT] << "Reading lastfiles `"
			    << lyxrc.lastfiles << "'..." << endl;
	lastfiles.reset(new LastFiles(lyxrc.lastfiles,
				      lyxrc.check_lastfiles,
				      lyxrc.num_lastfiles));
}


void LyX::defaultKeyBindings(kb_keymap  * kbmap)
{
	kbmap->bind("Right", LFUN_RIGHT);
	kbmap->bind("Left", LFUN_LEFT);
	kbmap->bind("Up", LFUN_UP);
	kbmap->bind("Down", LFUN_DOWN);

	kbmap->bind("Tab", LFUN_CELL_FORWARD);
	kbmap->bind("ISO_Left_Tab", LFUN_CELL_FORWARD); // jbl 2001-23-02

	kbmap->bind("Home", LFUN_HOME);
	kbmap->bind("End", LFUN_END);
	kbmap->bind("Prior", LFUN_PRIOR);
	kbmap->bind("Next", LFUN_NEXT);

	kbmap->bind("Return", LFUN_BREAKPARAGRAPH);
	//kbmap->bind("~C-~S-~M-nobreakspace", LFUN_PROTECTEDSPACE);

	kbmap->bind("Delete", LFUN_DELETE);
	kbmap->bind("BackSpace", LFUN_BACKSPACE);

	// sub- and superscript -MV
	kbmap->bind("~S-underscore", LFUN_SUBSCRIPT);
	kbmap->bind("~S-asciicircum", LFUN_SUPERSCRIPT);

	// kbmap->bindings to enable the use of the numeric keypad
	// e.g. Num Lock set
	//kbmap->bind("KP_0", LFUN_SELFINSERT);
	//kbmap->bind("KP_Decimal", LFUN_SELFINSERT);
	kbmap->bind("KP_Enter", LFUN_BREAKPARAGRAPH);
	//kbmap->bind("KP_1", LFUN_SELFINSERT);
	//kbmap->bind("KP_2", LFUN_SELFINSERT);
	//kbmap->bind("KP_3", LFUN_SELFINSERT);
	//kbmap->bind("KP_4", LFUN_SELFINSERT);
	//kbmap->bind("KP_5", LFUN_SELFINSERT);
	//kbmap->bind("KP_6", LFUN_SELFINSERT);
	//kbmap->bind("KP_Add", LFUN_SELFINSERT);
	//kbmap->bind("KP_7", LFUN_SELFINSERT);
	//kbmap->bind("KP_8", LFUN_SELFINSERT);
	//kbmap->bind("KP_9", LFUN_SELFINSERT);
	//kbmap->bind("KP_Divide", LFUN_SELFINSERT);
	//kbmap->bind("KP_Multiply", LFUN_SELFINSERT);
	//kbmap->bind("KP_Subtract", LFUN_SELFINSERT);
	kbmap->bind("KP_Right", LFUN_RIGHT);
	kbmap->bind("KP_Left", LFUN_LEFT);
	kbmap->bind("KP_Up", LFUN_UP);
	kbmap->bind("KP_Down", LFUN_DOWN);
	kbmap->bind("KP_Home", LFUN_HOME);
	kbmap->bind("KP_End", LFUN_END);
	kbmap->bind("KP_Prior", LFUN_PRIOR);
	kbmap->bind("KP_Next", LFUN_NEXT);

	kbmap->bind("C-Tab", LFUN_CELL_SPLIT);  // ale970515
	kbmap->bind("S-Tab", LFUN_CELL_BACKWARD);  // jug20000522
	kbmap->bind("S-ISO_Left_Tab", LFUN_CELL_BACKWARD); // jbl 2001-23-02
}


void LyX::emergencyCleanup()
{
	// what to do about tmpfiles is non-obvious. we would
	// like to delete any we find, but our lyxdir might
	// contain documents etc. which might be helpful on
	// a crash

	bufferlist.emergencyWriteAll();
	if (lyxserver)
		lyxserver->emergencyCleanup();
}


void LyX::deadKeyBindings(kb_keymap * kbmap)
{
	// bindKeyings for transparent handling of deadkeys
	// The keysyms are gotten from XFree86 X11R6
	kbmap->bind("~C-~S-~M-dead_acute", LFUN_ACUTE);
	kbmap->bind("~C-~S-~M-dead_breve", LFUN_BREVE);
	kbmap->bind("~C-~S-~M-dead_caron", LFUN_CARON);
	kbmap->bind("~C-~S-~M-dead_cedilla", LFUN_CEDILLA);
	kbmap->bind("~C-~S-~M-dead_abovering", LFUN_CIRCLE);
	kbmap->bind("~C-~S-~M-dead_circumflex", LFUN_CIRCUMFLEX);
	kbmap->bind("~C-~S-~M-dead_abovedot", LFUN_DOT);
	kbmap->bind("~C-~S-~M-dead_grave", LFUN_GRAVE);
	kbmap->bind("~C-~S-~M-dead_doubleacute", LFUN_HUNG_UMLAUT);
	kbmap->bind("~C-~S-~M-dead_macron", LFUN_MACRON);
	// nothing with this name
	// kbmap->bind("~C-~S-~M-dead_special_caron", LFUN_SPECIAL_CARON);
	kbmap->bind("~C-~S-~M-dead_tilde", LFUN_TILDE);
	kbmap->bind("~C-~S-~M-dead_diaeresis", LFUN_UMLAUT);
	// nothing with this name either...
	//kbmap->bind("~C-~S-~M-dead_underbar", LFUN_UNDERBAR);
	kbmap->bind("~C-~S-~M-dead_belowdot", LFUN_UNDERDOT);
	kbmap->bind("~C-~S-~M-dead_tie", LFUN_TIE);
	kbmap->bind("~C-~S-~M-dead_ogonek", LFUN_OGONEK);
}


void LyX::queryUserLyXDir(bool explicit_userdir)
{
	string const configure_script = AddName(system_lyxdir, "configure");

	// Does user directory exist?
	FileInfo fileInfo(user_lyxdir);
	if (fileInfo.isOK() && fileInfo.isDir()) {
		first_start = false;
		FileInfo script(configure_script);
		FileInfo defaults(AddName(user_lyxdir, "lyxrc.defaults"));
		if (defaults.isOK() && script.isOK()
		    && defaults.getModificationTime() < script.getModificationTime()) {
			lyxerr << _("LyX: reconfiguring user directory")
			       << endl;
			Path p(user_lyxdir);
			::system(configure_script.c_str());
			lyxerr << "LyX: " << _("Done!") << endl;
		}
		return;
	}

	first_start = !explicit_userdir;

	lyxerr << bformat(_("LyX: Creating directory %1$s"
				  " and running configure..."), user_lyxdir) << endl;

	if (!createDirectory(user_lyxdir, 0755)) {
		// Failed, let's use $HOME instead.
		user_lyxdir = GetEnvPath("HOME");
		lyxerr << bformat(_("Failed. Will use %1$s instead."),
			user_lyxdir) << endl;
		return;
	}

	// Run configure in user lyx directory
	Path p(user_lyxdir);
	::system(configure_script.c_str());
	lyxerr << "LyX: " << _("Done!") << endl;
}


void LyX::readRcFile(string const & name)
{
	lyxerr[Debug::INIT] << "About to read " << name << "..." << endl;

	string const lyxrc_path = LibFileSearch(string(), name);
	if (!lyxrc_path.empty()) {

		lyxerr[Debug::INIT] << "Found " << name
				    << " in " << lyxrc_path << endl;

		if (lyxrc.read(lyxrc_path) >= 0)
			return;
	}

	showFileError(name);
}


// Read the ui file `name'
void LyX::readUIFile(string const & name)
{
	enum Uitags {
		ui_menuset = 1,
		ui_toolbar,
		ui_toolbars,
		ui_include,
		ui_last
	};

	struct keyword_item uitags[ui_last - 1] = {
		{ "include", ui_include },
		{ "menuset", ui_menuset },
		{ "toolbar", ui_toolbar },
		{ "toolbars", ui_toolbars }
	};

	// Ensure that a file is read only once (prevents include loops)
	static std::list<string> uifiles;
	std::list<string>::const_iterator it  = uifiles.begin();
	std::list<string>::const_iterator end = uifiles.end();
	it = std::find(it, end, name);
	if (it != end) {
		lyxerr[Debug::INIT] << "UI file '" << name
				    << "' has been read already. "
				    << "Is this an include loop?"
				    << endl;
		return;
	}

	lyxerr[Debug::INIT] << "About to read " << name << "..." << endl;

	string const ui_path = LibFileSearch("ui", name, "ui");

	if (ui_path.empty()) {
		lyxerr[Debug::INIT] << "Could not find " << name << endl;
		showFileError(name);
		return;
	}
	uifiles.push_back(name);

	lyxerr[Debug::INIT] << "Found " << name
			    << " in " << ui_path << endl;
	LyXLex lex(uitags, ui_last - 1);
	lex.setFile(ui_path);
	if (!lex.isOK()) {
		lyxerr << "Unable to set LyXLeX for ui file: " << ui_path
		       << endl;
	}

	if (lyxerr.debugging(Debug::PARSER))
		lex.printTable(lyxerr);

	while (lex.isOK()) {
		switch (lex.lex()) {
		case ui_include: {
			lex.next(true);
			string const file = lex.getString();
			readUIFile(file);
			break;
		}
		case ui_menuset:
			menubackend.read(lex);
			break;

		case ui_toolbar:
			toolbarbackend.read(lex);
			break;

		case ui_toolbars:
			toolbarbackend.readToolbars(lex);
			break;

		default:
			if (!rtrim(lex.getString()).empty())
				lex.printError("LyX::ReadUIFile: "
					       "Unknown menu tag: `$$Token'");
			break;
		}
	}
}


// Read the languages file `name'
void LyX::readLanguagesFile(string const & name)
{
	lyxerr[Debug::INIT] << "About to read " << name << "..." << endl;

	string const lang_path = LibFileSearch(string(), name);
	if (lang_path.empty()) {
		showFileError(name);
		return;
	}
	languages.read(lang_path);
}


// Read the encodings file `name'
void LyX::readEncodingsFile(string const & name)
{
	lyxerr[Debug::INIT] << "About to read " << name << "..." << endl;

	string const enc_path = LibFileSearch(string(), name);
	if (enc_path.empty()) {
		showFileError(name);
		return;
	}
	encodings.read(enc_path);
}


namespace {

bool is_gui = true;
string batch;

/// return the the number of arguments consumed
typedef boost::function<int(string const &, string const &)> cmd_helper;

int parse_dbg(string const & arg, string const &)
{
	if (arg.empty()) {
		lyxerr << _("List of supported debug flags:") << endl;
		Debug::showTags(lyxerr);
		exit(0);
	}
	lyxerr << bformat(_("Setting debug level to %1$s"), arg) << endl;

	lyxerr.level(Debug::value(arg));
	Debug::showLevel(lyxerr, lyxerr.level());
	return 1;
}


int parse_help(string const &, string const &)
{
	lyxerr <<
		_("Usage: lyx [ command line switches ] [ name.lyx ... ]\n"
		  "Command line switches (case sensitive):\n"
		  "\t-help              summarize LyX usage\n"
		  "\t-userdir dir       try to set user directory to dir\n"
		  "\t-sysdir dir        try to set system directory to dir\n"
		  "\t-geometry WxH+X+Y  set geometry of the main window\n"
		  "\t-dbg feature[,feature]...\n"
		  "                  select the features to debug.\n"
		  "                  Type `lyx -dbg' to see the list of features\n"
		  "\t-x [--execute] command\n"
		  "                  where command is a lyx command.\n"
		  "\t-e [--export] fmt\n"
		  "                  where fmt is the export format of choice.\n"
		  "\t-i [--import] fmt file.xxx\n"
		  "                  where fmt is the import format of choice\n"
		  "                  and file.xxx is the file to be imported.\n"
		  "\t-version        summarize version and build info\n"
		  "Check the LyX man page for more details.") << endl;
	exit(0);
	return 0;
}

int parse_version(string const &, string const &)
{
	lyxerr << "LyX " << lyx_version
	       << " of " << lyx_release_date << endl;
	lyxerr << "Built on " << __DATE__ << ", " << __TIME__ << endl;

	lyxerr << lyx_version_info << endl;
	exit(0);
	return 0;
}

int parse_sysdir(string const & arg, string const &)
{
	if (arg.empty()) {
		lyxerr << _("Missing directory for -sysdir switch") << endl;
		exit(1);
	}
	system_lyxdir = arg;
	return 1;
}

int parse_userdir(string const & arg, string const &)
{
	if (arg.empty()) {
		lyxerr << _("Missing directory for -userdir switch") << endl;
		exit(1);
	}
	user_lyxdir = arg;
	return 1;
}

int parse_execute(string const & arg, string const &)
{
	if (arg.empty()) {
		lyxerr << _("Missing command string after --execute switch") << endl;
		exit(1);
	}
	batch = arg;
	// Argh. Setting gui to false segfaults..
	// FIXME: when ? how ?
	// is_gui = false;
	return 1;
}

int parse_export(string const & type, string const &)
{
	if (type.empty()) {
		lyxerr << _("Missing file type [eg latex, ps...] after "
			"--export switch") << endl;
		exit(1);
	}
	batch = "buffer-export " + type;
	is_gui = false;
	return 1;
}

int parse_import(string const & type, string const & file)
{
	if (type.empty()) {
		lyxerr << _("Missing file type [eg latex, ps...] after "
			"--import switch") << endl;
		exit(1);
	}
	if (file.empty()) {
		lyxerr << _("Missing filename for --import") << endl;
		exit(1);
	}

	batch = "buffer-import " + type + ' ' + file;
	return 2;
}

} // namespace anon


bool LyX::easyParse(int & argc, char * argv[])
{
	std::map<string, cmd_helper> cmdmap;

	cmdmap["-dbg"] = parse_dbg;
	cmdmap["-help"] = parse_help;
	cmdmap["--help"] = parse_help;
	cmdmap["-version"] = parse_version;
	cmdmap["--version"] = parse_version;
	cmdmap["-sysdir"] = parse_sysdir;
	cmdmap["-userdir"] = parse_userdir;
	cmdmap["-x"] = parse_execute;
	cmdmap["--execute"] = parse_execute;
	cmdmap["-e"] = parse_export;
	cmdmap["--export"] = parse_export;
	cmdmap["-i"] = parse_import;
	cmdmap["--import"] = parse_import;

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

	batch_command = batch;

	return is_gui;
}

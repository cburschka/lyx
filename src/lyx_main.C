/**
 * \file lyx_main.C
 * Copyright 1995-2002 the LyX Team
 * Read the file COPYING
 *
 * \author unknown
 */

#include <config.h>
#include <version.h>

#include "lyx_main.h"

#include "buffer.h"
#include "bufferlist.h"
#include "commandtags.h"
#include "converter.h"
#include "debug.h"
#include "encoding.h"
#include "gettext.h"
#include "kbmap.h"
#include "language.h"
#include "lastfiles.h"
#include "lyxlex.h"
#include "lyxrc.h"
#include "lyxserver.h"
#include "lyxtextclasslist.h"
#include "MenuBackend.h"
#include "ToolbarDefaults.h"

#include "frontends/Alert.h"
#include "frontends/lyx_gui.h"

#include "support/FileInfo.h"
#include "support/filetools.h"
#include "support/package.h"
#include "support/path.h"

#include "BoostFormat.h"
#include <boost/function.hpp>

#include <csignal>
#include <cstdlib>

using lyx::FileInfo;

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

DebugStream lyxerr;

boost::scoped_ptr<LastFiles> lastfiles;

// This is the global bufferlist object
BufferList bufferlist;

// convenient to have it here.
boost::scoped_ptr<kb_keymap> toplevel_keymap;


namespace {

// Filled with the command line arguments "foo" of "-sysdir foo" or
// "-userdir foo".
string cl_system_support;
string cl_user_support;


void reconfigureUserLyXDir()
{
	string const configure_script =
		AddName(lyx::package().system_support(), "configure");
	string const configure_command =
		"sh " + QuoteName(configure_script);

	lyxerr << _("LyX: reconfiguring user directory") << endl;
	Path p(lyx::package().user_support());
	::system(configure_command.c_str());
	lyxerr << "LyX: " << _("Done!") << endl;
}

} // namespace anon


LyX::LyX(int & argc, char * argv[])
{
	// Here we need to parse the command line. At least
	// we need to parse for "-dbg" and "-help"
	bool const want_gui = easyParse(argc, argv);

	lyx::init_package(argv[0], cl_system_support, cl_user_support);

	// Set the locale_dir.
	string locale_dir = lyx::package().locale_dir();
	FileInfo fi(locale_dir);
	if (fi.isOK() && fi.isDir()) {
		lyxerr[Debug::INIT]
			<< "Setting locale directory to "
			<< locale_dir << endl;
		gettext_init(locale_dir);
	}

	// Global bindings (this must be done as early as possible.) (Lgb)
	toplevel_keymap.reset(new kb_keymap);
	defaultKeyBindings(toplevel_keymap.get());

	// set the DisplayTranslator only once; should that be done here??
	// if this should not be in this file, please also remove
	// #include "graphics/GraphicsTypes.h" at the top -- Rob Lahaye.
	grfx::setDisplayTranslator();

	if (want_gui) {
		lyx_gui::parse_init(argc, argv);
	}

	// check for any spurious extra arguments
	// other than documents
	for (int argi = 1; argi < argc ; ++argi) {
		if (argv[argi][0] == '-') {
#if USE_BOOST_FORMAT
			lyxerr << boost::format(_("Wrong command line option `%1$s'. Exiting."))
				% argv[argi]
			       << endl;
#else
			lyxerr << _("Wrong command line option `")
			       << argv[argi] << _("'. Exiting.")
			       << endl;
#endif
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

	if (first_start) {
		files.push_back(i18nLibFileSearch("examples", "splash.lyx"));
	}

	// Execute batch commands if available
	if (!batch_command.empty()) {
		lyxerr[Debug::INIT] << "About to handle -x '"
		       << batch_command << '\'' << endl;

		Buffer * last_loaded = 0;

		vector<string>::iterator it = files.begin();
		vector<string>::iterator end = files.end();
		for (; it != end; ++it) {
			last_loaded = bufferlist.loadLyXFile(*it);
		}

		files.clear();

		// no buffer loaded, create one
		if (!last_loaded)
			last_loaded = bufferlist.newFile("tmpfile", string());

		bool success = false;

		// try to dispatch to last loaded buffer first
		bool const dispatched = last_loaded->dispatch(batch_command, &success);

		if (dispatched) {
			QuitLyX();
			exit(!success);
		}
	}

	lyx_gui::start(batch_command, files);
}


/*
Signals and Windows
===================
The SIGHUP signal does not exist on Windows and does not need to be handled.

Windows handles SIGFPE and SIGSEGV signals as expected.

Cntl+C interrupts (mapped to SIGINT by Windows' POSIX compatability layer)
cause a new thread to be spawned. This may well result in unexpected
behaviour by the single-threaded LyX.

SIGTERM signals will come only from another process actually sending
that signal using 'raise' in Windows' POSIX compatability layer. It will
not come from the general "terminate process" methods that everyone
actually uses (and which can't be trapped). Killing an app 'politely' on
Windows involves first sending a WM_CLOSE message, something that is
caught already by the Qt frontend.

For more information see:

http://aspn.activestate.com/ASPN/Mail/Message/ActiveTcl/2034055
...signals are mostly useless on Windows for a variety of reasons that are
Windows specific...

'UNIX Application Migration Guide, Chapter 9'
http://msdn.microsoft.com/library/en-us/dnucmg/html/UCMGch09.asp

'How To Terminate an Application "Cleanly" in Win32'
http://support.microsoft.com/default.aspx?scid=kb;en-us;178893
*/
extern "C" {

static void error_handler(int err_sig)
{
	switch (err_sig) {
#ifdef SIGHUP
	case SIGHUP:
		lyxerr << "\nlyx: SIGHUP signal caught" << endl;
		break;
#endif
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
#ifdef SIGHUP
	signal(SIGHUP, SIG_DFL);
#endif
	signal(SIGINT, SIG_DFL);
	signal(SIGFPE, SIG_DFL);
	signal(SIGSEGV, SIG_DFL);
	signal(SIGTERM, SIG_DFL);

	LyX::emergencyCleanup();

	lyxerr << "Bye." << endl;
#ifdef SIGHUP
	if (err_sig!= SIGHUP &&
	   (!GetEnv("LYXDEBUG").empty() || err_sig == SIGSEGV))
#else
	if (err_sig == SIGSEGV || !GetEnv("LYXDEBUG").empty())
#endif
		lyx::abort();
	exit(0);
}

}


void LyX::init(bool gui)
{
#ifdef SIGHUP
	signal(SIGHUP, error_handler);
#endif
	signal(SIGFPE, error_handler);
	signal(SIGSEGV, error_handler);
	signal(SIGINT, error_handler);
	signal(SIGTERM, error_handler);

	lyx::Package const & package = lyx::package();

#if !defined (USE_POSIX_PACKAGING)
	// Add the directory containing the LyX executable to the path
	// so that LyX can find things like reLyX.
	if (package.build_support().empty())
		prependEnvPath("PATH", package.binary_dir());
#endif

	// Check that user LyX directory is ok. We don't do that if
	// running in batch mode.
	bool reconfigure = false;
	if (gui) {
		reconfigure = queryUserLyXDir(package.explicit_user_support());
	} else {
		first_start = false;
	}

	//
	// Shine up lyxrc defaults

	lyxrc.tempdir_path = package.temp_dir();
	lyxrc.document_path = package.document_dir();

	// Default template path: system_dir/templates
	if (lyxrc.template_path.empty()) {
		lyxrc.template_path =
			AddPath(package.system_support(), "templates");
	}

	// Default lastfiles file: $HOME/.lyx/lastfiles
	if (lyxrc.lastfiles.empty()) {
		lyxrc.lastfiles = AddName(package.user_support(),
					  "lastfiles");
	}

	// Disable gui when either lyxrc or easyparse says so
	if (!gui)
		lyxrc.use_gui = false;

	//
	// Read configuration files
	//

	readRcFile("lyxrc.defaults");
	system_lyxrc = lyxrc;
	system_formats = formats;
	system_converters = converters;
	system_lcolor = lcolor;

	// If there is a preferences file we read that instead
	// of the old lyxrc file.
	if (!readRcFile("preferences"))
	    readRcFile("lyxrc");

	readEncodingsFile("encodings");
	readLanguagesFile("languages");

	// Load the layouts
	lyxerr[Debug::INIT] << "Reading layouts..." << endl;
	LyXSetStyle();

	// Ensure that we have really read a bind file, so that LyX is
	// usable.
	lyxrc.readBindFileIfNeeded();

	// Read menus
	readUIFile(lyxrc.ui_file);

	if (lyxerr.debugging(Debug::LYXRC)) {
		lyxrc.print();
	}

	prependEnvPath("PATH", lyxrc.path_prefix);

	// Having reset the PATH we're now in a position to run configure
	// if necessary.
	if (reconfigure)
		reconfigureUserLyXDir();

	FileInfo fi(lyxrc.document_path);
	if (fi.isOK() && fi.isDir())
		package.document_dir() = lyxrc.document_path;

	package.temp_dir() = CreateLyXTmpDir(lyxrc.tempdir_path);
	if (package.temp_dir().empty()) {
#if USE_BOOST_FORMAT
		boost::format const fmt =
			boost::format(_("Could not create a temporary directory in\n"
					"%1$s. Make sure that this\n"
					"path exists and is writable and try again."))
			% lyxrc.tempdir_path;
		string const message = fmt.str();
#else
		string const message =
			_("Could not create a temporary directory in\n") +
			lyxrc.tempdir_path +
			_(". Make sure that this\n"
			  "path exists and is writable and try again.");
#endif

		Alert::alert(_("Could not create temporary directory"), message);
		// createLyXTmpDir() tries sufficiently hard to create a
		// usable temp dir, so the probability to come here is
		// close to zero. We therefore don't try to overcome this
		// problem with e.g. asking the user for a new path and
		// trying again but simply exit.
		exit(EXIT_FAILURE);
	}

	if (lyxerr.debugging(Debug::INIT)) {
		lyxerr << "LyX tmp dir: `" << package.temp_dir() << '\''
		       << endl;
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

	kbmap->bind("Tab", LFUN_TAB);
	kbmap->bind("ISO_Left_Tab", LFUN_TAB); // jbl 2001-23-02

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

	kbmap->bind("C-Tab", LFUN_TABINSERT);  // ale970515
	kbmap->bind("S-Tab", LFUN_SHIFT_TAB);  // jug20000522
	kbmap->bind("S-ISO_Left_Tab", LFUN_SHIFT_TAB); // jbl 2001-23-02
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


bool LyX::queryUserLyXDir(bool explicit_userdir)
{
	bool reconfigure = false;
	lyx::Package const & package = lyx::package();

	// Does user directory exist?
	string const & user_support = package.user_support();
	FileInfo fileInfo(user_support);
	if (fileInfo.isOK() && fileInfo.isDir()) {
		first_start = false;
		string const configure_script =
			AddName(package.system_support(), "configure");
		FileInfo script(configure_script);
		FileInfo defaults(AddName(user_support, "lyxrc.defaults"));
		if (defaults.isOK() && script.isOK()
		    && defaults.getModificationTime() < script.getModificationTime()) {
			reconfigure = true;
		}
		return reconfigure;
	}

	first_start = !explicit_userdir;

	// If the user specified explicitly a directory, ask whether
	// to create it. If the user says "no", then exit.
	if (explicit_userdir &&
	    !Alert::askQuestion(
		    _("You have specified a non-existent user LyX directory."),
		    _("It is needed to keep your own configuration."),
		    _("Should I try to set it up for you? I'll exit if \"No\"."))) {
		lyxerr << _("No user LyX directory. Exiting.") << endl;
		exit(1);
	}

#if USE_BOOST_FORMAT
	lyxerr << boost::format(_("LyX: Creating directory %1$s"))
				% package.user_support()
	       << endl;
#else
	lyxerr << _("LyX: Creating directory ") << package.user_support()
	       << endl;
#endif
	reconfigure = true;

	if (!createDirectory(package.user_support(), 0755)) {
		// Failed, so let's exit.
		lyxerr << _("Failed to create directory. Exiting.")
		       << endl;
		exit(1);
	}

	return reconfigure;
}


bool LyX::readRcFile(string const & name)
{
	lyxerr[Debug::INIT] << "About to read " << name << "..." << endl;

	string const lyxrc_path = LibFileSearch(string(), name);
	if (!lyxrc_path.empty()) {
		lyxerr[Debug::INIT] << "Found " << name
				    << " in " << lyxrc_path << endl;
		if (lyxrc.read(lyxrc_path) < 0) {
#if USE_BOOST_FORMAT
			Alert::alert(_("LyX Warning!"),
				   boost::io::str(boost::format(_("Error while reading %1$s.")) % lyxrc_path),
				   _("Using built-in defaults."));
#else
			Alert::alert(_("LyX Warning!"),
				   _("Error while reading ") + lyxrc_path,
				   _("Using built-in defaults."));
#endif
			return false;
		}
		return true;
	} else {
		lyxerr[Debug::INIT] << "Could not find " << name << endl;
	}

	return false;
}


// Read the ui file `name'
void LyX::readUIFile(string const & name)
{
	enum Uitags {
		ui_menuset = 1,
		ui_toolbar,
		ui_last
	};

	struct keyword_item uitags[ui_last - 1] = {
		{ "menuset", ui_menuset },
		{ "toolbar", ui_toolbar }
	};

	lyxerr[Debug::INIT] << "About to read " << name << "..." << endl;

	string const ui_path = LibFileSearch("ui", name, "ui");

	if (ui_path.empty()) {
		lyxerr[Debug::INIT] << "Could not find " << name << endl;
		menubackend.defaults();
		return;
	}

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
		case ui_menuset:
			menubackend.read(lex);
			break;

		case ui_toolbar:
			toolbardefaults.read(lex);
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
		lyxerr[Debug::INIT] << "Could not find " << name << endl;
		languages.setDefaults();
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
		lyxerr[Debug::INIT] << "Could not find " << name << endl;
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
#if USE_BOOST_FORMAT
	lyxerr << boost::format(_("Setting debug level to %1$s"))
		% arg
	       << endl;
#else
	lyxerr << _("Setting debug level to ") << arg << endl;
#endif

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
	cl_system_support = arg;
	return 1;
}

int parse_userdir(string const & arg, string const &)
{
	if (arg.empty()) {
		lyxerr << _("Missing directory for -userdir switch") << endl;
		exit(1);
	}
	cl_user_support = arg;
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

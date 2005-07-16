/**
 * \file lyx_main.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alfredo Braunstein
 * \author Lars Gullik Bjønnes
 * \author Jean-Marc Lasgouttes
 * \author John Levon
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>
#include <version.h>

#include "lyx_main.h"

#include "buffer.h"
#include "buffer_funcs.h"
#include "bufferlist.h"
#include "converter.h"
#include "debug.h"
#include "encoding.h"
#include "errorlist.h"
#include "format.h"
#include "gettext.h"
#include "kbmap.h"
#include "language.h"
#include "lastfiles.h"
#include "LColor.h"
#include "lyxfunc.h"
#include "lyxlex.h"
#include "lyxrc.h"
#include "lyxtextclasslist.h"
#include "lyxserver.h"
#include "MenuBackend.h"
#include "mover.h"
#include "ToolbarBackend.h"

#include "mathed/math_inset.h"

#include "frontends/Alert.h"
#include "frontends/lyx_gui.h"
#include "frontends/LyXView.h"

#include "support/environment.h"
#include "support/filetools.h"
#include "support/lyxlib.h"
#include "support/os.h"
#include "support/package.h"
#include "support/path.h"

#include <boost/bind.hpp>
#include <boost/filesystem/operations.hpp>

#include <iostream>
#include <csignal>

using lyx::support::AddName;
using lyx::support::AddPath;
using lyx::support::bformat;
using lyx::support::createDirectory;
using lyx::support::createLyXTmpDir;
using lyx::support::FileSearch;
using lyx::support::getEnv;
using lyx::support::i18nLibFileSearch;
using lyx::support::LibFileSearch;
using lyx::support::package;
using lyx::support::Path;
using lyx::support::prependEnvPath;
using lyx::support::QuoteName;
using lyx::support::rtrim;

namespace os = lyx::support::os;
namespace fs = boost::filesystem;

using std::endl;
using std::string;
using std::vector;

#ifndef CXX_GLOBAL_CSTD
using std::exit;
using std::signal;
using std::system;
#endif


extern void QuitLyX(bool);

extern LyXServer * lyxserver;

// This is the global bufferlist object
BufferList bufferlist;

// convenient to have it here.
boost::scoped_ptr<kb_keymap> toplevel_keymap;

namespace {

// Filled with the command line arguments "foo" of "-sysdir foo" or
// "-userdir foo".
string cl_system_support;
string cl_user_support;


void showFileError(string const & error)
{
	Alert::warning(_("Could not read configuration file"),
		   bformat(_("Error while reading the configuration file\n%1$s.\n"
		     "Please check your installation."), error));
	exit(EXIT_FAILURE);
}


void reconfigureUserLyXDir()
{
	string const configure_script =
		AddName(package().system_support(), "configure");
	string const configure_command =
		"sh " + QuoteName(configure_script);

	lyxerr << _("LyX: reconfiguring user directory") << endl;
	Path p(package().user_support());
	::system(configure_command.c_str());
	lyxerr << "LyX: " << _("Done!") << endl;
}

} // namespace anon


boost::scoped_ptr<LyX> LyX::singleton_;

void LyX::exec(int & argc, char * argv[])
{
	BOOST_ASSERT(!singleton_.get());
	// We must return from this before launching the gui so that
	// other parts of the code can access singleton_ through
	// LyX::ref and LyX::cref.
	singleton_.reset(new LyX);
	// Start the real execution loop.
	singleton_->priv_exec(argc, argv);
}


LyX & LyX::ref()
{
	BOOST_ASSERT(singleton_.get());
	return *singleton_.get();
}


LyX const & LyX::cref()
{
	BOOST_ASSERT(singleton_.get());
	return *singleton_.get();
}


LyX::LyX()
	: first_start(false)
{}


LastFiles & LyX::lastfiles()
{
	BOOST_ASSERT(lastfiles_.get());
	return *lastfiles_.get();
}


LastFiles const & LyX::lastfiles() const
{
	BOOST_ASSERT(lastfiles_.get());
	return *lastfiles_.get();
}


void LyX::addLyXView(boost::shared_ptr<LyXView> const & lyxview)
{
	views_.push_back(lyxview);
}


Buffer const * const LyX::updateInset(InsetBase const * inset) const
{
	if (!inset)
		return 0;

	Buffer const * buffer_ptr = 0;
	ViewList::const_iterator it = views_.begin();
	ViewList::const_iterator const end = views_.end();
	for (; it != end; ++it) {
		Buffer const * ptr = (*it)->updateInset(inset);
		if (ptr)
			buffer_ptr = ptr;
	}
	return buffer_ptr;
}


void LyX::priv_exec(int & argc, char * argv[])
{
	// Here we need to parse the command line. At least
	// we need to parse for "-dbg" and "-help"
	bool const want_gui = easyParse(argc, argv);

	lyx::support::init_package(argv[0], cl_system_support, cl_user_support,
				   lyx::support::top_build_dir_is_one_level_up);

	if (want_gui)
		lyx_gui::parse_init(argc, argv);

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

	if (want_gui)
		lyx_gui::parse_lyxrc();

	initMath();

	vector<string> files;

	for (int argi = argc - 1; argi >= 1; --argi)
		files.push_back(os::internal_path(argv[argi]));

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
				last_loaded = newFile(*it, string(), true);
			} else {
				Buffer * buf = bufferlist.newBuffer(s, false);
				buf->error.connect(boost::bind(&LyX::printError, this, _1));
				if (loadLyXFile(buf, s))
					last_loaded = buf;
				else
					bufferlist.release(buf);
			}
		}

		// try to dispatch to last loaded buffer first
		if (last_loaded) {
			bool success = false;
			if (last_loaded->dispatch(batch_command, &success)) {
				QuitLyX(false);
				exit(!success);
			}
		}
		files.clear(); // the files are already loaded
	}

	if (want_gui)
		lyx_gui::start(batch_command, files);
	else {
		// Something went wrong above
		QuitLyX(false);
		exit(EXIT_FAILURE);
	}
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
	// Throw away any signals other than the first one received.
	static sig_atomic_t handling_error = false;
	if (handling_error)
		return;
	handling_error = true;

	// We have received a signal indicating a fatal error, so
	// try and save the data ASAP.
	LyX::cref().emergencyCleanup();

	// These lyxerr calls may or may not work:

	// Signals are asynchronous, so the main program may be in a very
	// fragile state when a signal is processed and thus while a signal
	// handler function executes.
	// In general, therefore, we should avoid performing any
	// I/O operations or calling most library and system functions from
	// signal handlers.

	// This shouldn't matter here, however, as we've already invoked
	// emergencyCleanup.
	switch (err_sig) {
#ifdef SIGHUP
	case SIGHUP:
		lyxerr << "\nlyx: SIGHUP signal caught\nBye." << endl;
		break;
#endif
	case SIGFPE:
		lyxerr << "\nlyx: SIGFPE signal caught\nBye." << endl;
		break;
	case SIGSEGV:
		lyxerr << "\nlyx: SIGSEGV signal caught\n"
		          "Sorry, you have found a bug in LyX. "
		          "Please read the bug-reporting instructions "
		          "in Help->Introduction and send us a bug report, "
		          "if necessary. Thanks !\nBye." << endl;
		break;
	case SIGINT:
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

#ifdef SIGHUP
	if (err_sig == SIGSEGV ||
	    (err_sig != SIGHUP && !getEnv("LYXDEBUG").empty()))
#else
	if (err_sig == SIGSEGV || !getEnv("LYXDEBUG").empty())
#endif
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
#ifdef SIGHUP
	signal(SIGHUP, error_handler);
#endif
	signal(SIGFPE, error_handler);
	signal(SIGSEGV, error_handler);
	signal(SIGINT, error_handler);
	signal(SIGTERM, error_handler);
	// SIGPIPE can be safely ignored.

	// Check that user LyX directory is ok. We don't do that if
	// running in batch mode.
	bool reconfigure = false;
	if (gui) {
		reconfigure =
			queryUserLyXDir(package().explicit_user_support());
	} else {
		first_start = false;
	}

	// Disable gui when easyparse says so
	lyx_gui::use_gui = gui;

	lyxrc.tempdir_path = package().temp_dir();
	lyxrc.document_path = package().document_dir();

	if (lyxrc.template_path.empty()) {
		lyxrc.template_path = AddPath(package().system_support(),
					      "templates");
	}

	if (lyxrc.lastfiles.empty()) {
		lyxrc.lastfiles = AddName(package().user_support(), "lastfiles");
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
	system_movers = movers;
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

	os::cygwin_path_fix(lyxrc.cygwin_path_fix);
	if (!lyxrc.path_prefix.empty())
		prependEnvPath("PATH", lyxrc.path_prefix);

#if !defined (USE_POSIX_PACKAGING)
	// Add the directory containing the LyX executable to the path
	// so that LyX can find things like reLyX.
	if (package().build_support().empty())
		prependEnvPath("PATH", package().binary_dir());
#endif

	// Having reset the PATH we're now in a position to run configure
	// if necessary.
	if (reconfigure)
		reconfigureUserLyXDir();

	if (fs::exists(lyxrc.document_path) &&
	    fs::is_directory(lyxrc.document_path))
		package().document_dir() = lyxrc.document_path;

	package().temp_dir() = createLyXTmpDir(lyxrc.tempdir_path);
	if (package().temp_dir().empty()) {
		Alert::error(_("Could not create temporary directory"),
		             bformat(_("Could not create a temporary directory in\n"
		                       "%1$s. Make sure that this\n"
		                       "path exists and is writable and try again."),
		                     lyxrc.tempdir_path));
		// createLyXTmpDir() tries sufficiently hard to create a
		// usable temp dir, so the probability to come here is
		// close to zero. We therefore don't try to overcome this
		// problem with e.g. asking the user for a new path and
		// trying again but simply exit.
		exit(EXIT_FAILURE);
	}

	if (lyxerr.debugging(Debug::INIT)) {
		lyxerr << "LyX tmp dir: `" << package().temp_dir() << '\'' << endl;
	}

	lyxerr[Debug::INIT] << "Reading lastfiles `"
			    << lyxrc.lastfiles << "'..." << endl;
	lastfiles_.reset(new LastFiles(lyxrc.lastfiles,
				       lyxrc.check_lastfiles,
				       lyxrc.num_lastfiles));
}


void LyX::defaultKeyBindings(kb_keymap  * kbmap)
{
	kbmap->bind("Right", FuncRequest(LFUN_RIGHT));
	kbmap->bind("Left", FuncRequest(LFUN_LEFT));
	kbmap->bind("Up", FuncRequest(LFUN_UP));
	kbmap->bind("Down", FuncRequest(LFUN_DOWN));

	kbmap->bind("Tab", FuncRequest(LFUN_CELL_FORWARD));
	kbmap->bind("C-Tab", FuncRequest(LFUN_CELL_SPLIT));
	kbmap->bind("~S-ISO_Left_Tab", FuncRequest(LFUN_CELL_BACKWARD));
	kbmap->bind("~S-BackTab", FuncRequest(LFUN_CELL_BACKWARD));

	kbmap->bind("Home", FuncRequest(LFUN_HOME));
	kbmap->bind("End", FuncRequest(LFUN_END));
	kbmap->bind("Prior", FuncRequest(LFUN_PRIOR));
	kbmap->bind("Next", FuncRequest(LFUN_NEXT));

	kbmap->bind("Return", FuncRequest(LFUN_BREAKPARAGRAPH));
	//kbmap->bind("~C-~S-~M-nobreakspace", FuncRequest(LFUN_PROTECTEDSPACE));

	kbmap->bind("Delete", FuncRequest(LFUN_DELETE));
	kbmap->bind("BackSpace", FuncRequest(LFUN_BACKSPACE));

	// kbmap->bindings to enable the use of the numeric keypad
	// e.g. Num Lock set
	//kbmap->bind("KP_0", FuncRequest(LFUN_SELFINSERT));
	//kbmap->bind("KP_Decimal", FuncRequest(LFUN_SELFINSERT));
	kbmap->bind("KP_Enter", FuncRequest(LFUN_BREAKPARAGRAPH));
	//kbmap->bind("KP_1", FuncRequest(LFUN_SELFINSERT));
	//kbmap->bind("KP_2", FuncRequest(LFUN_SELFINSERT));
	//kbmap->bind("KP_3", FuncRequest(LFUN_SELFINSERT));
	//kbmap->bind("KP_4", FuncRequest(LFUN_SELFINSERT));
	//kbmap->bind("KP_5", FuncRequest(LFUN_SELFINSERT));
	//kbmap->bind("KP_6", FuncRequest(LFUN_SELFINSERT));
	//kbmap->bind("KP_Add", FuncRequest(LFUN_SELFINSERT));
	//kbmap->bind("KP_7", FuncRequest(LFUN_SELFINSERT));
	//kbmap->bind("KP_8", FuncRequest(LFUN_SELFINSERT));
	//kbmap->bind("KP_9", FuncRequest(LFUN_SELFINSERT));
	//kbmap->bind("KP_Divide", FuncRequest(LFUN_SELFINSERT));
	//kbmap->bind("KP_Multiply", FuncRequest(LFUN_SELFINSERT));
	//kbmap->bind("KP_Subtract", FuncRequest(LFUN_SELFINSERT));
	kbmap->bind("KP_Right", FuncRequest(LFUN_RIGHT));
	kbmap->bind("KP_Left", FuncRequest(LFUN_LEFT));
	kbmap->bind("KP_Up", FuncRequest(LFUN_UP));
	kbmap->bind("KP_Down", FuncRequest(LFUN_DOWN));
	kbmap->bind("KP_Home", FuncRequest(LFUN_HOME));
	kbmap->bind("KP_End", FuncRequest(LFUN_END));
	kbmap->bind("KP_Prior", FuncRequest(LFUN_PRIOR));
	kbmap->bind("KP_Next", FuncRequest(LFUN_NEXT));
}


void LyX::emergencyCleanup() const
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
	kbmap->bind("~C-~S-~M-dead_acute", FuncRequest(LFUN_ACUTE));
	kbmap->bind("~C-~S-~M-dead_breve", FuncRequest(LFUN_BREVE));
	kbmap->bind("~C-~S-~M-dead_caron", FuncRequest(LFUN_CARON));
	kbmap->bind("~C-~S-~M-dead_cedilla", FuncRequest(LFUN_CEDILLA));
	kbmap->bind("~C-~S-~M-dead_abovering", FuncRequest(LFUN_CIRCLE));
	kbmap->bind("~C-~S-~M-dead_circumflex", FuncRequest(LFUN_CIRCUMFLEX));
	kbmap->bind("~C-~S-~M-dead_abovedot", FuncRequest(LFUN_DOT));
	kbmap->bind("~C-~S-~M-dead_grave", FuncRequest(LFUN_GRAVE));
	kbmap->bind("~C-~S-~M-dead_doubleacute", FuncRequest(LFUN_HUNG_UMLAUT));
	kbmap->bind("~C-~S-~M-dead_macron", FuncRequest(LFUN_MACRON));
	// nothing with this name
	// kbmap->bind("~C-~S-~M-dead_special_caron", LFUN_SPECIAL_CARON);
	kbmap->bind("~C-~S-~M-dead_tilde", FuncRequest(LFUN_TILDE));
	kbmap->bind("~C-~S-~M-dead_diaeresis", FuncRequest(LFUN_UMLAUT));
	// nothing with this name either...
	//kbmap->bind("~C-~S-~M-dead_underbar", FuncRequest(LFUN_UNDERBAR));
	kbmap->bind("~C-~S-~M-dead_belowdot", FuncRequest(LFUN_UNDERDOT));
	kbmap->bind("~C-~S-~M-dead_tie", FuncRequest(LFUN_TIE));
	kbmap->bind("~C-~S-~M-dead_ogonek",FuncRequest(LFUN_OGONEK));
}


bool LyX::queryUserLyXDir(bool explicit_userdir)
{
	bool reconfigure = false;

	// Does user directory exist?
	if (fs::exists(package().user_support()) &&
	    fs::is_directory(package().user_support())) {
		first_start = false;
		string const configure_script =
			AddName(package().system_support(), "configure");
		string const userDefaults =
			AddName(package().user_support(), "lyxrc.defaults");
		if (fs::exists(configure_script) &&
		    fs::exists(userDefaults) &&
		    fs::last_write_time(configure_script)
		    > fs::last_write_time(userDefaults)) {
			reconfigure = true;
		}
		return reconfigure;
	}

	first_start = !explicit_userdir;

	// If the user specified explicitly a directory, ask whether
	// to create it. If the user says "no", then exit.
	if (explicit_userdir &&
	    Alert::prompt(
		    _("Missing LyX support directory"),
		    bformat(_("You have specified a non-existent user "
			      "LyX directory, %1$s.\n"
			      "It is needed to keep your own configuration."),
			    package().user_support()),
		    1, 0,
		    _("&Create directory."),
		    _("&Exit LyX."))) {
		lyxerr << _("No user LyX directory. Exiting.") << endl;
		exit(1);
	}

	lyxerr << bformat(_("LyX: Creating directory %1$s"),
			  package().user_support())
	       << endl;
	reconfigure = true;

	if (!createDirectory(package().user_support(), 0755)) {
		// Failed, so let's exit.
		lyxerr << _("Failed to create directory. Exiting.")
		       << endl;
		exit(1);
	}

	return reconfigure;
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

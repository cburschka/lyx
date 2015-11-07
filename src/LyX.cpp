/**
 * \file LyX.cpp
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

#include "LyX.h"

#include "AppleSpellChecker.h"
#include "AspellChecker.h"
#include "Buffer.h"
#include "BufferList.h"
#include "CmdDef.h"
#include "ColorSet.h"
#include "ConverterCache.h"
#include "Converter.h"
#include "CutAndPaste.h"
#include "EnchantChecker.h"
#include "Encoding.h"
#include "ErrorList.h"
#include "Format.h"
#include "FuncStatus.h"
#include "HunspellChecker.h"
#include "KeyMap.h"
#include "Language.h"
#include "LaTeXFonts.h"
#include "LayoutFile.h"
#include "Lexer.h"
#include "LyX.h"
#include "LyXAction.h"
#include "LyXRC.h"
#include "ModuleList.h"
#include "Mover.h"
#include "Server.h"
#include "ServerSocket.h"
#include "Session.h"
#include "WordList.h"

#include "frontends/alert.h"
#include "frontends/Application.h"

#include "support/ConsoleApplication.h"
#include "support/lassert.h"
#include "support/debug.h"
#include "support/environment.h"
#include "support/ExceptionMessage.h"
#include "support/filetools.h"
#include "support/gettext.h"
#include "support/lstrings.h"
#include "support/Messages.h"
#include "support/os.h"
#include "support/Package.h"

#include "support/bind.h"
#include <boost/scoped_ptr.hpp>

#include <algorithm>
#include <iostream>
#include <csignal>
#include <map>
#include <stdlib.h>
#include <string>
#include <vector>

using namespace std;
using namespace lyx::support;

#if defined (USE_MACOSX_PACKAGING)
#include <crt_externs.h>
#endif

namespace lyx {

namespace Alert = frontend::Alert;
namespace os = support::os;



// Are we using the GUI at all?  We default to true and this is changed
// to false when the export feature is used.

bool use_gui = true;


// We default to open documents in an already running instance, provided that
// the lyxpipe has been setup. This can be overridden either on the command
// line or through preference settings.

RunMode run_mode = PREFERRED;


// Tell what files can be silently overwritten during batch export.
// Possible values are: NO_FILES, MAIN_FILE, ALL_FILES, UNSPECIFIED.
// Unless specified on command line (through the -f switch) or through the
// environment variable LYX_FORCE_OVERWRITE, the default will be MAIN_FILE.

OverwriteFiles force_overwrite = UNSPECIFIED;


namespace {

// Filled with the command line arguments "foo" of "-sysdir foo" or
// "-userdir foo".
string cl_system_support;
string cl_user_support;

string geometryArg;

LyX * singleton_ = 0;

void showFileError(string const & error)
{
	Alert::warning(_("Could not read configuration file"),
		       bformat(_("Error while reading the configuration file\n%1$s.\n"
			   "Please check your installation."), from_utf8(error)));
}

} // namespace anon

/// The main application class private implementation.
struct LyX::Impl {
	Impl()
		: latexfonts_(0), spell_checker_(0), apple_spell_checker_(0), aspell_checker_(0), enchant_checker_(0), hunspell_checker_(0)
	{}

	~Impl()
	{
		delete latexfonts_;
		delete apple_spell_checker_;
		delete aspell_checker_;
		delete enchant_checker_;
		delete hunspell_checker_;
	}

	///
	BufferList buffer_list_;
	///
	KeyMap toplevel_keymap_;
	///
	CmdDef toplevel_cmddef_;
	///
	boost::scoped_ptr<Server> lyx_server_;
	///
	boost::scoped_ptr<ServerSocket> lyx_socket_;
	///
	boost::scoped_ptr<frontend::Application> application_;
	/// lyx session, containing lastfiles, lastfilepos, and lastopened
	boost::scoped_ptr<Session> session_;

	/// Files to load at start.
	vector<string> files_to_load_;

	/// The messages translators.
	map<string, Messages> messages_;

	/// The file converters.
	Converters converters_;

	// The system converters copy after reading lyxrc.defaults.
	Converters system_converters_;

	///
	Movers movers_;
	///
	Movers system_movers_;

	/// the parsed command line batch command if any
	vector<string> batch_commands;

	///
	LaTeXFonts * latexfonts_;

	///
	SpellChecker * spell_checker_;
	///
	SpellChecker * apple_spell_checker_;
	///
	SpellChecker * aspell_checker_;
	///
	SpellChecker * enchant_checker_;
	///
	SpellChecker * hunspell_checker_;
};


/// The main application class for console mode
class LyXConsoleApp : public ConsoleApplication
{
public:
	LyXConsoleApp(LyX * lyx, int & argc, char * argv[])
		: ConsoleApplication(lyx_package, argc, argv), lyx_(lyx),
		  argc_(argc), argv_(argv)
	{
	}
	void doExec()
	{
		int const exit_status = lyx_->execWithoutGui(argc_, argv_);
		exit(exit_status);
	}
private:
	LyX * lyx_;
	int & argc_;
	char ** argv_;
};


///
frontend::Application * theApp()
{
	if (singleton_)
		return singleton_->pimpl_->application_.get();
	else
		return 0;
}


LyX::~LyX()
{
	delete pimpl_;
	singleton_ = 0;
	WordList::cleanupWordLists();
}


void lyx_exit(int exit_code)
{
	if (exit_code)
		// Something wrong happened so better save everything, just in
		// case.
		emergencyCleanup();

#ifndef NDEBUG
	// Properly crash in debug mode in order to get a useful backtrace.
	abort();
#endif

	// In release mode, try to exit gracefully.
	if (theApp())
		theApp()->exit(exit_code);
	else
		exit(exit_code);
}


LyX::LyX()
	: first_start(false)
{
	singleton_ = this;
	pimpl_ = new Impl;
}


Messages & LyX::messages(string const & language)
{
	map<string, Messages>::iterator it = pimpl_->messages_.find(language);

	if (it != pimpl_->messages_.end())
		return it->second;

	pair<map<string, Messages>::iterator, bool> result =
			pimpl_->messages_.insert(make_pair(language, Messages(language)));

	LATTEST(result.second);
	return result.first->second;
}


int LyX::exec(int & argc, char * argv[])
{
	// Minimal setting of locale before parsing command line
	try {
		init_package(os::utf8_argv(0), string(), string());
		// we do not get to this point when init_package throws an exception
		setLocale();
	} catch (ExceptionMessage const & message) {
		LYXERR(Debug::LOCALE, message.title_ + ", " + message.details_);
	}

	// Here we need to parse the command line. At least
	// we need to parse for "-dbg" and "-help"
	easyParse(argc, argv);

	try {
		init_package(os::utf8_argv(0), cl_system_support, cl_user_support);
	} catch (ExceptionMessage const & message) {
		if (message.type_ == ErrorException) {
			Alert::error(message.title_, message.details_);
			lyx_exit(1);
		} else if (message.type_ == WarningException) {
			Alert::warning(message.title_, message.details_);
		}
	}

	// Reinit the messages machinery in case package() knows
	// something interesting about the locale directory.
	setLocale();

	if (!use_gui) {
		LyXConsoleApp app(this, argc, argv);

		// Reestablish our defaults, as Qt overwrites them
		// after creating app
		setLocale();//???

		return app.exec();
	}

	// Let the frontend parse and remove all arguments that it knows
	pimpl_->application_.reset(createApplication(argc, argv));

	// Reestablish our defaults, as Qt overwrites them
	// after createApplication()
	setLocale();//???

	// Parse and remove all known arguments in the LyX singleton
	// Give an error for all remaining ones.
	int exit_status = init(argc, argv);
	if (exit_status) {
		// Kill the application object before exiting.
		pimpl_->application_.reset();
		use_gui = false;
		prepareExit();
		return exit_status;
	}

	// If not otherwise specified by a command line option or
	// by preferences, we default to reuse a running instance.
	if (run_mode == PREFERRED)
		run_mode = USE_REMOTE;

	// FIXME
	/* Create a CoreApplication class that will provide the main event loop
	* and the socket callback registering. With Qt, only QtCore
	* library would be needed.
	* When this is done, a server_mode could be created and the following two
	* line would be moved out from here.
	* However, note that the first of the two lines below triggers the
	* "single instance" behavior, which should occur right at this point.
	*/
	// Note: socket callback must be registered after init(argc, argv)
	// such that package().temp_dir() is properly initialized.
	pimpl_->lyx_server_.reset(new Server(lyxrc.lyxpipes));
	pimpl_->lyx_socket_.reset(new ServerSocket(
			FileName(package().temp_dir().absFileName() + "/lyxsocket")));

	// Start the real execution loop.
	if (!theServer().deferredLoadingToOtherInstance())
		exit_status = pimpl_->application_->exec();
	else if (!pimpl_->files_to_load_.empty()) {
		vector<string>::const_iterator it = pimpl_->files_to_load_.begin();
		vector<string>::const_iterator end = pimpl_->files_to_load_.end();
		lyxerr << _("The following files could not be loaded:") << endl;
		for (; it != end; ++it)
			lyxerr << *it << endl;
	}

	prepareExit();

	return exit_status;
}


void LyX::prepareExit()
{
	// Clear the clipboard and selection stack:
	cap::clearCutStack();
	cap::clearSelection();

	// Write the index file of the converter cache
	ConverterCache::get().writeIndex();

	// close buffers first
	pimpl_->buffer_list_.closeAll();

	// register session changes and shutdown server and socket
	if (use_gui) {
		if (pimpl_->session_)
			pimpl_->session_->writeFile();
		pimpl_->session_.reset();
		pimpl_->lyx_server_.reset();
		pimpl_->lyx_socket_.reset();
	}

	// do any other cleanup procedures now
	if (package().temp_dir() != package().system_temp_dir()) {
		string const abs_tmpdir = package().temp_dir().absFileName();
		if (!contains(package().temp_dir().absFileName(), "lyx_tmpdir")) {
			docstring const msg =
				bformat(_("%1$s does not appear like a LyX created temporary directory."),
				from_utf8(abs_tmpdir));
			Alert::warning(_("Cannot remove temporary directory"), msg);
		} else {
			LYXERR(Debug::INFO, "Deleting tmp dir "
				<< package().temp_dir().absFileName());
			if (!package().temp_dir().destroyDirectory()) {
				LYXERR0(bformat(_("Unable to remove the temporary directory %1$s"),
					from_utf8(package().temp_dir().absFileName())));
			}
		}
	}

	// Kill the application object before exiting. This avoids crashes
	// when exiting on Linux.
	pimpl_->application_.reset();
}


void LyX::earlyExit(int status)
{
	LATTEST(pimpl_->application_.get());
	// LyX::pimpl_::application_ is not initialised at this
	// point so it's safe to just exit after some cleanup.
	prepareExit();
	exit(status);
}


int LyX::init(int & argc, char * argv[])
{
	// check for any spurious extra arguments
	// other than documents
	for (int argi = 1; argi < argc ; ++argi) {
		if (argv[argi][0] == '-') {
			lyxerr << to_utf8(
				bformat(_("Wrong command line option `%1$s'. Exiting."),
				from_utf8(os::utf8_argv(argi)))) << endl;
			return EXIT_FAILURE;
		}
	}

	// Initialization of LyX (reads lyxrc and more)
	LYXERR(Debug::INIT, "Initializing LyX::init...");
	bool success = init();
	LYXERR(Debug::INIT, "Initializing LyX::init...done");
	if (!success)
		return EXIT_FAILURE;

	// Remaining arguments are assumed to be files to load.
	for (int argi = 1; argi < argc; ++argi)
		pimpl_->files_to_load_.push_back(os::utf8_argv(argi));

	if (!use_gui && pimpl_->files_to_load_.empty()) {
		lyxerr << to_utf8(_("Missing filename for this operation.")) << endl;
		return EXIT_FAILURE;
	}

	if (first_start) {
		pimpl_->files_to_load_.push_back(
			i18nLibFileSearch("examples", "splash.lyx").absFileName());
	}

	return EXIT_SUCCESS;
}


int LyX::execWithoutGui(int & argc, char * argv[])
{
	int exit_status = init(argc, argv);
	if (exit_status) {
		prepareExit();
		return exit_status;
	}

	// Used to keep track of which buffers were explicitly loaded by user request.
	// This is necessary because master and child document buffers are loaded, even 
	// if they were not named on the command line. We do not want to dispatch to
	// those.
	vector<Buffer *> command_line_buffers;

	// Load the files specified on the command line
	vector<string>::const_iterator it = pimpl_->files_to_load_.begin();
	vector<string>::const_iterator end = pimpl_->files_to_load_.end();
	for (; it != end; ++it) {
		// get absolute path of file and add ".lyx" to the filename if necessary
		FileName fname = fileSearch(string(), os::internal_path(*it), "lyx",
																may_not_exist);

		if (fname.empty())
			continue;

		Buffer * buf = pimpl_->buffer_list_.newBuffer(fname.absFileName());
		LYXERR(Debug::FILES, "Loading " << fname);
		if (buf->loadLyXFile() == Buffer::ReadSuccess) {
			ErrorList const & el = buf->errorList("Parse");
			if (!el.empty())
					for_each(el.begin(), el.end(),
									 bind(&LyX::printError, this, _1));
			command_line_buffers.push_back(buf);
		} else {
			pimpl_->buffer_list_.release(buf);
			docstring const error_message =
					bformat(_("LyX failed to load the following file: %1$s"),
									from_utf8(fname.absFileName()));
			lyxerr << to_utf8(error_message) << endl;
			exit_status = 1; // failed
		}
	}

	if (exit_status || pimpl_->batch_commands.empty() || pimpl_->buffer_list_.empty()) {
		prepareExit();
		return exit_status;
	}

	// Iterate through the buffers that were specified on the command line
	bool final_success = false;
	vector<Buffer *>::iterator buf_it = command_line_buffers.begin();
	for (; buf_it != command_line_buffers.end(); ++buf_it) {
		Buffer * buf = *buf_it;
		vector<string>::const_iterator bcit  = pimpl_->batch_commands.begin();
		vector<string>::const_iterator bcend = pimpl_->batch_commands.end();
		DispatchResult dr;
		for (; bcit != bcend; ++bcit) {
			LYXERR(Debug::ACTION, "Buffer::dispatch: cmd: " << *bcit);
			buf->dispatch(*bcit, dr);
			final_success |= !dr.error();
		}
	}
	prepareExit();
	return !final_success;
}


void execBatchCommands()
{
	LAPPERR(singleton_);
	singleton_->execCommands();
}


void LyX::execCommands()
{
	// The advantage of doing this here is that the event loop
	// is already started. So any need for interaction will be
	// aknowledged.

	// if reconfiguration is needed.
	if (LayoutFileList::get().empty()) {
		switch (Alert::prompt(
			_("No textclass is found"),
			_("LyX will only have minimal functionality because no textclasses "
				"have been found. You can either try to reconfigure LyX normally, "
				"try to reconfigure without checking your LaTeX installation, or continue."),
			0, 2,
			_("&Reconfigure"),
			_("&Without LaTeX"),
			_("&Continue")))
		{
		case 0:
			// regular reconfigure
			lyx::dispatch(FuncRequest(LFUN_RECONFIGURE, ""));
			break;
		case 1:
			// reconfigure --without-latex-config
			lyx::dispatch(FuncRequest(LFUN_RECONFIGURE,
				" --without-latex-config"));
			break;
		default:
			break;
		}
	}

	// create the first main window
	lyx::dispatch(FuncRequest(LFUN_WINDOW_NEW, geometryArg));

	if (!pimpl_->files_to_load_.empty()) {
		// if some files were specified at command-line we assume that the
		// user wants to edit *these* files and not to restore the session.
		for (size_t i = 0; i != pimpl_->files_to_load_.size(); ++i) {
			lyx::dispatch(
				FuncRequest(LFUN_FILE_OPEN, pimpl_->files_to_load_[i]));
		}
		// clear this list to save a few bytes of RAM
		pimpl_->files_to_load_.clear();
	} else
		pimpl_->application_->restoreGuiSession();

	// Execute batch commands if available
	if (pimpl_->batch_commands.empty())
		return;

	vector<string>::const_iterator bcit  = pimpl_->batch_commands.begin();
	vector<string>::const_iterator bcend = pimpl_->batch_commands.end();
	for (; bcit != bcend; ++bcit) {
		LYXERR(Debug::INIT, "About to handle -x '" << *bcit << '\'');
		lyx::dispatch(lyxaction.lookupFunc(*bcit));
	}
}


/*
Signals and Windows
===================
The SIGHUP signal does not exist on Windows and does not need to be handled.

Windows handles SIGFPE and SIGSEGV signals as expected.

Ctrl+C interrupts (mapped to SIGINT by Windows' POSIX compatability layer)
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
	emergencyCleanup();

	// These lyxerr calls may or may not work:

	// Signals are asynchronous, so the main program may be in a very
	// fragile state when a signal is processed and thus while a signal
	// handler function executes.
	// In general, therefore, we should avoid performing any
	// I/O operations or calling most library and system functions from
	// signal handlers.

	// This shouldn't matter here, however, as we've already invoked
	// emergencyCleanup.
	docstring msg;
	switch (err_sig) {
#ifdef SIGHUP
	case SIGHUP:
		msg = _("SIGHUP signal caught!\nBye.");
		break;
#endif
	case SIGFPE:
		msg = _("SIGFPE signal caught!\nBye.");
		break;
	case SIGSEGV:
		msg = _("SIGSEGV signal caught!\n"
			  "Sorry, you have found a bug in LyX, "
			  "hope you have not lost any data.\n"
			  "Please read the bug-reporting instructions "
			  "in 'Help->Introduction' and send us a bug report, "
			  "if necessary. Thanks!\nBye.");
		break;
	case SIGINT:
	case SIGTERM:
		// no comments
		break;
	}

	if (!msg.empty()) {
		lyxerr << "\nlyx: " << msg << endl;
		// try to make a GUI message
		Alert::error(_("LyX crashed!"), msg, true);
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
		(err_sig != SIGHUP && !getEnv("LYXDEBUG").empty())) {
#else
	if (err_sig == SIGSEGV || !getEnv("LYXDEBUG").empty()) {
#endif
#ifdef _MSC_VER
		// with abort() it crashes again.
		exit(err_sig);
#else
		abort();
#endif
	}

	exit(0);
}

}


void LyX::printError(ErrorItem const & ei)
{
	docstring tmp = _("LyX: ") + ei.error + char_type(':')
		+ ei.description;
	cerr << to_utf8(tmp) << endl;
}

#if defined (USE_MACOSX_PACKAGING)
namespace {
	// Unexposed--extract an environment variable name from its NAME=VALUE
	// representation
	std::string varname(const char* line)
	{
		size_t nameLen = strcspn(line, "=");
		if (nameLen == strlen(line)) {
			return std::string();
		} else {
			return std::string(line, nameLen);
		}
	}
}

void cleanDuplicateEnvVars()
{
	std::set<std::string> seen;
	std::set<std::string> dupes;

	// Create a list of the environment variables that appear more than once
	for (char **read = *_NSGetEnviron(); *read; read++) {
		std::string name = varname(*read);
		if (name.size() == 0) {
			continue;
		}
		if (seen.find(name) != seen.end()) {
			dupes.insert(name);
		} else {
			seen.insert(name);
		}
	}

	// Loop over the list of duplicated variables
	std::set<std::string>::iterator dupe = dupes.begin();
	std::set<std::string>::iterator const dend = dupes.end();
	for (; dupe != dend; ++dupe) {
		const char *name = (*dupe).c_str();
		char *val = getenv(name);
		if (val != NULL) {
			LYXERR(Debug::INIT, "Duplicate environment variable: " << name);
			// unsetenv removes *all* instances of the variable from the environment
			unsetenv(name);

			// replace with the value from getenv (in practice appears to be the
			// first value in the list)
			setenv(name, val, 0);
		}
	}
}
#endif


static void initTemplatePath()
{
	FileName const package_template_path =
		FileName(addName(package().system_support().absFileName(), "templates"));

	if (lyxrc.template_path.empty()) {
		lyxrc.template_path = package_template_path.absFileName();
	}
#if defined (USE_MACOSX_PACKAGING)
	FileName const user_template_path =
		FileName(addName(package().user_support().absFileName(), "templates"));

	if (package_template_path != FileName(lyxrc.template_path) &&
		user_template_path != FileName(lyxrc.template_path))
	{
		return;
	}
	FileName const user_template_link =
		FileName(addName(user_template_path.absFileName(),"SystemTemplates"));
	if (user_template_link.isSymLink() && !equivalent(user_template_link, package_template_path)) {
		user_template_link.removeFile();
	}
	if (!user_template_link.exists()) {
		if (!package_template_path.link(user_template_link)) {
			FileName const user_support = package().user_support();
			if (user_support.exists() && user_support.isDirectory()) {
				LYXERR(Debug::INIT, "Cannot create symlink " + user_template_link.absFileName());
				lyxrc.template_path = package_template_path.absFileName();
			}
			return;
		}
		LYXERR(Debug::INIT, "Symlink \"" << user_template_link.absFileName() << "\" created.");
	}
	lyxrc.template_path = user_template_path.absFileName();
#endif
}


bool LyX::init()
{
#ifdef SIGHUP
	signal(SIGHUP, error_handler);
#endif
	signal(SIGFPE, error_handler);
	signal(SIGSEGV, error_handler);
	signal(SIGINT, error_handler);
	signal(SIGTERM, error_handler);
	// SIGPIPE can be safely ignored.

#if defined (USE_MACOSX_PACKAGING)
	cleanDuplicateEnvVars();
#endif

	lyxrc.tempdir_path = package().temp_dir().absFileName();
	lyxrc.document_path = package().document_dir().absFileName();

	if (lyxrc.example_path.empty()) {
		lyxrc.example_path = addPath(package().system_support().absFileName(),
					      "examples");
	}
	initTemplatePath();

	// init LyXDir environment variable
	string const lyx_dir = package().lyx_dir().absFileName();
	LYXERR(Debug::INIT, "Setting LyXDir... to \"" << lyx_dir << "\"");
	if (!setEnv("LyXDir", lyx_dir))
		LYXERR(Debug::INIT, "\t... failed!");

	if (package().explicit_user_support() && getEnv(LYX_USERDIR_VER).empty()) {
		// -userdir was given on the command line.
		// Make it available to child processes, otherwise tex2lyx
		// would not find all layout files, and other converters might
		// use it as well.
		string const user_dir = package().user_support().absFileName();
		LYXERR(Debug::INIT, "Setting " LYX_USERDIR_VER "... to \""
		                    << user_dir << '"');
		if (!setEnv(LYX_USERDIR_VER, user_dir))
			LYXERR(Debug::INIT, "\t... failed!");
	}

	//
	// Read configuration files
	//

	// This one may have been distributed along with LyX.
	if (!readRcFile("lyxrc.dist"))
		return false;

	// Set the PATH correctly.
#if !defined (USE_POSIX_PACKAGING) && !defined (USE_HAIKU_PACKAGING)
	// Add the directory containing the LyX executable to the path
	// so that LyX can find things like tex2lyx.
	if (package().build_support().empty())
		prependEnvPath("PATH", package().binary_dir().absFileName());
#endif
	if (!lyxrc.path_prefix.empty())
		prependEnvPath("PATH", replaceEnvironmentPath(lyxrc.path_prefix));

	// Check that user LyX directory is ok.
	{
		string const lock_file = package().getConfigureLockName();
		int fd = fileLock(lock_file.c_str());

		if (queryUserLyXDir(package().explicit_user_support())) {
			package().reconfigureUserLyXDir("");
			// Now the user directory is present on first start.
			initTemplatePath();
		}
		fileUnlock(fd, lock_file.c_str());
	}

	if (!use_gui) {
		// No need for a splash when there is no GUI
		first_start = false;
		// Default is to overwrite the main file during export, unless
		// the -f switch was specified or LYX_FORCE_OVERWRITE was set
		if (force_overwrite == UNSPECIFIED) {
			string const what = getEnv("LYX_FORCE_OVERWRITE");
			if (what == "all")
				force_overwrite = ALL_FILES;
			else if (what == "none")
				force_overwrite = NO_FILES;
			else
				force_overwrite = MAIN_FILE;
		}
	}

	// This one is generated in user_support directory by lib/configure.py.
	if (!readRcFile("lyxrc.defaults"))
		return false;

	// Query the OS to know what formats are viewed natively
	formats.setAutoOpen();

	// Read lyxrc.dist again to be able to override viewer auto-detection.
	readRcFile("lyxrc.dist");

	system_lyxrc = lyxrc;
	system_formats = formats;
	pimpl_->system_converters_ = pimpl_->converters_;
	pimpl_->system_movers_ = pimpl_->movers_;
	system_lcolor = lcolor;

	// This one is edited through the preferences dialog.
	if (!readRcFile("preferences", true))
		return false;

	// The language may have been set to someting useful through prefs
	setLocale();

	if (!readEncodingsFile("encodings", "unicodesymbols"))
		return false;
	if (!readLanguagesFile("languages"))
		return false;

	LYXERR(Debug::INIT, "Reading layouts...");
	// Load the layouts
	LayoutFileList::get().read();
	//...and the modules
	theModuleList.read();

	// read keymap and ui files in batch mode as well
	// because InsetInfo needs to know these to produce
	// the correct output

	// Set up command definitions
	pimpl_->toplevel_cmddef_.read(lyxrc.def_file);

	// FIXME
	// Set up bindings
	pimpl_->toplevel_keymap_.read("site");
	pimpl_->toplevel_keymap_.read(lyxrc.bind_file);
	// load user bind file user.bind
	pimpl_->toplevel_keymap_.read("user", 0, KeyMap::MissingOK);

	if (lyxerr.debugging(Debug::LYXRC))
		lyxrc.print();

	os::windows_style_tex_paths(lyxrc.windows_style_tex_paths);
	// Prepend path prefix a second time to take the user preferences into a account
	if (!lyxrc.path_prefix.empty())
		prependEnvPath("PATH", replaceEnvironmentPath(lyxrc.path_prefix));

	FileName const document_path(lyxrc.document_path);
	if (document_path.exists() && document_path.isDirectory())
		package().document_dir() = document_path;

	package().set_temp_dir(createLyXTmpDir(FileName(lyxrc.tempdir_path)));
	if (package().temp_dir().empty()) {
		Alert::error(_("Could not create temporary directory"),
			     bformat(_("Could not create a temporary directory in\n"
						       "\"%1$s\"\n"
							   "Make sure that this path exists and is writable and try again."),
				     from_utf8(lyxrc.tempdir_path)));
		// createLyXTmpDir() tries sufficiently hard to create a
		// usable temp dir, so the probability to come here is
		// close to zero. We therefore don't try to overcome this
		// problem with e.g. asking the user for a new path and
		// trying again but simply exit.
		return false;
	}

	LYXERR(Debug::INIT, "LyX tmp dir: `"
			    << package().temp_dir().absFileName() << '\'');

	LYXERR(Debug::INIT, "Reading session information '.lyx/session'...");
	pimpl_->session_.reset(new Session(lyxrc.num_lastfiles));

	// This must happen after package initialization and after lyxrc is
	// read, therefore it can't be done by a static object.
	ConverterCache::init();

	return true;
}


void emergencyCleanup()
{
	// what to do about tmpfiles is non-obvious. we would
	// like to delete any we find, but our lyxdir might
	// contain documents etc. which might be helpful on
	// a crash

	singleton_->pimpl_->buffer_list_.emergencyWriteAll();
	if (use_gui) {
		if (singleton_->pimpl_->lyx_server_)
			singleton_->pimpl_->lyx_server_->emergencyCleanup();
		singleton_->pimpl_->lyx_server_.reset();
		singleton_->pimpl_->lyx_socket_.reset();
	}
}


bool LyX::queryUserLyXDir(bool explicit_userdir)
{
	// Does user directory exist?
	FileName const sup = package().user_support();
	if (sup.exists() && sup.isDirectory()) {
		first_start = false;

		return configFileNeedsUpdate("lyxrc.defaults")
			|| configFileNeedsUpdate("lyxmodules.lst")
			|| configFileNeedsUpdate("textclass.lst")
			|| configFileNeedsUpdate("packages.lst");
	}

	first_start = !explicit_userdir;

	// If the user specified explicitly a directory, ask whether
	// to create it. If the user says "no", then exit.
	if (explicit_userdir &&
	    Alert::prompt(
		    _("Missing user LyX directory"),
		    bformat(_("You have specified a non-existent user "
					   "LyX directory, %1$s.\n"
					   "It is needed to keep your own configuration."),
			    from_utf8(package().user_support().absFileName())),
		    1, 0,
		    _("&Create directory"),
		    _("&Exit LyX"))) {
		lyxerr << to_utf8(_("No user LyX directory. Exiting.")) << endl;
		earlyExit(EXIT_FAILURE);
	}

	lyxerr << to_utf8(bformat(_("LyX: Creating directory %1$s"),
			  from_utf8(sup.absFileName()))) << endl;

	if (!sup.createDirectory(0755)) {
		// Failed, so let's exit.
		lyxerr << to_utf8(_("Failed to create directory. Exiting."))
		       << endl;
		earlyExit(EXIT_FAILURE);
	}

	return true;
}


bool LyX::readRcFile(string const & name, bool check_format)
{
	LYXERR(Debug::INIT, "About to read " << name << "... ");

	FileName const lyxrc_path = libFileSearch(string(), name);
	if (lyxrc_path.empty()) {
		LYXERR(Debug::INIT, "Not found." << lyxrc_path);
		// FIXME
		// This was the previous logic, but can it be right??
		return true;
	}
	LYXERR(Debug::INIT, "Found in " << lyxrc_path);
	bool const success = lyxrc.read(lyxrc_path, check_format);
	if (!success)
		showFileError(name);
	return success;
}

// Read the languages file `name'
bool LyX::readLanguagesFile(string const & name)
{
	LYXERR(Debug::INIT, "About to read " << name << "...");

	FileName const lang_path = libFileSearch(string(), name);
	if (lang_path.empty()) {
		showFileError(name);
		return false;
	}
	languages.read(lang_path);
	return true;
}


// Read the encodings file `name'
bool LyX::readEncodingsFile(string const & enc_name,
			    string const & symbols_name)
{
	LYXERR(Debug::INIT, "About to read " << enc_name << " and "
			    << symbols_name << "...");

	FileName const symbols_path = libFileSearch(string(), symbols_name);
	if (symbols_path.empty()) {
		showFileError(symbols_name);
		return false;
	}

	FileName const enc_path = libFileSearch(string(), enc_name);
	if (enc_path.empty()) {
		showFileError(enc_name);
		return false;
	}
	encodings.read(enc_path, symbols_path);
	return true;
}


namespace {

/// return the the number of arguments consumed
typedef boost::function<int(string const &, string const &, string &)> cmd_helper;

int parse_dbg(string const & arg, string const &, string &)
{
	if (arg.empty()) {
		cout << to_utf8(_("List of supported debug flags:")) << endl;
		Debug::showTags(cout);
		exit(0);
	}
	lyxerr << to_utf8(bformat(_("Setting debug level to %1$s"), from_utf8(arg))) << endl;

	lyxerr.setLevel(Debug::value(arg));
	Debug::showLevel(lyxerr, lyxerr.level());
	return 1;
}


int parse_help(string const &, string const &, string &)
{
	cout <<
		to_utf8(_("Usage: lyx [ command line switches ] [ name.lyx ... ]\n"
		  "Command line switches (case sensitive):\n"
		  "\t-help              summarize LyX usage\n"
		  "\t-userdir dir       set user directory to dir\n"
		  "\t-sysdir dir        set system directory to dir\n"
		  "\t-geometry WxH+X+Y  set geometry of the main window\n"
		  "\t-dbg feature[,feature]...\n"
		  "                  select the features to debug.\n"
		  "                  Type `lyx -dbg' to see the list of features\n"
		  "\t-x [--execute] command\n"
		  "                  where command is a lyx command.\n"
		  "\t-e [--export] fmt\n"
		  "                  where fmt is the export format of choice. Look in\n"
		  "                  Tools->Preferences->File Handling->File Formats->Short Name\n"
		  "                  to see which parameter (which differs from the format name\n"
		  "                  in the File->Export menu) should be passed.\n"
		  "                  Note that the order of -e and -x switches matters.\n"
		  "\t-E [--export-to] fmt filename\n"
		  "                  where fmt is the export format of choice (see --export),\n"
		  "                  and filename is the destination filename.\n"
		  "\t-i [--import] fmt file.xxx\n"
		  "                  where fmt is the import format of choice\n"
		  "                  and file.xxx is the file to be imported.\n"
		  "\t-f [--force-overwrite] what\n"
		  "                  where what is either `all', `main' or `none',\n"
		  "                  specifying whether all files, main file only, or no files,\n"
		  "                  respectively, are to be overwritten during a batch export.\n"
		  "                  Anything else is equivalent to `all', but is not consumed.\n"
		  "\t-n [--no-remote]\n"
		  "                  open documents in a new instance\n"
		  "\t-r [--remote]\n"
		  "                  open documents in an already running instance\n"
		  "                  (a working lyxpipe is needed)\n"
		  "\t-batch    execute commands without launching GUI and exit.\n"
		  "\t-version  summarize version and build info\n"
			       "Check the LyX man page for more details.")) << endl;
	exit(0);
	return 0;
}


int parse_version(string const &, string const &, string &)
{
	cout << "LyX " << lyx_version
	       << " (" << lyx_release_date << ")" << endl;
	if (string(lyx_git_commit_hash) != "none")
		cout << to_utf8(_("  Git commit hash "))
		     << string(lyx_git_commit_hash).substr(0,8) << endl;
	cout << lyx_version_info << endl;
	exit(0);
	return 0;
}


int parse_sysdir(string const & arg, string const &, string &)
{
	if (arg.empty()) {
		Alert::error(_("No system directory"),
			_("Missing directory for -sysdir switch"));
		exit(1);
	}
	cl_system_support = arg;
	return 1;
}


int parse_userdir(string const & arg, string const &, string &)
{
	if (arg.empty()) {
		Alert::error(_("No user directory"),
			_("Missing directory for -userdir switch"));
		exit(1);
	}
	cl_user_support = arg;
	return 1;
}


int parse_execute(string const & arg, string const &, string & batch)
{
	if (arg.empty()) {
		Alert::error(_("Incomplete command"),
			_("Missing command string after --execute switch"));
		exit(1);
	}
	batch = arg;
	return 1;
}


int parse_export_to(string const & type, string const & output_file, string & batch)
{
	if (type.empty()) {
		lyxerr << to_utf8(_("Missing file type [eg latex, ps...] after "
					 "--export-to switch")) << endl;
		exit(1);
	}
	if (output_file.empty()) {
		lyxerr << to_utf8(_("Missing destination filename after "
					 "--export-to switch")) << endl;
		exit(1);
	}
	batch = "buffer-export " + type + " " + output_file;
	use_gui = false;
	return 2;
}


int parse_export(string const & type, string const &, string & batch)
{
	if (type.empty()) {
		lyxerr << to_utf8(_("Missing file type [eg latex, ps...] after "
					 "--export switch")) << endl;
		exit(1);
	}
	batch = "buffer-export " + type;
	use_gui = false;
	return 1;
}


int parse_import(string const & type, string const & file, string & batch)
{
	if (type.empty()) {
		lyxerr << to_utf8(_("Missing file type [eg latex, ps...] after "
					 "--import switch")) << endl;
		exit(1);
	}
	if (file.empty()) {
		lyxerr << to_utf8(_("Missing filename for --import")) << endl;
		exit(1);
	}
	batch = "buffer-import " + type + ' ' + file;
	return 2;
}


int parse_geometry(string const & arg1, string const &, string &)
{
	geometryArg = arg1;
	// don't remove "-geometry", it will be pruned out later in the
	// frontend if need be.
	return -1;
}


int parse_batch(string const &, string const &, string &)
{
	use_gui = false;
	return 0;
}


int parse_noremote(string const &, string const &, string &)
{
	run_mode = NEW_INSTANCE;
	return 0;
}


int parse_remote(string const &, string const &, string &)
{
	run_mode = USE_REMOTE;
	return 0;
}


int parse_force(string const & arg, string const &, string &)
{
	if (arg == "all") {
		force_overwrite = ALL_FILES;
		return 1;
	} else if (arg == "main") {
		force_overwrite = MAIN_FILE;
		return 1;
	} else if (arg == "none") {
		force_overwrite = NO_FILES;
		return 1;
	}
	force_overwrite = ALL_FILES;
	return 0;
}


} // namespace anon


void LyX::easyParse(int & argc, char * argv[])
{
	map<string, cmd_helper> cmdmap;

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
	cmdmap["-E"] = parse_export_to;
	cmdmap["--export-to"] = parse_export_to;
	cmdmap["-i"] = parse_import;
	cmdmap["--import"] = parse_import;
	cmdmap["-geometry"] = parse_geometry;
	cmdmap["-batch"] = parse_batch;
	cmdmap["-f"] = parse_force;
	cmdmap["--force-overwrite"] = parse_force;
	cmdmap["-n"] = parse_noremote;
	cmdmap["--no-remote"] = parse_noremote;
	cmdmap["-r"] = parse_remote;
	cmdmap["--remote"] = parse_remote;

	for (int i = 1; i < argc; ++i) {
		map<string, cmd_helper>::const_iterator it
			= cmdmap.find(argv[i]);

		// don't complain if not found - may be parsed later
		if (it == cmdmap.end())
			continue;

		string const arg =
			(i + 1 < argc) ? os::utf8_argv(i + 1) : string();
		string const arg2 =
			(i + 2 < argc) ? os::utf8_argv(i + 2) : string();

		string batch;
		int const remove = 1 + it->second(arg, arg2, batch);
		if (!batch.empty())
			pimpl_->batch_commands.push_back(batch);

		// Now, remove used arguments by shifting
		// the following ones remove places down.
		if (remove > 0) {
			os::remove_internal_args(i, remove);
			argc -= remove;
			for (int j = i; j < argc; ++j)
				argv[j] = argv[j + remove];
			--i;
		}
	}
}


FuncStatus getStatus(FuncRequest const & action)
{
	LAPPERR(theApp());
	return theApp()->getStatus(action);
}


DispatchResult const & dispatch(FuncRequest const & action)
{
	LAPPERR(theApp());
	return theApp()->dispatch(action);
}


void dispatch(FuncRequest const & action, DispatchResult & dr)
{
	LAPPERR(theApp());
	theApp()->dispatch(action, dr);
}


vector<string> & theFilesToLoad()
{
	LAPPERR(singleton_);
	return singleton_->pimpl_->files_to_load_;
}


BufferList & theBufferList()
{
	LAPPERR(singleton_);
	return singleton_->pimpl_->buffer_list_;
}


Server & theServer()
{
	// FIXME: this should not be use_gui dependent
	LWARNIF(use_gui);
	LAPPERR(singleton_);
	return *singleton_->pimpl_->lyx_server_.get();
}


ServerSocket & theServerSocket()
{
	// FIXME: this should not be use_gui dependent
	LWARNIF(use_gui);
	LAPPERR(singleton_);
	return *singleton_->pimpl_->lyx_socket_.get();
}


KeyMap & theTopLevelKeymap()
{
	LAPPERR(singleton_);
	return singleton_->pimpl_->toplevel_keymap_;
}


Converters & theConverters()
{
	LAPPERR(singleton_);
	return  singleton_->pimpl_->converters_;
}


Converters & theSystemConverters()
{
	LAPPERR(singleton_);
	return  singleton_->pimpl_->system_converters_;
}


Movers & theMovers()
{
	LAPPERR(singleton_);
	return singleton_->pimpl_->movers_;
}


Mover const & getMover(string  const & fmt)
{
	LAPPERR(singleton_);
	return singleton_->pimpl_->movers_(fmt);
}


void setMover(string const & fmt, string const & command)
{
	LAPPERR(singleton_);
	singleton_->pimpl_->movers_.set(fmt, command);
}


Movers & theSystemMovers()
{
	LAPPERR(singleton_);
	return singleton_->pimpl_->system_movers_;
}


Messages const & getMessages(string const & language)
{
	LAPPERR(singleton_);
	return singleton_->messages(language);
}


Messages const & getGuiMessages()
{
	LAPPERR(singleton_);
	return singleton_->messages(Messages::guiLanguage());
}


Session & theSession()
{
	LAPPERR(singleton_);
	return *singleton_->pimpl_->session_.get();
}


LaTeXFonts & theLaTeXFonts()
{
	LAPPERR(singleton_);
	if (!singleton_->pimpl_->latexfonts_)
		singleton_->pimpl_->latexfonts_ = new LaTeXFonts;
	return *singleton_->pimpl_->latexfonts_;
}


CmdDef & theTopLevelCmdDef()
{
	LAPPERR(singleton_);
	return singleton_->pimpl_->toplevel_cmddef_;
}


SpellChecker * theSpellChecker()
{
	if (!singleton_->pimpl_->spell_checker_)
		setSpellChecker();
	return singleton_->pimpl_->spell_checker_;
}


void setSpellChecker()
{
	SpellChecker::ChangeNumber speller_change_number =singleton_->pimpl_->spell_checker_ ?
		singleton_->pimpl_->spell_checker_->changeNumber() : 0;

	if (lyxrc.spellchecker == "native") {
#if defined(USE_MACOSX_PACKAGING)
		if (!singleton_->pimpl_->apple_spell_checker_)
			singleton_->pimpl_->apple_spell_checker_ = new AppleSpellChecker;
		singleton_->pimpl_->spell_checker_ = singleton_->pimpl_->apple_spell_checker_;
#else
		singleton_->pimpl_->spell_checker_ = 0;
#endif
	} else if (lyxrc.spellchecker == "aspell") {
#if defined(USE_ASPELL)
		if (!singleton_->pimpl_->aspell_checker_)
			singleton_->pimpl_->aspell_checker_ = new AspellChecker;
		singleton_->pimpl_->spell_checker_ = singleton_->pimpl_->aspell_checker_;
#else
		singleton_->pimpl_->spell_checker_ = 0;
#endif
	} else if (lyxrc.spellchecker == "enchant") {
#if defined(USE_ENCHANT)
		if (!singleton_->pimpl_->enchant_checker_)
			singleton_->pimpl_->enchant_checker_ = new EnchantChecker;
		singleton_->pimpl_->spell_checker_ = singleton_->pimpl_->enchant_checker_;
#else
		singleton_->pimpl_->spell_checker_ = 0;
#endif
	} else if (lyxrc.spellchecker == "hunspell") {
#if defined(USE_HUNSPELL)
		if (!singleton_->pimpl_->hunspell_checker_)
			singleton_->pimpl_->hunspell_checker_ = new HunspellChecker;
		singleton_->pimpl_->spell_checker_ = singleton_->pimpl_->hunspell_checker_;
#else
		singleton_->pimpl_->spell_checker_ = 0;
#endif
	} else {
		singleton_->pimpl_->spell_checker_ = 0;
	}
	if (singleton_->pimpl_->spell_checker_) {
		singleton_->pimpl_->spell_checker_->changeNumber(speller_change_number);
		singleton_->pimpl_->spell_checker_->advanceChangeNumber();
	}
}

} // namespace lyx

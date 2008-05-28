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

#include "LayoutFile.h"
#include "Buffer.h"
#include "BufferList.h"
#include "CmdDef.h"
#include "Color.h"
#include "ConverterCache.h"
#include "Converter.h"
#include "CutAndPaste.h"
#include "Encoding.h"
#include "ErrorList.h"
#include "Format.h"
#include "FuncStatus.h"
#include "KeyMap.h"
#include "Language.h"
#include "Lexer.h"
#include "LyXAction.h"
#include "LyXFunc.h"
#include "LyXRC.h"
#include "ModuleList.h"
#include "Mover.h"
#include "Server.h"
#include "ServerSocket.h"
#include "Session.h"

#include "frontends/alert.h"
#include "frontends/Application.h"

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
#include "support/Path.h"
#include "support/Systemcall.h"

#include <boost/bind.hpp>
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

namespace lyx {

namespace Alert = frontend::Alert;
namespace os = support::os;



// Are we using the GUI at all?  We default to true and this is changed
// to false when the export feature is used.

bool use_gui = true;

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


void reconfigureUserLyXDir()
{
	string const configure_command = package().configure_command();

	lyxerr << to_utf8(_("LyX: reconfiguring user directory")) << endl;
	PathChanger p(package().user_support());
	Systemcall one;
	one.startscript(Systemcall::Wait, configure_command);
	lyxerr << "LyX: " << to_utf8(_("Done!")) << endl;
}

} // namespace anon


/// The main application class private implementation.
struct LyX::Impl
{
	Impl()
	{
		// Set the default User Interface language as soon as possible.
		// The language used will be derived from the environment
		// variables.
		messages_["GUI"] = Messages();
	}
	/// our function handler
	LyXFunc lyxfunc_;
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

	/// has this user started lyx for the first time?
	bool first_start;
	/// the parsed command line batch command if any
	string batch_command;
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
}


void LyX::exit(int exit_code) const
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


LyX & LyX::ref()
{
	LASSERT(singleton_, /**/);
	return *singleton_;
}


LyX const & LyX::cref()
{
	LASSERT(singleton_, /**/);
	return *singleton_;
}


LyX::LyX()
	: first_start(false)
{
	singleton_ = this;
	pimpl_ = new Impl;
}


BufferList & LyX::bufferList()
{
	return pimpl_->buffer_list_;
}


BufferList const & LyX::bufferList() const
{
	return pimpl_->buffer_list_;
}


Session & LyX::session()
{
	LASSERT(pimpl_->session_.get(), /**/);
	return *pimpl_->session_.get();
}


Session const & LyX::session() const
{
	LASSERT(pimpl_->session_.get(), /**/);
	return *pimpl_->session_.get();
}


LyXFunc & LyX::lyxFunc()
{
	return pimpl_->lyxfunc_;
}


LyXFunc const & LyX::lyxFunc() const
{
	return pimpl_->lyxfunc_;
}


Server & LyX::server()
{
	LASSERT(pimpl_->lyx_server_.get(), /**/);
	return *pimpl_->lyx_server_.get();
}


Server const & LyX::server() const
{
	LASSERT(pimpl_->lyx_server_.get(), /**/);
	return *pimpl_->lyx_server_.get();
}


ServerSocket & LyX::socket()
{
	LASSERT(pimpl_->lyx_socket_.get(), /**/);
	return *pimpl_->lyx_socket_.get();
}


ServerSocket const & LyX::socket() const
{
	LASSERT(pimpl_->lyx_socket_.get(), /**/);
	return *pimpl_->lyx_socket_.get();
}


frontend::Application & LyX::application()
{
	LASSERT(pimpl_->application_.get(), /**/);
	return *pimpl_->application_.get();
}


frontend::Application const & LyX::application() const
{
	LASSERT(pimpl_->application_.get(), /**/);
	return *pimpl_->application_.get();
}


CmdDef & LyX::topLevelCmdDef()
{
	return pimpl_->toplevel_cmddef_;
}


Converters & LyX::converters()
{
	return pimpl_->converters_;
}


Converters & LyX::systemConverters()
{
	return pimpl_->system_converters_;
}


Messages & LyX::getMessages(string const & language)
{
	map<string, Messages>::iterator it = pimpl_->messages_.find(language);

	if (it != pimpl_->messages_.end())
		return it->second;

	pair<map<string, Messages>::iterator, bool> result =
			pimpl_->messages_.insert(make_pair(language, Messages(language)));

	LASSERT(result.second, /**/);
	return result.first->second;
}


Messages & LyX::getGuiMessages()
{
	return pimpl_->messages_["GUI"];
}


void LyX::setGuiLanguage(string const & language)
{
	pimpl_->messages_["GUI"] = Messages(language);
}


int LyX::exec(int & argc, char * argv[])
{
	// Here we need to parse the command line. At least
	// we need to parse for "-dbg" and "-help"
	easyParse(argc, argv);

	try {
		init_package(to_utf8(from_local8bit(argv[0])),
			      cl_system_support, cl_user_support,
			      top_build_dir_is_one_level_up);
	} catch (ExceptionMessage const & message) {
		if (message.type_ == ErrorException) {
			Alert::error(message.title_, message.details_);
			exit(1);
		} else if (message.type_ == WarningException) {
			Alert::warning(message.title_, message.details_);
		}
	}

	// Reinit the messages machinery in case package() knows
	// something interesting about the locale directory.
	Messages::init();

	if (!use_gui) {
		// FIXME: create a ConsoleApplication
		int exit_status = init(argc, argv);
		if (exit_status) {
			prepareExit();
			return exit_status;
		}

		// this is correct, since return values are inverted.
		exit_status = !loadFiles();

		if (pimpl_->batch_command.empty() || pimpl_->buffer_list_.empty()) {
			prepareExit();
			return exit_status;
		}

		BufferList::iterator begin = pimpl_->buffer_list_.begin();

		bool final_success = false;
		for (BufferList::iterator I = begin; I != pimpl_->buffer_list_.end(); ++I) {
			Buffer * buf = *I;
			if (buf != buf->masterBuffer())
				continue;
			bool success = false;
			buf->dispatch(pimpl_->batch_command, &success);
			final_success |= success;
		}
		prepareExit();
		return !final_success;
	}

	// Let the frontend parse and remove all arguments that it knows
	pimpl_->application_.reset(createApplication(argc, argv));

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

	// FIXME
	/* Create a CoreApplication class that will provide the main event loop
	* and the socket callback registering. With Qt4, only QtCore
	* library would be needed.
	* When this is done, a server_mode could be created and the following two
	* line would be moved out from here.
	*/
	// Note: socket callback must be registered after init(argc, argv)
	// such that package().temp_dir() is properly initialized.
	pimpl_->lyx_server_.reset(new Server(&pimpl_->lyxfunc_, lyxrc.lyxpipes));
	pimpl_->lyx_socket_.reset(new ServerSocket(&pimpl_->lyxfunc_,
			FileName(package().temp_dir().absFilename() + "/lyxsocket")));

	// Start the real execution loop.
	exit_status = pimpl_->application_->exec();

	prepareExit();

	return exit_status;
}


void LyX::prepareExit()
{
	// Clear the clipboard and selection stack:
	cap::clearCutStack();
	cap::clearSelection();

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
		LYXERR(Debug::INFO, "Deleting tmp dir "
				    << package().temp_dir().absFilename());

		if (!package().temp_dir().destroyDirectory()) {
			docstring const msg =
				bformat(_("Unable to remove the temporary directory %1$s"),
				from_utf8(package().temp_dir().absFilename()));
			Alert::warning(_("Unable to remove temporary directory"), msg);
		}
	}

	// Kill the application object before exiting. This avoids crashes
	// when exiting on Linux.
	if (pimpl_->application_)
		pimpl_->application_.reset();
}


void LyX::earlyExit(int status)
{
	LASSERT(pimpl_->application_.get(), /**/);
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
				from_utf8(argv[argi]))) << endl;
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
	for (int argi = argc - 1; argi >= 1; --argi)
		pimpl_->files_to_load_.push_back(to_utf8(from_local8bit(argv[argi])));

	if (first_start) {
		pimpl_->files_to_load_.push_back(
			i18nLibFileSearch("examples", "splash.lyx").absFilename());
	}

	return EXIT_SUCCESS;
}


void LyX::addFileToLoad(string const & fname)
{
	vector<string>::const_iterator cit = find(
		pimpl_->files_to_load_.begin(), pimpl_->files_to_load_.end(),
		fname);

	if (cit == pimpl_->files_to_load_.end())
		pimpl_->files_to_load_.push_back(fname);
}


bool LyX::loadFiles()
{
	LASSERT(!use_gui, /**/);
	bool success = true;
	vector<string>::const_iterator it = pimpl_->files_to_load_.begin();
	vector<string>::const_iterator end = pimpl_->files_to_load_.end();

	for (; it != end; ++it) {
		// get absolute path of file and add ".lyx" to
		// the filename if necessary
		FileName fname = fileSearch(string(), os::internal_path(*it), "lyx",
			may_not_exist);

		if (fname.empty())
			continue;

		Buffer * buf = pimpl_->buffer_list_.newBuffer(fname.absFilename(), false);
		if (buf->loadLyXFile(fname)) {
			ErrorList const & el = buf->errorList("Parse");
			if (!el.empty())
				for_each(el.begin(), el.end(),
				boost::bind(&LyX::printError, this, _1));
		}
		else {
			pimpl_->buffer_list_.release(buf);
			success = false;
		}
	}
	return success;
}


void LyX::execBatchCommands()
{
	// The advantage of doing this here is that the event loop
	// is already started. So any need for interaction will be
	// aknowledged.

	// if reconfiguration is needed.
	while (LayoutFileList::get().empty()) {
		switch (Alert::prompt(
			_("No textclass is found"),
			_("LyX cannot continue because no textclass is found. "
				"You can either reconfigure normally, or reconfigure using "
				"default textclasses, or quit LyX."),
			0, 2,
			_("&Reconfigure"),
			_("&Use Default"),
			_("&Exit LyX")))
		{
		case 0:
			// regular reconfigure
			pimpl_->lyxfunc_.dispatch(FuncRequest(LFUN_RECONFIGURE, ""));
			break;
		case 1:
			// reconfigure --without-latex-config
			pimpl_->lyxfunc_.dispatch(FuncRequest(LFUN_RECONFIGURE,
				" --without-latex-config"));
			break;
		default:
			pimpl_->lyxfunc_.dispatch(FuncRequest(LFUN_LYX_QUIT));
			return;
		}
	}
	
	// create the first main window
	pimpl_->lyxfunc_.dispatch(FuncRequest(LFUN_WINDOW_NEW, geometryArg));

	if (!pimpl_->files_to_load_.empty()) {
		// if some files were specified at command-line we assume that the
		// user wants to edit *these* files and not to restore the session.
		for (size_t i = 0; i != pimpl_->files_to_load_.size(); ++i) {
			pimpl_->lyxfunc_.dispatch(
				FuncRequest(LFUN_FILE_OPEN, pimpl_->files_to_load_[i]));
		}
		// clear this list to save a few bytes of RAM
		pimpl_->files_to_load_.clear();
	}
	else
		pimpl_->application_->restoreGuiSession();

	// Execute batch commands if available
	if (pimpl_->batch_command.empty())
		return;

	LYXERR(Debug::INIT, "About to handle -x '" << pimpl_->batch_command << '\'');

	pimpl_->lyxfunc_.dispatch(lyxaction.lookupFunc(pimpl_->batch_command));
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
		abort();
	exit(0);
}

}


void LyX::printError(ErrorItem const & ei)
{
	docstring tmp = _("LyX: ") + ei.error + char_type(':')
		+ ei.description;
	cerr << to_utf8(tmp) << endl;
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

	lyxrc.tempdir_path = package().temp_dir().absFilename();
	lyxrc.document_path = package().document_dir().absFilename();

	if (lyxrc.example_path.empty()) {
		lyxrc.example_path = addPath(package().system_support().absFilename(),
					      "examples");
	}
	if (lyxrc.template_path.empty()) {
		lyxrc.template_path = addPath(package().system_support().absFilename(),
					      "templates");
	}

	//
	// Read configuration files
	//

	// This one may have been distributed along with LyX.
	if (!readRcFile("lyxrc.dist"))
		return false;

	// Set the language defined by the distributor.
	//setGuiLanguage(lyxrc.gui_language);

	// Set the PATH correctly.
#if !defined (USE_POSIX_PACKAGING)
	// Add the directory containing the LyX executable to the path
	// so that LyX can find things like tex2lyx.
	if (package().build_support().empty())
		prependEnvPath("PATH", package().binary_dir().absFilename());
#endif
	if (!lyxrc.path_prefix.empty())
		prependEnvPath("PATH", lyxrc.path_prefix);

	// Check that user LyX directory is ok.
	if (queryUserLyXDir(package().explicit_user_support()))
		reconfigureUserLyXDir();

	// no need for a splash when there is no GUI
	if (!use_gui) {
		first_start = false;
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
	if (!readRcFile("preferences"))
		return false;

	if (!readEncodingsFile("encodings", "unicodesymbols"))
		return false;
	if (!readLanguagesFile("languages"))
		return false;

	// Load the layouts
	LYXERR(Debug::INIT, "Reading layouts...");
	if (!LyXSetStyle())
		return false;
	//...and the modules
	moduleList.load();

	// read keymap and ui files in batch mode as well
	// because InsetInfo needs to know these to produce
	// the correct output

	// Set the language defined by the user.
	//setGuiLanguage(lyxrc.gui_language);

	// Set up command definitions
	pimpl_->toplevel_cmddef_.read(lyxrc.def_file);

	// Set up bindings
	pimpl_->toplevel_keymap_.read("site");
	pimpl_->toplevel_keymap_.read(lyxrc.bind_file);
	// load user bind file user.bind
	pimpl_->toplevel_keymap_.read("user");

	pimpl_->lyxfunc_.initKeySequences(&pimpl_->toplevel_keymap_);

	if (lyxerr.debugging(Debug::LYXRC))
		lyxrc.print();

	os::windows_style_tex_paths(lyxrc.windows_style_tex_paths);
	if (!lyxrc.path_prefix.empty())
		prependEnvPath("PATH", lyxrc.path_prefix);

	FileName const document_path(lyxrc.document_path);
	if (document_path.exists() && document_path.isDirectory())
		package().document_dir() = document_path;

	package().temp_dir() = createLyXTmpDir(FileName(lyxrc.tempdir_path));
	if (package().temp_dir().empty()) {
		Alert::error(_("Could not create temporary directory"),
			     bformat(_("Could not create a temporary directory in\n"
						    "%1$s. Make sure that this\n"
						    "path exists and is writable and try again."),
				     from_utf8(lyxrc.tempdir_path)));
		// createLyXTmpDir() tries sufficiently hard to create a
		// usable temp dir, so the probability to come here is
		// close to zero. We therefore don't try to overcome this
		// problem with e.g. asking the user for a new path and
		// trying again but simply exit.
		return false;
	}

	LYXERR(Debug::INIT, "LyX tmp dir: `"
			    << package().temp_dir().absFilename() << '\'');

	LYXERR(Debug::INIT, "Reading session information '.lyx/session'...");
	pimpl_->session_.reset(new Session(lyxrc.num_lastfiles));

	// This must happen after package initialization and after lyxrc is
	// read, therefore it can't be done by a static object.
	ConverterCache::init();
		
	return true;
}


void LyX::emergencyCleanup() const
{
	// what to do about tmpfiles is non-obvious. we would
	// like to delete any we find, but our lyxdir might
	// contain documents etc. which might be helpful on
	// a crash

	pimpl_->buffer_list_.emergencyWriteAll();
	if (use_gui) {
		if (pimpl_->lyx_server_)
			pimpl_->lyx_server_->emergencyCleanup();
		pimpl_->lyx_server_.reset();
		pimpl_->lyx_socket_.reset();
	}
}


// return true if file does not exist or is older than configure.py.
static bool needsUpdate(string const & file)
{
	// We cannot initialize configure_script directly because the package
	// is not initialized yet when  static objects are constructed.
	static FileName configure_script;
	static bool firstrun = true;
	if (firstrun) {
		configure_script =
			FileName(addName(package().system_support().absFilename(),
				"configure.py"));
		firstrun = false;
	}

	FileName absfile = 
		FileName(addName(package().user_support().absFilename(), file));
	return !absfile.exists()
		|| configure_script.lastModified() > absfile.lastModified();
}


bool LyX::queryUserLyXDir(bool explicit_userdir)
{
	// Does user directory exist?
	FileName const sup = package().user_support();
	if (sup.exists() && sup.isDirectory()) {
		first_start = false;

		return needsUpdate("lyxrc.defaults")
			|| needsUpdate("lyxmodules.lst")
			|| needsUpdate("textclass.lst")
			|| needsUpdate("packages.lst");
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
			    from_utf8(package().user_support().absFilename())),
		    1, 0,
		    _("&Create directory"),
		    _("&Exit LyX"))) {
		lyxerr << to_utf8(_("No user LyX directory. Exiting.")) << endl;
		earlyExit(EXIT_FAILURE);
	}

	lyxerr << to_utf8(bformat(_("LyX: Creating directory %1$s"),
			  from_utf8(sup.absFilename()))) << endl;

	if (!sup.createDirectory(0755)) {
		// Failed, so let's exit.
		lyxerr << to_utf8(_("Failed to create directory. Exiting."))
		       << endl;
		earlyExit(EXIT_FAILURE);
	}

	return true;
}


bool LyX::readRcFile(string const & name)
{
	LYXERR(Debug::INIT, "About to read " << name << "... ");

	FileName const lyxrc_path = libFileSearch(string(), name);
	if (!lyxrc_path.empty()) {
		LYXERR(Debug::INIT, "Found in " << lyxrc_path);
		if (lyxrc.read(lyxrc_path) < 0) {
			showFileError(name);
			return false;
		}
	} else {
		LYXERR(Debug::INIT, "Not found." << lyxrc_path);
	}
	return true;
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

string batch;

/// return the the number of arguments consumed
typedef boost::function<int(string const &, string const &)> cmd_helper;

int parse_dbg(string const & arg, string const &)
{
	if (arg.empty()) {
		lyxerr << to_utf8(_("List of supported debug flags:")) << endl;
		Debug::showTags(lyxerr);
		exit(0);
	}
	lyxerr << to_utf8(bformat(_("Setting debug level to %1$s"), from_utf8(arg))) << endl;

	lyxerr.level(Debug::value(arg));
	Debug::showLevel(lyxerr, lyxerr.level());
	return 1;
}


int parse_help(string const &, string const &)
{
	lyxerr <<
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
		  "                  where fmt is the export format of choice.\n"
		  "                  Look on Tools->Preferences->File formats->Format\n"
		  "                  to get an idea which parameters should be passed.\n"
		  "\t-i [--import] fmt file.xxx\n"
		  "                  where fmt is the import format of choice\n"
		  "                  and file.xxx is the file to be imported.\n"
		  "\t-version        summarize version and build info\n"
			       "Check the LyX man page for more details.")) << endl;
	exit(0);
	return 0;
}


int parse_version(string const &, string const &)
{
	lyxerr << "LyX " << lyx_version
	       << " (" << lyx_release_date << ")" << endl;
	lyxerr << "Built on " << __DATE__ << ", " << __TIME__ << endl;

	lyxerr << lyx_version_info << endl;
	exit(0);
	return 0;
}


int parse_sysdir(string const & arg, string const &)
{
	if (arg.empty()) {
		Alert::error(_("No system directory"),
			_("Missing directory for -sysdir switch"));
		exit(1);
	}
	cl_system_support = arg;
	return 1;
}


int parse_userdir(string const & arg, string const &)
{
	if (arg.empty()) {
		Alert::error(_("No user directory"),
			_("Missing directory for -userdir switch"));
		exit(1);
	}
	cl_user_support = arg;
	return 1;
}


int parse_execute(string const & arg, string const &)
{
	if (arg.empty()) {
		Alert::error(_("Incomplete command"),
			_("Missing command string after --execute switch"));
		exit(1);
	}
	batch = arg;
	return 1;
}


int parse_export(string const & type, string const &)
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


int parse_import(string const & type, string const & file)
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


int parse_geometry(string const & arg1, string const &)
{
	geometryArg = arg1;
#if defined(_WIN32) || (defined(__CYGWIN__) && defined(X_DISPLAY_MISSING))
	// remove also the arg
	return 1;
#else
	// don't remove "-geometry"
	return -1;
#endif
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
	cmdmap["-i"] = parse_import;
	cmdmap["--import"] = parse_import;
	cmdmap["-geometry"] = parse_geometry;

	for (int i = 1; i < argc; ++i) {
		map<string, cmd_helper>::const_iterator it
			= cmdmap.find(argv[i]);

		// don't complain if not found - may be parsed later
		if (it == cmdmap.end())
			continue;

		string const arg =
			(i + 1 < argc) ? to_utf8(from_local8bit(argv[i + 1])) : string();
		string const arg2 =
			(i + 2 < argc) ? to_utf8(from_local8bit(argv[i + 2])) : string();

		int const remove = 1 + it->second(arg, arg2);

		// Now, remove used arguments by shifting
		// the following ones remove places down.
		if (remove > 0) {
			argc -= remove;
			for (int j = i; j < argc; ++j)
				argv[j] = argv[j + remove];
			--i;
		}
	}

	pimpl_->batch_command = batch;
}


FuncStatus getStatus(FuncRequest const & action)
{
	return LyX::ref().lyxFunc().getStatus(action);
}


void dispatch(FuncRequest const & action)
{
	LyX::ref().lyxFunc().dispatch(action);
}


BufferList & theBufferList()
{
	return LyX::ref().bufferList();
}


LyXFunc & theLyXFunc()
{
	return LyX::ref().lyxFunc();
}


Server & theServer()
{
	// FIXME: this should not be use_gui dependent
	LASSERT(use_gui, /**/);
	return LyX::ref().server();
}


ServerSocket & theServerSocket()
{
	// FIXME: this should not be use_gui dependent
	LASSERT(use_gui, /**/);
	return LyX::ref().socket();
}


KeyMap & theTopLevelKeymap()
{
	return LyX::ref().pimpl_->toplevel_keymap_;
}


Converters & theConverters()
{
	return  LyX::ref().converters();
}


Converters & theSystemConverters()
{
	return  LyX::ref().systemConverters();
}


Movers & theMovers()
{
	return  LyX::ref().pimpl_->movers_;
}


Mover const & getMover(string  const & fmt)
{
	return  LyX::ref().pimpl_->movers_(fmt);
}


void setMover(string const & fmt, string const & command)
{
	LyX::ref().pimpl_->movers_.set(fmt, command);
}


Movers & theSystemMovers()
{
	return  LyX::ref().pimpl_->system_movers_;
}


Messages & getMessages(string const & language)
{
	return LyX::ref().getMessages(language);
}


Messages & getGuiMessages()
{
	return LyX::ref().getGuiMessages();
}

} // namespace lyx

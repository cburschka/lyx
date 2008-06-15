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

#include "ConverterCache.h"
#include "Buffer.h"
#include "buffer_funcs.h"
#include "BufferList.h"
#include "Converter.h"
#include "CutAndPaste.h"
#include "debug.h"
#include "Encoding.h"
#include "ErrorList.h"
#include "Format.h"
#include "gettext.h"
#include "KeyMap.h"
#include "Language.h"
#include "Session.h"
#include "Color.h"
#include "callback.h"
#include "LyXAction.h"
#include "LyXFunc.h"
#include "Lexer.h"
#include "LyXRC.h"
#include "Server.h"
#include "ServerSocket.h"
#include "TextClassList.h"
#include "MenuBackend.h"
#include "Messages.h"
#include "Mover.h"
#include "ToolbarBackend.h"

#include "frontends/alert.h"
#include "frontends/Application.h"
#include "frontends/Dialogs.h"
#include "frontends/Gui.h"
#include "frontends/LyXView.h"

#include "support/environment.h"
#include "support/filetools.h"
#include "support/lyxlib.h"
#include "support/convert.h"
#include "support/ExceptionMessage.h"
#include "support/os.h"
#include "support/Package.h"
#include "support/Path.h"
#include "support/Systemcall.h"

#include <boost/bind.hpp>
#include <boost/filesystem/operations.hpp>

#include <algorithm>
#include <iostream>
#include <csignal>
#include <map>
#include <string>
#include <vector>


namespace lyx {

using support::addName;
using support::addPath;
using support::bformat;
using support::changeExtension;
using support::createDirectory;
using support::createLyXTmpDir;
using support::destroyDir;
using support::doesFileExist;
using support::FileName;
using support::fileSearch;
using support::getEnv;
using support::i18nLibFileSearch;
using support::libFileSearch;
using support::package;
using support::prependEnvPath;
using support::rtrim;
using support::Systemcall;

namespace Alert = frontend::Alert;
namespace os = support::os;
namespace fs = boost::filesystem;

using std::endl;
using std::for_each;
using std::map;
using std::make_pair;
using std::string;
using std::vector;

#ifndef CXX_GLOBAL_CSTD
using std::exit;
using std::signal;
using std::system;
#endif


/// are we using the GUI at all?
/**
* We default to true and this is changed to false when the export feature is used.
*/
bool use_gui = true;


namespace {

// Filled with the command line arguments "foo" of "-sysdir foo" or
// "-userdir foo".
string cl_system_support;
string cl_user_support;

std::string geometryArg;

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
	support::Path p(package().user_support());
	Systemcall one;
	one.startscript(Systemcall::Wait, configure_command);
	lyxerr << "LyX: " << to_utf8(_("Done!")) << endl;
}

} // namespace anon


/// The main application class private implementation.
struct LyX::Singletons
{
	Singletons()
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
	boost::scoped_ptr<KeyMap> toplevel_keymap_;
	///
	boost::scoped_ptr<Server> lyx_server_;
	///
	boost::scoped_ptr<ServerSocket> lyx_socket_;
	///
	boost::scoped_ptr<frontend::Application> application_;
	/// lyx session, containing lastfiles, lastfilepos, and lastopened
	boost::scoped_ptr<Session> session_;

	/// Files to load at start.
	vector<FileName> files_to_load_;

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
}


LyX & LyX::ref()
{
	BOOST_ASSERT(singleton_);
	return *singleton_;
}


LyX const & LyX::cref()
{
	BOOST_ASSERT(singleton_);
	return *singleton_;
}


LyX::LyX()
	: first_start(false)
{
	singleton_ = this;
	pimpl_.reset(new Singletons);
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
	BOOST_ASSERT(pimpl_->session_.get());
	return *pimpl_->session_.get();
}


Session const & LyX::session() const
{
	BOOST_ASSERT(pimpl_->session_.get());
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
	BOOST_ASSERT(pimpl_->lyx_server_.get());
	return *pimpl_->lyx_server_.get();
}


Server const & LyX::server() const
{
	BOOST_ASSERT(pimpl_->lyx_server_.get());
	return *pimpl_->lyx_server_.get();
}


ServerSocket & LyX::socket()
{
	BOOST_ASSERT(pimpl_->lyx_socket_.get());
	return *pimpl_->lyx_socket_.get();
}


ServerSocket const & LyX::socket() const
{
	BOOST_ASSERT(pimpl_->lyx_socket_.get());
	return *pimpl_->lyx_socket_.get();
}


frontend::Application & LyX::application()
{
	BOOST_ASSERT(pimpl_->application_.get());
	return *pimpl_->application_.get();
}


frontend::Application const & LyX::application() const
{
	BOOST_ASSERT(pimpl_->application_.get());
	return *pimpl_->application_.get();
}


KeyMap & LyX::topLevelKeymap()
{
	BOOST_ASSERT(pimpl_->toplevel_keymap_.get());
	return *pimpl_->toplevel_keymap_.get();
}


Converters & LyX::converters()
{
	return pimpl_->converters_;
}


Converters & LyX::systemConverters()
{
	return pimpl_->system_converters_;
}


KeyMap const & LyX::topLevelKeymap() const
{
	BOOST_ASSERT(pimpl_->toplevel_keymap_.get());
	return *pimpl_->toplevel_keymap_.get();
}


Messages & LyX::getMessages(std::string const & language)
{
	map<string, Messages>::iterator it = pimpl_->messages_.find(language);

	if (it != pimpl_->messages_.end())
		return it->second;

	std::pair<map<string, Messages>::iterator, bool> result =
			pimpl_->messages_.insert(std::make_pair(language, Messages(language)));

	BOOST_ASSERT(result.second);
	return result.first->second;
}


Messages & LyX::getGuiMessages()
{
	return pimpl_->messages_["GUI"];
}


void LyX::setGuiLanguage(std::string const & language)
{
	pimpl_->messages_["GUI"] = Messages(language);
}


Buffer const * const LyX::updateInset(Inset const * inset) const
{
	if (quitting || !inset)
		return 0;

	Buffer const * buffer_ptr = 0;
	vector<int> const & view_ids = pimpl_->application_->gui().viewIds();
	vector<int>::const_iterator it = view_ids.begin();
	vector<int>::const_iterator const end = view_ids.end();
	for (; it != end; ++it) {
		Buffer const * ptr =
			pimpl_->application_->gui().view(*it).updateInset(inset);
		if (ptr)
			buffer_ptr = ptr;
	}
	return buffer_ptr;
}


void LyX::hideDialogs(std::string const & name, Inset * inset) const
{
	if (quitting || !use_gui)
		return;

	vector<int> const & view_ids = pimpl_->application_->gui().viewIds();
	vector<int>::const_iterator it = view_ids.begin();
	vector<int>::const_iterator const end = view_ids.end();
	for (; it != end; ++it)
		pimpl_->application_->gui().view(*it).getDialogs().
			hide(name, inset);
}


int LyX::exec(int & argc, char * argv[])
{
	// Here we need to parse the command line. At least
	// we need to parse for "-dbg" and "-help"
	easyParse(argc, argv);

	try { support::init_package(to_utf8(from_local8bit(argv[0])),
			      cl_system_support, cl_user_support,
			      support::top_build_dir_is_one_level_up);
	} catch (support::ExceptionMessage const & message) {
		if (message.type_ == support::ErrorException) {
			Alert::error(message.title_, message.details_);
			exit(1);
		} else if (message.type_ == support::WarningException) {
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

		loadFiles();

		if (batch_command.empty() || pimpl_->buffer_list_.empty()) {
			prepareExit();
			return EXIT_SUCCESS;
		}

		BufferList::iterator begin = pimpl_->buffer_list_.begin();
		BufferList::iterator end = pimpl_->buffer_list_.end();

		bool final_success = false;
		for (BufferList::iterator I = begin; I != end; ++I) {
			Buffer * buf = *I;
			bool success = false;
			buf->dispatch(batch_command, &success);
			final_success |= success;
		}
		prepareExit();
		return !final_success;
	}

	// Let the frontend parse and remove all arguments that it knows
	pimpl_->application_.reset(createApplication(argc, argv));

	initGuiFont();

	// Reestablish our defaults, as Qt overwrites them
	// after createApplication()
	locale_init();

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

	// Set a flag that we do quitting from the program,
	// so no refreshes are necessary.
	quitting = true;

	// close buffers first
	pimpl_->buffer_list_.closeAll();

	// do any other cleanup procedures now
	if (package().temp_dir() != package().system_temp_dir()) {
		LYXERR(Debug::INFO) << "Deleting tmp dir "
				    << package().temp_dir().absFilename() << endl;

		if (!destroyDir(package().temp_dir())) {
			docstring const msg =
				bformat(_("Unable to remove the temporary directory %1$s"),
				from_utf8(package().temp_dir().absFilename()));
			Alert::warning(_("Unable to remove temporary directory"), msg);
		}
	}

	if (use_gui) {
		if (pimpl_->session_)
			pimpl_->session_->writeFile();
		pimpl_->session_.reset();
		pimpl_->lyx_server_.reset();
		pimpl_->lyx_socket_.reset();
	}

	// Kill the application object before exiting. This avoids crashes
	// when exiting on Linux.
	if (pimpl_->application_)
		pimpl_->application_.reset();
}


void LyX::earlyExit(int status)
{
	BOOST_ASSERT(pimpl_->application_.get());
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
	LYXERR(Debug::INIT) << "Initializing LyX::init..." << endl;
	bool success = init();
	LYXERR(Debug::INIT) << "Initializing LyX::init...done" << endl;
	if (!success)
		return EXIT_FAILURE;

	for (int argi = argc - 1; argi >= 1; --argi) {
		// get absolute path of file and add ".lyx" to
		// the filename if necessary
		pimpl_->files_to_load_.push_back(fileSearch(string(),
			os::internal_path(to_utf8(from_local8bit(argv[argi]))),
			"lyx", support::allow_unreadable));
	}

	if (first_start)
		pimpl_->files_to_load_.push_back(i18nLibFileSearch("examples", "splash.lyx"));

	return EXIT_SUCCESS;
}


void LyX::addFileToLoad(FileName const & fname)
{
	vector<FileName>::const_iterator cit = std::find(
		pimpl_->files_to_load_.begin(), pimpl_->files_to_load_.end(),
		fname);

	if (cit == pimpl_->files_to_load_.end())
		pimpl_->files_to_load_.push_back(fname);
}


void LyX::loadFiles()
{
	vector<FileName>::const_iterator it = pimpl_->files_to_load_.begin();
	vector<FileName>::const_iterator end = pimpl_->files_to_load_.end();

	for (; it != end; ++it) {
		if (it->empty())
			continue;

		Buffer * buf = pimpl_->buffer_list_.newBuffer(it->absFilename(), false);
		if (loadLyXFile(buf, *it)) {
			ErrorList const & el = buf->errorList("Parse");
			if (!el.empty())
				for_each(el.begin(), el.end(),
				boost::bind(&LyX::printError, this, _1));
		}
		else
			pimpl_->buffer_list_.release(buf);
	}
}


void LyX::execBatchCommands()
{
	// The advantage of doing this here is that the event loop
	// is already started. So any need for interaction will be
	// aknowledged.
	restoreGuiSession();

	// if reconfiguration is needed.
	if (textclasslist.empty()) {
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
		}
		pimpl_->lyxfunc_.dispatch(FuncRequest(LFUN_LYX_QUIT));
		return;
	}
	
	// Execute batch commands if available
	if (batch_command.empty())
		return;

	LYXERR(Debug::INIT) << "About to handle -x '"
		<< batch_command << '\'' << endl;

	pimpl_->lyxfunc_.dispatch(lyxaction.lookupFunc(batch_command));
}


void LyX::restoreGuiSession()
{
	LyXView * view = newLyXView();

	// if there is no valid class list, do not load any file. 
	if (textclasslist.empty())
		return;

	// if some files were specified at command-line we assume that the
	// user wants to edit *these* files and not to restore the session.
	if (!pimpl_->files_to_load_.empty()) {
		for_each(pimpl_->files_to_load_.begin(),
			pimpl_->files_to_load_.end(),
			bind(&LyXView::loadLyXFile, view, _1, true, false, false));
		// clear this list to save a few bytes of RAM
		pimpl_->files_to_load_.clear();
		pimpl_->session_->lastOpened().clear();
		return;
	}

	if (!lyxrc.load_session)
		return;

	vector<FileName> const & lastopened = pimpl_->session_->lastOpened().getfiles();
	// do not add to the lastfile list since these files are restored from
	// last session, and should be already there (regular files), or should
	// not be added at all (help files).
	for_each(lastopened.begin(), lastopened.end(),
		bind(&LyXView::loadLyXFile, view, _1, false, false, false));

	// clear this list to save a few bytes of RAM
	pimpl_->session_->lastOpened().clear();
}


LyXView * LyX::newLyXView()
{
	if (!lyx::use_gui)
		return 0;

	// determine windows size and position, from lyxrc and/or session
	// initial geometry
	unsigned int width = 690;
	unsigned int height = 510;
	// default icon size, will be overwritten by  stored session value
	unsigned int iconSizeXY = 0;
	int maximized = LyXView::NotMaximized;
	// first try lyxrc
	if (lyxrc.geometry_width != 0 && lyxrc.geometry_height != 0 ) {
		width = lyxrc.geometry_width;
		height = lyxrc.geometry_height;
	}
	// if lyxrc returns (0,0), then use session info
	else {
		string val = session().sessionInfo().load("WindowWidth");
		if (!val.empty())
			width = convert<unsigned int>(val);
		val = session().sessionInfo().load("WindowHeight");
		if (!val.empty())
			height = convert<unsigned int>(val);
		val = session().sessionInfo().load("WindowMaximized");
		if (!val.empty())
			maximized = convert<int>(val);
		val = session().sessionInfo().load("IconSizeXY");
		if (!val.empty())
			iconSizeXY = convert<unsigned int>(val);
	}

	// if user wants to restore window position
	int posx = -1;
	int posy = -1;
	if (lyxrc.geometry_xysaved) {
		string val = session().sessionInfo().load("WindowPosX");
		if (!val.empty())
			posx = convert<int>(val);
		val = session().sessionInfo().load("WindowPosY");
		if (!val.empty())
			posy = convert<int>(val);
	}

	if (!geometryArg.empty())
	{
		width = 0;
		height = 0;
	}

	// create the main window
	LyXView * view = &pimpl_->application_->createView(width, height, posx, posy, maximized, iconSizeXY, geometryArg);

	return view;
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
		support::abort();
	exit(0);
}

}


void LyX::printError(ErrorItem const & ei)
{
	docstring tmp = _("LyX: ") + ei.error + char_type(':')
		+ ei.description;
	std::cerr << to_utf8(tmp) << std::endl;
}


void LyX::initGuiFont()
{
	if (lyxrc.roman_font_name.empty())
		lyxrc.roman_font_name = pimpl_->application_->romanFontName();

	if (lyxrc.sans_font_name.empty())
		lyxrc.sans_font_name = pimpl_->application_->sansFontName();

	if (lyxrc.typewriter_font_name.empty())
		lyxrc.typewriter_font_name
			= pimpl_->application_->typewriterFontName();
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
	LYXERR(Debug::INIT) << "Reading layouts..." << endl;
	if (!LyXSetStyle())
		return false;

	if (use_gui) {
		// Set the language defined by the user.
		//setGuiLanguage(lyxrc.gui_language);

		// Set up bindings
		pimpl_->toplevel_keymap_.reset(new KeyMap);
		defaultKeyBindings(pimpl_->toplevel_keymap_.get());
		pimpl_->toplevel_keymap_->read(lyxrc.bind_file);

		pimpl_->lyxfunc_.initKeySequences(pimpl_->toplevel_keymap_.get());

		// Read menus
		if (!readUIFile(lyxrc.ui_file))
			return false;
	}

	if (lyxerr.debugging(Debug::LYXRC))
		lyxrc.print();

	os::windows_style_tex_paths(lyxrc.windows_style_tex_paths);
	if (!lyxrc.path_prefix.empty())
		prependEnvPath("PATH", lyxrc.path_prefix);

	FileName const document_path(lyxrc.document_path);
	if (doesFileExist(document_path) &&
	    fs::is_directory(document_path.toFilesystemEncoding()))
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

	LYXERR(Debug::INIT) << "LyX tmp dir: `"
			    << package().temp_dir().absFilename()
			    << '\'' << endl;

	LYXERR(Debug::INIT) << "Reading session information '.lyx/session'..." << endl;
	pimpl_->session_.reset(new Session(lyxrc.num_lastfiles));

	// This must happen after package initialization and after lyxrc is
	// read, therefore it can't be done by a static object.
	ConverterCache::init();

	return true;
}


void LyX::defaultKeyBindings(KeyMap  * kbmap)
{
	kbmap->bind("Right", FuncRequest(LFUN_CHAR_FORWARD));
	kbmap->bind("Left", FuncRequest(LFUN_CHAR_BACKWARD));
	kbmap->bind("Up", FuncRequest(LFUN_UP));
	kbmap->bind("Down", FuncRequest(LFUN_DOWN));

	kbmap->bind("Tab", FuncRequest(LFUN_CELL_FORWARD));
	kbmap->bind("C-Tab", FuncRequest(LFUN_CELL_SPLIT));
	kbmap->bind("~S-ISO_Left_Tab", FuncRequest(LFUN_CELL_BACKWARD));
	kbmap->bind("~S-BackTab", FuncRequest(LFUN_CELL_BACKWARD));

	kbmap->bind("Home", FuncRequest(LFUN_LINE_BEGIN));
	kbmap->bind("End", FuncRequest(LFUN_LINE_END));
	kbmap->bind("Prior", FuncRequest(LFUN_SCREEN_UP));
	kbmap->bind("Next", FuncRequest(LFUN_SCREEN_DOWN));

	kbmap->bind("Return", FuncRequest(LFUN_BREAK_PARAGRAPH));
	//kbmap->bind("~C-~S-~M-nobreakspace", FuncRequest(LFUN_PROTECTEDSPACE));

	kbmap->bind("Delete", FuncRequest(LFUN_CHAR_DELETE_FORWARD));
	kbmap->bind("BackSpace", FuncRequest(LFUN_CHAR_DELETE_BACKWARD));

	// kbmap->bindings to enable the use of the numeric keypad
	// e.g. Num Lock set
	//kbmap->bind("KP_0", FuncRequest(LFUN_SELF_INSERT));
	//kbmap->bind("KP_Decimal", FuncRequest(LFUN_SELF_INSERT));
	kbmap->bind("KP_Enter", FuncRequest(LFUN_BREAK_PARAGRAPH));
	//kbmap->bind("KP_1", FuncRequest(LFUN_SELF_INSERT));
	//kbmap->bind("KP_2", FuncRequest(LFUN_SELF_INSERT));
	//kbmap->bind("KP_3", FuncRequest(LFUN_SELF_INSERT));
	//kbmap->bind("KP_4", FuncRequest(LFUN_SELF_INSERT));
	//kbmap->bind("KP_5", FuncRequest(LFUN_SELF_INSERT));
	//kbmap->bind("KP_6", FuncRequest(LFUN_SELF_INSERT));
	//kbmap->bind("KP_Add", FuncRequest(LFUN_SELF_INSERT));
	//kbmap->bind("KP_7", FuncRequest(LFUN_SELF_INSERT));
	//kbmap->bind("KP_8", FuncRequest(LFUN_SELF_INSERT));
	//kbmap->bind("KP_9", FuncRequest(LFUN_SELF_INSERT));
	//kbmap->bind("KP_Divide", FuncRequest(LFUN_SELF_INSERT));
	//kbmap->bind("KP_Multiply", FuncRequest(LFUN_SELF_INSERT));
	//kbmap->bind("KP_Subtract", FuncRequest(LFUN_SELF_INSERT));
	kbmap->bind("KP_Right", FuncRequest(LFUN_CHAR_FORWARD));
	kbmap->bind("KP_Left", FuncRequest(LFUN_CHAR_BACKWARD));
	kbmap->bind("KP_Up", FuncRequest(LFUN_UP));
	kbmap->bind("KP_Down", FuncRequest(LFUN_DOWN));
	kbmap->bind("KP_Home", FuncRequest(LFUN_LINE_BEGIN));
	kbmap->bind("KP_End", FuncRequest(LFUN_LINE_END));
	kbmap->bind("KP_Prior", FuncRequest(LFUN_SCREEN_UP));
	kbmap->bind("KP_Next", FuncRequest(LFUN_SCREEN_DOWN));
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


void LyX::deadKeyBindings(KeyMap * kbmap)
{
	// bindKeyings for transparent handling of deadkeys
	// The keysyms are gotten from XFree86 X11R6
	kbmap->bind("~C-~S-~M-dead_acute", FuncRequest(LFUN_ACCENT_ACUTE));
	kbmap->bind("~C-~S-~M-dead_breve", FuncRequest(LFUN_ACCENT_BREVE));
	kbmap->bind("~C-~S-~M-dead_caron", FuncRequest(LFUN_ACCENT_CARON));
	kbmap->bind("~C-~S-~M-dead_cedilla", FuncRequest(LFUN_ACCENT_CEDILLA));
	kbmap->bind("~C-~S-~M-dead_abovering", FuncRequest(LFUN_ACCENT_CIRCLE));
	kbmap->bind("~C-~S-~M-dead_circumflex", FuncRequest(LFUN_ACCENT_CIRCUMFLEX));
	kbmap->bind("~C-~S-~M-dead_abovedot", FuncRequest(LFUN_ACCENT_DOT));
	kbmap->bind("~C-~S-~M-dead_grave", FuncRequest(LFUN_ACCENT_GRAVE));
	kbmap->bind("~C-~S-~M-dead_doubleacute", FuncRequest(LFUN_ACCENT_HUNGARIAN_UMLAUT));
	kbmap->bind("~C-~S-~M-dead_macron", FuncRequest(LFUN_ACCENT_MACRON));
	// nothing with this name
	// kbmap->bind("~C-~S-~M-dead_special_caron", LFUN_ACCENT_SPECIAL_CARON);
	kbmap->bind("~C-~S-~M-dead_tilde", FuncRequest(LFUN_ACCENT_TILDE));
	kbmap->bind("~C-~S-~M-dead_diaeresis", FuncRequest(LFUN_ACCENT_UMLAUT));
	// nothing with this name either...
	//kbmap->bind("~C-~S-~M-dead_underbar", FuncRequest(LFUN_ACCENT_UNDERBAR));
	kbmap->bind("~C-~S-~M-dead_belowdot", FuncRequest(LFUN_ACCENT_UNDERDOT));
	kbmap->bind("~C-~S-~M-dead_tie", FuncRequest(LFUN_ACCENT_TIE));
	kbmap->bind("~C-~S-~M-dead_ogonek",FuncRequest(LFUN_ACCENT_OGONEK));
}


namespace {

// return true if file does not exist or is older than configure.py.
bool needsUpdate(string const & file)
{
	// We cannot initialize configure_script directly because the package
	// is not initialized yet when  static objects are constructed.
	static string configure_script;
	static bool firstrun = true;
	if (firstrun) {
		configure_script = FileName(addName(
				package().system_support().absFilename(),
				"configure.py")).toFilesystemEncoding();
		firstrun = false;
	}

	FileName absfile = FileName(addName(
		package().user_support().absFilename(), file));
	return (!doesFileExist(absfile))
		|| (fs::last_write_time(configure_script)
		    > fs::last_write_time(absfile.toFilesystemEncoding()));
}

}


bool LyX::queryUserLyXDir(bool explicit_userdir)
{
	// Does user directory exist?
	string const user_support =
		package().user_support().toFilesystemEncoding();
	if (doesFileExist(package().user_support()) &&
	    fs::is_directory(user_support)) {
		first_start = false;

		return needsUpdate("lyxrc.defaults")
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
			  from_utf8(package().user_support().absFilename())))
	       << endl;

	if (!createDirectory(package().user_support(), 0755)) {
		// Failed, so let's exit.
		lyxerr << to_utf8(_("Failed to create directory. Exiting."))
		       << endl;
		earlyExit(EXIT_FAILURE);
	}

	return true;
}


bool LyX::readRcFile(string const & name)
{
	LYXERR(Debug::INIT) << "About to read " << name << "... ";

	FileName const lyxrc_path = libFileSearch(string(), name);
	if (!lyxrc_path.empty()) {

		LYXERR(Debug::INIT) << "Found in " << lyxrc_path << endl;

		if (lyxrc.read(lyxrc_path) < 0) {
			showFileError(name);
			return false;
		}
	} else
		LYXERR(Debug::INIT) << "Not found." << lyxrc_path << endl;
	return true;

}


// Read the ui file `name'
bool LyX::readUIFile(string const & name, bool include)
{
	enum Uitags {
		ui_menuset = 1,
		ui_toolbars,
		ui_toolbarset,
		ui_include,
		ui_last
	};

	struct keyword_item uitags[ui_last - 1] = {
		{ "include", ui_include },
		{ "menuset", ui_menuset },
		{ "toolbars", ui_toolbars },
		{ "toolbarset", ui_toolbarset }
	};

	// Ensure that a file is read only once (prevents include loops)
	static std::list<string> uifiles;
	std::list<string>::const_iterator it  = uifiles.begin();
	std::list<string>::const_iterator end = uifiles.end();
	it = std::find(it, end, name);
	if (it != end) {
		LYXERR(Debug::INIT) << "UI file '" << name
				    << "' has been read already. "
				    << "Is this an include loop?"
				    << endl;
		return false;
	}

	LYXERR(Debug::INIT) << "About to read " << name << "..." << endl;


	FileName ui_path;
	if (include) {
		ui_path = libFileSearch("ui", name, "inc");
		if (ui_path.empty())
			ui_path = libFileSearch("ui",
						changeExtension(name, "inc"));
	}
	else
		ui_path = libFileSearch("ui", name, "ui");

	if (ui_path.empty()) {
		LYXERR(Debug::INIT) << "Could not find " << name << endl;
		showFileError(name);
		return false;
	}

	uifiles.push_back(name);

	LYXERR(Debug::INIT) << "Found " << name
			    << " in " << ui_path << endl;
	Lexer lex(uitags, ui_last - 1);
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
			if (!readUIFile(file, true))
				return false;
			break;
		}
		case ui_menuset:
			menubackend.read(lex);
			break;

		case ui_toolbarset:
			toolbarbackend.readToolbars(lex);
			break;

		case ui_toolbars:
			toolbarbackend.readToolbarSettings(lex);
			break;

		default:
			if (!rtrim(lex.getString()).empty())
				lex.printError("LyX::ReadUIFile: "
					       "Unknown menu tag: `$$Token'");
			break;
		}
	}
	return true;
}


// Read the languages file `name'
bool LyX::readLanguagesFile(string const & name)
{
	LYXERR(Debug::INIT) << "About to read " << name << "..." << endl;

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
	LYXERR(Debug::INIT) << "About to read " << enc_name << " and "
			    << symbols_name << "..." << endl;

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
	cmdmap["-geometry"] = parse_geometry;

	for (int i = 1; i < argc; ++i) {
		std::map<string, cmd_helper>::const_iterator it
			= cmdmap.find(argv[i]);

		// don't complain if not found - may be parsed later
		if (it == cmdmap.end())
			continue;

		string const arg((i + 1 < argc) ? to_utf8(from_local8bit(argv[i + 1])) : string());
		string const arg2((i + 2 < argc) ? to_utf8(from_local8bit(argv[i + 2])) : string());

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

	batch_command = batch;
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
	BOOST_ASSERT(use_gui);
	return LyX::ref().server();
}


ServerSocket & theServerSocket()
{
	// FIXME: this should not be use_gui dependent
	BOOST_ASSERT(use_gui);
	return LyX::ref().socket();
}


KeyMap & theTopLevelKeymap()
{
	BOOST_ASSERT(use_gui);
	return LyX::ref().topLevelKeymap();
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


Mover const & getMover(std::string  const & fmt)
{
	return  LyX::ref().pimpl_->movers_(fmt);
}


void setMover(std::string const & fmt, std::string const & command)
{
	LyX::ref().pimpl_->movers_.set(fmt, command);
}


Movers & theSystemMovers()
{
	return  LyX::ref().pimpl_->system_movers_;
}


Messages & getMessages(std::string const & language)
{
	return LyX::ref().getMessages(language);
}


Messages & getGuiMessages()
{
	return LyX::ref().getGuiMessages();
}

} // namespace lyx

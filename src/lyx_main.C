/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 * 	 
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2000 The LyX Team.
 *
 * ====================================================== */

#include <config.h>

#include <cstdlib>
#include <csignal>

#ifdef __GNUG__
#pragma implementation
#endif

#include "version.h"
#include "lyx_main.h"
#include "lyx_gui.h"
#include "LyXView.h"
#include "lyxfunc.h"
#include "lyx_gui_misc.h"
#include "lyxrc.h"
#include "support/path.h"
#include "support/filetools.h"
#include "bufferlist.h"
#include "debug.h"
#include "support/FileInfo.h"
#include "lastfiles.h"
#include "intl.h"
#include "lyxserver.h"
#include "layout.h"
#include "gettext.h"
#include "kbmap.h"
#include "MenuBackend.h"
#include "ToolbarDefaults.h"
#include "lyxlex.h"

using std::endl;

extern void LoadLyXFile(string const &);
extern void QuitLyX();

string system_lyxdir;
string build_lyxdir;
string system_tempdir;
string user_lyxdir;	// Default $HOME/.lyx

// Should this be kept global? Asger says Yes.
DebugStream lyxerr;

LastFiles * lastfiles;

// This is the global bufferlist object
BufferList bufferlist;

LyXServer * lyxserver = 0;
// this should be static, but I need it in buffer.C
bool finished = false;	// flag, that we are quitting the program

// convenient to have it here.
kb_keymap * toplevel_keymap;


LyX::LyX(int * argc, char * argv[])
{
	// Prevent crash with --help
	lyxGUI = 0;
	lastfiles = 0;

	// Here we need to parse the command line. At least
	// we need to parse for "-dbg" and "-help"
	bool gui = easyParse(argc, argv);

	// Global bindings (this must be done as early as possible.) (Lgb)
	toplevel_keymap = new kb_keymap;
	defaultKeyBindings(toplevel_keymap);
	
	// Make the GUI object, and let it take care of the
	// command line arguments that concerns it.
	lyxerr[Debug::INIT] << "Initializing LyXGUI..." << endl;
	lyxGUI = new LyXGUI(this, argc, argv, gui);
	lyxerr[Debug::INIT] << "Initializing LyXGUI...done" << endl;

	// Now the GUI and LyX have taken care of their arguments, so
	// the only thing left on the command line should be
	// filenames. Let's check anyway.
	for (int argi = 1; argi < *argc ; ++argi) {
		if (argv[argi][0] == '-') {
			lyxerr << _("Wrong command line option `")
			       << argv[argi]
			       << _("'. Exiting.") << endl;
			exit(0);
		}
	}
	
	// Initialization of LyX (reads lyxrc and more)
	lyxerr[Debug::INIT] << "Initializing LyX::init..." << endl;
	init(argc, argv, gui);
	lyxerr[Debug::INIT] << "Initializing LyX::init...done" << endl;

	lyxGUI->init();

	// Load the files specified in the command line.
	if ((*argc) == 2) 
		lyxerr[Debug::INFO] << "Opening document..." << endl;
	else if ((*argc) > 2)
		lyxerr[Debug::INFO] << "Opening documents..." << endl;

	Buffer * last_loaded = 0;

	for (int argi = (*argc) - 1; argi >= 1; --argi) {
		Buffer * loadb = bufferlist.loadLyXFile(argv[argi]);
		if (loadb != 0) {
			last_loaded = loadb;
		}
	}

	if (first_start) {
		string splash = i18nLibFileSearch("examples", "splash.lyx");
		lyxerr[Debug::INIT] << "Opening splash document "
			       << splash << "..." << endl;
		Buffer * loadb = bufferlist.loadLyXFile(splash);
		if (loadb != 0) {
			last_loaded = loadb;
		}
	}

	if (last_loaded != 0) {
		lyxerr[Debug::INIT] << "Yes we loaded some files." << endl;
		if (lyxrc.use_gui)
			lyxGUI->regBuf(last_loaded);
	}

	// Execute batch commands if available
	if (!batch_command.empty()) {
		lyxerr << "About to handle -x '"
		       << batch_command << "'" << endl;

		// no buffer loaded, create one
		if (!last_loaded)
			last_loaded = bufferlist.newFile("tmpfile", string());

		// try to dispatch to last loaded buffer first
		bool dispatched	= last_loaded->Dispatch(batch_command);

		// if this was successful, return. 
		// Maybe we could do something more clever than aborting...
		if (dispatched) {
			lyxerr << "We are done!" << endl;
			QuitLyX();
			return;
		}

		// otherwise, let the GUI handle the batch command
		lyxGUI->regBuf(last_loaded);
		lyxGUI->getLyXView()->getLyXFunc()->Dispatch(batch_command);

		// fall through...
	}
	
	// Let the ball begin...
	lyxGUI->runTime();
}


// A destructor is always necessary  (asierra-970604)
LyX::~LyX()
{
	delete lastfiles;
	delete lyxGUI;
}


extern "C" void error_handler(int err_sig);

void LyX::init(int */*argc*/, char **argv, bool gui)
{
	// Install the signal handlers
	signal(SIGHUP, error_handler);
	signal(SIGFPE, error_handler);
	signal(SIGSEGV, error_handler);
	signal(SIGINT, error_handler);
	signal(SIGTERM, error_handler);

	//
	// Determine path of binary
	//

	string fullbinpath;
	string binpath = subst(argv[0], '\\', '/');
	string binname = OnlyFilename(argv[0]);
	// Sorry for system specific code. (SMiyata)
	if (suffixIs(binname, ".exe")) 
		binname.erase(binname.length()-4, string::npos);
	
	binpath = ExpandPath(binpath); // This expands ./ and ~/
	
	if (!AbsolutePath(binpath)) {
		string binsearchpath = GetEnvPath("PATH");
		// This will make "src/lyx" work always :-)
		binsearchpath += ";."; 
		binpath = FileOpenSearch(binsearchpath, argv[0]);
	}

	fullbinpath = binpath;
	binpath = MakeAbsPath(OnlyPath(binpath));

	// In case we are running in place and compiled with shared libraries
	if (suffixIs(binpath, "/.libs/"))
		binpath.erase(binpath.length()-6, string::npos);

	if (binpath.empty()) {
		lyxerr << _("Warning: could not determine path of binary.")
		       << "\n"
		       << _("If you have problems, try starting LyX with an absolute path.")
		       << endl;
	}
	lyxerr[Debug::INIT] << "Path of binary: " << binpath << endl;

	//
	// Determine system directory.
	//

	// Directories are searched in this order:
	// 1) -sysdir command line parameter
	// 2) LYX_DIR_11x environment variable
	// 3) Maybe <path of binary>/TOP_SRCDIR/lib
	// 4) <path of binary>/../share/<name of binary>/
	// 4a) repeat 4 after following the Symlink if <path of
	//     binary> is a symbolic link.
	// 5) hardcoded lyx_dir
	// The directory is checked for the presence of the file
	// "chkconfig.ltx", and if that is present, the directory
	// is accepted as the system directory.
	// If no chkconfig.ltx file is found, a warning is given,
	// and the hardcoded lyx_dir is used.

	// If we had a command line switch, system_lyxdir is already set
	string searchpath;
	if (!system_lyxdir.empty())
		searchpath= MakeAbsPath(system_lyxdir) + ';';

	// LYX_DIR_11x environment variable
	string lyxdir = GetEnvPath("LYX_DIR_11x");
	
	if (!lyxdir.empty()) {
		lyxerr[Debug::INIT] << "LYX_DIR_11x: " << lyxdir << endl;
		searchpath += lyxdir + ';';
	}

	// <path of binary>/TOP_SRCDIR/lib
	build_lyxdir = MakeAbsPath("../lib", binpath);
	if (!FileSearch(build_lyxdir, "lyxrc.defaults").empty()) {
		searchpath += MakeAbsPath(AddPath(TOP_SRCDIR, "lib"),
					  binpath) + ';';
		lyxerr[Debug::INIT] << "Checking whether LyX is run in "
			"place... yes" << endl;
	} else {
		lyxerr[Debug::INIT]
			<< "Checking whether LyX is run in place... no"
			<< endl;
		build_lyxdir.erase();
	}

        bool FollowLink;
	do {
	  // Path of binary/../share/name of binary/
		searchpath += NormalizePath(AddPath(binpath, "../share/") + 
		      OnlyFilename(binname)) + ';';

	  // Follow Symlinks
		FileInfo file(fullbinpath, true);
		FollowLink = file.isLink();
		if (FollowLink) {
			string Link;
			if (LyXReadLink(fullbinpath, Link)) {
				fullbinpath = Link;
				binpath = MakeAbsPath(OnlyPath(fullbinpath));
			}
			else {
				FollowLink = false;
			}
		}
	} while (FollowLink);

	// Hardcoded dir
	searchpath += LYX_DIR;

	// If debugging, show complete search path
	lyxerr[Debug::INIT] << "System directory search path: "
			    << searchpath << endl;

	string const filename = "chkconfig.ltx";
	string temp = FileOpenSearch(searchpath, filename, string());
	system_lyxdir = OnlyPath(temp);

	// Reduce "path/../path" stuff out of system directory
	system_lyxdir = NormalizePath(system_lyxdir);

	bool path_shown = false;

	// Warn if environment variable is set, but unusable
	if (!lyxdir.empty()) {
		if (system_lyxdir != NormalizePath(lyxdir)) {
			lyxerr <<_("LYX_DIR_11x environment variable no good.")
			       << '\n'
			       << _("System directory set to: ") 
			       << system_lyxdir << endl;
			path_shown = true;
		}
	}

	// Warn the user if we couldn't find "chkconfig.ltx"
	if (system_lyxdir == "./") {
		lyxerr <<_("LyX Warning! Couldn't determine system directory.")
		       <<_("Try the '-sysdir' command line parameter or")
		       <<_("set the environment variable LYX_DIR_11x to the "
			   "LyX system directory")
		       << _("containing the file `chkconfig.ltx'.") << endl;
		if (!path_shown)
			lyxerr << _("Using built-in default ") 
			       << LYX_DIR << _(" but expect problems.")
			       << endl;
		else
			lyxerr << _("Expect problems.") << endl;
		system_lyxdir = LYX_DIR;
		path_shown = true;
	}

	// Report the system directory if debugging is on
	if (!path_shown)
		lyxerr[Debug::INIT] << "System directory: '"
				    << system_lyxdir << '\'' << endl; 

	//
	// Determine user lyx-dir
	//
	
	// Directories are searched in this order:
	// 1) -userdir command line parameter
	// 2) LYX_USERDIR_11x environment variable
	// 3) $HOME/.<name of binary>

	// If we had a command line switch, user_lyxdir is already set
	bool explicit_userdir = true;
	if (user_lyxdir.empty()) {

	// LYX_USERDIR_11x environment variable
		user_lyxdir = GetEnvPath("LYX_USERDIR_11x");

	// default behaviour
		if (user_lyxdir.empty())
			user_lyxdir = AddPath(GetEnvPath("HOME"),
							string(".") + PACKAGE);
			explicit_userdir = false;
	}

	lyxerr[Debug::INIT] << "User LyX directory: '" 
			    <<  user_lyxdir << '\'' << endl;

	// Check that user LyX directory is ok.
	queryUserLyXDir(explicit_userdir);

	//
	// Load the layouts first
	//

	lyxerr[Debug::INIT] << "Reading layouts..." << endl;
	LyXSetStyle();

	//
	// Shine up lyxrc defaults
	//

	// Default template path: system_dir/templates
	if (lyxrc.template_path.empty()){
		lyxrc.template_path = AddPath(system_lyxdir, "templates");
	}
   
	// Default lastfiles file: $HOME/.lyx/lastfiles
	if (lyxrc.lastfiles.empty()){
		lyxrc.lastfiles = AddName(user_lyxdir, "lastfiles");
	}

	// Disable gui when either lyxrc or easyparse says so
	if (!gui)
		lyxrc.use_gui = false;
 
        // Calculate screen dpi as average of x-DPI and y-DPI:
	if (lyxrc.use_gui) {
		lyxrc.dpi = getScreenDPI();
		lyxerr[Debug::INIT] << "DPI setting detected to be "
                                                << lyxrc.dpi + 0.5 << endl;
	} else {
		lyxrc.dpi = 1; // I hope this is safe
	}

	//
	// Read configuration files
	//

	ReadRcFile("lyxrc.defaults");
//	system_lyxrc = lyxrc;
	// If there is a preferences file we read that instead
	// of the old lyxrc file.
	if (!ReadRcFile("preferences"))
	    ReadRcFile("lyxrc");

	// Ensure that we have really read a bind file, so that LyX is
	// usable.
	if (!lyxrc.hasBindFile)
		lyxrc.ReadBindFile();

	// Read menus
	ReadUIFile(lyxrc.ui_file);

	// Bind the X dead keys to the corresponding LyX functions if
	// necessary. 
	if (lyxrc.override_x_deadkeys)
		deadKeyBindings(toplevel_keymap);

	if (lyxerr.debugging(Debug::LYXRC)) {
		lyxrc.print();
	}

	// Create temp directory	
	system_tempdir = CreateLyXTmpDir(lyxrc.tempdir_path);
	if (lyxerr.debugging(Debug::INIT)) {
		lyxerr << "LyX tmp dir: `" << system_tempdir << '\'' << endl;
	}

	// load the lastfiles mini-database
	lyxerr[Debug::INIT] << "Reading lastfiles `"
			    << lyxrc.lastfiles << "'..." << endl; 
	lastfiles = new LastFiles(lyxrc.lastfiles, 
				  lyxrc.check_lastfiles,
				  lyxrc.num_lastfiles);

	// start up the lyxserver. (is this a bit early?) (Lgb)
	// 0.12 this will be way to early, we need the GUI to be initialized
	// first, so move it for now.
	// lyxserver = new LyXServer;
}

// These are the default bindings known to LyX
void LyX::defaultKeyBindings(kb_keymap  * kbmap)
{
	kbmap->bind("Right", LFUN_RIGHT);
	kbmap->bind("Left", LFUN_LEFT);
	kbmap->bind("Up", LFUN_UP);
	kbmap->bind("Down", LFUN_DOWN);
	
	kbmap->bind("Tab", LFUN_TAB);
	
	kbmap->bind("Home", LFUN_HOME);
	kbmap->bind("End", LFUN_END);
	kbmap->bind("Prior", LFUN_PRIOR);
	kbmap->bind("Next", LFUN_NEXT);
	
	kbmap->bind("Return", LFUN_BREAKPARAGRAPH);
	kbmap->bind("~C-~S-~M-nobreakspace", LFUN_PROTECTEDSPACE);
	
	kbmap->bind("Delete", LFUN_DELETE);
	kbmap->bind("BackSpace", LFUN_BACKSPACE);
	
	// kbmap->bindings to enable the use of the numeric keypad
	// e.g. Num Lock set
	kbmap->bind("KP_0", LFUN_SELFINSERT);
	kbmap->bind("KP_Decimal", LFUN_SELFINSERT);
	kbmap->bind("KP_Enter", LFUN_SELFINSERT);
	kbmap->bind("KP_1", LFUN_SELFINSERT);
	kbmap->bind("KP_2", LFUN_SELFINSERT);
	kbmap->bind("KP_3", LFUN_SELFINSERT);
	kbmap->bind("KP_4", LFUN_SELFINSERT);
	kbmap->bind("KP_5", LFUN_SELFINSERT);
	kbmap->bind("KP_6", LFUN_SELFINSERT);
	kbmap->bind("KP_Add", LFUN_SELFINSERT);
	kbmap->bind("KP_7", LFUN_SELFINSERT);
	kbmap->bind("KP_8", LFUN_SELFINSERT);
	kbmap->bind("KP_9", LFUN_SELFINSERT);
	kbmap->bind("KP_Divide", LFUN_SELFINSERT);
	kbmap->bind("KP_Multiply", LFUN_SELFINSERT);
	kbmap->bind("KP_Subtract", LFUN_SELFINSERT);
	
	/* Most self-insert keys are handled in the 'default:' section of
	 * WorkAreaKeyPress - so we don't have to define them all.
	 * However keys explicit decleared as self-insert are
	 * handled seperatly (LFUN_SELFINSERT.) Lgb. */
	
        kbmap->bind("C-Tab", LFUN_TABINSERT);  // ale970515
	kbmap->bind("S-Tab", LFUN_SHIFT_TAB);  // jug20000522
}

// LyX can optionally take over the handling of deadkeys
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



// This one is not allowed to use anything on the main form, since that
// one does not exist yet. (Asger)
void LyX::queryUserLyXDir(bool explicit_userdir)
{
	// Does user directory exist?
	FileInfo fileInfo(user_lyxdir);
	if (fileInfo.isOK() && fileInfo.isDir()) {
		first_start = false;
		return;
	} else {
		first_start = true;
	}
	
	// Nope
	// Different wording if the user specifically requested a directory
	if (!AskQuestion( explicit_userdir
			 ? _("You have specified an invalid LyX directory.")
			 : _("You don't have a personal LyX directory.") ,

			 _("It is needed to keep your own configuration."),
			 _("Should I try to set it up for you (recommended)?"))) {
		lyxerr << _("Running without personal LyX directory.") << endl;
		// No, let's use $HOME instead.
		user_lyxdir = GetEnvPath("HOME");
		return;
	}

	// Tell the user what is going on
	lyxerr << _("LyX: Creating directory ") << user_lyxdir
	       << _(" and running configure...") << endl;

	// Create directory structure
	if (!createDirectory(user_lyxdir, 0755)) {
		// Failed, let's use $HOME instead.
		user_lyxdir = GetEnvPath("HOME");
		lyxerr << _("Failed. Will use ") << user_lyxdir
		       << _(" instead.") << endl;
		return;
	}

	// Run configure in user lyx directory
	Path p(user_lyxdir);
	::system(AddName(system_lyxdir, "configure").c_str());
	lyxerr << "LyX: " << _("Done!") << endl;
}


// Read the rc file `name'
bool LyX::ReadRcFile(string const & name)
{
	lyxerr[Debug::INIT] << "About to read " << name << "..." << endl;
	
	string lyxrc_path = LibFileSearch(string(), name);
	if (!lyxrc_path.empty()){
	        lyxerr[Debug::INIT] << "Found " << name
				    << " in " << lyxrc_path << endl;
		if (lyxrc.read(lyxrc_path) < 0) { 
		        WriteAlert(_("LyX Warning!"), 
				   _("Error while reading ")+lyxrc_path+".",
				   _("Using built-in defaults."));
			return false;
		}
		return true;
	} else
	  	lyxerr[Debug::INIT] << "Could not find " << name << endl;
	return false;
}


// Read the ui file `name'
void LyX::ReadUIFile(string const & name)
{
	enum Uitags {
		ui_menuset = 1,
		ui_toolbar,
		ui_last
	};

	struct keyword_item uitags[ui_last-1] = {
		{ "menuset", ui_menuset },
		{ "toolbar", ui_toolbar }
	};

	lyxerr[Debug::INIT] << "About to read " << name << "..." << endl;
	
	string ui_path = LibFileSearch("ui", name, "ui");

	if (ui_path.empty()) {
	  	lyxerr[Debug::INIT] << "Could not find " << name << endl;
		return;
	}
	
	lyxerr[Debug::INIT] << "Found " << name
			    << " in " << ui_path << endl;
	LyXLex lex(uitags, ui_last - 1);
	lex.setFile(ui_path);
	if (!lex.IsOK()) {
		lyxerr << "Unable to set LyXLeX for ui file: " << ui_path
		       << endl;
	}
	
	if (lyxerr.debugging(Debug::PARSER))
		lex.printTable(lyxerr);

	while (lex.IsOK()) {
		switch(lex.lex()) {
		case ui_menuset: 
			menubackend.read(lex);
			break;

		case ui_toolbar:
			toolbardefaults.read(lex);
			break;

		default:
			lex.printError("LyX::ReadUFile: "
				       "Unknown menu tag: `$$Token'");
			break;
		}
	}
}


// Set debugging level and report result to user
void setDebuggingLevel(string const & dbgLevel)
{
	lyxerr << _("Setting debug level to ") <<  dbgLevel << endl;
	lyxerr.level(Debug::value(dbgLevel));
	Debug::showLevel(lyxerr, lyxerr.level());
}


// Give command line help
void commandLineHelp()
{
	lyxerr << "LyX " LYX_VERSION << " of " LYX_RELEASE << endl;
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
		  "Check the LyX man page for more options.") << endl;
}

bool LyX::easyParse(int * argc, char * argv[])
{
	bool gui = true;
	int removeargs = 0; // used when options are read
	for(int i = 1; i < *argc; ++i) {
		string arg = argv[i];

		// Check for -dbg int
		if (arg == "-dbg") {
			if (i + 1 < *argc) {
				setDebuggingLevel(argv[i + 1]);
				removeargs = 2;
			} else {
				lyxerr << _("List of supported debug flags:")
				       << endl;
				Debug::showTags(lyxerr);
				exit(0);
			}
		} 
		// Check for "-sysdir"
		else if (arg == "-sysdir") {
			if (i + 1 < *argc) {
				system_lyxdir = argv[i + 1];
				removeargs = 2;
			} else {
				lyxerr << _("Missing directory for -sysdir switch!") 
				       << endl;
				exit(0);
			}
		}
		// Check for "-userdir"
		else if (arg == "-userdir") {
			if (i + 1 < *argc) {
				user_lyxdir = argv[i + 1];
				removeargs = 2;
			} else {
				lyxerr << _("Missing directory for -userdir switch!")
				       << endl;
				exit(0);
			}
		}
		// Check for --help or -help
		else if (arg == "--help" || arg == "-help") {
			commandLineHelp();
			exit(0);
		} 
		// Check for "-nw": No XWindows as for emacs this should
		// give a LyX that could be used in a terminal window.
		//else if (arg == "-nw") {
		//	gui = false;
		//}

		// Check for "-x": Execute commands
		else if (arg == "-x" || arg == "--execute") {
			if (i + 1 < *argc) {
				batch_command = string(argv[i + 1]);
				removeargs = 2;
			}
			else
				lyxerr << _("Missing command string after  -x switch!") << endl;

			// Argh. Setting gui to false segfaults..
			//gui = false;
		}

		else if (arg == "-e" || arg == "--export") {
			if (i + 1 < *argc) {
				string type(argv[i+1]);
				removeargs = 2;
				batch_command = "buffer-export " + type;
			} else
				lyxerr << _("Missing file type [eg latex, "
					    "ps...] after ")
				       << arg << _(" switch!") << endl;
		}
		else if (arg == "--import") {
			if (i + 1 < *argc) {
				string type(argv[i+1]);
				string file(argv[i+2]);
				removeargs = 3;
	
				batch_command = "buffer-import " + type + " " + file;
				lyxerr << "batch_command: "
				       << batch_command << endl;

			} else
				lyxerr << _("Missing type [eg latex, "
					    "ps...] after ")
				       << arg << _(" switch!") << endl;
		}

		if (removeargs > 0) {
			// Now, remove used arguments by shifting
			// the following ones removeargs places down.
			(*argc) -= removeargs;
			for (int j = i; j < (*argc); ++j)
				argv[j] = argv[j + removeargs];
			--i; // After shift, check this number again.
			removeargs = 0;
		}

	}

	return gui;
}


extern "C"
void error_handler(int err_sig)
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
			"Sorry, you have found a bug in LyX."
			" If possible, please read 'Known bugs'\n"
			"under the Help menu and then send us "
			"a full bug report. Thanks!" << endl;
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

	bufferlist.emergencyWriteAll();

	lyxerr << "Bye." << endl;
	if(err_sig!= SIGHUP && 
	   (!GetEnv("LYXDEBUG").empty() || err_sig == SIGSEGV))
		lyx::abort();
	exit(0);
}

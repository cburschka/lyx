/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *	    Copyright 1995 Matthias Ettrich
 *          Copyright 1995-1999 The LyX Team.
 *
 * ======================================================*/

#include <config.h>

#include <cstdlib>
#include <csignal>

#include "version.h"
#include "lyx_main.h"
#include "lyx_gui.h"
#include "lyx_gui_misc.h"
#include "lyxrc.h"
#include "pathstack.h"
#include "support/filetools.h"
#include "bufferlist.h"
#include "error.h"
#include "support/FileInfo.h"
#include "lastfiles.h"
#include "intl.h"
#include "lyxserver.h"
#include "layout.h"
#include "gettext.h"

extern void LoadLyXFile(string const &);

string system_lyxdir;
string build_lyxdir;
string system_tempdir;
string user_lyxdir;	// Default $HOME/.lyx

// Should this be kept global? Asger says Yes.
Error lyxerr;

LastFiles *lastfiles;
LyXRC *lyxrc;

// This is the global bufferlist object
BufferList bufferlist;

LyXServer *lyxserver = 0;
// this should be static, but I need it in buffer.C
bool finished = false;	// flag, that we are quitting the program

// convenient to have it here.
kb_keymap *toplevel_keymap;

// from spellchecker.C
#if 0
extern void sigchldhandler(int sig);
#endif

LyX::LyX(int *argc, char *argv[])
{
	// Prevent crash with --help
	lyxGUI = 0;
	lastfiles = 0;

	// Here we need to parse the command line. At least
	// we need to parse for "-dbg" and "-help"
	bool gui = easyParse(argc, argv);

	// Global bindings (this must be done as early as possible.) (Lgb)
	toplevel_keymap = new kb_keymap;

	lyxerr.debug("Initializing lyxrc");
	lyxrc = new LyXRC;

	// Make the GUI object, and let it take care of the
	// command line arguments that concerns it.
	lyxerr.debug("Initializing LyXGUI...");
	lyxGUI = new LyXGUI(this, argc, argv, gui);
	lyxerr.debug("Initializing LyXGUI...done");

	// Initialization of LyX (reads lyxrc and more)
	lyxerr.debug("Initializing LyX::init...");
	init(argc, argv);
	lyxerr.debug("Initializing LyX::init...done");

	lyxGUI->init();

	// Load the files specified in the command line.
	// Now the GUI and LyX have taken care of their arguments, so
	// the only thing left on the command line should be
	// filenames.
	if ((*argc)==2) 
		lyxerr.debug("Opening document...");
	else if ((*argc)>2)
		lyxerr.debug("Opening documents...");

	Buffer *last_loaded = 0;

	for (int argi = (*argc) - 1; argi >= 1; argi--) {
		Buffer * loadb = bufferlist.loadLyXFile(argv[argi]);
		if (loadb != 0) {
			last_loaded = loadb;
		}
	}

	if (first_start) {
		string splash = i18nLibFileSearch("examples", "splash.lyx");
		lyxerr.debug("Opening splash document "+splash+"...");
		Buffer * loadb = bufferlist.loadLyXFile(splash);
		if (loadb != 0) {
			last_loaded = loadb;
		}
	}

	if (last_loaded != 0) {
		lyxerr.debug("Yes we loaded some files.");
		lyxGUI->regBuf(last_loaded);
	}
	
	// Let the ball begin...
	lyxGUI->runTime();
}


// A destructor is always necessary  (asierra-970604)
LyX::~LyX()
{
	if (lastfiles)
		delete lastfiles;

	if (lyxGUI)
		delete lyxGUI;
}


extern "C" void error_handler(int err_sig);

void LyX::init(int */*argc*/, char **argv)
{
	// Install the signal handlers
	signal(SIGHUP, error_handler);
	signal(SIGFPE, error_handler);
	signal(SIGSEGV, error_handler);
	signal(SIGINT, error_handler);
	signal(SIGTERM, error_handler);

#if 0
	// Install the SIGCHLD handler
	act_.sa_handler = sigchldhandler;
        //act_.sa_mask = SIGCHLD;
        act_.sa_flags = 0;
	//act_.sa_flags = SA_RESTART; //perhaps
        sigaction(SIGCHLD, &act_, 0);
#endif
	//
	// Determine path of binary
	//

	string fullbinpath, binpath = argv[0];
	subst(binpath, '\\', '/');
	string binname = OnlyFilename(argv[0]);
	// Sorry for system specific code. (SMiyata)
	if (suffixIs(binname, ".exe")) binname.erase(binname.length()-4, string::npos);
	
	binpath = ExpandPath(binpath); // This expands ./ and ~/
	
	if (!AbsolutePath(binpath)) {
		string binsearchpath = GetEnvPath("PATH");
		binsearchpath += ";."; // This will make "src/lyx" work always :-)
		binpath = FileOpenSearch(binsearchpath, argv[0]);
	}

	fullbinpath = binpath;
	binpath = MakeAbsPath(OnlyPath(binpath));

	if (binpath.empty()) {
		lyxerr.print(_("Warning: could not determine path of binary."));
		lyxerr.print(_("If you have problems, try starting LyX with an absolute path."));
	}
	lyxerr.debug("Path of binary: " + binpath);

	//
	// Determine system directory.
	//

	// Directories are searched in this order:
	// 1) -sysdir command line parameter
	// 2) LYX_DIR_10x environment variable
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
		searchpath=MakeAbsPath(system_lyxdir) + ';';

	// LYX_DIR_10x environment variable
	string lyxdir = GetEnvPath("LYX_DIR_10x");
	
	if (!lyxdir.empty()) {
		lyxerr.debug("LYX_DIR_10x: " + lyxdir, Error::INIT);
		searchpath += lyxdir + ';';
	}

	// <path of binary>/TOP_SRCDIR/lib
	build_lyxdir = MakeAbsPath("../lib", binpath);
	if (!FileSearch(build_lyxdir, "lyxrc.defaults").empty()) {
		searchpath += MakeAbsPath(AddPath(TOP_SRCDIR, "lib"),
					  binpath) + ';';
		lyxerr.debug("Checking whether LyX is run in "
			      "place... yes", Error::INIT);
	} else {
		lyxerr.debug("Checking whether LyX is run in place... no",
			      Error::INIT);
		build_lyxdir.erase();
	}


        bool FollowLink;
	do {
	  // Path of binary/../share/name of binary/
		searchpath += NormalizePath(AddPath(binpath, "../share/") + 
		      OnlyFilename(binname)) + ';';

	  // Follow Symlinks
		FileInfo file(fullbinpath,true);
		FollowLink = file.isLink();
		if (FollowLink) {
			string Link;
			if (LyXReadLink(fullbinpath,Link)) {
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
	lyxerr.debug("System directory search path: "+searchpath, Error::INIT);

	string const filename = "chkconfig.ltx";
	string temp = FileOpenSearch(searchpath, filename, string());
	system_lyxdir = OnlyPath(temp);

	// Reduce "path/../path" stuff out of system directory
	system_lyxdir = NormalizePath(system_lyxdir);

	bool path_shown = false;

	// Warn if environment variable is set, but unusable
	if (!lyxdir.empty()) {
		if (system_lyxdir != NormalizePath(lyxdir)) {
			lyxerr.print(_("LYX_DIR_10x environment variable no good."));
			lyxerr.print(_("System directory set to: ") 
				     + system_lyxdir);
			path_shown = true;
		}
	}

	// Warn the user if we couldn't find "chkconfig.ltx"
	if (system_lyxdir.empty()) {
		lyxerr.print(_("LyX Warning! Couldn't determine system directory."));
		lyxerr.print(_("Try the '-sysdir' command line parameter or"));
		lyxerr.print(_("set the environment variable LYX_DIR_10x to the "
			      "LyX system directory"));
		lyxerr.print(_("containing the file `chkconfig.ltx'."));
		if (!path_shown)
			lyxerr.print(_("Using built-in default ") 
				     + string(LYX_DIR) + _(" but expect problems."));
		else
			lyxerr.print(_("Expect problems."));
		system_lyxdir = LYX_DIR;
		path_shown = true;
	}

	// Report the system directory if debugging is on
	if (!path_shown)
		lyxerr.debug("System directory: '" + system_lyxdir + '\'', 
			      Error::INIT);

	//
	// Determine user lyx-dir
	//
	
	user_lyxdir = AddPath(GetEnvPath("HOME"), string(".") + LYX_NAME);
	lyxerr.debug("User LyX directory: '" 
		     + user_lyxdir + '\'', Error::INIT);

	// Check that user LyX directory is ok.
	queryUserLyXDir();

	//
	// Load the layouts first
	//

	lyxerr.debug("Reading layouts...", Error::INIT);
	LyXSetStyle();

	//
	// Shine up lyxrc defaults
	//

	// Default template path: system_dir/templates
	if (lyxrc->template_path.empty()){
		lyxrc->template_path = AddPath(system_lyxdir, "templates");
	}
   
	// Default lastfiles file: $HOME/.lyx/lastfiles
	if (lyxrc->lastfiles.empty()){
		lyxrc->lastfiles = AddName(user_lyxdir, "lastfiles");
	}

	// Calculate screen dpi as average of x-DPI and y-DPI:
	Screen * scr=(DefaultScreenOfDisplay(fl_get_display()));
	lyxrc->dpi = ((HeightOfScreen(scr)* 25.4 / HeightMMOfScreen(scr)) +
		      (WidthOfScreen(scr)* 25.4 / WidthMMOfScreen(scr))) / 2;
	lyxerr.debug(string("DPI setting detected to be ") + 
		      tostr(lyxrc->dpi+0.5));

	//
	// Read configuration files
	//

	ReadRcFile("lyxrc.defaults");
	ReadRcFile("lyxrc");

	// Ensure that we have really read a bind file, so that LyX is
	// usable.
	if (!lyxrc->hasBindFile)
		lyxrc->ReadBindFile();

	if (lyxerr.debugging(Error::LYXRC)) {
		lyxrc->Print();
	}

	// Create temp directory	
	system_tempdir = CreateLyXTmpDir(lyxrc->tempdir_path);
	if (lyxerr.debugging(Error::INIT)) {
		lyxerr.print("LyX tmp dir: `" + system_tempdir + '\'');
	}

	// load the lastfiles mini-database
	lyxerr.debug("Reading lastfiles `" + lyxrc->lastfiles + "'...", 
		      Error::INIT);
	lastfiles = new LastFiles(lyxrc->lastfiles, 
				  lyxrc->check_lastfiles,
				  lyxrc->num_lastfiles);

	// start up the lyxserver. (is this a bit early?) (Lgb)
	// 0.12 this will be way to early, we need the GUI to be initialized
	// first, so move it for now.
	// lyxserver = new LyXServer;
}


// This one is not allowed to use anything on the main form, since that
// one does not exist yet. (Asger)
void LyX::queryUserLyXDir()
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
	if (!AskQuestion(_("You don't have a personal LyX directory."),
			 _("It is needed to keep your own configuration."),
			 _("Should I try to set it up for you (recommended)?"))) {
		lyxerr.print(_("Running without personal LyX directory."));
		// No, let's use $HOME instead.
		user_lyxdir = GetEnvPath("HOME");
		return;
	}

	// Tell the user what is going on
	lyxerr.print(_("LyX: Creating directory ") + user_lyxdir + _(" and running configure..."));

	// Create directory structure
	if (!createDirectory(user_lyxdir, 0755)) {
		// Failed, let's use $HOME instead.
		user_lyxdir = GetEnvPath("HOME");
		lyxerr.print(_("Failed. Will use ") + user_lyxdir + _(" instead."));
		return;
	}

	// Run configure in user lyx directory
	PathPush(user_lyxdir);
	system(AddName(system_lyxdir,"configure").c_str());
	PathPop();
	lyxerr.print(string("LyX: ") + _("Done!"));
}


// Read the rc file `name'
void LyX::ReadRcFile(string const & name)
{
	lyxerr.debug("About to read "+name+"...", Error::INIT);
	
	string lyxrc_path = LibFileSearch(string(), name);
	if (!lyxrc_path.empty()){
	        lyxerr.debug("Found "+name+" in " + lyxrc_path, Error::INIT);
		if (lyxrc->Read(lyxrc_path) < 0) { 
		        WriteAlert(_("LyX Warning!"), 
				   _("Error while reading ")+lyxrc_path+".",
				   _("Using built-in defaults."));
		}
	} else
	  	lyxerr.debug("Could not find "+name, Error::INIT);
}


// Set debugging level and report result to user
void setDebuggingLevel(int dbgLevel)
{
	lyxerr.print(string(_("Setting debug level to ")) + tostr(dbgLevel));
	lyxerr.setDebugLevel(dbgLevel);
	lyxerr.debug(string("Debugging INFO #") + tostr(Error::INFO),
		     Error::INFO);
	lyxerr.debug(string("Debugging INIT #") + tostr(Error::INIT),
		     Error::INIT);
	lyxerr.debug(string("Debugging KEY #") + tostr(Error::KEY),
		     Error::KEY);
	lyxerr.debug(string("Debugging TOOLBAR #") + tostr(Error::TOOLBAR), 
		     Error::TOOLBAR);
	lyxerr.debug(string("Debugging LEX and PARSER #") +
		     tostr(Error::LEX_PARSER),
		     Error::LEX_PARSER);
	lyxerr.debug(string("Debugging LYXRC #") + tostr(Error::LYXRC),
		     Error::LYXRC);
	lyxerr.debug(string("Debugging KBMAP #") + tostr(Error::KBMAP),
		     Error::KBMAP);
	lyxerr.debug(string("Debugging LATEX #") + tostr(Error::LATEX),
		     Error::LATEX);
	lyxerr.debug(string("Debugging MATHED #") + tostr(Error::MATHED), 
		     Error::MATHED);
	lyxerr.debug(string("Debugging FONT #") + tostr(Error::FONT),
		     Error::FONT);
	lyxerr.debug(string("Debugging TCLASS #") + tostr(Error::TCLASS), 
		     Error::TCLASS);
	lyxerr.debug(string("Debugging LYXVC #") + tostr(Error::LYXVC),
		     Error::LYXVC);
	lyxerr.debug(string("Debugging LYXSERVER #") + tostr(Error::LYXSERVER),
		     Error::LYXVC);
}


// Give command line help
void commandLineHelp()
{
	lyxerr.print(_("LyX " LYX_VERSION " of " LYX_RELEASE ".\n"));
	lyxerr.print(_("Usage: lyx [ command line switches ] [ name.lyx ... ]\n"));
	lyxerr.print(_("Command line switches (case sensitive):"));
	lyxerr.print(_("   -help           summarize LyX usage"));
	lyxerr.print(_("   -sysdir x       try to set system directory to x"));
	lyxerr.print(_("   -width x        set the width of the main window"));
	lyxerr.print(_("   -height y       set the height of the main window"));
	lyxerr.print(_("   -xpos x         set the x position of the main window"));
	lyxerr.print(_("   -ypos y         set the y position of the main window"));
	lyxerr.print(_("   -dbg n          where n is a sum of debugging options. Try -dbg 65535 -help"));
	lyxerr.print(_("   -Reverse        swaps foreground & background colors"));
	lyxerr.print(_("   -Mono           runs LyX in black and white mode"));
	lyxerr.print(_("   -FastSelection  use a fast routine for drawing selections\n"));
	lyxerr.print(_("Check the LyX man page for more options."));
}


bool LyX::easyParse(int *argc, char *argv[])
{
	bool gui = true;
	for(int i=1; i < *argc; i++) {
		string arg = argv[i];
		// Check for -dbg int
		if (arg == "-dbg") {
			if (i+1 < *argc) {
				int erri = 0;
				sscanf(argv[i+1],"%d", &erri);

				setDebuggingLevel(erri);

				// Now, remove these two arguments by shifting
				// the following two places down.
				(*argc) -= 2;
				for (int j=i; j < (*argc); j++)
					argv[j] = argv[j+2];
				i--; // After shift, check this number again.
			} else
				lyxerr.print(_("Missing number for -dbg switch!"));
		} 
		// Check for "-sysdir"
		else if (arg == "-sysdir") {
			if (i+1 < *argc) {
				system_lyxdir = argv[i+1];

				// Now, remove these two arguments by shifting
				// the following two places down.
				(*argc) -= 2;
				for (int j=i; j < (*argc); j++)
					argv[j] = argv[j+2];
				i--; // After shift, check this number again.
			} else
				lyxerr.print(_("Missing directory for -sysdir switch!"));
		// Check for --help or -help
		} else if (arg == "--help" || arg == "-help") {
			commandLineHelp();
			exit(0);
		} 
		// Check for "-nw": No window
		else if (arg == "-nw") {
			gui = false;
		}
	}
	return gui;
}


void error_handler(int err_sig)
{
	switch (err_sig) {
	case SIGHUP:
		fprintf(stderr, "\nlyx: SIGHUP signal caught\n");
		break;
	case SIGINT:
		// no comments
		break;
	case SIGFPE:
		fprintf(stderr, "\nlyx: SIGFPE signal caught\n");
		break;
	case SIGSEGV:
		fprintf(stderr, "\nlyx: SIGSEGV signal caught\n");
		fprintf(stderr,
			"Sorry, you have found a bug in LyX."
			" If possible, please read 'Known bugs'\n"
			"under the Help menu and then send us "
			"a full bug report. Thanks!\n");
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

	lyxerr.print("Bye.");
	if(err_sig!=SIGHUP && (!GetEnv("LYXDEBUG").empty() || err_sig == SIGSEGV))
		abort();
	exit(0);
}

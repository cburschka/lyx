// -*- C++ -*-
/**
 * \file LyX.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Jean-Marc Lasgouttes
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef LYX_H
#define LYX_H

#include "support/strfwd.h"

#include <vector>

namespace lyx {

class BufferList;
class CmdDef;
class Converters;
class DispatchResult;
class ErrorItem;
class FuncRequest;
class FuncStatus;
class KeyMap;
class LaTeXFonts;
class Messages;
class Mover;
class Movers;
class Server;
class ServerSocket;
class Session;
class SpellChecker;

enum RunMode {
	NEW_INSTANCE,
	USE_REMOTE,
	PREFERRED
};

enum OverwriteFiles {
	NO_FILES,
	MAIN_FILE,
	ALL_FILES,
	UNSPECIFIED
};

extern bool use_gui;
extern RunMode run_mode;
extern OverwriteFiles force_overwrite;

namespace frontend {
class Application;
}

/// initial startup
class LyX {
	friend class LyXConsoleApp;
public:
	LyX();
	~LyX();

	/// Execute LyX.
	int exec(int & argc, char * argv[]);

private:
	/// noncopyable
	LyX(LyX const &);
	void operator=(LyX const &);

	/// Do some cleanup in preparation of an exit.
	void prepareExit();

	/// Early exit during the initialisation process.
	void earlyExit(int status);

	/// Initialise LyX and fills-in the vector of files to be loaded.
	/**
	\return exit code failure if any.
	*/
	int init(int & argc, char * argv[]);

	/// Execute commandline commands if no GUI was requested.
	int execWithoutGui(int & argc, char * argv[]);

	/// Execute batch commands if available.
	void execCommands();

	/// Load files passed at command-line.
	/// return true on success false if we encounter an error
	/**
	This method is used only in non-GUI mode.
	*/
	bool loadFiles();

	/// initial LyX set up
	bool init();
	/** Check for the existence of the user's support directory and,
	 *  if not present, create it. Exits the program if the directory
	 *  cannot be created.
	 *  \returns true if the user-side configuration script
	 *  (lib/configure) should be re-run in this directory.
	 */
	bool queryUserLyXDir(bool explicit_userdir);
	/// read lyxrc/preferences
	/// \param check_format: whether to try to convert the format of
	/// the file, if there is a mismatch.
	bool readRcFile(std::string const & name, bool check_format = false);
	/// read the given languages file
	bool readLanguagesFile(std::string const & name);
	/// read the encodings.
	/// \param enc_name encodings definition file
	/// \param symbols_name unicode->LaTeX mapping file
	bool readEncodingsFile(std::string const & enc_name,
			       std::string const & symbols_name);
	/// parsing of non-gui LyX options.
	void easyParse(int & argc, char * argv[]);
	/// shows up a parsing error on screen
	void printError(ErrorItem const &);

	///
	Messages & messages(std::string const & language);

	/// Use the Pimpl idiom to hide the internals.
	// Mostly used for singletons.
	struct Impl;
	Impl * pimpl_;

	/// has this user started lyx for the first time?
	bool first_start;

	friend FuncStatus getStatus(FuncRequest const & action);
	friend DispatchResult const & dispatch(FuncRequest const & action);
	friend void dispatch(FuncRequest const & action, DispatchResult & dr);
	friend std::vector<std::string> & theFilesToLoad();
	friend BufferList & theBufferList();
	friend Server & theServer();
	friend ServerSocket & theServerSocket();
	friend Converters & theConverters();
	friend Converters & theSystemConverters();
	friend Messages const & getMessages(std::string const & language);
	friend Messages const & getGuiMessages();
	friend KeyMap & theTopLevelKeymap();
	friend Movers & theMovers();
	friend Mover const & getMover(std::string const & fmt);
	friend void setMover(std::string const & fmt, std::string const & command);
	friend Movers & theSystemMovers();
	friend frontend::Application * theApp();
	friend Session & theSession();
	friend LaTeXFonts & theLaTeXFonts();
	friend CmdDef & theTopLevelCmdDef();
	friend SpellChecker * theSpellChecker();
	friend void setSpellChecker();
	friend void emergencyCleanup();
	friend void execBatchCommands();
	friend void lyx_exit(int exit_code);
};


/// in the case of failure
void emergencyCleanup();
/// Try to exit LyX properly.
/// \p exit_code is 0 by default, if a non zero value is passed,
/// emergencyCleanup() will be called before exiting.
void lyx_exit(int exit_code);
/// Execute batch commands if available.
void execBatchCommands();

///
FuncStatus getStatus(FuncRequest const & action);

///
DispatchResult const & dispatch(FuncRequest const & action);

///
void dispatch(FuncRequest const & action, DispatchResult & dr);

} // namespace lyx

#endif // LYX_H

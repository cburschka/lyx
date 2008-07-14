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

namespace lyx {

class BufferList;
class CmdDef;
class Converters;
class ErrorItem;
class FuncRequest;
class FuncStatus;
class KeyMap;
class LyXFunc;
class Messages;
class Mover;
class Movers;
class Server;
class ServerSocket;
class Session;

extern bool use_gui;

namespace frontend {
class Application;
class LyXView;
}

namespace graphics {
class Previews;
}

/// initial startup
class LyX {
public:

	LyX();
	~LyX();

	/// Execute LyX.
	int exec(int & argc, char * argv[]);

	///
	frontend::LyXView * newLyXView();

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
	bool readRcFile(std::string const & name);
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
	friend void dispatch(FuncRequest const & action);
	friend BufferList & theBufferList();
	friend LyXFunc & theLyXFunc();
	friend Server & theServer();
	friend ServerSocket & theServerSocket();
	friend Converters & theConverters();
	friend Converters & theSystemConverters();
	friend Messages & getMessages(std::string const & language);
	friend Messages & getGuiMessages();
	friend KeyMap & theTopLevelKeymap();
	friend Movers & theMovers();
	friend Mover const & getMover(std::string  const & fmt);
	friend void setMover(std::string const & fmt, std::string const & command);
	friend Movers & theSystemMovers();
	friend frontend::Application * theApp();
	friend graphics::Previews & thePreviews();
	friend Session & theSession();
	friend CmdDef & theTopLevelCmdDef();
	friend void setRcGuiLanguage();
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
/// Set the language defined by the user.
void setRcGuiLanguage();
/// Execute batch commands if available.
void execBatchCommands();

} // namespace lyx

#endif // LYX_H

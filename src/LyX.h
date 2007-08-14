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

#include <boost/scoped_ptr.hpp>
#include <boost/utility.hpp>

#include <string>

namespace lyx {

namespace support {
class FileName;
}

class Buffer;
class BufferList;
class Converters;
class ErrorItem;
class Inset;
class LyXFunc;
class Server;
class ServerSocket;
class Messages;
class Mover;
class Movers;
class Session;
class KeyMap;

extern bool use_gui;

namespace frontend {
class Application;
class LyXView;
}

/// initial startup
class LyX : boost::noncopyable {
public:

	LyX();
	~LyX();

	/// Execute LyX.
	int exec(int & argc, char * argv[]);

	static LyX & ref();
	static LyX const & cref();

	/// in the case of failure
	void emergencyCleanup() const;

	///
	BufferList & bufferList();
	BufferList const & bufferList() const;
	///
	Session & session();
	Session const & session() const;
	///
	LyXFunc & lyxFunc();
	LyXFunc const & lyxFunc() const;
	///
	Server & server();
	Server const & server() const;
	///
	ServerSocket & socket();
	ServerSocket const & socket() const;

	///
	frontend::Application & application();
	frontend::Application const & application() const;

	///
	KeyMap & topLevelKeymap();
	KeyMap const & topLevelKeymap() const;

	///
	Converters & converters();
	Converters & systemConverters();

	///
	Messages & getMessages(std::string const & language);
	///
	Messages & getGuiMessages();
	///
	void setGuiLanguage(std::string const & language);

	///
	frontend::LyXView * newLyXView();

	/** redraw \c inset in all the BufferViews in which it is currently
	 *  visible. If successful return a pointer to the owning Buffer.
	 */
	Buffer const * const updateInset(Inset const *) const;

	void hideDialogs(std::string const & name, Inset * inset) const;

	/// Execute batch commands if available.
	void execBatchCommands();

	///
	void addFileToLoad(support::FileName const &);

private:
	/// Do some cleanup in preparation of an exit.
	void prepareExit();

	/// Early exit during the initialisation process.
	void earlyExit(int status);

	/// Initialise LyX and fills-in the vector of files to be loaded.
	/**
	\return exit code failure if any.
	*/
	int init(int & argc, char * argv[]);

	/// Load files passed at command-line.
	/**
	This method is used only in non-GUI mode.
	*/
	void loadFiles();

	/// Create a View, load files and restore GUI Session.
	void restoreGuiSession();

	/// Initialize RC font for the GUI.
	void initGuiFont();

	/// initial LyX set up
	bool init();
	/// set up the default key bindings
	void defaultKeyBindings(KeyMap * kbmap);
	/// set up the default dead key bindings if requested
	void deadKeyBindings(KeyMap * kbmap);
	/** Check for the existence of the user's support directory and,
	 *  if not present, create it. Exits the program if the directory
	 *  cannot be created.
	 *  \returns true if the user-side configuration script
	 *  (lib/configure) should be re-run in this directory.
	 */
	bool queryUserLyXDir(bool explicit_userdir);
	/// read lyxrc/preferences
	bool readRcFile(std::string const & name);
	/// read the given ui (menu/toolbar) file
	bool readUIFile(std::string const & name, bool include = false);
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

	/// has this user started lyx for the first time?
	bool first_start;
	/// the parsed command line batch command if any
	std::string batch_command;

	/// Use the Pimpl idiom to hide the internals.
	struct Singletons;
	boost::scoped_ptr<Singletons> pimpl_;

	friend Movers & theMovers();
	friend Mover const & getMover(std::string  const & fmt);
	friend void setMover(std::string const & fmt, std::string const & command);
	friend Movers & theSystemMovers();
	friend frontend::Application * theApp();
};

} // namespace lyx

#endif // LYX_H

// -*- C++ -*-
/**
 * \file lyx_main.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Jean-Marc Lasgouttes
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef LYX_MAIN_H
#define LYX_MAIN_H

#include <boost/scoped_ptr.hpp>
#include <boost/utility.hpp>

#include <list>
#include <string>
#include <vector>

namespace lyx {

class Buffer;
class BufferList;
class ErrorItem;
class InsetBase;
class LyXFunc;
class LyXServer;
class LyXServerSocket;
class LyXView;
class Session;
class kb_keymap;

extern bool use_gui;

namespace frontend { class Application; }

/// initial startup
class LyX : boost::noncopyable {
public:
	/**
	 * Execute LyX. The startup sequence is as follows:
	 * -# LyX::exec()
	 * -# LyX::priv_exec()
	 * -# lyx::createApplication()
	 * -# LyX::exec2()
	 * Step 3 is omitted if no gui is wanted. We need lyx::createApplication()
	 * only to create the QApplication object in the qt frontend. All
	 * attempts with static and dynamically allocated QApplication
	 * objects lead either to harmless error messages on exit
	 * ("Mutex destroy failure") or crashes (OS X).
	 */
	static int exec(int & argc, char * argv[]);
	/// Execute LyX (inner execution loop, \sa exec)
	int exec2(int & argc, char * argv[]);
	static LyX & ref();
	static LyX const & cref();

	/// in the case of failure
	void emergencyCleanup() const;

	/// Ask the LyX class to exit.
	/**
	In GUI mode, after this function has been called, application_ leaves
	the main event loop and returns from the call to Application::start().
	*/
	void quit(bool noask);

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
	LyXServer & server();
	LyXServer const & server() const;
	///
	LyXServerSocket & socket();
	LyXServerSocket const & socket() const;

	///
	frontend::Application & application();
	frontend::Application const & application() const;

	///
	kb_keymap & topLevelKeymap();
	kb_keymap const & topLevelKeymap() const;

	LyXView * newLyXView();

	/** redraw \c inset in all the BufferViews in which it is currently
	 *  visible. If successful return a pointer to the owning Buffer.
	 */
	Buffer const * const updateInset(InsetBase const *) const;

private:
	static boost::scoped_ptr<LyX> singleton_;

	LyX();
	int priv_exec(int & argc, char * argv[]);

	/// Do some cleanup in preparation of an exit.
	void prepareExit();

	/// Early exit during the initialisation process.
	void earlyExit(int status);

	/// Initialise LyX and execute batch commands if available.
	/**
	\param files is filled in with the command-line file names.
	\return exit code failure if any.
	*/
	int execBatchCommands(int & argc, char * argv[],
		std::vector<std::string> & files);

	/// Create a View and restore GUI Session.
	void restoreGuiSession(std::vector<std::string> const & files);

	///
	void addLyXView(LyXView * lyxview);

	/// Initialize RC font for the GUI.
	void initGuiFont();

	/// initial LyX set up
	bool init();
	/// set up the default key bindings
	void defaultKeyBindings(kb_keymap * kbmap);
	/// set up the default dead key bindings if requested
	void deadKeyBindings(kb_keymap * kbmap);
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
	bool readUIFile(std::string const & name);
	/// read the given languages file
	bool readLanguagesFile(std::string const & name);
	/// read the given encodings file
	bool readEncodingsFile(std::string const & name);
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
	///
	typedef std::list<LyXView *> ViewList;
	ViewList views_;

	///
	bool geometryOption_;
};

} // namespace lyx

#endif // LYX_MAIN_H

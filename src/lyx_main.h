// -*- C++ -*-
/**
 * \file lyx_main.h
 * Copyright 2002 the LyX Team
 * Read the file COPYING
 *
 * \author unknown
 */

#ifndef LYX_MAIN_H
#define LYX_MAIN_H

#include "LString.h"
#include "errorlist.h"

#include <boost/scoped_ptr.hpp>
#include <boost/utility.hpp>

#include <csignal>


class LyXRC;
class LastFiles;
class Buffer;
class kb_keymap;


/// last files loaded
extern boost::scoped_ptr<LastFiles> lastfiles;


/// initial startup
class LyX : boost::noncopyable {
public:
	LyX(int & argc, char * argv[]);

	/// in the case of failure
	static void emergencyCleanup();

private:
	/// initial LyX set up
	void init(bool);
	/// set up the default key bindings
	void defaultKeyBindings(kb_keymap * kbmap);
	/// set up the default dead key bindings if requested
	void deadKeyBindings(kb_keymap * kbmap);
	/// check, set up and configure the user dir if necessary
	void queryUserLyXDir(bool explicit_userdir);
	/// read lyxrc/preferences
	void readRcFile(string const & name);
	/// read the given ui (menu/toolbar) file
	void readUIFile(string const & name);
	/// read the given languages file
	void readLanguagesFile(string const & name);
	/// read the given encodings file
	void readEncodingsFile(string const & name);
	/// parsing of non-gui LyX options. Returns true if gui
	bool easyParse(int & argc, char * argv[]);
	/// shows up a parsing error on screen
	void printError(ErrorItem const &);

	/// has this user started lyx for the first time?
	bool first_start;
	/// the parsed command line batch command if any
	string batch_command;
};

#endif // LYX_MAIN_H

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

#ifdef __GNUG__
#pragma interface
#endif

#include "LString.h"

#include <boost/scoped_ptr.hpp>
#include <boost/utility.hpp>

#include <csignal>

class LyXRC;
class LastFiles;
class Buffer;
class kb_keymap;


/// e.g. $HOME/.lyx/
extern string user_lyxdir;
/// e.g. /usr/share/lyx/
extern string system_lyxdir;
/// e.g. /tmp
extern string system_tempdir;
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
	/// return true if the given prefs file was successfully read
	bool readRcFile(string const & name);
	/// read the given ui (menu/toolbar) file
	void readUIFile(string const & name);
	/// read the given languages file
	void readLanguagesFile(string const & name);
	/// read the given encodings file
	void readEncodingsFile(string const & name);
	/// parsing of non-gui LyX options. Returns true if gui
	bool easyParse(int & argc, char * argv[]);
 
	/// has this user started lyx for the first time?
	bool first_start;
	/// the parsed command line batch command if any
	string batch_command;
};

#endif // LYX_MAIN_H

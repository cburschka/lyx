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


///
extern string user_lyxdir;
///
extern string system_lyxdir;
///
extern string system_tempdir;
///
extern boost::scoped_ptr<LastFiles> lastfiles;


class LyX : boost::noncopyable {
public:
	LyX(int & argc, char * argv[]);

	/// in the case of failure
	static void emergencyCleanup();

private:
	/// does this user start lyx for the first time?
	bool first_start;
	///
	string batch_command;
	///
	void runtime();
	///
	void init(bool);
	///
	void defaultKeyBindings(kb_keymap * kbmap);
	///
	void deadKeyBindings(kb_keymap * kbmap);
	///
	void queryUserLyXDir(bool explicit_userdir);
	/** Search for and read the LyXRC file name, return
	    true if successfull.
	*/
	bool readRcFile(string const & name);
	/// Read the ui file `name'
	void readUIFile(string const & name);
	/// Read the languages file `name'
	void readLanguagesFile(string const & name);
	/// Read the encodings file `name'
	void readEncodingsFile(string const & name);
	///
	bool easyParse(int & argc, char * argv[]);
};

#endif

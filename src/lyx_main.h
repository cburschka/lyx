// -*- C++ -*-
/* This file is part of
* ====================================================== 
* 
*           LyX, The Document Processor
* 	 
*           Copyright 1995 Matthias Ettrich
*           Copyright 1995-2001 The LyX Team.
*
* ====================================================== */

/* This is the declaration of the LyX class, there should only
 * exist _one_ instance of this in the application. */

#ifndef LYX_MAIN_H
#define LYX_MAIN_H

#ifdef __GNUG__
#pragma interface
#endif

#include <csignal>

#include "LString.h"
#include <boost/utility.hpp>

class LyXGUI;
class LyXRC;
class LastFiles;
class Buffer;
class kb_keymap;

///
extern string system_lyxdir;
///
extern string user_lyxdir;
///
extern string system_tempdir;
///
extern LastFiles * lastfiles; /* we should hopefully be able to move this
			      * inside the LyX class */


/**
  This is the main LyX object it encapsulates most of the other objects.
*/
class LyX : boost::noncopyable {
public:
	/// the only allowed constructor
	LyX(int * argc, char * argv[]); // constructor
	/// Always is useful a destructor
	~LyX();

	/// in the case of failure
	static void emergencyCleanup();
 
	///
	LyXGUI * lyxGUI;  // should be only one of this
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
	bool easyParse(int * argc, char * argv[]);
};

#endif

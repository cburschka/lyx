// -*- C++ -*-
/* This file is part of
* ====================================================== 
* 
*           LyX, The Document Processor
* 	 
*           Copyright 1995 Matthias Ettrich
*           Copyright 1995-2000 The LyX Team.
*
* ====================================================== */

/* This is the declaration of the LyX class, there should only
 * exist _one_ instance of this in the application. */

#ifndef LYX_MAIN_H
#define LYX_MAIN_H

#include <csignal>

#include "LString.h"

class LyXGUI;
class LyXRC;
class LastFiles;
class Buffer;
class kb_keymap;

extern string system_lyxdir;
extern string user_lyxdir;
extern string system_tempdir;

extern LastFiles * lastfiles; /* we should hopefully be able to move this
			      * inside the LyX class */


/**
  This is the main LyX object it encapsulates most of the other objects.
*/
class LyX {
public:
	/**@name Constructors and Deconstructors */
	//@{
	/// the only allowed constructor
	LyX(int * argc, char * argv[]); // constructor
	// Always is useful a destructor
	~LyX();
	//@}

	/**@name Pointers to... */
	//@{
	///
	LyXGUI * lyxGUI;  // should be only one of this
	//@}
private:
	/**@name Constructors and Deconstructors */
	//@{
	/// not allowed
	LyX(const LyX &) {} // not allowed
	/// not allowed
	LyX() {} // not allowed
	//@}

	/**@name Private variables */
	//@{
	/// does this user start lyx for the first time?
	bool first_start;
	///
	string batch_command;
	///
	struct sigaction act_;
	//@}
	/**@name Private Members */
	//@{
	///
	void runtime();
	///
	void init(int * argc, char * argv[], bool);
	///
	void defaultKeyBindings(kb_keymap * kbmap);
	///
	void deadKeyBindings(kb_keymap * kbmap);
	///
	void queryUserLyXDir(bool explicit_userdir);
	///
        void ReadRcFile(string const & name);
        ///
	bool easyParse(int * argc, char * argv[]);
	//@}
};

#endif

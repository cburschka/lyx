// -*- C++ -*-
/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 *
 *	     Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2000 The LyX Team.
 *
 *           This file is Copyright 1997
 *           Asger Alstrup
 *
 * ====================================================== 
 */

#ifndef CHKTEX_H
#define CHKTEX_H

#ifdef __GNUG__
#pragma interface
#endif

#include "LString.h"

class LyXLex;
class TeXErrors;

///
class Chktex {
public:
	/**
	  cmd = the chktex command, file = name of the (temporary) latex file,
	  path = name of the files original path.
	  */
	Chktex(string const & cmd, string const & file,
	       string const & path);
	
	/** Runs chktex.
	  Returns -1 if fail, number of messages otherwise.
	  */
	int run(TeXErrors &);
private:
	///
	int scanLogFile(TeXErrors &);

	///
	string cmd;

	///
	string file;
	
	///
	string path;
};

#endif

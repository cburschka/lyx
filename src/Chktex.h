// -*- C++ -*-
/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 *
 *	     Copyright (C) 1995 Matthias Ettrich
 *           Copyright (C) 1995-1998 The LyX Team.
 *
 *           This file is Copyright (C) 1997
 *           Asger Alstrup
 *
 *======================================================
 */

#ifndef _CHKTEX_H
#define _CHKTEX_H

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
	Chktex(LString const & cmd, LString const & file,
	       LString const & path);
	
	/** Runs chktex.
	  Returns -1 if fail, number of messages otherwise.
	  */
	int run(TeXErrors &);
private:
	///
	int scanLogFile(TeXErrors &);

	///
	LString cmd;

	///
	LString file;
	
	///
	LString path;
};

#endif

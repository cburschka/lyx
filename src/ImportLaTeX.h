// -*- C++ -*-
/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 *
 *           Copyright (C) 1995-1998 The LyX Team.
 *
 *           This file is Copyright (C) 1998
 *           Asger Alstrup
 *
 *======================================================
 */

#ifndef _IMPORTLATEX_H
#define _IMPORTLATEX_H

#ifdef __GNUG__
#pragma interface
#endif

#include "LString.h"

class Buffer;

///
class ImportLaTeX {
public:
	/**
	  file = name and path of the latex file
	  */
	ImportLaTeX(LString const & file);
	
	/** Imports the document.
	  Return 0 if fail.
	  */
	Buffer * run();
private:
	///
	LString file;
};

#endif

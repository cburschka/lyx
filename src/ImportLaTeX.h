// -*- C++ -*-
/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 *
 *           Copyright 1995-2000 The LyX Team.
 *
 *           This file is Copyright 1998
 *           Asger Alstrup
 *
 * ======================================================
 */

#ifndef IMPORTLATEX_H
#define IMPORTLATEX_H

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
	explicit ImportLaTeX(string const & file);
	
	/** Imports the document.
	  Return 0 if fail.
	  */
	Buffer * run();
private:
	///
	string file;
};

#endif

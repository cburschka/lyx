// -*- C++ -*-
/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 *
 *           Copyright (C) 1995-1999 The LyX Team.
 *
 *           This file is Copyright (C) 1999
 *           Kayvan A. Sylvan
 *
 *======================================================
 */

#ifndef IMPORTNOWEB_H
#define IMPORTNOWEB_H

#ifdef __GNUG__
#pragma interface
#endif

#include "LString.h"

class Buffer;

///
class ImportNoweb {
public:
	/**
	  file = name and path of the noweb file to import
	  */
	ImportNoweb(string const & file) : file(file) {};
	
	/** Imports the document.
	  Return 0 if fail.
	  */
	Buffer * run();
private:
	///
	string file;
	///
	string documentclass();
	///
	enum{ 
		BUFSIZE = 512 
	};
};

#endif

// -*- C++ -*-
/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 *
 *           Copyright 1995-2000 The LyX Team.
 *
 *           This file is Copyright 1999
 *           Kayvan A. Sylvan
 *
 * ====================================================== 
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
	explicit ImportNoweb(string const & file) : file(file) {}
	
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
	enum { 
		BUFSIZE = 512 
	};
};

#endif

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

#ifndef _IMPORTNOWEB_H
#define _IMPORTNOWEB_H

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
	ImportNoweb(LString const & file) : file(file) {};
	
	/** Imports the document.
	  Return 0 if fail.
	  */
	Buffer * run();
private:
	///
	LString file;
	///
	LString documentclass();
	///
	enum{ 
		BUFSIZE = 512 
	};
};

#endif

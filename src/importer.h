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

#ifndef IMPORTER_H
#define IMPORTER_H

#ifdef __GNUG__
#pragma interface
#endif

#include "LString.h"

class LyXView;

class Importer {
public:
	///
	static
	void Import(LyXView * lv, string const & filename, 
		    string const & format);
	static
        bool IsImportable(string const & format);
};
#endif

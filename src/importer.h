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

#ifndef IMPORTER_H
#define IMPORTER_H

#ifdef __GNUG__
#pragma interface
#endif

#include <vector>
#include "LString.h"

class LyXView;
class Format;

class Importer {
public:
	///
	static
	bool Import(LyXView * lv, string const & filename, 
		    string const & format);
#if 0
	///
	static
        bool IsImportable(string const & format);
#endif
	///
	static
	std::vector<Format const *> const GetImportableFormats();
private:
	///
	static
	std::vector<string> const Loaders();
};
#endif

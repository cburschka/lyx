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

#ifndef EXPORTER_H
#define EXPORTER_H

#ifdef __GNUG__
#pragma interface
#endif

#include <vector>
#include "LString.h"

class Buffer;

///
class Exporter {
public:
	///
	static
	bool Export(Buffer * buffer, string const & format,
		    bool put_in_tempdir, string * view_file = 0);
	///
	static
	bool Preview(Buffer * buffer, string const & format);
	///
	static
	bool IsExportable(Buffer const * buffer, string const & format);
	///
	static
	std::vector<std::pair<string, string> > const
	GetExportableFormats(Buffer const * buffer);
	///
	static
	std::vector<std::pair<string, string> > const
	GetViewableFormats(Buffer const * buffer);
	///
	static
	string const BufferExtension(Buffer const * buffer);
};
#endif

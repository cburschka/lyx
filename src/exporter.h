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

#ifndef EXPORTER_H
#define EXPORTER_H

#ifdef __GNUG__
#pragma interface
#endif

#include <vector>
#include "LString.h"

class Buffer;
class Format;

class Exporter {
public:
	///
	static
	bool Export(Buffer * buffer, string const & format,
		    bool put_in_tempdir, string & result_file);
	///
	static
	bool Export(Buffer * buffer, string const & format,
		    bool put_in_tempdir);
	///
	static
	bool Preview(Buffer * buffer, string const & format);
	///
	static
	bool IsExportable(Buffer const * buffer, string const & format);
	///
	static
	std::vector<Format const *> const
	GetExportableFormats(Buffer const * buffer, bool only_viewable);
	///
private:
	static
	string const BufferFormat(Buffer const * buffer);
	///
	static
	std::vector<string> const Backends(Buffer const * buffer);
};
#endif

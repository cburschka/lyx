/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor 	 
 *	     Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2000 The LyX Team.
 *
 *           This file is Copyright 1998
 *           Asger Alstrup
 *
 * ====================================================== 
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "ImportLaTeX.h"
#include "lyxrc.h"
#include "support/syscall.h"
#include "support/filetools.h"
#include "bufferlist.h"

extern BufferList bufferlist;

/*
 * CLASS ImportLaTeX
 */

ImportLaTeX::ImportLaTeX(string const & file)
		: file(file)
{
}


Buffer * ImportLaTeX::run()
{
	// run reLyX
	string tmp = lyxrc.relyx_command + " -f " + file;
        Systemcalls one;
	Buffer * buf = 0;
	int result = one.startscript(Systemcalls::System, tmp);
	if (result == 0) {
		string filename = ChangeExtension(file, ".lyx", false);
		// File was generated without problems. Load it.
		buf = bufferlist.loadLyXFile(filename);
	}
	return buf;
}

/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor 	 
 *	     Copyright (C) 1995 Matthias Ettrich
 *           Copyright (C) 1995-1998 The LyX Team.
 *
 *           This file is Copyright (C) 1998
 *           Asger Alstrup
 *
 *======================================================
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "ImportLaTeX.h"
#include "lyxrc.h"
#include "syscall.h"
#include "filetools.h"
#include "bufferlist.h"

extern LyXRC * lyxrc;
extern BufferList bufferlist;

// 	$Id: ImportLaTeX.C,v 1.1 1999/09/27 18:44:36 larsbj Exp $

#if !defined(lint) && !defined(WITH_WARNINGS)
static char vcid[] = "$Id: ImportLaTeX.C,v 1.1 1999/09/27 18:44:36 larsbj Exp $";
#endif /* lint */


/*
 * CLASS ImportLaTeX
 */

ImportLaTeX::ImportLaTeX(LString const & file)
		: file(file)
{
}


Buffer * ImportLaTeX::run()
{
	// run reLyX
	LString tmp = lyxrc->relyx_command + " -f " + file;
        Systemcalls one;
	Buffer * buf = 0;
	int result= one.Startscript(Systemcalls::System, tmp);
	if (result==0) {
		LString filename = ChangeExtension(file, ".lyx", false);
		// File was generated without problems. Load it.
		buf = bufferlist.loadLyXFile(filename);
	}
	return buf;
}

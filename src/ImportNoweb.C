/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor 	 
 *	     Copyright (C) 1995 Matthias Ettrich
 *           Copyright (C) 1995-1999 The LyX Team.
 *
 *           This file is Copyright (C) 1999
 *           Kayvan A. Sylvan
 *
 *======================================================
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "ImportNoweb.h"
#include "lyxrc.h"
#include "syscall.h"
#include "filetools.h"
#include "bufferlist.h"

extern LyXRC * lyxrc;
extern BufferList bufferlist;

// 	$Id: ImportNoweb.C,v 1.1 1999/09/27 18:44:36 larsbj Exp $

#if !defined(lint) && !defined(WITH_WARNINGS)
static char vcid[] = "$Id: ImportNoweb.C,v 1.1 1999/09/27 18:44:36 larsbj Exp $";
#endif /* lint */

/*
 * Implementation the ImportNoweb methods.
 */

Buffer * ImportNoweb::run()
{
	// run reLyX -n
	LString tmp = lyxrc->relyx_command + " -n -c " +
					documentclass() + " -f " + file;
        Systemcalls one;
	Buffer * buf = 0;
	int result= one.Startscript(Systemcalls::System, tmp);
	if (result==0) {
		LString filename = file + ".lyx";
		// File was generated without problems. Load it.
		buf = bufferlist.loadLyXFile(filename);
	}
	return buf;
}

// Provide the literate documentclass by parsing the file.
//
LString ImportNoweb::documentclass()
{
	LString result = "literate-article"; // Default

	FilePtr inputfile(file, FilePtr::read);	
	if (!inputfile()) return "nofile"; // Should not happen!

	char buf[BUFSIZE], *p, *q;

	while(!feof(inputfile())) {
		(void)fgets(buf, BUFSIZE, inputfile());
		if ((p = strstr(buf, "\\documentclass"))) {
			while ((*p) && (*p != '{'))
				p++;
			q = p++;
			while ((*q) && (*q != '}'))
				q++;
			*q = '\0';
			result = p;
			result = "literate-" + result;
		}
	}

	return result;
}

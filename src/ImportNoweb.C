/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor 	 
 *	     Copyright 1995 Matthias Ettrich
 *           Copyright 1995-1999 The LyX Team.
 *
 *           This file is Copyright 1999
 *           Kayvan A. Sylvan
 *
 * ======================================================
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "ImportNoweb.h"
#include "lyxrc.h"
#include "support/syscall.h"
#include "support/filetools.h"
#include "bufferlist.h"

extern LyXRC * lyxrc;
extern BufferList bufferlist;

/*
 * Implementation the ImportNoweb methods.
 */

Buffer * ImportNoweb::run()
{
	// run reLyX -n
	string tmp = lyxrc->relyx_command + " -n -c " +
					documentclass() + " -f " + file;
        Systemcalls one;
	Buffer * buf = 0;
	int result= one.startscript(Systemcalls::System, tmp);
	if (result==0) {
		string filename = file + ".lyx";
		// File was generated without problems. Load it.
		buf = bufferlist.loadLyXFile(filename);
	}
	return buf;
}

// Provide the literate documentclass by parsing the file.
//
string ImportNoweb::documentclass()
{
	string result = "literate-article"; // Default

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

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

#include <fstream>

#include "ImportNoweb.h"
#include "lyxrc.h"
#include "support/syscall.h"
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
	if (result == 0) {
		string filename = file + ".lyx";
		// File was generated without problems. Load it.
		buf = bufferlist.loadLyXFile(filename);
	}
	return buf;
}


// Provide the literate documentclass by parsing the file.
string ImportNoweb::documentclass()
{
	string result = "literate-article"; // Default

#warning If you use literate programming you should verify that this works
	// This method has been rewritten to use ifstream, but since I
	// don't use literate programming myself I am unable to check
	// this correclty. (Lgb)
	ifstream ifs(file.c_str());

	if (!ifs) return "nofile"; // Should not happen!
	string line;
	while (getline(ifs, line)) {
		int p = line.find("\\documentclass");
		if (p != string::npos) {
			p = line.find('{', p);
			int q = line.find('}', p);
			result = "literate-" + line.substr(p, q - p);
			break;
		}
	}
	return result;
}

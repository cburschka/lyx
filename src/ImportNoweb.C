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
using std::ifstream;

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

	ifstream ifs(file.c_str());

	if (!ifs) return "nofile"; // Should not happen!
	string line;
	while (getline(ifs, line)) {
		string::size_type p = line.find("\\documentclass");
		if (p != string::npos) {
			p = line.find('{', p);
			string::size_type q = line.find('}', p);
			result = "literate-" + line.substr(p + 1, q - p - 1);
			break;
		}
	}
	return result;
}

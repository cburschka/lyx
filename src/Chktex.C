/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor 	 
 *	     Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2001 The LyX Team.
 *
 *           This file is Copyright 1997-1998
 *           Asger Alstrup
 *
 * ====================================================== 
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "Chktex.h"
#include "LaTeX.h" // TeXErrors
#include "lyxlex.h"
#include "debug.h"
#include "gettext.h"

#include "support/FileInfo.h"
#include "support/filetools.h"
#include "support/syscall.h"
#include "support/syscontr.h"
#include "support/path.h"
#include "support/lstrings.h"

#include <fstream>

using std::ifstream;
using std::getline;

/*
 * CLASS Chktex
 */

Chktex::Chktex(string const & chktex, string const & f, string const & p)
		: cmd(chktex), file(f), path(p)
{
}


int Chktex::run(TeXErrors &terr)
{
	// run bibtex
	string log = OnlyFilename(ChangeExtension(file, ".log"));
	string tmp = cmd + " -q -v0 -b0 -x " + file + " -o " + log;
        Systemcalls one;
	int result= one.startscript(Systemcalls::System, tmp);
	if (result == 0) {
		result = scanLogFile(terr);
	} else {
		result = -1;
	}
	return result;
}


int Chktex::scanLogFile(TeXErrors & terr)
{
	string token;
	int retval = 0;

	string tmp = OnlyFilename(ChangeExtension(file, ".log"));

	ifstream ifs(tmp.c_str());
	while (getline(ifs, token)) {
		string srcfile;
		string line;
		string pos;
		string warno;
		string warning;
		token = split(token, srcfile, ':');
		token = split(token, line, ':');
		token = split(token, pos, ':');
		token = split(token, warno, ':');
		token = split(token, warning, ':');

		int lineno = lyx::atoi(line);
		warno = _("ChkTeX warning id #") + warno;
		terr.insertError(lineno, warno, warning);
		++retval;
	}
	return retval;
}

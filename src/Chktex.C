/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor 	 
 *	     Copyright (C) 1995 Matthias Ettrich
 *           Copyright (C) 1995-1998 The LyX Team.
 *
 *           This file is Copyright (C) 1997-1998
 *           Asger Alstrup
 *
 *======================================================
 */

#include <config.h>

#include <stdlib.h> // atoi

#ifdef __GNUG__
#pragma implementation
#endif

#include "Chktex.h"
#include "LaTeX.h" // TeXErrors
#include "filetools.h"
#include "lyxlex.h"
#include "FileInfo.h"
#include "error.h"
#include "syscall.h"
#include "syscontr.h"
#include "pathstack.h"
#include "gettext.h"

// 	$Id: Chktex.C,v 1.1 1999/09/27 18:44:36 larsbj Exp $	

#if !defined(lint) && !defined(WITH_WARNINGS)
static char vcid[] = "$Id: Chktex.C,v 1.1 1999/09/27 18:44:36 larsbj Exp $";
#endif /* lint */


/*
 * CLASS Chktex
 */

Chktex::Chktex(LString const & chktex, LString const & f, LString const & p)
		: cmd(chktex), file(f), path(p)
{
}


int Chktex::run(TeXErrors &terr)
{
	// run bibtex
	LString log = ChangeExtension(file, ".log", true);
	LString tmp = cmd + " -q -v0 -b0 -x " + file + " -o " + log;
        Systemcalls one;
	int result= one.Startscript(Systemcalls::System, tmp);
	if (result == 0) {
		result = scanLogFile(terr);
	} else {
		result = -1;
	}
	return result;
}


int Chktex::scanLogFile(TeXErrors &terr)
{
	LString token;
	int retval = 0;

	LyXLex lex(NULL, 0);

	LString tmp = ChangeExtension(file, ".log", true);

	if (!lex.setFile(tmp)) {
		// Unable to open file. Return at once
		return -1;
	}

	while (lex.IsOK()) {
		if (lex.EatLine())
			token = lex.GetString();
		else // blank line in the file being read
			continue;

		LString srcfile, line, pos, warno, warning;
		token.split(srcfile, ':');
		token.split(line, ':');
		token.split(pos, ':');
		token.split(warno, ':');
		token.split(warning, ':');

		int lineno = atoi(line.c_str());
		warno = _("ChkTeX warning id #") + warno;
		terr.insertError(lineno, warno, warning);
		retval++;
	}
	return retval;
}

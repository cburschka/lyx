/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor 	 
 *	     Copyright (C) 1995 Matthias Ettrich
 *           Copyright (C) 1995-1998 The LyX Team.
 *
 *           This file is Copyright (C) 1996-1998
 *           Lars Gullik Bjønnes
 *
 *======================================================
 */

#include <config.h>

#include <stdio.h>
#include <stdlib.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "filetools.h"
#include "LaTeX.h"
#include "Literate.h"
#include "lyxlex.h"
#include "FileInfo.h"
#include "error.h"
#include "lyxlib.h"
#include "syscall.h"
#include "syscontr.h"
#include "pathstack.h"
#include "bufferlist.h"
#include "minibuffer.h"
#include "gettext.h"

// 	$Id: Literate.C,v 1.1 1999/09/27 18:44:36 larsbj Exp $	

#if !defined(lint) && !defined(WITH_WARNINGS)
static char vcid[] = "$Id: Literate.C,v 1.1 1999/09/27 18:44:36 larsbj Exp $";
#endif /* lint */

extern BufferList bufferlist;

Literate::Literate(LString const & latex, LString const & f, LString const & p, 
		   LString const & l, 
		   LString const & literate, LString const & literate_f, 
		   LString const & build, LString const & build_f)
                   : LaTeX (latex, f, p),
		     litfile(l),
		     literate_cmd(literate), literate_filter(literate_f), 
		     build_cmd(build), build_filter(build_f)
{
}


int Literate::weave(TeXErrors &terr, MiniBuffer *minib)
{
        int scanres = Literate::NO_ERRORS;
        LString tmp1, tmp2;
        int ret1, ret2;
        Systemcalls one, two;

        // The class LaTeX does not know the temp path.
        bufferlist.updateIncludedTeXfiles(GetCWD());
        
        lyxerr.debug(LString(_("Weaving document")),
                     Error::LATEX);
        minib->Set(LString(_("Weaving document")));
        minib->Store();

        // Run the literate program to convert \literate_extension file to .tex file
        //
        tmp1 = literate_cmd + " < " + litfile + " > " + file + " 2> " + litfile + ".out";
        tmp2 = literate_filter + " < " + litfile + ".out" + " > " + litfile + ".log";
        ret1 = one.Startscript(Systemcalls::System, tmp1);
        ret2 = two.Startscript(Systemcalls::System, tmp2);
        lyxerr.debug(LString(_("LITERATE")) + " {" + tmp1 + "} {" + tmp2 + "}");
	scanres = scanLiterateLogFile(terr);
	if (scanres & Literate::ERRORS) return scanres; // return on literate error

	return run(terr, minib);
}


int Literate::build(TeXErrors &terr, MiniBuffer *minib)
        // We know that this function will only be run if the lyx buffer
        // has been changed. 
{
        int scanres = Literate::NO_ERRORS;
        num_errors = 0; // just to make sure.
        // DepTable head; // empty head // unused
        // bool rerun = false; // rerun requested // unused
        LString tmp1, tmp2;
        int ret1, ret2;
        Systemcalls one, two;
        
        // The class LaTeX does not know the temp path.
        bufferlist.updateIncludedTeXfiles(GetCWD());
        
        lyxerr.debug(LString(_("Building program")), 
                     Error::LATEX);
        minib->Set(LString(_("Building program")));
        minib->Store();

        // Run the build program
        //
        tmp1 = build_cmd + ' ' + litfile + " > " + litfile + ".out 2>&1";
        tmp2 = build_filter + " < " + litfile + ".out" + " > " + litfile + ".log";
        ret1 = one.Startscript(Systemcalls::System, tmp1);
        ret2 = two.Startscript(Systemcalls::System, tmp2);
        scanres = scanBuildLogFile(terr);
        lyxerr.debug("Done.", Error::LATEX);

        return scanres;
}


int Literate::scanLiterateLogFile(TeXErrors &terr)
{
        LString token;
        int retval = NO_ERRORS;
        
        LyXLex lex(NULL, 0);
 
        LString tmp = litfile + ".log";
        
        if (!lex.setFile(tmp)) {
                // unable to open file
                // return at once
                retval |= NO_LOGFILE;
                return retval;
        }
        
        while (lex.IsOK()) {
                if (lex.EatLine())
                        token = lex.GetString();
                else // blank line in the file being read
                        continue;
 
                lyxerr.debug(token, Error::LATEX);
                
                if (token.prefixIs("Build Warning:")) {
                        // Here shall we handle different
                        // types of warnings
                        retval |= LATEX_WARNING;
                        lyxerr.debug("Build Warning.", Error::LATEX);
                } else if (token.prefixIs("! Build Error:")) {
                        // Here shall we handle different
                        // types of errors
                        retval |= LATEX_ERROR;
                        lyxerr.debug("Build Error.", Error::LATEX);
                        // this is not correct yet
                        terr.scanError(lex);
                        num_errors++;
                }
        }       
        return retval;
}


int Literate::scanBuildLogFile(TeXErrors &terr)
{
        LString token;
        int retval = NO_ERRORS;
        
        LyXLex lex(NULL, 0);
 
        LString tmp = litfile + ".log";
        
        if (!lex.setFile(tmp)) {
                // unable to open file
                // return at once
                retval |= NO_LOGFILE;
                return retval;
        }
        
        while (lex.IsOK()) {
                if (lex.EatLine())
                        token = lex.GetString();
                else // blank line in the file being read
                        continue;
 
                lyxerr.debug(token, Error::LATEX);
                
                if (token.prefixIs("Build Warning:")) {
                        // Here shall we handle different
                        // types of warnings
                        retval |= LATEX_WARNING;
                        lyxerr.debug("Build Warning.", Error::LATEX);
                } else if (token.prefixIs("! Build Error:")) {
                        // Here shall we handle different
                        // types of errors
                        retval |= LATEX_ERROR;
                        lyxerr.debug("Build Error.", Error::LATEX);
                        // this is not correct yet
                        terr.scanError(lex);
                        num_errors++;
                }
        }       
        return retval;
}



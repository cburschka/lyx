/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor 	 
 *	     Copyright 1995 Matthias Ettrich
 *           Copyright 1995-1999 The LyX Team.
 *
 * ====================================================== 
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif
#include <fstream>

#include "support/filetools.h"
#include "LaTeX.h"
#include "Literate.h"
#include "support/FileInfo.h"
#include "debug.h"
#include "support/lyxlib.h"
#include "support/syscall.h"
#include "support/syscontr.h"
#include "support/path.h"
#include "bufferlist.h"
#include "minibuffer.h"
#include "gettext.h"

using std::ifstream;

extern BufferList bufferlist;

Literate::Literate(string const & latex, string const & f, string const & p, 
		   string const & l, 
		   string const & literate, string const & literate_f, 
		   string const & build, string const & build_f)
                   : LaTeX(latex, f, p),
		     litfile(l),
		     literate_cmd(literate), literate_filter(literate_f), 
		     build_cmd(build), build_filter(build_f)
{}


int Literate::weave(TeXErrors & terr, MiniBuffer * minib)
{
        int scanres = Literate::NO_ERRORS;
        string tmp1, tmp2;
        int ret1, ret2;
        Systemcalls one, two;

        // The class LaTeX does not know the temp path.
        bufferlist.updateIncludedTeXfiles(GetCWD());
        
        lyxerr[Debug::LATEX] << "Weaving document" << endl;
        minib->Set(string(_("Weaving document")));
        minib->Store();

        // Run the literate program to convert \literate_extension file to .tex file
        //
        tmp1 = literate_cmd + " < " + litfile + " > " + file + " 2> " + litfile + ".out";
        tmp2 = literate_filter + " < " + litfile + ".out" + " > " + litfile + ".log";
        ret1 = one.startscript(Systemcalls::System, tmp1);
        ret2 = two.startscript(Systemcalls::System, tmp2);
        lyxerr.debug() << "LITERATE {" << tmp1 << "} {" << tmp2 << "}" << endl;
	scanres = scanLiterateLogFile();
	if (scanres & Literate::ERRORS) return scanres; // return on literate error
	return run(terr, minib);
}


int Literate::build(TeXErrors & /*terr*/, MiniBuffer * minib)
        // We know that this function will only be run if the lyx buffer
        // has been changed. 
{
        int scanres = Literate::NO_ERRORS;
        num_errors = 0; // just to make sure.
        string tmp1, tmp2;
        int ret1, ret2;
        Systemcalls one, two;
        
        // The class LaTeX does not know the temp path.
        bufferlist.updateIncludedTeXfiles(GetCWD());
        
        lyxerr[Debug::LATEX] << "Building program" << endl;
        minib->Set(string(_("Building program")));
        minib->Store();

        // Run the build program
        //
        tmp1 = build_cmd + ' ' + litfile + " > " + litfile + ".out 2>&1";
        tmp2 = build_filter + " < " + litfile + ".out" + " > " + litfile + ".log";
        ret1 = one.startscript(Systemcalls::System, tmp1);
        ret2 = two.startscript(Systemcalls::System, tmp2);
        scanres = scanBuildLogFile();
        lyxerr[Debug::LATEX] << "Done." << endl;

        return scanres;
}


int Literate::scanLiterateLogFile()
{
        string token;
        int retval = NO_ERRORS;
        
        string tmp = litfile + ".log";
        
        ifstream ifs(tmp.c_str());
	while (getline(ifs, token)) {
                lyxerr[Debug::LATEX] << token << endl;
                
                if (prefixIs(token, "Build Warning:")) {
                        // Here shall we handle different
                        // types of warnings
                        retval |= LATEX_WARNING;
                        lyxerr[Debug::LATEX] << "Build Warning." << endl;
                } else if (prefixIs(token, "! Build Error:")) {
                        // Here shall we handle different
                        // types of errors
                        retval |= LATEX_ERROR;
                        lyxerr[Debug::LATEX] << "Build Error." << endl;
                        // this is not correct yet
                        num_errors++;
                }
        }       
        return retval;
}


int Literate::scanBuildLogFile()
{
        string token;
        int retval = NO_ERRORS;
 
        string tmp = litfile + ".log";
        
        ifstream ifs(tmp.c_str());
	while (getline(ifs, token)) {
                lyxerr[Debug::LATEX] << token << endl;
                
                if (prefixIs(token, "Build Warning:")) {
                        // Here shall we handle different
                        // types of warnings
                        retval |= LATEX_WARNING;
                        lyxerr[Debug::LATEX] << "Build Warning." << endl;
                } else if (prefixIs(token, "! Build Error:")) {
                        // Here shall we handle different
                        // types of errors
                        retval |= LATEX_ERROR;
                        lyxerr[Debug::LATEX] << "Build Error." << endl;
                        // this is not correct yet
                        num_errors++;
                }
        }       
        return retval;
}

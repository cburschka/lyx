/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor 	 
 *	     Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2000 The LyX Team.
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
using std::getline;
using std::endl;

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
	scanres = scanLiterateLogFile(terr);
	if (scanres & Literate::ERRORS) return scanres; // return on literate error
	return run(terr, minib);
}


int Literate::build(TeXErrors & terr, MiniBuffer * minib)
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
        scanres = scanBuildLogFile(terr);
        lyxerr[Debug::LATEX] << "Done." << endl;

        return scanres;
}


int Literate::scanLiterateLogFile(TeXErrors & terr)
{
        string tmp = litfile + ".log";
        
	int last_line = -1;
	int line_count = 1;
	int retval = NO_ERRORS;
	lyxerr[Debug::LATEX] << "Log file: " << tmp << endl;
	ifstream ifs(tmp.c_str());

	string token;
	while (getline(ifs, token)) {
		lyxerr[Debug::LATEX] << "Log line: " << token << endl;
		
		if (token.empty())
			continue;

		if (prefixIs(token, "! ")) {
			// Ok, we have something that looks like a TeX Error
			// but what do we really have.

			// Just get the error description:
			string desc(token, 2);
			if (contains(token, "Build Error:"))
				retval |= LATEX_ERROR;
			// get the next line
			string tmp;
			int count = 0;
			do {
				if (!getline(ifs, tmp))
					break;
				if (++count > 10)
					break;
			} while (!prefixIs(tmp, "l."));
			if (prefixIs(tmp, "l.")) {
				// we have a build error
				retval |=  TEX_ERROR;
				// get the line number:
				int line = 0;
				sscanf(tmp.c_str(), "l.%d", &line);
				// get the rest of the message:
				string errstr(tmp, tmp.find(' '));
				errstr += '\n';
				getline(ifs, tmp);
				while (!contains(errstr, "l.")
				       && !tmp.empty()
				       && !prefixIs(tmp, "! ")
				       && !contains(tmp, " ...")) {
					errstr += tmp;
					errstr += "\n";
					getline(ifs, tmp);
				}
				lyxerr[Debug::LATEX]
					<< "line: " << line << '\n'
					<< "Desc: " << desc << '\n'
					<< "Text: " << errstr << endl;
				if (line == last_line)
					++line_count;
				else {
					line_count = 1;
					last_line = line;
				}
				if (line_count <= 5) {
					terr.insertError(line, desc, errstr);
					++num_errors;
				}
			}
		}
	}
	lyxerr[Debug::LATEX] << "Log line: " << token << endl;
	return retval;
}


int Literate::scanBuildLogFile(TeXErrors & terr)
{
        string tmp = litfile + ".log";
        
	int last_line = -1;
	int line_count = 1;
	int retval = NO_ERRORS;
	lyxerr[Debug::LATEX] << "Log file: " << tmp << endl;
	ifstream ifs(tmp.c_str());

	string token;
	while (getline(ifs, token)) {
		lyxerr[Debug::LATEX] << "Log line: " << token << endl;
		
		if (token.empty())
			continue;

		if (prefixIs(token, "! ")) {
			// Ok, we have something that looks like a TeX Error
			// but what do we really have.

			// Just get the error description:
			string desc(token, 2);
			if (contains(token, "Build Error:"))
				retval |= LATEX_ERROR;
			// get the next line
			string tmp;
			int count = 0;
			do {
				if (!getline(ifs, tmp))
					break;
				if (++count > 10)
					break;
			} while (!prefixIs(tmp, "l."));
			if (prefixIs(tmp, "l.")) {
				// we have a build error
				retval |=  TEX_ERROR;
				// get the line number:
				int line = 0;
				sscanf(tmp.c_str(), "l.%d", &line);
				// get the rest of the message:
				string errstr(tmp, tmp.find(' '));
				errstr += '\n';
				getline(ifs, tmp);
				while (!contains(errstr, "l.")
				       && !tmp.empty()
				       && !prefixIs(tmp, "! ")
				       && !contains(tmp, " ...")) {
					errstr += tmp;
					errstr += "\n";
					getline(ifs, tmp);
				}
				lyxerr[Debug::LATEX]
					<< "line: " << line << '\n'
					<< "Desc: " << desc << '\n'
					<< "Text: " << errstr << endl;
				if (line == last_line)
					++line_count;
				else {
					line_count = 1;
					last_line = line;
				}
				if (line_count <= 5) {
					terr.insertError(line, desc, errstr);
					++num_errors;
				}
			}
		}
	}
	lyxerr[Debug::LATEX] << "Log line: " << token << endl;
	return retval;
}

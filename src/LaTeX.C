/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor 	 
 *	     Copyright 1995 Matthias Ettrich
 *           Copyright 1995-1999 The LyX Team.
 *
 *           This file is Copyright 1996-1999
 *           Lars Gullik Bjønnes
 *
 * ======================================================
 */

#include <config.h>

#include <cstdio>
#include <cstdlib>

#ifdef __GNUG__
#pragma implementation
#endif

#include "support/filetools.h"
#include "LaTeX.h"
#include "lyxlex.h"
#include "support/FileInfo.h"
#include "debug.h"
#include "support/lyxlib.h"
#include "support/syscall.h"
#include "support/syscontr.h"
#include "pathstack.h"
#include "bufferlist.h"
#include "minibuffer.h"
#include "gettext.h"

extern BufferList bufferlist;

struct texfile_struct {
	LaTeX::TEX_FILES file;
	char const *extension;
};

static
const texfile_struct all_files[] = {
	{ LaTeX::AUX, ".aux"},
	{ LaTeX::BBL, ".bbl"},
	{ LaTeX::DVI, ".dvi"},
	{ LaTeX::GLO, ".glo"},
	{ LaTeX::IDX, ".idx"},
	{ LaTeX::IND, ".ind"},
	{ LaTeX::LOF, ".lof"},
	{ LaTeX::LOA, ".loa"},
	{ LaTeX::LOG, ".log"},
	{ LaTeX::LOT, ".lot"},
	{ LaTeX::TOC, ".toc"},
	{ LaTeX::LTX, ".ltx"},
	{ LaTeX::TEX, ".tex"}
};

// This should perhaps be placed in LyXLex
static
string readLine(FILE *file)
{
	if (feof(file))
		return string();

	int i = 0;
	char s[512];

	do {
		s[i] = fgetc(file);
		i++;
	} while (!feof(file) && s[i-1] != '\n' && i<510);
	s[i] = '\0';
	string tmp;
	if (i == 1 && feof(file))
		;
	else
		tmp = s;

	return tmp;
}



/*
 * CLASS TEXERRORS
 */

// I did not leave this inlined because DEC cxx does not like
// variables declarations in inlined code (JMarc)
TeXErrors::~TeXErrors()
{
	Error *tmp;
	while (errors) {
		tmp = errors->next_error;
		delete errors;
		errors = tmp;
	}
}


void TeXErrors::scanError(LyXLex &lex)
{
	string token = lex.GetString();
	// Sometimes the error string goes over more than one
	// line, and we need to get them all.
	string errstr;
	string tmp = frontStrip(readLine(lex.getFile()));
	if (tmp == "\n" || tmp.empty()) {
		tmp = frontStrip(readLine(lex.getFile()));
		if (contains(tmp, "See the LaTeX manual")) {
			do {
				tmp = readLine(lex.getFile());
			} while (!tmp.empty() && !contains(tmp, "..."));
		}
		tmp = frontStrip(readLine(lex.getFile()));
	}

	while ((tmp != "\n" || !contains(errstr, "l."))
		&& !prefixIs(tmp, "! ")
		&& !contains(tmp, "(job aborted")
		&& !tmp.empty()) {
		errstr += tmp;
		tmp = frontStrip(readLine(lex.getFile()));
	}
	lyxerr.debug() << "tmp: " << errstr << endl;
	int line = 0;
	// unfortunately the error line is not always given
	// by "l.###" in the beginning of the error string
	// therefore we must search for "l.###" in the error
	// msg.
	if (contains(errstr, "l.")) {
		// We make a const copy to make [] fast. (Asger)
		string const es = errstr;
		for (string::size_type i = 2; i < es.length(); ++i) {
			if (es[i-2] == 'l' && es[i-1] == '.' &&
			    (es[i] >= '0' && es[i]<= '9')) {
				line = atoi(es.c_str() + i);
				break;
			}
		}
	}
	insertError(line, token, errstr);

	if (prefixIs(tmp, "! ")) {
		scanError(lex);
	}
}


bool TeXErrors::getFirstError(int *line, string *text)
{
        next_error = errors;
        if (next_error) {
                *line = next_error->error_in_line;
                *text = next_error->error_desc + "\n" + next_error->error_text;
                next_error = next_error->next_error;
                return true;
        }
        return false;
}


bool TeXErrors::getNextError(int *line, string *text)
{
        if (next_error) {
                *line = next_error->error_in_line;
                *text = next_error->error_desc + "\n" + next_error->error_text;
                next_error = next_error->next_error;
                return true;
        }
        return false;
}


void TeXErrors::insertError(int line, string const &error_desc,
			    string const &error_text)
{
        Error *newerr = new Error(line, error_desc, error_text);
        if (errors) {
                Error *tmperr = errors;
                while (tmperr->next_error) tmperr = tmperr->next_error;
                tmperr->next_error = newerr;
        } else {
                errors = newerr;
        }
}


void TeXErrors::printErrors()
{
        lyxerr << "Printing errors." << endl;
        if (errors) {
                Error *tmperr = errors;
                do {
                        lyxerr << "Error in line "
			       << tmperr->error_in_line
			       << ": " << tmperr->error_desc
			       << '\n' << tmperr->error_text << endl;
			//%d: %s\n%s\n", tmperr->error_in_line,
			//     tmperr->error_desc.c_str(),
			//     tmperr->error_text.c_str());
                        tmperr = tmperr->next_error;
                } while (tmperr);
        }
}


void TeXErrors::printWarnings()
{
}


void TeXErrors::printStatus()
{
        lyxerr << "Error struct:"
	       << "\n   status: " << status
	       << "\n   no err: " << number_of_errors << endl;
        if (status == LaTeX::NO_ERRORS)  lyxerr << "NO_ERRORS" << endl;
        if (status & LaTeX::NO_LOGFILE)  lyxerr << "NO_LOGFILE" << endl;
        if (status & LaTeX::NO_OUTPUT)   lyxerr << "NO_OUTPUT" << endl;
        if (status & LaTeX::UNDEF_REF)   lyxerr << "UNDEF_REF" << endl;
	if (status & LaTeX::RERUN)       lyxerr << "RERUN" << endl;
        if (status & LaTeX::TEX_ERROR)   lyxerr << "TEX_ERROR" << endl;
        if (status & LaTeX::TEX_WARNING) lyxerr << "TEX_WARNING" << endl;
        if (status & LaTeX::NO_FILE)     lyxerr << "NO_FILE" << endl;
}


/*
 * CLASS LaTeX
 */

LaTeX::LaTeX(string const & latex, string const & f, string const & p)
		: cmd(latex), file(f), path(p)
{
	tex_files = NO_FILES;
	file_count = sizeof(all_files) / sizeof(texfile_struct);
	num_errors = 0;
	depfile = file + ".dep";
}


int LaTeX::run(TeXErrors &terr, MiniBuffer *minib)
	// We know that this function will only be run if the lyx buffer
	// has been changed. We also know that a newly written .tex file
	// is always different from the previous one because of the date
	// in it. However it seems safe to run latex (at least) on time each
	// time the .tex file changes.
{
	int scanres = LaTeX::NO_ERRORS;
	unsigned int count = 0; // number of times run
	num_errors = 0; // just to make sure.
	const unsigned int MAX_RUN = 6;
	DepTable head; // empty head
	bool rerun = false; // rerun requested
	
	// The class LaTeX does not know the temp path.
	bufferlist.updateIncludedTeXfiles(GetCWD());
	
	// Never write the depfile if an error was encountered.
	
	// 0
	// first check if the file dependencies exist:
	//     ->If it does exist
	//             check if any of the files mentioned in it have
	//             changed (done using a checksum).
	//                 -> if changed:
	//                        run latex once and
	//                        remake the dependency file
	//                 -> if not changed:
	//                        just return there is nothing to do for us.
	//     ->if it doesn't exist
	//             make it and
	//             run latex once (we need to run latex once anyway) and
	//             remake the dependency file.
	//
	FileInfo fi(depfile);
	if (fi.exist()) {
		// Read the dep file:
		head.read(depfile);
		// Update the checksums
		head.update();
		
		lyxerr[Debug::LATEX] << "Dependency file exists" << endl;
		if (head.sumchange()) {
			++count;
			lyxerr[Debug::LATEX]
				<< "Dependency file has changed\n"
				<< "Run #" << count << endl; 
			minib->Set(string(_("LaTeX run number ")) + tostr(count));
			minib->Store();
			this->operator()();
			scanres = scanLogFile(terr);
			if (scanres & LaTeX::ERRORS) return scanres; // return on error
		} else {
			lyxerr[Debug::LATEX] << "return no_change" << endl;
			return LaTeX::NO_CHANGE;
		}
	} else {
		++count;
		lyxerr[Debug::LATEX] << "Dependency file does not exist\n"
				     << "Run #" << count << endl;
		head.insert(file, true);
		minib->Set(string(_("LaTeX run number ")) + tostr(count));
		minib->Store();
		this->operator()();
		scanres = scanLogFile(terr);
		if (scanres & LaTeX::ERRORS) return scanres; // return on error
	}

	// update the dependencies.
	deplog(head); // reads the latex log
	deptex(head); // checks for latex files
	head.update();

	// 0.5
	// At this point we must run external programs if needed.
	// makeindex will be run if a .idx file changed or was generated.
	// And if there were undefined citations or changes in references
	// the .aux file is checked for signs of bibtex. Bibtex is then run
	// if needed.
	
	// run makeindex
	if (head.haschanged(ChangeExtension(file, ".idx", true))) {
		// no checks for now
		minib->Set(_("Running MakeIndex."));
		minib->Store();
		rerun=runMakeIndex(ChangeExtension(file,".idx",true));
	}

	// run bibtex
	if (scanres & LaTeX::UNDEF_CIT || scanres & LaTeX::RERUN) {
		// Here we must scan the .aux file and look for
		// "\bibdata" and/or "\bibstyle". If one of those
		// tags is found -> run bibtex and set rerun = true;
		// no checks for now
		minib->Set(_("Running BibTeX."));
		minib->Store();
		rerun = runBibTeX(ChangeExtension(file, ".aux", true));
	}
	
	// 1
	// we know on this point that latex has been run once (or we just
	// returned) and the question now is to decide if we need to run
	// it any more. This is done by asking if any of the files in the
	// dependency file has changed. (remember that the checksum for
	// a given file is reported to have changed if it just was created)
	//     -> if changed or rerun == true:
	//             run latex once more and
	//             update the dependency structure
	//     -> if not changed:
	//             we does nothing at this point
	//
	if (rerun || head.sumchange()) {
		rerun = false;
		++count;
		lyxerr[Debug::LATEX]
			<< "Dep. file has changed or rerun requested\n"
			<< "Run #" << count << endl;
		minib->Set(string(_("LaTeX run number ")) + tostr(count));
		minib->Store();
		this->operator()();
		scanres = scanLogFile(terr);
		if (scanres & LaTeX::ERRORS) return scanres; // return on error
		// update the depedencies
		deplog(head); // reads the latex log
		head.update();
	} else {
		lyxerr[Debug::LATEX] << "Dep. file has NOT changed" << endl;
	}

	// 1.5
	// The inclusion of files generated by external programs like
	// makeindex or bibtex might have done changes to pagenumbereing,
	// etc. And because of this we must run the external programs
	// again to make sure everything is redone correctly.
	// Also there should be no need to run the external programs any
	// more after this.
	
	// run makeindex if the <file>.idx has changed or was generated.
	if (head.haschanged(ChangeExtension(file, ".idx", true))) {
		// no checks for now
		minib->Set(_("Running MakeIndex."));
		minib->Store();
		rerun = runMakeIndex(ChangeExtension(file, ".idx", true));
	}
	
	// 2
	// we will only run latex more if the log file asks for it.
	// or if the sumchange() is true.
	//     -> rerun asked for:
	//             run latex and
	//             remake the dependency file
	//             goto 2 or return if max runs are reached.
	//     -> rerun not asked for:
	//             just return (fall out of bottom of func)
	//
	while ((head.sumchange() || rerun || (scanres & LaTeX::RERUN)) 
	       && count < MAX_RUN) {
		// Yes rerun until message goes away, or until
		// MAX_RUNS are reached.
		rerun = false;
		++count;
		lyxerr[Debug::LATEX] << "Run #" << count << endl;
		minib->Set(string(_("LaTeX run number ")) + tostr(count));
		minib->Store();
		this->operator()();
		scanres = scanLogFile(terr);
		if (scanres & LaTeX::ERRORS) return scanres; // return on error
		// keep this updated
		head.update();
	}

	// Write the dependencies to file.
	head.write(depfile);
	lyxerr[Debug::LATEX] << "Done." << endl;
	return scanres;
}


int LaTeX::operator()()
{
#ifndef __EMX__
	string tmp = cmd + ' ' + file + " > /dev/null";
#else // cmd.exe (OS/2) causes SYS0003 error at "/dev/null"
	string tmp = cmd + ' ' + file + " > nul";
#endif
        Systemcalls one;
	return one.Startscript(Systemcalls::System, tmp);
}


bool LaTeX::runMakeIndex(string const &file)
{
	lyxerr[Debug::LATEX] << "idx file has been made,"
		" running makeindex on file "
			     <<  file << endl;

	// It should be possible to set the switches for makeindex
	// sorting style and such. It would also be very convenient
	// to be able to make style files from within LyX. This has
	// to come for a later time. (0.13 perhaps?)
	string tmp = "makeindex -c -q ";
	tmp += file;
	Systemcalls one;
	one.Startscript(Systemcalls::System, tmp);
	return true;
}


bool LaTeX::runBibTeX(string const &file)
{
	LyXLex lex(0, 0);
	string token;
	if (!lex.setFile(file)) {
		// unable to open .aux file
		// return at once
		return false;
	}

	while (lex.IsOK()) {
		if (lex.EatLine())
			token=lex.GetString();
		else // blank line in the file being read
			continue;

		if (contains(token, "\\bibdata{")) {
			// run bibtex and
			string tmp="bibtex ";
			tmp += ChangeExtension(file, string(), true);
			Systemcalls one;
			one.Startscript(Systemcalls::System, tmp);
			return true;
		}
		
	}
	// bibtex was not run.
	return false;
}


int LaTeX::scanLogFile(TeXErrors &terr)
{
	string token;
	int retval = NO_ERRORS;
	
	LyXLex lex(0, 0);

	string tmp = ChangeExtension(file, ".log", true);
	
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

		lyxerr[Debug::LATEX] << token << endl;
		
		if (prefixIs(token, "LaTeX Warning:")) {
			// Here shall we handle different
			// types of warnings
			retval |= LATEX_WARNING;
			lyxerr[Debug::LATEX] << "LaTeX Warning." << endl;
			if (contains(token, "Rerun to get cross-references")) {
				retval |= RERUN;
				lyxerr[Debug::LATEX]
					<< "We should rerun." << endl;
			} else if (contains(token, "Citation")
				   && contains(token, "on page")
				   && contains(token, "undefined")) {
				retval |= UNDEF_CIT;
			}
		} else if (prefixIs(token, "Package")) {
			// Package warnings
			retval |= PACKAGE_WARNING;
			if (contains(token, "natbib Warning:")) {
				// Natbib warnings
				if (contains(token, "Citation")
				    && contains(token, "on page")
				    && contains(token, "undefined")) {
					retval |= UNDEF_CIT;
				}
			} else if (contains(token, "Rerun LaTeX.")) {
				// at least longtable.sty might use this.
				retval |= RERUN;
			}
		} else if (prefixIs(token, "! LaTeX Error:")) {
			// Here shall we handle different
			// types of errors
			retval |= LATEX_ERROR;
			lyxerr[Debug::LATEX] << "LaTeX Error." << endl;
			// this is not correct yet
			terr.scanError(lex);
			num_errors++;
		} else if (prefixIs(token, "! ")) {
			// Ok, we have something that looks like a TeX Error
			// but what do we really have.

			// Just get the error description:
			string desc(token);
			desc.erase(0, 2);

			if (contains(desc, "Undefined control sequence")) {
				retval |= TEX_ERROR;
				lyxerr[Debug::LATEX] << "TeX Error." << endl;
				terr.scanError(lex);
				num_errors++;
			} else {
				// get the next line
				lex.next();
				string tmp = lex.GetString();
				if (prefixIs(tmp, "l.")) {
				// we have a latex error
					retval |=  TEX_ERROR;
					lyxerr[Debug::LATEX]
						<<"TeX Error." << endl;
				// get the line number:
					int line = 0;
					sscanf(tmp.c_str(), "l.%d", &line);
				// get the rest of the message:
					string errstr;
					lex.EatLine();
					tmp = lex.GetString();
					while ((tmp != "\n" || !contains(errstr, "l."))
					       && !prefixIs(tmp, "! ")
					       && !contains(tmp, "(job aborted")
					       && !tmp.empty()) {
						errstr += tmp;
						errstr += "\n";
						lex.EatLine();
						tmp = lex.GetString();
					}
					terr.insertError(line, desc, errstr);
					num_errors++;
				}
			}
		} else {
			// information messages, TeX warnings and other
			// warnings we have not caught earlier.
			if (prefixIs(token, "Overfull ")) {
				retval |= TEX_WARNING;
			} else if (prefixIs(token, "Underfull ")) {
				retval |= TEX_WARNING;
			} else if (contains(token, "Rerun to get citations")) {
				// Natbib seems to use this.
				retval |= RERUN;
			} else if (contains(token, "No pages of output")) {
				// A dvi file was not created
				retval |= NO_OUTPUT;
			} else if (contains(token, "That makes 100 errors")) {
				// More than 100 errors were reprted
				retval |= TOO_MANY_ERRORS;
			}
		}
	}	
	return retval;
}


void LaTeX::deplog(DepTable & head)
{
	// This function reads the LaTeX log file end extracts all the external
	// files used by the LaTeX run. The files are then entered into the
	// dependency file.

	string logfile = ChangeExtension(file, ".log", true);
	FilePtr in(logfile, FilePtr::read);
	bool not_eof = true;
	if (in()) while (not_eof) { // We were able to open the file
		// Now we read chars until we find a '('
		int c;
		do {
			c = fgetc(in());
		} while (c != EOF && c != '(');
		if (c == EOF) { 
			// Nothing more we can do
			not_eof = false; 
			continue;
		} 

		// We now have c == '(', we now read the the sequence of
		// chars until reaching EOL, or ' ' and put that into a string.

		string foundfile;
		c = fgetc(in());
		while (c != '\n' && c != ' ' && c != ')') {
			foundfile += char(c);
			c = fgetc(in());
		}
		if (foundfile.empty()) continue;

		lyxerr[Debug::LATEX] << "Found file: " 
				     << foundfile << endl;
		// Ok now we found a file.
		// Now we should make sure that
		// this is a file that we can
		// access through the normal
		// paths:
		// (1) foundfile is an
		//     absolute path and should
		//     be inserted.
		if (AbsolutePath(foundfile)) {
			lyxerr[Debug::LATEX] << "AbsolutePath file: " 
					     << foundfile << endl;
			// On inital insert we want to do the update at once
			// since this file can not be a file generated by
			// the latex run.
			head.insert(foundfile, true);
			continue;
		}

		// (2) foundfile is in the tmpdir
		//     insert it into head
		if (FileInfo(OnlyFilename(foundfile)).exist()) {
			if (suffixIs(foundfile, ".aux")) {
				lyxerr[Debug::LATEX] << "We don't want "
						     << OnlyFilename(foundfile)
						     << " in the dep file"
						     << endl;
			} else if (suffixIs(foundfile, ".tex")) {
				// This is a tex file generated by LyX
				// and latex is not likely to change this
				// during its runs.
				lyxerr[Debug::LATEX] << "Tmpdir TeX file: "
						     << OnlyFilename(foundfile)
						     << endl;
				head.insert(foundfile, true);
			} else {
				lyxerr[Debug::LATEX] << "In tmpdir file:"
						     << OnlyFilename(foundfile)
						     << endl;
				head.insert(OnlyFilename(foundfile));
			}
			continue;
		}

		// (3) the foundfile can be
		//     found in the same dir
		//     as the .lyx file and
		//     should be inserted.
		PathPush(path);
		if (FileInfo(foundfile).exist()) {
			lyxerr << "LyX Strange: this should actually never"
				" happen anymore, this it should be"
				" handled by the Absolute check."
			       << endl;
			lyxerr[Debug::LATEX] << "Same Directory file: " 
					     << foundfile << endl;
			head.insert(foundfile);
			PathPop();
			continue;
		}
		PathPop();
		
		lyxerr[Debug::LATEX]
			<< "Not a file or we are unable to find it."
			<< endl;
	}
}


void LaTeX::deptex(DepTable &head)
{
	int except = AUX|LOG|DVI|BBL|IND|GLO; 
	string tmp;
	FileInfo fi;
	for (int i = 0; i < file_count; i++) {
		if (!(all_files[i].file & except)) {
			tmp = ChangeExtension(file,
					      all_files[i].extension,
					      true);
			lyxerr[Debug::LATEX] << "deptex: " << tmp << endl;
			if (fi.newFile(tmp).exist())
				head.insert(tmp);
		}
	}
}

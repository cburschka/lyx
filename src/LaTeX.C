/* This file is part of
 * ======================================================
 *
 *           LyX, The Document Processor
 *	     Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2001 The LyX Team.
 *
 *           This file is Copyright 1996-2001
 *           Lars Gullik Bjønnes
 *
 * ======================================================
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif
#include <fstream>

#include "LaTeX.h"
#include "bufferlist.h"
#include "gettext.h"
#include "lyxfunc.h"
#include "debug.h"
#include "support/filetools.h"
#include "support/FileInfo.h"
#include "support/LRegex.h"
#include "support/LSubstring.h"
#include "support/lstrings.h"
#include "support/lyxlib.h"
#include "support/systemcall.h"
#include "support/os.h"
#include "support/path.h"
#include <cstdio>  // sscanf

#ifndef CXX_GLOBAL_CSTD
using std::sscanf;
#endif

using std::ifstream;
using std::getline;
using std::endl;
using std::vector;
using std::set;

// TODO: in no particular order
// - get rid of the extern BufferList and the call to
//   BufferList::updateIncludedTeXfiles, this should either
//   be done before calling LaTeX::funcs or in a completely
//   different way.
// - the bibtex command options should be supported.
// - the makeindex style files should be taken care of with
//   the dependency mechanism.
// - makeindex commandline options should be supported
// - somewhere support viewing of bibtex and makeindex log files.
// - we should perhaps also scan the bibtex log file
// - we should perhaps also scan the bibtex log file

extern BufferList bufferlist;

/*
 * CLASS TEXERRORS
 */

void TeXErrors::insertError(int line, string const & error_desc,
			    string const & error_text)
{
	Error newerr(line, error_desc, error_text);
	errors.push_back(newerr);
}

/*
 * CLASS LaTeX
 */

LaTeX::LaTeX(string const & latex, string const & f, string const & p)
		: cmd(latex), file(f), path(p)
{
	num_errors = 0;
	depfile = file + ".dep";
	if (prefixIs(cmd, "pdf")) { // Do we use pdflatex ?
		depfile += "-pdf";
		output_file = ChangeExtension(file,".pdf");
	} else {
		output_file = ChangeExtension(file,".dvi");
	}
}


void LaTeX::deleteFilesOnError() const
{
	// currently just a dummy function.

	// What files do we have to delete?

	// This will at least make latex do all the runs
	lyx::unlink(depfile);

	// but the reason for the error might be in a generated file...

	string ofname = OnlyFilename(file);

	// bibtex file
	string bbl = ChangeExtension(ofname, ".bbl");
	lyx::unlink(bbl);

	// makeindex file
	string ind = ChangeExtension(ofname, ".ind");
	lyx::unlink(ind);

	// Also remove the aux file
	string aux = ChangeExtension(ofname, ".aux");
	lyx::unlink(aux);
}


int LaTeX::run(TeXErrors & terr, LyXFunc * lfun)
	// We know that this function will only be run if the lyx buffer
	// has been changed. We also know that a newly written .tex file
	// is always different from the previous one because of the date
	// in it. However it seems safe to run latex (at least) on time each
	// time the .tex file changes.
{
	int scanres = NO_ERRORS;
	unsigned int count = 0; // number of times run
	num_errors = 0; // just to make sure.
	const unsigned int MAX_RUN = 6;
	DepTable head; // empty head
	bool rerun = false; // rerun requested

	// The class LaTeX does not know the temp path.
	bufferlist.updateIncludedTeXfiles(lyx::getcwd()); //GetCWD());

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
	bool had_depfile = fi.exist();
	bool run_bibtex = false;
	string aux_file = OnlyFilename(ChangeExtension(file, "aux"));

	if (had_depfile) {
		lyxerr[Debug::DEPEND] << "Dependency file exists" << endl;
		// Read the dep file:
		head.read(depfile);
		// Update the checksums
		head.update();
		// Can't just check if anything has changed because it might have aborted
		// on error last time... in which cas we need to re-run latex
		// and collect the error messages (even if they are the same).
		if (!FileInfo(output_file).exist()) {
			lyxerr[Debug::DEPEND]
				<< "re-running LaTeX because output file doesn't exist." << endl;
		} else if (!head.sumchange()) {
			lyxerr[Debug::DEPEND] << "return no_change" << endl;
			return NO_CHANGE;
		} else {
			lyxerr[Debug::DEPEND]
				<< "Dependency file has changed" << endl;
		}

		if (head.extchanged(".bib") || head.extchanged(".bst"))
			run_bibtex = true;
	} else
		lyxerr[Debug::DEPEND]
			<< "Dependency file does not exist" << endl;

	/// We scan the aux file even when had_depfile = false,
	/// because we can run pdflatex on the file after running latex on it,
	/// in which case we will not need to run bibtex again.
	vector<Aux_Info> bibtex_info_old;
	if (!run_bibtex)
		bibtex_info_old = scanAuxFiles(aux_file);

	++count;
	lyxerr[Debug::LATEX] << "Run #" << count << endl;
	if (lfun) {
		ostringstream str;
		str << _("LaTeX run number") << ' ' << count;
		lfun->dispatch(LFUN_MESSAGE, str.str().c_str());
	}

	this->operator()();
	scanres = scanLogFile(terr);
	if (scanres & ERROR_RERUN) {
		lyxerr[Debug::LATEX] << "Rerunning LaTeX" << endl;
		this->operator()();
		scanres = scanLogFile(terr);
	}

	if (scanres & ERRORS) {
		deleteFilesOnError();
		return scanres; // return on error
	}

	vector<Aux_Info> const bibtex_info = scanAuxFiles(aux_file);
	if (!run_bibtex && bibtex_info_old != bibtex_info)
		run_bibtex = true;

	// update the dependencies.
	deplog(head); // reads the latex log
	head.update();

	// 0.5
	// At this point we must run external programs if needed.
	// makeindex will be run if a .idx file changed or was generated.
	// And if there were undefined citations or changes in references
	// the .aux file is checked for signs of bibtex. Bibtex is then run
	// if needed.

	// run makeindex
	if (head.haschanged(OnlyFilename(ChangeExtension(file, ".idx")))) {
		// no checks for now
		lyxerr[Debug::LATEX] << "Running MakeIndex." << endl;
		if (lfun) {
			lfun->dispatch(LFUN_MESSAGE, _("Running MakeIndex."));
		}

		rerun = runMakeIndex(OnlyFilename(ChangeExtension(file, ".idx")));
	}

	// run bibtex
	// if (scanres & UNDEF_CIT || scanres & RERUN || run_bibtex)
	if (scanres & UNDEF_CIT || run_bibtex) {
		// Here we must scan the .aux file and look for
		// "\bibdata" and/or "\bibstyle". If one of those
		// tags is found -> run bibtex and set rerun = true;
		// no checks for now
		lyxerr[Debug::LATEX] << "Running BibTeX." << endl;
		if (lfun) {
			lfun->dispatch(LFUN_MESSAGE, _("Running BibTeX."));
		}

		updateBibtexDependencies(head, bibtex_info);
		rerun |= runBibTeX(bibtex_info);
	} else if (!had_depfile) {
		/// If we run pdflatex on the file after running latex on it,
		/// then we do not need to run bibtex, but we do need to
		/// insert the .bib and .bst files into the .dep-pdf file.
		updateBibtexDependencies(head, bibtex_info);
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
		lyxerr[Debug::DEPEND]
			<< "Dep. file has changed or rerun requested" << endl;
		lyxerr[Debug::LATEX]
			<< "Run #" << count << endl;
		if (lfun) {
			ostringstream str;
			str << _("LaTeX run number") << ' ' << count;
			lfun->dispatch(LFUN_MESSAGE, str.str().c_str());
		}

		this->operator()();
		scanres = scanLogFile(terr);
		if (scanres & ERRORS) {
			deleteFilesOnError();
			return scanres; // return on error
		}

		// update the depedencies
		deplog(head); // reads the latex log
		head.update();
	} else {
		lyxerr[Debug::DEPEND] << "Dep. file has NOT changed" << endl;
	}

	// 1.5
	// The inclusion of files generated by external programs like
	// makeindex or bibtex might have done changes to pagenumbereing,
	// etc. And because of this we must run the external programs
	// again to make sure everything is redone correctly.
	// Also there should be no need to run the external programs any
	// more after this.

	// run makeindex if the <file>.idx has changed or was generated.
	if (head.haschanged(OnlyFilename(ChangeExtension(file, ".idx")))) {
		// no checks for now
		lyxerr[Debug::LATEX] << "Running MakeIndex." << endl;
		if (lfun) {
			lfun->dispatch(LFUN_MESSAGE, _("Running MakeIndex."));
		}

		rerun = runMakeIndex(OnlyFilename(ChangeExtension(file, ".idx")));
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
	while ((head.sumchange() || rerun || (scanres & RERUN))
	       && count < MAX_RUN) {
		// Yes rerun until message goes away, or until
		// MAX_RUNS are reached.
		rerun = false;
		++count;
		lyxerr[Debug::LATEX] << "Run #" << count << endl;
		if (lfun) {
			ostringstream str;
			str << _("LaTeX run number") << ' ' << count;
			lfun->dispatch(LFUN_MESSAGE, str.str().c_str());
		}

		this->operator()();
		scanres = scanLogFile(terr);
		if (scanres & ERRORS) {
			deleteFilesOnError();
			return scanres; // return on error
		}

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
	string tmp = cmd + ' ' + QuoteName(file) + " > /dev/null";
#else // cmd.exe (OS/2) causes SYS0003 error at "/dev/null"
	string tmp = cmd + ' ' + file + " > nul";
#endif
	Systemcall one;
	return one.startscript(Systemcall::Wait, tmp);
}


bool LaTeX::runMakeIndex(string const & f)
{
	lyxerr[Debug::LATEX] << "idx file has been made,"
		" running makeindex on file "
			     <<  f << endl;

	// It should be possible to set the switches for makeindex
	// sorting style and such. It would also be very convenient
	// to be able to make style files from within LyX. This has
	// to come for a later time. (0.13 perhaps?)
	string tmp = "makeindex -c -q ";
	tmp += f;
	Systemcall one;
	one.startscript(Systemcall::Wait, tmp);
	return true;
}


vector<Aux_Info> const
LaTeX::scanAuxFiles(string const & file)
{
	vector<Aux_Info> result;

	result.push_back(scanAuxFile(file));

	for (int i = 1; i < 1000; ++i) {
		string file2 = ChangeExtension(file, "") + "." + tostr(i)
			+ ".aux";
		FileInfo fi(file2);
		if (!fi.exist())
			break;
		result.push_back(scanAuxFile(file2));
	}
	return result;
}


Aux_Info const LaTeX::scanAuxFile(string const & file)
{
	Aux_Info result;
	result.aux_file = file;
	scanAuxFile(file, result);
	return result;
}


void LaTeX::scanAuxFile(string const & file, Aux_Info & aux_info)
{
	lyxerr[Debug::LATEX] << "Scanning aux file: " << file << endl;

	ifstream ifs(file.c_str());
	string token;
	LRegex reg1("\\\\citation\\{([^}]+)\\}");
	LRegex reg2("\\\\bibdata\\{([^}]+)\\}");
	LRegex reg3("\\\\bibstyle\\{([^}]+)\\}");
	LRegex reg4("\\\\@input\\{([^}]+)\\}");

	while (getline(ifs, token)) {
		token = strip(token, '\r');
		if (reg1.exact_match(token)) {
			LRegex::SubMatches const & sub = reg1.exec(token);
			string data = LSubstring(token, sub[1].first,
						 sub[1].second);
			while (!data.empty()) {
				string citation;
				data = split(data, citation, ',');
				lyxerr[Debug::LATEX] << "Citation: "
						     << citation << endl;
				aux_info.citations.insert(citation);
			}
		} else if (reg2.exact_match(token)) {
			LRegex::SubMatches const & sub = reg2.exec(token);
			string data = LSubstring(token, sub[1].first,
						 sub[1].second);
			// data is now all the bib files separated by ','
			// get them one by one and pass them to the helper
			while (!data.empty()) {
				string database;
				data = split(data, database, ',');
				database = ChangeExtension(database, "bib");
				lyxerr[Debug::LATEX] << "Bibtex database: `"
						     << database << "'" << endl;
				aux_info.databases.insert(database);
			}
		} else if (reg3.exact_match(token)) {
			LRegex::SubMatches const & sub = reg3.exec(token);
			string style = LSubstring(token, sub[1].first,
						  sub[1].second);
			// token is now the style file
			// pass it to the helper
			style = ChangeExtension(style, "bst");
			lyxerr[Debug::LATEX] << "Bibtex style: `"
					     << style << "'" << endl;
			aux_info.styles.insert(style);
		} else if (reg4.exact_match(token)) {
			LRegex::SubMatches const & sub = reg4.exec(token);
			string file2 = LSubstring(token, sub[1].first,
						  sub[1].second);
			scanAuxFile(file2, aux_info);
		}
	}
}


void LaTeX::updateBibtexDependencies(DepTable & dep,
				     vector<Aux_Info> const & bibtex_info)
{
	// Since a run of Bibtex mandates more latex runs it is ok to
	// remove all ".bib" and ".bst" files.
	dep.remove_files_with_extension(".bib");
	dep.remove_files_with_extension(".bst");
	//string aux = OnlyFilename(ChangeExtension(file, ".aux"));

	for (vector<Aux_Info>::const_iterator it = bibtex_info.begin();
	     it != bibtex_info.end(); ++it) {
		for (set<string>::const_iterator it2 = it->databases.begin();
		     it2 != it->databases.end(); ++it2) {
			string file = findtexfile(*it2, "bib");
			if (!file.empty())
				dep.insert(file, true);
		}

		for (set<string>::const_iterator it2 = it->styles.begin();
		     it2 != it->styles.end(); ++it2) {
			string file = findtexfile(*it2, "bst");
			if (!file.empty())
				dep.insert(file, true);
		}
	}
}


bool LaTeX::runBibTeX(vector<Aux_Info> const & bibtex_info)
{
	bool result = false;
	for (vector<Aux_Info>::const_iterator it = bibtex_info.begin();
	     it != bibtex_info.end(); ++it) {
		if (it->databases.empty())
			continue;
		result = true;

		string tmp = "bibtex ";
		tmp += OnlyFilename(ChangeExtension(it->aux_file, string()));
		Systemcall one;
		one.startscript(Systemcall::Wait, tmp);
	}
	// Return whether bibtex was run
	return result;
}


int LaTeX::scanLogFile(TeXErrors & terr)
{
	int last_line = -1;
	int line_count = 1;
	int retval = NO_ERRORS;
	string tmp = OnlyFilename(ChangeExtension(file, ".log"));
	lyxerr[Debug::LATEX] << "Log file: " << tmp << endl;
	ifstream ifs(tmp.c_str());

	string token;
	while (getline(ifs, token)) {
		lyxerr[Debug::LATEX] << "Log line: " << token << endl;

		if (token.empty())
			continue;

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
			} else if (contains(token, "run BibTeX")) {
				retval |= UNDEF_CIT;
			} else if (contains(token, "Rerun LaTeX") ||
				   contains(token, "Rerun to get")) {
				// at least longtable.sty and bibtopic.sty
				// might use this.
				lyxerr[Debug::LATEX]
					<< "We should rerun." << endl;
				retval |= RERUN;
			}
		} else if (prefixIs(token, "(")) {
			if (contains(token, "Rerun LaTeX") ||
			    contains(token, "Rerun to get")) {
				// Used by natbib
				lyxerr[Debug::LATEX]
					<< "We should rerun." << endl;
				retval |= RERUN;
			}
		} else if (prefixIs(token, "! ")) {
			// Ok, we have something that looks like a TeX Error
			// but what do we really have.

			// Just get the error description:
			string desc(token, 2);
			if (contains(token, "LaTeX Error:"))
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
				// we have a latex error
				retval |=  TEX_ERROR;
				if (contains(desc, "Package babel Error: You haven't defined the language"))
					retval |= ERROR_RERUN;
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
				       && !contains(tmp, "(job aborted")) {
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
		} else {
			// information messages, TeX warnings and other
			// warnings we have not caught earlier.
			if (prefixIs(token, "Overfull ")) {
				retval |= TEX_WARNING;
			} else if (prefixIs(token, "Underfull ")) {
				retval |= TEX_WARNING;
			} else if (contains(token, "Rerun to get citations")) {
				// Natbib seems to use this.
				retval |= UNDEF_CIT;
			} else if (contains(token, "No pages of output")) {
				// A dvi file was not created
				retval |= NO_OUTPUT;
			} else if (contains(token, "That makes 100 errors")) {
				// More than 100 errors were reprted
				retval |= TOO_MANY_ERRORS;
			}
		}
	}
	lyxerr[Debug::LATEX] << "Log line: " << token << endl;
	return retval;
}


void LaTeX::deplog(DepTable & head)
{
	// This function reads the LaTeX log file end extracts all the external
	// files used by the LaTeX run. The files are then entered into the
	// dependency file.

	string const logfile = OnlyFilename(ChangeExtension(file, ".log"));

	LRegex reg1("\\)* *\\(([^ )]+).*");
	LRegex reg2("File: ([^ ]+).*");
	LRegex reg3("No file ([^ ]+)\\..*");
	LRegex reg4("\\\\openout[0-9]+.*=.*`([^ ]+)'\\..*");
	// If an index should be created, MikTex does not write a line like
	//    \openout# = 'sample,idx'.
	// but intstead only a line like this into the log:
	//   Writing index file sample.idx
	LRegex reg5("Writing index file ([^ ]+).*");
	LRegex unwanted("^.*\\.(aux|log|dvi|bbl|ind|glo)$");

	ifstream ifs(logfile.c_str());
	while (ifs) {
		// Ok, the scanning of files here is not sufficient.
		// Sometimes files are named by "File: xxx" only
		// So I think we should use some regexps to find files instead.
		// "(\([^ ]+\)"   should match the "(file " variant
		// "File: \([^ ]+\)" should match the "File: file" variant
		string foundfile;
		string token;
		getline(ifs, token);
		token = strip(token, '\r');
		if (token.empty()) continue;

		if (reg1.exact_match(token)) {
			LRegex::SubMatches const & sub = reg1.exec(token);
			foundfile = LSubstring(token, sub[1].first,
					       sub[1].second);
		} else if (reg2.exact_match(token)) {
			LRegex::SubMatches const & sub = reg2.exec(token);
			foundfile = LSubstring(token, sub[1].first,
					       sub[1].second);
		} else if (reg3.exact_match(token)) {
			LRegex::SubMatches const & sub = reg3.exec(token);
			foundfile = LSubstring(token, sub[1].first,
					       sub[1].second);
		} else if (reg4.exact_match(token)) {
			LRegex::SubMatches const & sub = reg4.exec(token);
			foundfile = LSubstring(token, sub[1].first,
					       sub[1].second);
		} else if (reg5.exact_match(token)) {
			LRegex::SubMatches const & sub = reg5.exec(token);
			foundfile = LSubstring(token, sub[1].first,
					       sub[1].second);
		} else {
			continue;
		}

		// convert from native os path to unix path
		foundfile = os::internal_path(foundfile);

		lyxerr[Debug::DEPEND] << "Found file: "
				      << foundfile << endl;

		// Ok now we found a file.
		// Now we should make sure that this is a file that we can
		// access through the normal paths.
		// We will not try any fancy search methods to
		// find the file.

		// (1) foundfile is an
		//     absolute path and should
		//     be inserted.
		if (AbsolutePath(foundfile)) {
			lyxerr[Debug::DEPEND] << "AbsolutePath file: "
					      << foundfile << endl;
			// On initial insert we want to do the update at once
			// since this file can not be a file generated by
			// the latex run.
			if (FileInfo(foundfile).exist())
				head.insert(foundfile, true);
		}

		// (2) foundfile is in the tmpdir
		//     insert it into head
		else if (FileInfo(OnlyFilename(foundfile)).exist()) {
			if (unwanted.exact_match(foundfile)) {
				lyxerr[Debug::DEPEND]
					<< "We don't want "
					<< OnlyFilename(foundfile)
					<< " in the dep file"
					<< endl;
			} else if (suffixIs(foundfile, ".tex")) {
				// This is a tex file generated by LyX
				// and latex is not likely to change this
				// during its runs.
				lyxerr[Debug::DEPEND]
					<< "Tmpdir TeX file: "
					<< OnlyFilename(foundfile)
					<< endl;
				head.insert(foundfile, true);
			} else {
				lyxerr[Debug::DEPEND]
					<< "In tmpdir file:"
					<< OnlyFilename(foundfile)
					<< endl;
				head.insert(OnlyFilename(foundfile));
			}
		} else
			lyxerr[Debug::DEPEND]
				<< "Not a file or we are unable to find it."
				<< endl;
	}

	// Make sure that the main .tex file is in the dependancy file.
	head.insert(OnlyFilename(file), true);
}

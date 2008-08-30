/**
 * \file LaTeX.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alfredo Braunstein
 * \author Lars Gullik Bjønnes
 * \author Jean-Marc Lasgouttes
 * \author Angus Leeming
 * \author Dekel Tsur
 * \author Jürgen Spitzmüller
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "BufferList.h"
#include "LaTeX.h"
#include "gettext.h"
#include "LyXRC.h"
#include "debug.h"
#include "DepTable.h"

#include "support/filetools.h"
#include "support/convert.h"
#include "support/lstrings.h"
#include "support/lyxlib.h"
#include "support/Systemcall.h"
#include "support/os.h"

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/regex.hpp>

#include <fstream>

using boost::regex;
using boost::smatch;

#ifndef CXX_GLOBAL_CSTD
using std::sscanf;
#endif

using std::endl;
using std::getline;
using std::string;
using std::ifstream;
using std::set;
using std::vector;


namespace lyx {

using support::absolutePath;
using support::bformat;
using support::changeExtension;
using support::contains;
using support::doesFileExist;
using support::FileName;
using support::findtexfile;
using support::getcwd;
using support::makeAbsPath;
using support::onlyFilename;
using support::prefixIs;
using support::quoteName;
using support::removeExtension;
using support::rtrim;
using support::rsplit;
using support::split;
using support::subst;
using support::suffixIs;
using support::Systemcall;
using support::unlink;
using support::trim;

namespace os = support::os;
namespace fs = boost::filesystem;

// TODO: in no particular order
// - get rid of the call to
//   BufferList::updateIncludedTeXfiles, this should either
//   be done before calling LaTeX::funcs or in a completely
//   different way.
// - the makeindex style files should be taken care of with
//   the dependency mechanism.
// - makeindex commandline options should be supported
// - somewhere support viewing of bibtex and makeindex log files.
// - we should perhaps also scan the bibtex log file

namespace {

docstring runMessage(unsigned int count)
{
	return bformat(_("Waiting for LaTeX run number %1$d"), count);
}

} // anon namespace

/*
 * CLASS TEXERRORS
 */

void TeXErrors::insertError(int line, docstring const & error_desc,
			    docstring const & error_text)
{
	Error newerr(line, error_desc, error_text);
	errors.push_back(newerr);
}


bool operator==(Aux_Info const & a, Aux_Info const & o)
{
	return a.aux_file == o.aux_file &&
		a.citations == o.citations &&
		a.databases == o.databases &&
		a.styles == o.styles;
}


bool operator!=(Aux_Info const & a, Aux_Info const & o)
{
	return !(a == o);
}


/*
 * CLASS LaTeX
 */

LaTeX::LaTeX(string const & latex, OutputParams const & rp,
	     FileName const & f)
	: cmd(latex), file(f), runparams(rp)
{
	num_errors = 0;
	if (prefixIs(cmd, "pdf")) { // Do we use pdflatex ?
		depfile = FileName(file.absFilename() + ".dep-pdf");
		output_file =
			FileName(changeExtension(file.absFilename(), ".pdf"));
	} else {
		depfile = FileName(file.absFilename() + ".dep");
		output_file =
			FileName(changeExtension(file.absFilename(), ".dvi"));
	}
}


void LaTeX::deleteFilesOnError() const
{
	// currently just a dummy function.

	// What files do we have to delete?

	// This will at least make latex do all the runs
	unlink(depfile);

	// but the reason for the error might be in a generated file...

	// bibtex file
	FileName const bbl(changeExtension(file.absFilename(), ".bbl"));
	unlink(bbl);

	// makeindex file
	FileName const ind(changeExtension(file.absFilename(), ".ind"));
	unlink(ind);

	// nomencl file
	FileName const nls(changeExtension(file.absFilename(), ".nls"));
	unlink(nls);

	// nomencl file (old version of the package)
	FileName const gls(changeExtension(file.absFilename(), ".gls"));
	unlink(gls);

	// Also remove the aux file
	FileName const aux(changeExtension(file.absFilename(), ".aux"));
	unlink(aux);
}


int LaTeX::run(TeXErrors & terr)
	// We know that this function will only be run if the lyx buffer
	// has been changed. We also know that a newly written .tex file
	// is always different from the previous one because of the date
	// in it. However it seems safe to run latex (at least) on time
	// each time the .tex file changes.
{
	int scanres = NO_ERRORS;
	unsigned int count = 0; // number of times run
	num_errors = 0; // just to make sure.
	unsigned int const MAX_RUN = 6;
	DepTable head; // empty head
	bool rerun = false; // rerun requested

	// The class LaTeX does not know the temp path.
	theBufferList().updateIncludedTeXfiles(getcwd().absFilename(),
		runparams);

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

	bool had_depfile = doesFileExist(depfile);
	bool run_bibtex = false;
	FileName const aux_file(changeExtension(file.absFilename(), "aux"));

	if (had_depfile) {
		LYXERR(Debug::DEPEND) << "Dependency file exists" << endl;
		// Read the dep file:
		had_depfile = head.read(depfile);
	}

	if (had_depfile) {
		// Update the checksums
		head.update();
		// Can't just check if anything has changed because it might
		// have aborted on error last time... in which cas we need
		// to re-run latex and collect the error messages
		// (even if they are the same).
		if (!doesFileExist(output_file)) {
			LYXERR(Debug::DEPEND)
				<< "re-running LaTeX because output file doesn't exist."
				<< endl;
		} else if (!head.sumchange()) {
			LYXERR(Debug::DEPEND) << "return no_change" << endl;
			return NO_CHANGE;
		} else {
			LYXERR(Debug::DEPEND)
				<< "Dependency file has changed" << endl;
		}

		if (head.extchanged(".bib") || head.extchanged(".bst"))
			run_bibtex = true;
	} else
		LYXERR(Debug::DEPEND)
			<< "Dependency file does not exist, or has wrong format"
			<< endl;

	/// We scan the aux file even when had_depfile = false,
	/// because we can run pdflatex on the file after running latex on it,
	/// in which case we will not need to run bibtex again.
	vector<Aux_Info> bibtex_info_old;
	if (!run_bibtex)
		bibtex_info_old = scanAuxFiles(aux_file);

	++count;
	LYXERR(Debug::LATEX) << "Run #" << count << endl;
	message(runMessage(count));

	startscript();
	scanres = scanLogFile(terr);
	if (scanres & ERROR_RERUN) {
		LYXERR(Debug::LATEX) << "Rerunning LaTeX" << endl;
		startscript();
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

	// memoir (at least) writes an empty *idx file in the first place.
	// A second latex run is needed.
	FileName const idxfile(changeExtension(file.absFilename(), ".idx"));
	rerun = doesFileExist(idxfile) &&
		fs::is_empty(idxfile.toFilesystemEncoding());

	// run makeindex
	if (head.haschanged(idxfile)) {
		// no checks for now
		LYXERR(Debug::LATEX) << "Running MakeIndex." << endl;
		message(_("Running MakeIndex."));
		// onlyFilename() is needed for cygwin
		rerun |= runMakeIndex(onlyFilename(idxfile.absFilename()),
				runparams);
	}
	FileName const nlofile(changeExtension(file.absFilename(), ".nlo"));
	if (head.haschanged(nlofile))
		rerun |= runMakeIndexNomencl(file, runparams, ".nlo", ".nls");
	FileName const glofile(changeExtension(file.absFilename(), ".glo"));
	if (head.haschanged(glofile))
		rerun |= runMakeIndexNomencl(file, runparams, ".glo", ".gls");

	// run bibtex
	// if (scanres & UNDEF_CIT || scanres & RERUN || run_bibtex)
	if (scanres & UNDEF_CIT || run_bibtex) {
		// Here we must scan the .aux file and look for
		// "\bibdata" and/or "\bibstyle". If one of those
		// tags is found -> run bibtex and set rerun = true;
		// no checks for now
		LYXERR(Debug::LATEX) << "Running BibTeX." << endl;
		message(_("Running BibTeX."));
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
		LYXERR(Debug::DEPEND)
			<< "Dep. file has changed or rerun requested"
			<< endl;
		LYXERR(Debug::LATEX)
			<< "Run #" << count << endl;
		message(runMessage(count));
		startscript();
		scanres = scanLogFile(terr);
		if (scanres & ERRORS) {
			deleteFilesOnError();
			return scanres; // return on error
		}

		// update the depedencies
		deplog(head); // reads the latex log
		head.update();
	} else {
		LYXERR(Debug::DEPEND)
			<< "Dep. file has NOT changed"
			<< endl;
	}

	// 1.5
	// The inclusion of files generated by external programs like
	// makeindex or bibtex might have done changes to pagenumbering,
	// etc. And because of this we must run the external programs
	// again to make sure everything is redone correctly.
	// Also there should be no need to run the external programs any
	// more after this.

	// run makeindex if the <file>.idx has changed or was generated.
	if (head.haschanged(idxfile)) {
		// no checks for now
		LYXERR(Debug::LATEX) << "Running MakeIndex." << endl;
		message(_("Running MakeIndex."));
		// onlyFilename() is needed for cygwin
		rerun = runMakeIndex(onlyFilename(changeExtension(
				file.absFilename(), ".idx")), runparams);
	}

	// I am not pretty sure if need this twice.
	if (head.haschanged(nlofile))
		rerun |= runMakeIndexNomencl(file, runparams, ".nlo", ".nls");
	if (head.haschanged(glofile))
		rerun |= runMakeIndexNomencl(file, runparams, ".glo", ".gls");

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
		LYXERR(Debug::LATEX) << "Run #" << count << endl;
		message(runMessage(count));
		startscript();
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
	LYXERR(Debug::LATEX) << "Done." << endl;
	return scanres;
}


int LaTeX::startscript()
{
	// onlyFilename() is needed for cygwin
	string tmp = cmd + ' '
		     + quoteName(onlyFilename(file.toFilesystemEncoding()))
		     + " > " + os::nulldev();
	Systemcall one;
	return one.startscript(Systemcall::Wait, tmp);
}


bool LaTeX::runMakeIndex(string const & f, OutputParams const & runparams,
			 string const & params)
{
	LYXERR(Debug::LATEX)
		<< "idx file has been made, running makeindex on file "
		<< f << endl;
	string tmp = lyxrc.index_command + ' ';

	tmp = subst(tmp, "$$lang", runparams.document_language);
	tmp += quoteName(f);
	tmp += params;
	Systemcall one;
	one.startscript(Systemcall::Wait, tmp);
	return true;
}


bool LaTeX::runMakeIndexNomencl(FileName const & file,
		OutputParams const & runparams,
		string const & nlo, string const & nls)
{
	LYXERR(Debug::LATEX) << "Running MakeIndex for nomencl." << endl;
	message(_("Running MakeIndex for nomencl."));
	string tmp = lyxrc.nomencl_command + ' ';
	// onlyFilename() is needed for cygwin
	tmp += quoteName(onlyFilename(changeExtension(file.absFilename(), nlo)));
	tmp += " -o "
		+ onlyFilename(changeExtension(file.toFilesystemEncoding(), nls));
	Systemcall one;
	one.startscript(Systemcall::Wait, tmp);
	return true;
}


vector<Aux_Info> const
LaTeX::scanAuxFiles(FileName const & file)
{
	vector<Aux_Info> result;

	result.push_back(scanAuxFile(file));

	string const basename = removeExtension(file.absFilename());
	for (int i = 1; i < 1000; ++i) {
		FileName const file2(basename
			+ '.' + convert<string>(i)
			+ ".aux");
		if (!doesFileExist(file2))
			break;
		result.push_back(scanAuxFile(file2));
	}
	return result;
}


Aux_Info const LaTeX::scanAuxFile(FileName const & file)
{
	Aux_Info result;
	result.aux_file = file;
	scanAuxFile(file, result);
	return result;
}


void LaTeX::scanAuxFile(FileName const & file, Aux_Info & aux_info)
{
	LYXERR(Debug::LATEX) << "Scanning aux file: " << file << endl;

	ifstream ifs(file.toFilesystemEncoding().c_str());
	string token;
	static regex const reg1("\\\\citation\\{([^}]+)\\}");
	static regex const reg2("\\\\bibdata\\{([^}]+)\\}");
	static regex const reg3("\\\\bibstyle\\{([^}]+)\\}");
	static regex const reg4("\\\\@input\\{([^}]+)\\}");

	while (getline(ifs, token)) {
		token = rtrim(token, "\r");
		smatch sub;
		// FIXME UNICODE: We assume that citation keys and filenames
		// in the aux file are in the file system encoding.
		token = to_utf8(from_filesystem8bit(token));
		if (regex_match(token, sub, reg1)) {
			string data = sub.str(1);
			while (!data.empty()) {
				string citation;
				data = split(data, citation, ',');
				LYXERR(Debug::LATEX) << "Citation: "
						     << citation << endl;
				aux_info.citations.insert(citation);
			}
		} else if (regex_match(token, sub, reg2)) {
			string data = sub.str(1);
			// data is now all the bib files separated by ','
			// get them one by one and pass them to the helper
			while (!data.empty()) {
				string database;
				data = split(data, database, ',');
				database = changeExtension(database, "bib");
				LYXERR(Debug::LATEX) << "BibTeX database: `"
						     << database << '\'' << endl;
				aux_info.databases.insert(database);
			}
		} else if (regex_match(token, sub, reg3)) {
			string style = sub.str(1);
			// token is now the style file
			// pass it to the helper
			style = changeExtension(style, "bst");
			LYXERR(Debug::LATEX) << "BibTeX style: `"
					     << style << '\'' << endl;
			aux_info.styles.insert(style);
		} else if (regex_match(token, sub, reg4)) {
			string const file2 = sub.str(1);
			scanAuxFile(makeAbsPath(file2), aux_info);
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
			FileName const file = findtexfile(*it2, "bib");
			if (!file.empty())
				dep.insert(file, true);
		}

		for (set<string>::const_iterator it2 = it->styles.begin();
		     it2 != it->styles.end(); ++it2) {
			FileName const file = findtexfile(*it2, "bst");
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

		string tmp = lyxrc.bibtex_command + " ";
		// onlyFilename() is needed for cygwin
		tmp += quoteName(onlyFilename(removeExtension(
				it->aux_file.absFilename())));
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
	string tmp =
		onlyFilename(changeExtension(file.absFilename(), ".log"));
	LYXERR(Debug::LATEX) << "Log file: " << tmp << endl;
	FileName const fn = FileName(makeAbsPath(tmp));
	ifstream ifs(fn.toFilesystemEncoding().c_str());
	bool fle_style = false;
	static regex file_line_error(".+\\.\\D+:[0-9]+: (.+)");

	string token;
	while (getline(ifs, token)) {
		// MikTeX sometimes inserts \0 in the log file. They can't be
		// removed directly with the existing string utility
		// functions, so convert them first to \r, and remove all
		// \r's afterwards, since we need to remove them anyway.
		token = subst(token, '\0', '\r');
		token = subst(token, "\r", "");
		smatch sub;

		LYXERR(Debug::LATEX) << "Log line: " << token << endl;

		if (token.empty())
			continue;

		if (contains(token, "file:line:error style messages enabled"))
			fle_style = true;

		if (prefixIs(token, "LaTeX Warning:") ||
		    prefixIs(token, "! pdfTeX warning")) {
			// Here shall we handle different
			// types of warnings
			retval |= LATEX_WARNING;
			LYXERR(Debug::LATEX) << "LaTeX Warning." << endl;
			if (contains(token, "Rerun to get cross-references")) {
				retval |= RERUN;
				LYXERR(Debug::LATEX)
					<< "We should rerun." << endl;
			// package clefval needs 2 latex runs before bibtex
			} else if (contains(token, "Value of")
				   && contains(token, "on page")
				   && contains(token, "undefined")) {
				retval |= ERROR_RERUN;
				LYXERR(Debug::LATEX)
					<< "Force rerun." << endl;
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
				LYXERR(Debug::LATEX)
					<< "We should rerun." << endl;
				retval |= RERUN;
			}
		} else if (token[0] == '(') {
			if (contains(token, "Rerun LaTeX") ||
			    contains(token, "Rerun to get")) {
				// Used by natbib
				LYXERR(Debug::LATEX)
					<< "We should rerun." << endl;
				retval |= RERUN;
			}
		} else if (prefixIs(token, "! ") ||
			   fle_style && regex_match(token, sub, file_line_error)) {
			   // Ok, we have something that looks like a TeX Error
			   // but what do we really have.

			// Just get the error description:
			string desc;
			if (prefixIs(token, "! "))
				desc = string(token, 2);
			else if (fle_style)
				desc = sub.str();
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
				if (contains(desc,
				    "Package babel Error: You haven't defined the language") ||
				    contains(desc,
				    "Package babel Error: You haven't loaded the option"))
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
				LYXERR(Debug::LATEX)
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
					// FIXME UNICODE
					// We have no idea what the encoding of
					// the log file is.
					// It seems that the output from the
					// latex compiler itself is pure ASCII,
					// but it can include bits from the
					// document, so whatever encoding we
					// assume here it can be wrong.
					terr.insertError(line,
							 from_local8bit(desc),
							 from_local8bit(errstr));
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
	LYXERR(Debug::LATEX) << "Log line: " << token << endl;
	return retval;
}


namespace {

bool insertIfExists(FileName const & absname, DepTable & head)
{
	if (doesFileExist(absname) &&
	    !fs::is_directory(absname.toFilesystemEncoding())) {
		head.insert(absname, true);
		return true;
	}
	return false;
}


bool handleFoundFile(string const & ff, DepTable & head)
{
	// convert from native os path to unix path
	string foundfile = os::internal_path(trim(ff));

	LYXERR(Debug::DEPEND) << "Found file: " << foundfile << endl;

	// Ok now we found a file.
	// Now we should make sure that this is a file that we can
	// access through the normal paths.
	// We will not try any fancy search methods to
	// find the file.

	// (1) foundfile is an
	//     absolute path and should
	//     be inserted.
	if (absolutePath(foundfile)) {
		LYXERR(Debug::DEPEND) << "AbsolutePath file: "
				      << foundfile << endl;
		// On initial insert we want to do the update at once
		// since this file cannot be a file generated by
		// the latex run.
		FileName absname(foundfile);
		if (!insertIfExists(absname, head)) {
			// check for spaces
			string strippedfile = foundfile;
			while (contains(strippedfile, " ")) {
				// files with spaces are often enclosed in quotation
				// marks; those have to be removed
				string unquoted = subst(strippedfile, "\"", "");
				absname.set(unquoted);
				if (insertIfExists(absname, head))
					return true;
				// strip off part after last space and try again
				string tmp = strippedfile;
				string const stripoff =
					rsplit(tmp, strippedfile, ' ');
				absname.set(strippedfile);
				if (insertIfExists(absname, head))
					return true;
			}
		}
	}

	string onlyfile = onlyFilename(foundfile);
	FileName absname(makeAbsPath(onlyfile));

	// check for spaces
	while (contains(foundfile, ' ')) {
		if (doesFileExist(absname))
			// everything o.k.
			break;
		else {
			// files with spaces are often enclosed in quotation
			// marks; those have to be removed
			string unquoted = subst(foundfile, "\"", "");
			absname = makeAbsPath(unquoted);
			if (doesFileExist(absname))
				break;
			// strip off part after last space and try again
			string strippedfile;
			string const stripoff =
				rsplit(foundfile, strippedfile, ' ');
			foundfile = strippedfile;
			onlyfile = onlyFilename(strippedfile);
			absname = makeAbsPath(onlyfile);
		}
	}

	// (2) foundfile is in the tmpdir
	//     insert it into head
	if (doesFileExist(absname) &&
	    !fs::is_directory(absname.toFilesystemEncoding())) {
		// FIXME: This regex contained glo, but glo is used by the old
		// version of nomencl.sty. Do we need to put it back?
		static regex unwanted("^.*\\.(aux|log|dvi|bbl|ind)$");
		if (regex_match(onlyfile, unwanted)) {
			LYXERR(Debug::DEPEND)
				<< "We don't want "
				<< onlyfile
				<< " in the dep file"
				<< endl;
		} else if (suffixIs(onlyfile, ".tex")) {
			// This is a tex file generated by LyX
			// and latex is not likely to change this
			// during its runs.
			LYXERR(Debug::DEPEND)
				<< "Tmpdir TeX file: "
				<< onlyfile
				<< endl;
			head.insert(absname, true);
		} else {
			LYXERR(Debug::DEPEND)
				<< "In tmpdir file:"
				<< onlyfile
				<< endl;
			head.insert(absname);
		}
		return true;
	} else {
		LYXERR(Debug::DEPEND)
			<< "Not a file or we are unable to find it."
			<< endl;
		return false;
	}
}


bool checkLineBreak(string const & ff, DepTable & head)
{
	if (contains(ff, '.'))
		// if we have a dot, we let handleFoundFile decide
		return handleFoundFile(ff, head);
	else
		// else, we suspect a line break
		return false;
}

} // anon namespace


void LaTeX::deplog(DepTable & head)
{
	// This function reads the LaTeX log file end extracts all the
	// external files used by the LaTeX run. The files are then
	// entered into the dependency file.

	string const logfile =
		onlyFilename(changeExtension(file.absFilename(), ".log"));

	static regex reg1("File: (.+).*");
	static regex reg2("No file (.+)(.).*");
	static regex reg3("\\\\openout[0-9]+.*=.*`(.+)(..).*");
	// If an index should be created, MikTex does not write a line like
	//    \openout# = 'sample.idx'.
	// but instead only a line like this into the log:
	//   Writing index file sample.idx
	static regex reg4("Writing index file (.+).*");
	// files also can be enclosed in <...>
	static regex reg5("<([^>]+)(.).*");
	static regex regoldnomencl("Writing glossary file (.+).*");
	static regex regnomencl("Writing nomenclature file (.+).*");
	// If a toc should be created, MikTex does not write a line like
	//    \openout# = `sample.toc'.
	// but only a line like this into the log:
	//    \tf@toc=\write#
	// This line is also written by tetex.
	// This line is not present if no toc should be created.
	static regex miktexTocReg("\\\\tf@toc=\\\\write.*");
	static regex reg6(".*\\([^)]+.*");

	FileName const fn(makeAbsPath(logfile));
	ifstream ifs(fn.toFilesystemEncoding().c_str());
	string lastline;
	while (ifs) {
		// Ok, the scanning of files here is not sufficient.
		// Sometimes files are named by "File: xxx" only
		// So I think we should use some regexps to find files instead.
		// Note: all file names and paths might contains spaces.
		bool found_file = false;
		string token;
		getline(ifs, token);
		// MikTeX sometimes inserts \0 in the log file. They can't be
		// removed directly with the existing string utility
		// functions, so convert them first to \r, and remove all
		// \r's afterwards, since we need to remove them anyway.
		token = subst(token, '\0', '\r');
		token = subst(token, "\r", "");
		if (token.empty() || token == ")") {
			lastline = string();
			continue;
		}

		// Sometimes, filenames are broken across lines.
		// We care for that and save suspicious lines.
		// Here we exclude some cases where we are sure
		// that there is no continued filename
		if (!lastline.empty()) {
			static regex package_info("Package \\w+ Info: .*");
			static regex package_warning("Package \\w+ Warning: .*");
			if (prefixIs(token, "File:") || prefixIs(token, "(Font)")
			    || prefixIs(token, "Package:")
			    || prefixIs(token, "Language:")
			    || prefixIs(token, "LaTeX Info:")
			    || prefixIs(token, "LaTeX Font Info:")
			    || prefixIs(token, "\\openout[")
			    || prefixIs(token, "))")
			    || regex_match(token, package_info)
			    || regex_match(token, package_warning))
				lastline = string();
		}

		if (!lastline.empty())
			// probably a continued filename from last line
			token = lastline + token;
		if (token.length() > 255) {
			// string too long. Cut off.
			token.erase(0, token.length() - 251);
		}

		smatch sub;

		// FIXME UNICODE: We assume that the file names in the log
		// file are in the file system encoding.
		token = to_utf8(from_filesystem8bit(token));

		// (1) "File: file.ext"
		if (regex_match(token, sub, reg1)) {
			// check for dot
			found_file = checkLineBreak(sub.str(1), head);
			// However, ...
			if (suffixIs(token, ")"))
				// no line break for sure
				// pretend we've been succesfully searching
				found_file = true;
		// (2) "No file file.ext"
		} else if (regex_match(token, sub, reg2)) {
			// file names must contains a dot, line ends with dot
			if (contains(sub.str(1), '.') && sub.str(2) == ".")
				found_file = handleFoundFile(sub.str(1), head);
			else
				// we suspect a line break
				found_file = false;
		// (3) "\openout<nr> = `file.ext'."
		} else if (regex_match(token, sub, reg3)) {
			// search for closing '. at the end of the line
			if (sub.str(2) == "\'.")
				found_file = handleFoundFile(sub.str(1), head);
			else
				// probable line break
				found_file = false;
		// (4) "Writing index file file.ext"
		} else if (regex_match(token, sub, reg4))
			// check for dot
			found_file = checkLineBreak(sub.str(1), head);
		// (5) "<file.ext>"
		else if (regex_match(token, sub, reg5)) {
			// search for closing '>' and dot ('*.*>') at the eol
			if (contains(sub.str(1), '.') && sub.str(2) == ">")
				found_file = handleFoundFile(sub.str(1), head);
			else
				// probable line break
				found_file = false;
		// (6) "Writing nomenclature file file.ext"
		} else if (regex_match(token, sub, regnomencl) ||
			   regex_match(token, sub, regoldnomencl))
			// check for dot
			found_file = checkLineBreak(sub.str(1), head);
		// (7) "\tf@toc=\write<nr>" (for MikTeX)
		else if (regex_match(token, sub, miktexTocReg))
			found_file = handleFoundFile(onlyFilename(changeExtension(
						file.absFilename(), ".toc")), head);
		else
			// not found, but we won't check further
			// pretend we've been succesfully searching
			found_file = true;

		// (8) "(file.ext"
		// note that we can have several of these on one line
		// this must be queried separated, because of
		// cases such as "File: file.ext (type eps)"
		// where "File: file.ext" would be skipped
		if (regex_match(token, sub, reg6)) {
			// search for strings in (...)
			static regex reg6_1("\\(([^()]+)(.).*");
			smatch what;
			string::const_iterator first = token.begin();
			string::const_iterator end = token.end();

			while (regex_search(first, end, what, reg6_1)) {
				// if we have a dot, try to handle as file
				if (contains(what.str(1), '.')) {
					first = what[0].second;
					if (what.str(2) == ")") {
						handleFoundFile(what.str(1), head);
						// since we had a closing bracket,
						// do not investigate further
						found_file = true;
					} else
						// if we have no closing bracket,
						// try to handle as file nevertheless
						found_file = handleFoundFile(
							what.str(1) + what.str(2), head);
				}
				// if we do not have a dot, check if the line has
				// a closing bracket (else, we suspect a line break)
				else if (what.str(2) != ")") {
					first = what[0].second;
					found_file = false;
				} else {
					// we have a closing bracket, so the content
					// is not a file name.
					// no need to investigate further
					// pretend we've been succesfully searching
					first = what[0].second;
					found_file = true;
				}
			}
		}

		if (!found_file)
			// probable linebreak:
			// save this line
			lastline = token;
		else
			// no linebreak: reset
			lastline = string();
	}

	// Make sure that the main .tex file is in the dependency file.
	head.insert(file, true);
}


} // namespace lyx

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
#include "LyXRC.h"
#include "DepTable.h"

#include "support/debug.h"
#include "support/convert.h"
#include "support/FileName.h"
#include "support/filetools.h"
#include "support/gettext.h"
#include "support/lstrings.h"
#include "support/Systemcall.h"
#include "support/os.h"

#include "support/regex.h"

#include <fstream>
#include <stack>


using namespace std;
using namespace lyx::support;

namespace lyx {

namespace os = support::os;

// TODO: in no particular order
// - get rid of the call to
//   BufferList::updateIncludedTeXfiles, this should either
//   be done before calling LaTeX::funcs or in a completely
//   different way.
// - the makeindex style files should be taken care of with
//   the dependency mechanism.

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
			    docstring const & error_text,
			    string const & child_name)
{
	Error newerr(line, error_desc, error_text, child_name);
	errors.push_back(newerr);
}


bool operator==(AuxInfo const & a, AuxInfo const & o)
{
	return a.aux_file == o.aux_file
		&& a.citations == o.citations
		&& a.databases == o.databases
		&& a.styles == o.styles;
}


bool operator!=(AuxInfo const & a, AuxInfo const & o)
{
	return !(a == o);
}


/*
 * CLASS LaTeX
 */

LaTeX::LaTeX(string const & latex, OutputParams const & rp,
	     FileName const & f, string const & p, string const & lp,
	     bool const clean_start)
	: cmd(latex), file(f), path(p), lpath(lp), runparams(rp), biber(false)
{
	num_errors = 0;
	// lualatex can still produce a DVI with --output-format=dvi. However,
	// we do not use that internally (we use the "dvilualatex" command) so
	// it would only happen from a custom converter. Thus, it is better to
	// guess that lualatex produces a PDF than to guess a DVI.
	// FIXME we should base the extension on the output format, which we should
	// get in a robust way, e.g. from the converter.
	if (prefixIs(cmd, "pdf") || prefixIs(cmd, "lualatex") || prefixIs(cmd, "xelatex")) {
		depfile = FileName(file.absFileName() + ".dep-pdf");
		output_file =
			FileName(changeExtension(file.absFileName(), ".pdf"));
	} else {
		depfile = FileName(file.absFileName() + ".dep");
		output_file =
			FileName(changeExtension(file.absFileName(), ".dvi"));
	}
	if (clean_start)
		removeAuxiliaryFiles();
}


void LaTeX::removeAuxiliaryFiles() const
{
	// Note that we do not always call this function when there is an error.
	// For example, if there is an error but an output file is produced we
	// still would like to output (export/view) the file.

	// What files do we have to delete?

	// This will at least make latex do all the runs
	depfile.removeFile();

	// but the reason for the error might be in a generated file...

	// bibtex file
	FileName const bbl(changeExtension(file.absFileName(), ".bbl"));
	bbl.removeFile();

	// biber file
	FileName const bcf(changeExtension(file.absFileName(), ".bcf"));
	bcf.removeFile();

	// makeindex file
	FileName const ind(changeExtension(file.absFileName(), ".ind"));
	ind.removeFile();

	// nomencl file
	FileName const nls(changeExtension(file.absFileName(), ".nls"));
	nls.removeFile();

	// nomencl file (old version of the package)
	FileName const gls(changeExtension(file.absFileName(), ".gls"));
	gls.removeFile();

	// Also remove the aux file
	FileName const aux(changeExtension(file.absFileName(), ".aux"));
	aux.removeFile();

	// Also remove the .out file (e.g. hyperref bookmarks) (#9963)
	FileName const out(changeExtension(file.absFileName(), ".out"));
	out.removeFile();

	// Remove the output file, which is often generated even if error
	output_file.removeFile();
}


int LaTeX::run(TeXErrors & terr)
	// We know that this function will only be run if the lyx buffer
	// has been changed. We also know that a newly written .tex file
	// is always different from the previous one because of the date
	// in it. However it seems safe to run latex (at least) one time
	// each time the .tex file changes.
{
	int scanres = NO_ERRORS;
	int bscanres = NO_ERRORS;
	unsigned int count = 0; // number of times run
	num_errors = 0; // just to make sure.
	unsigned int const MAX_RUN = 6;
	DepTable head; // empty head
	bool rerun = false; // rerun requested

	// The class LaTeX does not know the temp path.
	theBufferList().updateIncludedTeXfiles(FileName::getcwd().absFileName(),
		runparams);

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

	bool had_depfile = depfile.exists();
	bool run_bibtex = false;
	FileName const aux_file(changeExtension(file.absFileName(), ".aux"));

	if (had_depfile) {
		LYXERR(Debug::DEPEND, "Dependency file exists");
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
		if (!output_file.exists()) {
			LYXERR(Debug::DEPEND,
				"re-running LaTeX because output file doesn't exist.");
		} else if (!head.sumchange()) {
			LYXERR(Debug::DEPEND, "return no_change");
			return NO_CHANGE;
		} else {
			LYXERR(Debug::DEPEND, "Dependency file has changed");
		}

		if (head.extchanged(".bib") || head.extchanged(".bst"))
			run_bibtex = true;
	} else
		LYXERR(Debug::DEPEND,
			"Dependency file does not exist, or has wrong format");

	/// We scan the aux file even when had_depfile = false,
	/// because we can run pdflatex on the file after running latex on it,
	/// in which case we will not need to run bibtex again.
	vector<AuxInfo> bibtex_info_old;
	if (!run_bibtex)
		bibtex_info_old = scanAuxFiles(aux_file);

	++count;
	LYXERR(Debug::LATEX, "Run #" << count);
	message(runMessage(count));

	int exit_code = startscript();

	scanres = scanLogFile(terr);
	if (scanres & ERROR_RERUN) {
		LYXERR(Debug::LATEX, "Rerunning LaTeX");
		terr.clearErrors();
		exit_code = startscript();
		scanres = scanLogFile(terr);
	}

	vector<AuxInfo> const bibtex_info = scanAuxFiles(aux_file);
	if (!run_bibtex && bibtex_info_old != bibtex_info)
		run_bibtex = true;

	// update the dependencies.
	deplog(head); // reads the latex log
	head.update();

	// 1
	// At this point we must run external programs if needed.
	// makeindex will be run if a .idx file changed or was generated.
	// And if there were undefined citations or changes in references
	// the .aux file is checked for signs of bibtex. Bibtex is then run
	// if needed.

	// memoir (at least) writes an empty *idx file in the first place.
	// A second latex run is needed.
	FileName const idxfile(changeExtension(file.absFileName(), ".idx"));
	rerun = idxfile.exists() && idxfile.isFileEmpty();

	// run makeindex
	if (head.haschanged(idxfile)) {
		// no checks for now
		LYXERR(Debug::LATEX, "Running MakeIndex.");
		message(_("Running Index Processor."));
		// onlyFileName() is needed for cygwin
		rerun |= runMakeIndex(onlyFileName(idxfile.absFileName()),
				runparams);
	}
	FileName const nlofile(changeExtension(file.absFileName(), ".nlo"));
	// If all nomencl entries are removed, nomencl writes an empty nlo file.
	// DepTable::hasChanged() returns false in this case, since it does not
	// distinguish empty files from non-existing files. This is why we need
	// the extra checks here (to trigger a rerun). Cf. discussions in #8905.
	// FIXME: Sort out the real problem in DepTable.
	if (head.haschanged(nlofile) || (nlofile.exists() && nlofile.isFileEmpty()))
		rerun |= runMakeIndexNomencl(file, ".nlo", ".nls");
	FileName const glofile(changeExtension(file.absFileName(), ".glo"));
	if (head.haschanged(glofile))
		rerun |= runMakeIndexNomencl(file, ".glo", ".gls");

	// check if we're using biber instead of bibtex
	// biber writes no info to the aux file, so we just check
	// if a bcf file exists (and if it was updated)
	FileName const bcffile(changeExtension(file.absFileName(), ".bcf"));
	biber |= head.exist(bcffile);

	// run bibtex
	// if (scanres & UNDEF_CIT || scanres & RERUN || run_bibtex)
	if (scanres & UNDEF_CIT || run_bibtex) {
		// Here we must scan the .aux file and look for
		// "\bibdata" and/or "\bibstyle". If one of those
		// tags is found -> run bibtex and set rerun = true;
		// no checks for now
		LYXERR(Debug::LATEX, "Running BibTeX.");
		message(_("Running BibTeX."));
		updateBibtexDependencies(head, bibtex_info);
		rerun |= runBibTeX(bibtex_info, runparams);
		FileName const blgfile(changeExtension(file.absFileName(), ".blg"));
		if (blgfile.exists())
			bscanres = scanBlgFile(head, terr);
	} else if (!had_depfile) {
		/// If we run pdflatex on the file after running latex on it,
		/// then we do not need to run bibtex, but we do need to
		/// insert the .bib and .bst files into the .dep-pdf file.
		updateBibtexDependencies(head, bibtex_info);
	}

	// 2
	// we know on this point that latex has been run once (or we just
	// returned) and the question now is to decide if we need to run
	// it any more. This is done by asking if any of the files in the
	// dependency file has changed. (remember that the checksum for
	// a given file is reported to have changed if it just was created)
	//     -> if changed or rerun == true:
	//             run latex once more and
	//             update the dependency structure
	//     -> if not changed:
	//             we do nothing at this point
	//
	if (rerun || head.sumchange()) {
		rerun = false;
		++count;
		LYXERR(Debug::DEPEND, "Dep. file has changed or rerun requested");
		LYXERR(Debug::LATEX, "Run #" << count);
		message(runMessage(count));
		startscript();
		scanres = scanLogFile(terr);

		// update the depedencies
		deplog(head); // reads the latex log
		head.update();
	} else {
		LYXERR(Debug::DEPEND, "Dep. file has NOT changed");
	}
	
	// 3
	// rerun bibtex?
	// Complex bibliography packages such as Biblatex require
	// an additional bibtex cycle sometimes.
	if (scanres & UNDEF_CIT) {
		// Here we must scan the .aux file and look for
		// "\bibdata" and/or "\bibstyle". If one of those
		// tags is found -> run bibtex and set rerun = true;
		// no checks for now
		LYXERR(Debug::LATEX, "Running BibTeX.");
		message(_("Running BibTeX."));
		updateBibtexDependencies(head, bibtex_info);
		rerun |= runBibTeX(bibtex_info, runparams);
		FileName const blgfile(changeExtension(file.absFileName(), ".blg"));
		if (blgfile.exists())
			bscanres = scanBlgFile(head, terr);
	}

	// 4
	// The inclusion of files generated by external programs such as
	// makeindex or bibtex might have done changes to pagenumbering,
	// etc. And because of this we must run the external programs
	// again to make sure everything is redone correctly.
	// Also there should be no need to run the external programs any
	// more after this.

	// run makeindex if the <file>.idx has changed or was generated.
	if (head.haschanged(idxfile)) {
		// no checks for now
		LYXERR(Debug::LATEX, "Running MakeIndex.");
		message(_("Running Index Processor."));
		// onlyFileName() is needed for cygwin
		rerun = runMakeIndex(onlyFileName(changeExtension(
				file.absFileName(), ".idx")), runparams);
	}

	// I am not pretty sure if need this twice.
	if (head.haschanged(nlofile))
		rerun |= runMakeIndexNomencl(file, ".nlo", ".nls");
	if (head.haschanged(glofile))
		rerun |= runMakeIndexNomencl(file, ".glo", ".gls");

	// 5
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
		LYXERR(Debug::LATEX, "Run #" << count);
		message(runMessage(count));
		startscript();
		scanres = scanLogFile(terr);

		// keep this updated
		head.update();
	}

	// Write the dependencies to file.
	head.write(depfile);

	if (exit_code) {
		// add flag here, just before return, instead of when exit_code
		// is defined because scanres is sometimes overwritten above
		// (e.g. rerun)
		scanres |= NONZERO_ERROR;
	}

	LYXERR(Debug::LATEX, "Done.");

	if (bscanres & ERRORS)
		return bscanres; // return on error

	return scanres;
}


int LaTeX::startscript()
{
	// onlyFileName() is needed for cygwin
	string tmp = cmd + ' '
		     + quoteName(onlyFileName(file.toFilesystemEncoding()))
		     + " > " + os::nulldev();
	Systemcall one;
	return one.startscript(Systemcall::Wait, tmp, path, lpath);
}


bool LaTeX::runMakeIndex(string const & f, OutputParams const & runparams,
			 string const & params)
{
	string tmp = runparams.use_japanese ?
		lyxrc.jindex_command : lyxrc.index_command;
	
	if (!runparams.index_command.empty())
		tmp = runparams.index_command;

	LYXERR(Debug::LATEX,
		"idx file has been made, running index processor ("
		<< tmp << ") on file " << f);

	tmp = subst(tmp, "$$lang", runparams.document_language);
	if (runparams.use_indices) {
		tmp = lyxrc.splitindex_command + " -m " + quoteName(tmp);
		LYXERR(Debug::LATEX,
		"Multiple indices. Using splitindex command: " << tmp);
	}
	tmp += ' ';
	tmp += quoteName(f);
	tmp += params;
	Systemcall one;
	one.startscript(Systemcall::Wait, tmp, path, lpath);
	return true;
}


bool LaTeX::runMakeIndexNomencl(FileName const & file,
		string const & nlo, string const & nls)
{
	LYXERR(Debug::LATEX, "Running MakeIndex for nomencl.");
	message(_("Running MakeIndex for nomencl."));
	string tmp = lyxrc.nomencl_command + ' ';
	// onlyFileName() is needed for cygwin
	tmp += quoteName(onlyFileName(changeExtension(file.absFileName(), nlo)));
	tmp += " -o "
		+ onlyFileName(changeExtension(file.toFilesystemEncoding(), nls));
	Systemcall one;
	one.startscript(Systemcall::Wait, tmp, path, lpath);
	return true;
}


vector<AuxInfo> const
LaTeX::scanAuxFiles(FileName const & file)
{
	vector<AuxInfo> result;

	result.push_back(scanAuxFile(file));

	string const basename = removeExtension(file.absFileName());
	for (int i = 1; i < 1000; ++i) {
		FileName const file2(basename
			+ '.' + convert<string>(i)
			+ ".aux");
		if (!file2.exists())
			break;
		result.push_back(scanAuxFile(file2));
	}
	return result;
}


AuxInfo const LaTeX::scanAuxFile(FileName const & file)
{
	AuxInfo result;
	result.aux_file = file;
	scanAuxFile(file, result);
	return result;
}


void LaTeX::scanAuxFile(FileName const & file, AuxInfo & aux_info)
{
	LYXERR(Debug::LATEX, "Scanning aux file: " << file);

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
				LYXERR(Debug::LATEX, "Citation: " << citation);
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
				LYXERR(Debug::LATEX, "BibTeX database: `" << database << '\'');
				aux_info.databases.insert(database);
			}
		} else if (regex_match(token, sub, reg3)) {
			string style = sub.str(1);
			// token is now the style file
			// pass it to the helper
			style = changeExtension(style, "bst");
			LYXERR(Debug::LATEX, "BibTeX style: `" << style << '\'');
			aux_info.styles.insert(style);
		} else if (regex_match(token, sub, reg4)) {
			string const file2 = sub.str(1);
			scanAuxFile(makeAbsPath(file2), aux_info);
		}
	}
}


void LaTeX::updateBibtexDependencies(DepTable & dep,
				     vector<AuxInfo> const & bibtex_info)
{
	// Since a run of Bibtex mandates more latex runs it is ok to
	// remove all ".bib" and ".bst" files.
	dep.remove_files_with_extension(".bib");
	dep.remove_files_with_extension(".bst");
	//string aux = OnlyFileName(ChangeExtension(file, ".aux"));

	for (vector<AuxInfo>::const_iterator it = bibtex_info.begin();
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

	// biber writes nothing into the aux file.
	// Instead, we have to scan the blg file
	if (biber) {
		TeXErrors terr;
		scanBlgFile(dep, terr);
	}
}


bool LaTeX::runBibTeX(vector<AuxInfo> const & bibtex_info,
		      OutputParams const & runparams)
{
	bool result = false;
	for (vector<AuxInfo>::const_iterator it = bibtex_info.begin();
	     it != bibtex_info.end(); ++it) {
		if (!biber && it->databases.empty())
			continue;
		result = true;

		string tmp = runparams.use_japanese ?
			lyxrc.jbibtex_command : lyxrc.bibtex_command;

		if (!runparams.bibtex_command.empty())
			tmp = runparams.bibtex_command;
		tmp += " ";
		// onlyFileName() is needed for cygwin
		tmp += quoteName(onlyFileName(removeExtension(
				it->aux_file.absFileName())));
		Systemcall one;
		one.startscript(Systemcall::Wait, tmp, path, lpath);
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
		onlyFileName(changeExtension(file.absFileName(), ".log"));
	LYXERR(Debug::LATEX, "Log file: " << tmp);
	FileName const fn = FileName(makeAbsPath(tmp));
	ifstream ifs(fn.toFilesystemEncoding().c_str());
	bool fle_style = false;
	static regex const file_line_error(".+\\.\\D+:[0-9]+: (.+)");
	static regex const child_file(".*([0-9]+[A-Za-z]*_.+\\.tex).*");
	// Flag for 'File ended while scanning' message.
	// We need to wait for subsequent processing.
	string wait_for_error;
	string child_name;
	int pnest = 0;
	stack <pair<string, int> > child;

	string token;
	while (getline(ifs, token)) {
		// MikTeX sometimes inserts \0 in the log file. They can't be
		// removed directly with the existing string utility
		// functions, so convert them first to \r, and remove all
		// \r's afterwards, since we need to remove them anyway.
		token = subst(token, '\0', '\r');
		token = subst(token, "\r", "");
		smatch sub;

		LYXERR(Debug::LATEX, "Log line: " << token);

		if (token.empty())
			continue;

		// Track child documents
		for (size_t i = 0; i < token.length(); ++i) {
			if (token[i] == '(') {
				++pnest;
				size_t j = token.find('(', i + 1);
				size_t len = j == string::npos
						? token.substr(i + 1).length()
						: j - i - 1;
				string const substr = token.substr(i + 1, len);
				if (regex_match(substr, sub, child_file)) {
					string const name = sub.str(1);
					child.push(make_pair(name, pnest));
					i += len;
				}
			} else if (token[i] == ')') {
				if (!child.empty()
				    && child.top().second == pnest)
					child.pop();
				--pnest;
			}
		}
		child_name = child.empty() ? empty_string() : child.top().first;

		if (contains(token, "file:line:error style messages enabled"))
			fle_style = true;

		if (prefixIs(token, "LaTeX Warning:") ||
		    prefixIs(token, "! pdfTeX warning")) {
			// Here shall we handle different
			// types of warnings
			retval |= LATEX_WARNING;
			LYXERR(Debug::LATEX, "LaTeX Warning.");
			if (contains(token, "Rerun to get cross-references")) {
				retval |= RERUN;
				LYXERR(Debug::LATEX, "We should rerun.");
			// package clefval needs 2 latex runs before bibtex
			} else if (contains(token, "Value of")
				   && contains(token, "on page")
				   && contains(token, "undefined")) {
				retval |= ERROR_RERUN;
				LYXERR(Debug::LATEX, "Force rerun.");
			// package etaremune
			} else if (contains(token, "Etaremune labels have changed")) {
				retval |= ERROR_RERUN;
				LYXERR(Debug::LATEX, "Force rerun.");
			} else if (contains(token, "Citation")
				   && contains(token, "on page")
				   && contains(token, "undefined")) {
				retval |= UNDEF_CIT;
			} else if (contains(token, "Citation")
				   && contains(token, "on input line")
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
			} else if (contains(token, "run Biber")) {
				retval |= UNDEF_CIT;
				biber = true;
			} else if (contains(token, "Rerun LaTeX") ||
				   contains(token, "Please rerun LaTeX") ||
				   contains(token, "Rerun to get")) {
				// at least longtable.sty and bibtopic.sty
				// might use this.
				LYXERR(Debug::LATEX, "We should rerun.");
				retval |= RERUN;
			}
		} else if (prefixIs(token, "LETTRE WARNING:")) {
			if (contains(token, "veuillez recompiler")) {
				// lettre.cls
				LYXERR(Debug::LATEX, "We should rerun.");
				retval |= RERUN;
			}
		} else if (token[0] == '(') {
			if (contains(token, "Rerun LaTeX") ||
			    contains(token, "Rerun to get")) {
				// Used by natbib
				LYXERR(Debug::LATEX, "We should rerun.");
				retval |= RERUN;
			}
		} else if (prefixIs(token, "! ")
			    || (fle_style
				&& regex_match(token, sub, file_line_error)
				&& !contains(token, "pdfTeX warning"))) {
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

			if (prefixIs(token, "! File ended while scanning")){
				if (prefixIs(token, "! File ended while scanning use of \\Hy@setref@link.")){
					// bug 7344. We must rerun LaTeX if hyperref has been toggled.
					retval |= ERROR_RERUN;
					LYXERR(Debug::LATEX, "Force rerun.");
				} else {
					// bug 6445. At this point its not clear we finish with error.
					wait_for_error = desc;
					continue;
				}
			}

			if (prefixIs(token, "! Paragraph ended before \\Hy@setref@link was complete.")){
					// bug 7344. We must rerun LaTeX if hyperref has been toggled.
					retval |= ERROR_RERUN;
					LYXERR(Debug::LATEX, "Force rerun.");
			}

			if (!wait_for_error.empty() && prefixIs(token, "! Emergency stop.")){
				retval |= LATEX_ERROR;
				string errstr;
				int count = 0;
				errstr = wait_for_error;
				do {
					if (!getline(ifs, tmp))
						break;
					tmp = rtrim(tmp, "\r");
					errstr += "\n" + tmp;
					if (++count > 5)
						break;
				} while (!contains(tmp, "(job aborted"));

				terr.insertError(0,
						 from_local8bit("Emergency stop"),
						 from_local8bit(errstr),
						 child_name);
			}

			// get the next line
			string tmp;
			int count = 0;
			do {
				if (!getline(ifs, tmp))
					break;
				tmp = rtrim(tmp, "\r");
				// 15 is somewhat arbitrarily chosen, based on practice.
				// We used 10 for 14 years and increased it to 15 when we
				// saw one case.
				if (++count > 15)
					break;
			} while (!prefixIs(tmp, "l."));
			if (prefixIs(tmp, "l.")) {
				// we have a latex error
				retval |=  TEX_ERROR;
				if (contains(desc,
					"Package babel Error: You haven't defined the language")
				    || contains(desc,
					"Package babel Error: You haven't loaded the option")
				    || contains(desc,
					"Package babel Error: Unknown language"))
					retval |= ERROR_RERUN;
				// get the line number:
				int line = 0;
				sscanf(tmp.c_str(), "l.%d", &line);
				// get the rest of the message:
				string errstr(tmp, tmp.find(' '));
				errstr += '\n';
				getline(ifs, tmp);
				tmp = rtrim(tmp, "\r");
				while (!contains(errstr, "l.")
				       && !tmp.empty()
				       && !prefixIs(tmp, "! ")
				       && !contains(tmp, "(job aborted")) {
					errstr += tmp;
					errstr += "\n";
					getline(ifs, tmp);
					tmp = rtrim(tmp, "\r");
				}
				LYXERR(Debug::LATEX, "line: " << line << '\n'
					<< "Desc: " << desc << '\n' << "Text: " << errstr);
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
							 from_local8bit(errstr),
							 child_name);
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
			} else if (contains(token, "No pages of output")
				|| contains(token, "no pages of output")) {
				// No output file (e.g. the DVI or PDF) was created
				retval |= NO_OUTPUT;
			} else if (contains(token, "That makes 100 errors")) {
				// More than 100 errors were reprted
				retval |= TOO_MANY_ERRORS;
			} else if (prefixIs(token, "!pdfTeX error:")) {
				// otherwise we dont catch e.g.:
				// !pdfTeX error: pdflatex (file feyn10): Font feyn10 at 600 not found
				retval |= ERRORS;
				terr.insertError(0,
						 from_local8bit("pdfTeX Error"),
						 from_local8bit(token),
						 child_name);
			} else if (prefixIs(token, "Missing character: There is no ")) {
				// XeTeX/LuaTeX error about missing glyph in selected font
				// (bug 9610)
				retval |= LATEX_ERROR;
				terr.insertError(0,
						 from_local8bit("Missing glyphs!"),
						 from_local8bit(token),
						 child_name);
			}
		}
	}
	LYXERR(Debug::LATEX, "Log line: " << token);
	return retval;
}


namespace {

bool insertIfExists(FileName const & absname, DepTable & head)
{
	if (absname.exists() && !absname.isDirectory()) {
		head.insert(absname, true);
		return true;
	}
	return false;
}


bool handleFoundFile(string const & ff, DepTable & head)
{
	// convert from native os path to unix path
	string foundfile = os::internal_path(trim(ff));

	LYXERR(Debug::DEPEND, "Found file: " << foundfile);

	// Ok now we found a file.
	// Now we should make sure that this is a file that we can
	// access through the normal paths.
	// We will not try any fancy search methods to
	// find the file.

	// (1) foundfile is an
	//     absolute path and should
	//     be inserted.
	FileName absname;
	if (FileName::isAbsolute(foundfile)) {
		LYXERR(Debug::DEPEND, "AbsolutePath file: " << foundfile);
		// On initial insert we want to do the update at once
		// since this file cannot be a file generated by
		// the latex run.
		absname.set(foundfile);
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
				rsplit(tmp, strippedfile, ' ');
				absname.set(strippedfile);
				if (insertIfExists(absname, head))
					return true;
			}
		}
	}

	string onlyfile = onlyFileName(foundfile);
	absname = makeAbsPath(onlyfile);

	// check for spaces
	while (contains(foundfile, ' ')) {
		if (absname.exists())
			// everything o.k.
			break;
		else {
			// files with spaces are often enclosed in quotation
			// marks; those have to be removed
			string unquoted = subst(foundfile, "\"", "");
			absname = makeAbsPath(unquoted);
			if (absname.exists())
				break;
			// strip off part after last space and try again
			string strippedfile;
			rsplit(foundfile, strippedfile, ' ');
			foundfile = strippedfile;
			onlyfile = onlyFileName(strippedfile);
			absname = makeAbsPath(onlyfile);
		}
	}

	// (2) foundfile is in the tmpdir
	//     insert it into head
	if (absname.exists() && !absname.isDirectory()) {
		// FIXME: This regex contained glo, but glo is used by the old
		// version of nomencl.sty. Do we need to put it back?
		static regex const unwanted("^.*\\.(aux|log|dvi|bbl|ind)$");
		if (regex_match(onlyfile, unwanted)) {
			LYXERR(Debug::DEPEND, "We don't want " << onlyfile
				<< " in the dep file");
		} else if (suffixIs(onlyfile, ".tex")) {
			// This is a tex file generated by LyX
			// and latex is not likely to change this
			// during its runs.
			LYXERR(Debug::DEPEND, "Tmpdir TeX file: " << onlyfile);
			head.insert(absname, true);
		} else {
			LYXERR(Debug::DEPEND, "In tmpdir file:" << onlyfile);
			head.insert(absname);
		}
		return true;
	} else {
		LYXERR(Debug::DEPEND, "Not a file or we are unable to find it.");
		return false;
	}
}


bool completeFilename(string const & ff, DepTable & head)
{
	// If we do not find a dot, we suspect
	// a fragmental file name
	if (!contains(ff, '.'))
		return false;

	// if we have a dot, we let handleFoundFile decide
	return handleFoundFile(ff, head);
}


int iterateLine(string const & token, regex const & reg, string const & closing,
		int fragment_pos, DepTable & head)
{
	smatch what;
	string::const_iterator first = token.begin();
	string::const_iterator end = token.end();
	bool fragment = false;
	string last_match;

	while (regex_search(first, end, what, reg)) {
		// if we have a dot, try to handle as file
		if (contains(what.str(1), '.')) {
			first = what[0].second;
			if (what.str(2) == closing) {
				handleFoundFile(what.str(1), head);
				// since we had a closing bracket,
				// do not investigate further
				fragment = false;
			} else
				// if we have no closing bracket,
				// try to handle as file nevertheless
				fragment = !handleFoundFile(
					what.str(1) + what.str(2), head);
		}
		// if we do not have a dot, check if the line has
		// a closing bracket (else, we suspect a line break)
		else if (what.str(2) != closing) {
			first = what[0].second;
			fragment = true;
		} else {
			// we have a closing bracket, so the content
			// is not a file name.
			// no need to investigate further
			first = what[0].second;
			fragment = false;
		}
		last_match = what.str(1);
	}

	// We need to consider the result from previous line iterations:
	// We might not find a fragment here, but another one might follow
	// E.g.: (filename.ext) <filenam
	// Vice versa, we consider the search completed if a real match
	// follows a potential fragment from a previous iteration.
	// E.g. <some text we considered a fragment (filename.ext)
	// result = -1 means we did not find a fragment!
	int result = -1;
	int last_match_pos = -1;
	if (!last_match.empty() && token.find(last_match) != string::npos)
		last_match_pos = int(token.find(last_match));
	if (fragment) {
		if (last_match_pos > fragment_pos)
			result = last_match_pos;
		else
			result = fragment_pos;
	} else
		if (last_match_pos < fragment_pos)
			result = fragment_pos;

	return result;
}

} // anon namespace


void LaTeX::deplog(DepTable & head)
{
	// This function reads the LaTeX log file end extracts all the
	// external files used by the LaTeX run. The files are then
	// entered into the dependency file.

	string const logfile =
		onlyFileName(changeExtension(file.absFileName(), ".log"));

	static regex const reg1("File: (.+).*");
	static regex const reg2("No file (.+)(.).*");
	static regex const reg3("\\\\openout[0-9]+.*=.*`(.+)(..).*");
	// If an index should be created, MikTex does not write a line like
	//    \openout# = 'sample.idx'.
	// but instead only a line like this into the log:
	//   Writing index file sample.idx
	static regex const reg4("Writing index file (.+).*");
	static regex const regoldnomencl("Writing glossary file (.+).*");
	static regex const regnomencl("Writing nomenclature file (.+).*");
	// If a toc should be created, MikTex does not write a line like
	//    \openout# = `sample.toc'.
	// but only a line like this into the log:
	//    \tf@toc=\write#
	// This line is also written by tetex.
	// This line is not present if no toc should be created.
	static regex const miktexTocReg("\\\\tf@toc=\\\\write.*");
	// file names can be enclosed in <...> (anywhere on the line)
	static regex const reg5(".*<[^>]+.*");
	// and also (...) anywhere on the line
	static regex const reg6(".*\\([^)]+.*");

	FileName const fn = makeAbsPath(logfile);
	ifstream ifs(fn.toFilesystemEncoding().c_str());
	string lastline;
	while (ifs) {
		// Ok, the scanning of files here is not sufficient.
		// Sometimes files are named by "File: xxx" only
		// Therefore we use some regexps to find files instead.
		// Note: all file names and paths might contains spaces.
		// Also, file names might be broken across lines. Therefore
		// we mark (potential) fragments and merge those lines.
		bool fragment = false;
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

		// FIXME UNICODE: We assume that the file names in the log
		// file are in the file system encoding.
		token = to_utf8(from_filesystem8bit(token));

		// Sometimes, filenames are broken across lines.
		// We care for that and save suspicious lines.
		// Here we exclude some cases where we are sure
		// that there is no continued filename
		if (!lastline.empty()) {
			static regex const package_info("Package \\w+ Info: .*");
			static regex const package_warning("Package \\w+ Warning: .*");
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

		// (1) "File: file.ext"
		if (regex_match(token, sub, reg1)) {
			// is this a fragmental file name?
			fragment = !completeFilename(sub.str(1), head);
			// However, ...
			if (suffixIs(token, ")"))
				// no fragment for sure
				fragment = false;
		// (2) "No file file.ext"
		} else if (regex_match(token, sub, reg2)) {
			// file names must contains a dot, line ends with dot
			if (contains(sub.str(1), '.') && sub.str(2) == ".")
				fragment = !handleFoundFile(sub.str(1), head);
			else
				// we suspect a line break
				fragment = true;
		// (3) "\openout<nr> = `file.ext'."
		} else if (regex_match(token, sub, reg3)) {
			// search for closing '. at the end of the line
			if (sub.str(2) == "\'.")
				fragment = !handleFoundFile(sub.str(1), head);
			else
				// potential fragment
				fragment = true;
		// (4) "Writing index file file.ext"
		} else if (regex_match(token, sub, reg4))
			// fragmential file name?
			fragment = !completeFilename(sub.str(1), head);
		// (5) "Writing nomenclature file file.ext"
		else if (regex_match(token, sub, regnomencl) ||
			   regex_match(token, sub, regoldnomencl))
			// fragmental file name?
			fragment= !completeFilename(sub.str(1), head);
		// (6) "\tf@toc=\write<nr>" (for MikTeX)
		else if (regex_match(token, sub, miktexTocReg))
			fragment = !handleFoundFile(onlyFileName(changeExtension(
						file.absFileName(), ".toc")), head);
		else
			// not found, but we won't check further
			fragment = false;

		int fragment_pos = -1;
		// (7) "<file.ext>"
		// We can have several of these on one line
		// (and in addition to those above)
		if (regex_match(token, sub, reg5)) {
			// search for strings in <...>
			static regex const reg5_1("<([^>]+)(.)");
			fragment_pos = iterateLine(token, reg5_1, ">",
						   fragment_pos, head);
			fragment = (fragment_pos != -1);
		}

		// (8) "(file.ext)"
		// We can have several of these on one line
		// this must be queried separated, because of
		// cases such as "File: file.ext (type eps)"
		// where "File: file.ext" would be skipped
		if (regex_match(token, sub, reg6)) {
			// search for strings in (...)
			static regex const reg6_1("\\(([^()]+)(.)");
			fragment_pos = iterateLine(token, reg6_1, ")",
						   fragment_pos, head);
			fragment = (fragment_pos != -1);
		}

		if (fragment)
			// probable linebreak within file name:
			// save this line
			lastline = token;
		else
			// no linebreak: reset
			lastline = string();
	}

	// Make sure that the main .tex file is in the dependency file.
	head.insert(file, true);
}


int LaTeX::scanBlgFile(DepTable & dep, TeXErrors & terr)
{
	FileName const blg_file(changeExtension(file.absFileName(), "blg"));
	LYXERR(Debug::LATEX, "Scanning blg file: " << blg_file);

	ifstream ifs(blg_file.toFilesystemEncoding().c_str());
	string token;
	static regex const reg1(".*Found (bibtex|BibTeX) data (file|source) '([^']+).*");
	static regex const bibtexError("^(.*---line [0-9]+ of file).*$");
	static regex const bibtexError2("^(.*---while reading file).*$");
	static regex const bibtexError3("(A bad cross reference---).*");
	static regex const bibtexError4("(Sorry---you've exceeded BibTeX's).*");
	static regex const bibtexError5("\\*Please notify the BibTeX maintainer\\*");
	static regex const biberError("^.*> (FATAL|ERROR) - (.*)$");
	int retval = NO_ERRORS;

	string prevtoken;
	while (getline(ifs, token)) {
		token = rtrim(token, "\r");
		smatch sub;
		// FIXME UNICODE: We assume that citation keys and filenames
		// in the aux file are in the file system encoding.
		token = to_utf8(from_filesystem8bit(token));
		if (regex_match(token, sub, reg1)) {
			string data = sub.str(3);
			if (!data.empty()) {
				LYXERR(Debug::LATEX, "Found bib file: " << data);
				handleFoundFile(data, dep);
			}
		}
		else if (regex_match(token, sub, bibtexError)
			 || regex_match(token, sub, bibtexError2)
			 || regex_match(token, sub, bibtexError4)
			 || regex_match(token, sub, bibtexError5)) {
			retval |= BIBTEX_ERROR;
			string errstr = N_("BibTeX error: ") + token;
			string message;
			if ((prefixIs(token, "while executing---line")
			     || prefixIs(token, "---line ")
			     || prefixIs(token, "*Please notify the BibTeX"))
			    && !prevtoken.empty()) {
				errstr = N_("BibTeX error: ") + prevtoken;
				message = prevtoken + '\n';
			}
			message += token;
			terr.insertError(0,
					 from_local8bit(errstr),
					 from_local8bit(message));
		} else if (regex_match(prevtoken, sub, bibtexError3)) {
			retval |= BIBTEX_ERROR;
			string errstr = N_("BibTeX error: ") + prevtoken;
			string message = prevtoken + '\n' + token;
			terr.insertError(0,
					 from_local8bit(errstr),
					 from_local8bit(message));
		} else if (regex_match(token, sub, biberError)) {
			retval |= BIBTEX_ERROR;
			string errstr = N_("Biber error: ") + sub.str(2);
			string message = token;
			terr.insertError(0,
					 from_local8bit(errstr),
					 from_local8bit(message));
		}
		prevtoken = token;
	}
	return retval;
}


} // namespace lyx

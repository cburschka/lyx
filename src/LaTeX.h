// -*- C++ -*-
/**
 * \file LaTeX.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Angus Leeming
 * \author Dekel Tsur
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef LATEX_H
#define LATEX_H

#include "latexrunparams.h"

#include <boost/utility.hpp>
#include <boost/signals/signal1.hpp>

#include <vector>
#include <set>

class DepTable;

///
class TeXErrors {
private:
	///
	struct Error {
		///
		Error () : error_in_line(0) {}
		///
		Error(int line, std::string const & desc, std::string const & text)
			: error_in_line(line),
			  error_desc(desc),
			  error_text(text) {}
		/// what line in the TeX file the error occured in
		int error_in_line;
		/// The kind of error
		std::string error_desc;
		/// The line/cmd that caused the error.
		std::string error_text;
	};
public:
	///
	typedef std::vector<Error> Errors;
	///
	Errors::const_iterator begin() const { return errors.begin(); }
	///
	Errors::const_iterator end() const { return errors.end(); }
	///
	void insertError(int line, std::string const & error_desc,
			 std::string const & error_text);
private:
	///
	Errors errors;
};


class Aux_Info {
public:
	///
	Aux_Info() {}
	///
	std::string aux_file;
	///
	std::set<std::string> citations;
	///
	std::set<std::string> databases;
	///
	std::set<std::string> styles;
};


///
bool operator==(Aux_Info const &, Aux_Info const &);
bool operator!=(Aux_Info const &, Aux_Info const &);


///
class LaTeX : boost::noncopyable {
public:
	/** Return values from scanLogFile() and run() (to come)

	    This enum should be enlarged a bit so that one could
	    get more feedback from the LaTeX run.
	*/
	enum log_status {
		///
		NO_ERRORS = 0,
		///
		NO_LOGFILE = 1,
		///
		NO_OUTPUT = 2,
		///
		UNDEF_REF = 4, // Reference '...' on page ... undefined.
		///
		UNDEF_CIT = 8, // Citation '...' on page ... undefined.
		///
		RERUN = 16, // Label(s) may have changed. Rerun to get...
		///
		TEX_ERROR = 32,
		///
		TEX_WARNING = 64,
		///
		LATEX_ERROR = 128,
		///
		LATEX_WARNING = 256,
		///
		PACKAGE_WARNING = 512,
		///
		NO_FILE = 1024,
		///
		NO_CHANGE = 2048,
		///
		TOO_MANY_ERRORS = 4096,
		///
		ERROR_RERUN = 8192,
		///
		ERRORS = TEX_ERROR + LATEX_ERROR,
		///
		WARNINGS = TEX_WARNING + LATEX_WARNING + PACKAGE_WARNING
	};

	/// This signal emits an informative message
	boost::signal1<void, std::string> message;


	/**
	   cmd = the latex command, file = name of the (temporary) latex file,
	   path = name of the files original path.
	*/
	LaTeX(std::string const & cmd, LatexRunParams const &,
	      std::string const & file, std::string const & path);

	/// runs LaTeX several times
	int run(TeXErrors &);

	///
	int getNumErrors() { return num_errors;}

	///
	int scanLogFile(TeXErrors &);

private:
	/// use this for running LaTeX once
	int startscript();

	/// The dependency file.
	std::string depfile;

	///
	void deplog(DepTable & head);

	///
	bool runMakeIndex(std::string const &);

	///
	std::vector<Aux_Info> const scanAuxFiles(std::string const &);

	///
	Aux_Info const scanAuxFile(std::string const &);

	///
	void scanAuxFile(std::string const &, Aux_Info &);

	///
	void updateBibtexDependencies(DepTable &,
				      std::vector<Aux_Info> const &);

	///
	bool runBibTeX(std::vector<Aux_Info> const &);

	///
	void deleteFilesOnError() const;

	///
	std::string cmd;

	///
	std::string file;

	///
	std::string path;

	/// used by scanLogFile
	int num_errors;

	/// The name of the final output file.
	std::string output_file;

	///
	LatexRunParams runparams;
};

#endif

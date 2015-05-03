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

#include "OutputParams.h"

#include "support/docstring.h"
#include "support/FileName.h"

#include <boost/signal.hpp>

#include <vector>
#include <set>


namespace lyx {

class DepTable;

///
class TeXErrors {
private:
	///
	class Error {
	public:
		///
		Error () : error_in_line(0) {}
		///
		Error(int line, docstring const & desc, docstring const & text,
		      std::string const & fname)
			: error_in_line(line),
			  error_desc(desc),
			  error_text(text),
			  child_name(fname) {}
		/// what line in the TeX file the error occurred in
		int error_in_line;
		/// The kind of error
		docstring error_desc;
		/// The line/cmd that caused the error.
		docstring error_text;
		/// The name of the child where error occurred, empty otherwise.
		std::string child_name;
	};
public:
	///
	typedef std::vector<Error> Errors;
	///
	Errors::const_iterator begin() const { return errors.begin(); }
	///
	Errors::const_iterator end() const { return errors.end(); }
	///
	void insertError(int line, docstring const & error_desc,
			 docstring const & error_text,
			 std::string const & child_name = empty_string());
private:
	///
	Errors errors;
};


class AuxInfo {
public:
	///
	AuxInfo() {}
	///
	support::FileName aux_file;
	///
	std::set<std::string> citations;
	///
	std::set<std::string> databases;
	///
	std::set<std::string> styles;
};


///
bool operator==(AuxInfo const &, AuxInfo const &);
bool operator!=(AuxInfo const &, AuxInfo const &);


/**
 * Class to run the LaTeX compiler and needed auxiliary programs.
 * The main .tex file must be in the current directory. The current directory
 * must not change as long as an object of this class lives.
 * This is required by the LaTeX compiler, and we also make use of it by
 * various support::makeAbsPath() calls.
 */
class LaTeX {
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
		BIBTEX_ERROR = 16384,
		///
		NONZERO_ERROR = 32768, // the command exited with nonzero status
		///
		ERRORS = TEX_ERROR + LATEX_ERROR + NONZERO_ERROR + BIBTEX_ERROR,
		///
		WARNINGS = TEX_WARNING + LATEX_WARNING + PACKAGE_WARNING
	};

	/// This signal emits an informative message
	boost::signal<void(docstring)> message;


	/**
	   cmd = the latex command, file = name of the (temporary) latex file,
	   path = name of the files original path,
	   clean_start = This forces a fresh run by deleting the files in the temp
			 dir. We set this e.g. if there was an error on previous
			 preview, which is good if the user installed a package
			 or changed certain document settings (#9061).
	*/
	LaTeX(std::string const & cmd, OutputParams const &,
	      support::FileName const & file,
	      std::string const & path = empty_string(),
	      bool const clean_start = false);

	/// runs LaTeX several times
	int run(TeXErrors &);

	///
	int getNumErrors() { return num_errors;}

	///
	int scanLogFile(TeXErrors &);

private:
	/// noncopyable
	LaTeX(LaTeX const &);
	void operator=(LaTeX const &);

	/// use this for running LaTeX once
	int startscript();

	/// The dependency file.
	support::FileName depfile;

	///
	void deplog(DepTable & head);

	///
	bool runMakeIndex(std::string const &, OutputParams const &,
			  std::string const & = std::string());

	///
	bool runMakeIndexNomencl(support::FileName const &, 
				 std::string const &, std::string const &);

	///
	std::vector<AuxInfo> const scanAuxFiles(support::FileName const &);

	///
	AuxInfo const scanAuxFile(support::FileName const &);

	///
	void scanAuxFile(support::FileName const &, AuxInfo &);

	///
	void updateBibtexDependencies(DepTable &,
				      std::vector<AuxInfo> const &);

	///
	int scanBlgFile(DepTable & head, TeXErrors & terr);

	///
	bool runBibTeX(std::vector<AuxInfo> const &,
		       OutputParams const &);

	///
	void removeAuxiliaryFiles() const;

	///
	std::string cmd;

	///
	support::FileName file;

	///
	std::string path;

	/// used by scanLogFile
	int num_errors;

	/// The name of the final output file.
	support::FileName output_file;

	///
	OutputParams runparams;
	
	/// Do we use biber?
	bool biber;
};


} // namespace lyx

#endif

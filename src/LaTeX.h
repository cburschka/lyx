// -*- C++ -*-
/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor 	 
 *	     Copyright 1995 Matthias Ettrich
 *           Copyright 1995-1996 The Lyx Team
 *
 *           This file is Copyright (C) 1996-1999
 *           Lars Gullik Bjønnes
 *
 * ====================================================== 
 */

#ifndef LATEX_H
#define LATEX_H

#ifdef __GNUG__
#pragma interface
#endif

#include "LString.h"
#include "DepTable.h"
#include <vector>
using std::vector;

#include <fstream>
using std::ifstream;
using std::ofstream;

class MiniBuffer;

///
class TeXErrors {
private:
	///
	struct Error {
		///
		Error () : error_in_line(0) {}
		///
		Error(int line, string const & desc, string const & text)
			: error_in_line(line),
			  error_desc(desc),
			  error_text(text) {}
		/// what line in the TeX file the error occured in
		int error_in_line;
		/// The kind of error
		string error_desc;
		/// The line/cmd that caused the error.
		string error_text;
	};
public:
	///
	typedef vector<Error> Errors;
	///
	Errors::const_iterator begin() const { return errors.begin(); }
	///
	Errors::const_iterator end() const { return errors.end(); }
	///
	void insertError(int line, string const & error_desc,
			 string const & error_text);
private:
	///
	Errors errors;
};


///
class LaTeX {
public:
	/** All the different files produced by TeX.
	    
	    This is the files mentioned on page 208-9 in Lamports book +
	    .ltx and .tex files.
	*/
	enum TEX_FILES {
		///
		NO_FILES = 0,
		/// used for table of contents et.al.
		AUX = 1,
		/// written by BibTeX
		BBL = 2,
		/// LaTeX's output
		DVI = 4,
		/// glossary (not supported by LyX so far)
		GLO = 8,
		///index
		IDX = 16,
		/// written by makeindex
		IND = 32,
		/// list of figures
		LOF = 64,
		/// the LaTeX log file
		LOG = 128,
		/// list of tables
		LOT = 256,
		/// table of contents
		TOC = 512,
		/// latex files
		LTX = 1024,
		/// tex files
		TEX = 2048,
		/// list of algorithms
		LOA = 4096
	};
	
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
		ERRORS = TEX_ERROR + LATEX_ERROR,
		///
		WARNINGS = TEX_WARNING + LATEX_WARNING + PACKAGE_WARNING
	};
	

	/**
	   cmd = the latex command, file = name of the (temporary) latex file,
	   path = name of the files original path.
	*/
	LaTeX(string const & cmd, string const & file, string const & path);
	
	/// runs LaTeX several times
	int run(TeXErrors &, MiniBuffer *);

	///
	int getNumErrors() { return num_errors;}

	/// use this for running LaTeX once
	int operator() ();
protected:
	/// The dependency file.
	string depfile;

	/// unavail
	LaTeX(LaTeX const &);
	/// unavail
	LaTeX & operator= (LaTeX const &);
	
	///
	void deplog(DepTable & head);

	///
	void deptex(DepTable & head);
	
	///
	int scanLogFile(TeXErrors &);

	///
	bool runMakeIndex(string const &);

	///
	bool runBibTeX(string const &, DepTable &);
	
	///
	string cmd;

	///
	string file;
	
	///
	string path;
	///
	TEX_FILES tex_files;
	
	///
	int file_count;

	// used by scanLogFile
	int num_errors;
};

#endif

// -*- C++ -*-
/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 * 	 
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2000 The LyX Team.
 *
 * ====================================================== */

#ifndef LASTFILES_H
#define LASTFILES_H

#ifdef __GNUG__
#pragma interface
#endif

#include <deque>

#include "LString.h"
#include "support/utility.hpp"

/** The latest documents loaded.
    This class takes care of the last .lyx files used by the LyX user. It
    both reads and writes this information to a file. The number of files
    kept are user defined, but defaults to four.
    @author Lars Gullik Bjønnes
*/
class LastFiles : public noncopyable {
public:
	///
	typedef std::deque<string> Files;

	///
	typedef Files::const_iterator const_iterator;
	
	/** Read the lastfiles file.
	   @param file The file to read the lastfiles form.
	   @param dostat Whether to check for file existance.
	   @param num number of files to remember.
	*/
	explicit
	LastFiles(string const & file,
		  bool dostat = true, unsigned int num = 4);
	
	/**
	   This funtion inserts #file# into the last files list. If the file
	   already exist it is moved to the top of the list, else exist it
	   is placed on the top of the list. If the list is full the last
	   file in the list is popped from the end.
	*/
	void newFile(string const & file);
	/**  Writes the lastfiles table to disk. One file on each line, this
	     way we can at least have some special chars (e.g. space), but
	     newline in filenames are thus not allowed.
	*/
	void writeFile(string const &) const;
	///
	string const operator[](unsigned int) const;
	///
	Files::const_iterator begin() const { return files.begin(); }
	///
	Files::const_iterator end() const { return files.end(); }
private:
	/** Local constants.
	    It is more portable among different C++ compilers to use
	    an enum instead of #int const XXX#
	*/
	enum local_constants {
		/// Default number of lastfiles.
		DEFAULTFILES = 4,
		/** Max number of lastfiles.
		    There is no point in keeping more than this number
		    of files at the same time. However perhaps someday
		    someone finds use for more files and wants to
		    change it. Please do. But don't show the files in
		    a menu...
		*/
		ABSOLUTEMAXLASTFILES = 20
	};
	
	/// a list of lastfiles
	Files files;
	/// number of files in the lastfiles list.
	unsigned int num_files;
	/// check for file existance or not.
	bool dostat;
	
	/** Read the lastfiles file.
	    Reads the .lyx_lastfiles at the beginning of the LyX session.
	    This will read the lastfiles file (usually .lyx_lastfiles). It
	    will normally discard files that don't exist anymore, unless
	    LastFiles has been initialized with dostat = false. 
	*/
	void readFile(string const &);
	/// used by the constructor to set the number of stored last files.
        void setNumberOfFiles(unsigned int num);
};
#endif

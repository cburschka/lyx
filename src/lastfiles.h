// -*- C++ -*-
/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 * 	 
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-1999 The LyX Team.
 *
 * ====================================================== */

#ifndef LASTFILES_H
#define LASTFILES_H

#ifdef __GNUG__
#pragma interface
#endif

#include <deque>
using std::deque;

#include "LString.h"


/** The latest documents loaded
    This class takes care of the last .lyx files used by the LyX user. It
    both reads and writes this information to a file. The number of files
    kept are user defined, but defaults to four.
*/
class LastFiles 
{
public:
	///
	typedef deque<string> Files;

	/**@name Constructors and Deconstructors */
	//@{
	/**
	   Parameters are: name of file to read. Whether LastFiles should
	   check for file existance, and the number of files to remember.
	*/
	LastFiles(string const &, bool dostat = true, unsigned int num = 4);
	//@}
	
	/**@name Methods */
	//@{
	/**
	   This funtion inserts #file# into the last files list. If the file
	   already exist it is moved to the top of the list, else exist it
	   is placed on the top of the list. If the list is full the last
	   file in the list is popped from the end.
	*/
	void newFile(string const &);
	/**  Writes the lastfiles table to disk. One file on each line, this
	     way we can at least have some special chars (e.g. space), but
	     newline in filenames are thus not allowed.
	*/
	void writeFile(string const &) const;
	///
	string operator[](unsigned int) const;
	///
	Files::const_iterator begin() const { return files.begin(); }
	///
	Files::const_iterator end() const { return files.end(); }
	//@}
private:
	/**@name const variables */
	//@{
	enum {
		///
		DEFAULTFILES = 4,
		/** There is no point in keeping more than this number
		    of files at the same time. However perhaps someday
		    someone finds use for more files and wants to
		    change it. Please do. But don't show the files in
		    a menu...
		*/
		ABSOLUTEMAXLASTFILES = 20
	};
	//@}
	
	/**@name Variables */
	//@{
	/// a list of lastfiles
	Files files;
	/// number of files in the lastfiles list.
	unsigned int num_files;
	/// check for file existance or not.
	bool dostat;
	//@}
	
	/**@name Methods */
	//@{
	/** reads the .lyx_lastfiles at the beginning of the LyX session.
	    This will read the lastfiles file (usually .lyx_lastfiles). It
	    will normally discard files that don't exist anymore, unless
	    LastFiles has been initialized with dostat = false. 
	*/
	void readFile(string const &);
	/// used by the constructor to set the number of stored last files.
        void setNumberOfFiles(unsigned int num);
	//@}
};
#endif

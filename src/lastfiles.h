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
#include <boost/utility.hpp>

/** The latest documents loaded.
    This class takes care of the last .lyx files used by the LyX user. It
    both reads and writes this information to a file. The number of files
    kept are user defined, but defaults to four.
    @author Lars Gullik Bjønnes
*/
class LastFiles : public boost::noncopyable {
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
	
	/** Insert #file# into the list.
	    This funtion inserts #file# into the last files list. If the file
	    already exist it is moved to the top of the list, else exist it
	    is placed on the top of the list. If the list is full the last
	    file in the list is popped from the end.
	    @param file the file to insert in the list.
	*/
	void newFile(string const & file);
	/** Writes the lastfiles table to disk.
	    Writes one file on each line, this way we can at least have
	    some special chars (e.g. space), but newline in filenames
	    are thus not allowed.
	    @param file the file we write the lastfiles list to.
	*/
	void writeFile(string const & file) const;
	/** Return file #n# in the lastfiles list.
	    @param n number in the list to get
	*/
	string const operator[](unsigned int n) const;
	/// Iterator to the beginning of the list.
	Files::const_iterator begin() const { return files.begin(); }
	/// Iterator to the end of the list.
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
	    Reads the #.lyx_lastfiles# at the beginning of the LyX session.
	    This will read the lastfiles file (usually #.lyx_lastfiles#). It
	    will normally discard files that don't exist anymore, unless
	    LastFiles has been initialized with #dostat = false#.
	    @param file the file containing the lastfiles.
	*/
	void readFile(string const & file);
	/** Used by the constructor to set the number of stored last files.
	    @param num the number of lastfiles to set.
	*/
        void setNumberOfFiles(unsigned int num);
};
#endif

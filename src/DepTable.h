// -*- C++ -*-
/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 *	     Copyright (C) 1995 Matthias Ettrich
 *        
 *           This file is Copyright (C) 1996-1998
 *           Lars Gullik Bjønnes
 *
 * ====================================================== 
 */

#ifndef DEP_TABLE_H
#define DEP_TABLE_H

#include "LString.h"
#include <map>
using std::map;
using std::pair;

#ifdef __GNUG__
#pragma interface
#endif

///
class DepTable {
public:
	/** This one is a little bit harder since we need the absolute
	  filename. Should we insert files with .sty .cls etc as
	  extension? */
	void insert(string const & f,
		    bool upd = false,
		    unsigned long one = 0,
		    unsigned long two = 0);
	///
	void update();

	///
	void write(string const & f) const;
	///
	void read(string const & f);
	/// returns true if any of the files has changed
	bool sumchange() const;
	/// return true if fil has changed.
	bool haschanged(string const & fil) const;
	/// return true if a file with extension ext has changed.
	bool extchanged(string const & ext) const;
	///
	bool exist(string const & fil) const;
	///
	void remove_files_with_extension(string const &);
private:
	///
	typedef map<string, pair<unsigned long, unsigned long> > DepList;
	///
	DepList deplist;
};

#endif

// -*- C++ -*-
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

#ifndef DEP_TABLE_H
#define DEP_TABLE_H

#include "LString.h"
#include <map>

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
	struct dep_info {
		unsigned long first;
		unsigned long second;
		long mtime;
	};
	///
	typedef std::map<string, dep_info> DepList;
	///
	DepList deplist;
};

#endif

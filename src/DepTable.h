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

#ifndef _DEP_TABLE_H
#define _DEP_TABLE_H

#include "LString.h"
#include <stdio.h>

///
class DepTable {
public:
	///
	DepTable();
	///
	/** This one is a little bit harder since we need the absolute
	  filename. Should we insert files with .sty .cls etc as
	  extension? */
	void insert(LString const &f,
		    bool upd = false,
		    unsigned long one = 0,
		    unsigned long two = 0);
	///
	void update();

	///
	void write(LString const &f);
	///
	void read(LString const &f);
	/// returns true if any of the files has changed
	bool sumchange();
	///
	bool haschanged(LString const &fil);
private:
	///
	DepTable(LString const &f,
		 bool upd,
		 unsigned long one,
		 unsigned long two);
	///
	LString file;
	/// The files new checksum
	unsigned long new_sum;
	/// The files old checksum
	unsigned long old_sum;
	///
	DepTable *next;
		
	///
	void write(FILE *f);
};

#endif

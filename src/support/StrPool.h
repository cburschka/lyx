// -*- C++ -*-
/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *	     Copyright 2000-2001 Jean-Marc Lasgouttes
 *
 * ======================================================*/


#ifndef STRPOOL_H
#define STRPOOL_H

#ifdef __GNUG__
#pragma interface
#endif

#include "LString.h"
#include <vector>

///
class StrPool {
public:
        /// delete all the strings that have been allocated by add()
        ~StrPool();
        /// Make a copy of the string, and remember it in the pool
        char const * add(string const & str);
        
private:
	///
	typedef std::vector<char const *> Pool;
	///
        Pool pool_;
};

//extern StrPool strPool;

#endif

// -*- C++ -*-
/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *	     Copyright 2000 Jean-Marc Lasgouttes
 *
 * ======================================================*/

#ifdef __GNUG__
#pragma implementation
#endif

#include <config.h>
#include "StrPool.h"

StrPool::~StrPool()
{
        for (Pool::const_iterator cit = pool_.begin() ; 
	     cit != pool_.end() ; ++cit) {
                delete[] (*cit);
        }
}

char const * StrPool::add(string const & str)
{
	int s = str.length();
        char * buf = new char [s + 1];
        str.copy(buf, s);
        buf[s] = '\0';
        pool_.push_back(buf);
        return buf;
}


// -*- C++ -*-
/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *	     Copyright 2000-2001 Jean-Marc Lasgouttes
 *
 * ======================================================*/

#ifdef __GNUG__
#pragma implementation
#endif

#include <config.h>
#include "StrPool.h"


StrPool::~StrPool()
{
        for (Pool::const_iterator cit = pool_.begin(); 
	     cit != pool_.end() ; ++cit) {
                delete[] (*cit);
        }
}

/* One interesting thing here would be to store the strings in a map,
   so that one string is only stored once. This would make things a
   bit slower, but memory requirements would be lower in the long run.
   I expect that it would be fast enough anyway. (Lgb)
*/
char const * StrPool::add(string const & str)
{
	string::size_type s = str.length();
        char * buf = new char [s + 1];
        str.copy(buf, s);
        buf[s] = '\0';
        pool_.push_back(buf);
        return buf;
}

//StrPool strPool;


// -*- C++ -*-
/* This file is part of
 * ======================================================
* 
*           LyX, The Document Processor
*        
*           Copyright (C) 1995 Matthias Ettrich
*           Copyright (C) 1995-1998 The LyX Team.
*
*======================================================*/

#ifndef _LYX_LIB_H
#define _LYX_LIB_H

#include <stdlib.h>
#include <time.h>
#include "LString.h"
#include "gettext.h"

/// generates an checksum
unsigned long lyxsum(char const *file);

/// returns a date string
inline char* date() 
{
	time_t tid;
	if ((tid=time(NULL)) == (time_t)-1)
		return (char*)NULL;
	else
		return (ctime(&tid));
}


// Where can I put this?  I found the occurence of the same code
// three/four times. Don't you think it better to use a macro definition
// (an inlined member of some class)?
///
inline LString getUserName()
{
	LString userName ;
	userName = getenv("LOGNAME");
	if (userName.empty())
		userName = getenv("USER");
	if (userName.empty())
		userName = _("unknown");
	return userName;
}


/// Returns the maximum of two integers.
inline
int Maximum(int a, int b)
{
	return ((a>b) ? a : b);
}


/// Returns the minimum of two integers.
inline
int Minimum(int a, int b)
{
	return ((a<b) ? a : b);
}

#endif

// -*- C++ -*-
/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 *        
 *           Copyright (C) 1995 Matthias Ettrich
 *           Copyright (C) 1995-1999 The LyX Team.
 *
 * ======================================================*/

#ifndef LYX_LIB_H
#define LYX_LIB_H

#include <ctime>
#include "LString.h"
#include "gettext.h"
#include "support/filetools.h"

/// generates an checksum
unsigned long lyxsum(char const * file);

/// returns a date string
inline char * date() 
{
	time_t tid;
	if ((tid=time(0)) == (time_t)-1)
		return (char*)0;
	else
		return (ctime(&tid));
}


// Where can I put this?  I found the occurence of the same code
// three/four times. Don't you think it better to use a macro definition
// (an inlined member of some class)?
///
inline string getUserName()
{
	string userName(GetEnv("LOGNAME"));
	if (userName.empty())
		userName = GetEnv("USER");
	if (userName.empty())
		userName = _("unknown");
	return userName;
}
#endif

// -*- C++ -*-
/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 *        
 *           Copyright (C) 1995 Matthias Ettrich
 *           Copyright (C) 1995-1999 The LyX Team.
 *
 * ====================================================== */

#ifndef LYX_LIB_H
#define LYX_LIB_H

#include "LString.h"

// Where can I put this?  I found the occurence of the same code
// three/four times. Don't you think it better to use a macro definition
// (an inlined member of some class)?

// This should have been a namespace
#ifdef CXX_WORKING_NAMESPACES
namespace lyx {
	///
	char * getcwd(char * buffer, size_t size);
	///
	int chdir(char const * name);
	/// generates an checksum
	unsigned long sum(char const * file);
	/// returns a date string
	char * date(); 
	///
	string getUserName();
	///
	int kill(int pid, int sig);
	///
	void abort();
}
#else
struct lyx {
	///
	static char * getcwd(char * buffer, size_t size);
	///
	static int chdir(char const * name);
	/// generates an checksum
	static unsigned long sum(char const * file);
	/// returns a date string
	static char * date(); 
	///
	static string getUserName();
	///
	static int kill(int pid, int sig);
	///
	static void abort();
};
#endif // CXX_WORKING_NAMESPACES
#endif

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

#ifndef LYX_LIB_H
#define LYX_LIB_H

#include "LString.h"

// Where can I put this?  I found the occurence of the same code
// three/four times. Don't you think it better to use a macro definition
// (an inlined member of some class)?

// This should have been a namespace
#ifdef CXX_WORKING_NAMESPACES
///
namespace lyx {
	///
	char * getcwd(char * buffer, size_t size);
	///
	int chdir(char const * name);
	/// Returns false it it fails
	bool rename(char const * from, char const * to);
	/// Returns false it it fails
	bool copy(char const * from, char const * to);
	/// generates a checksum
	unsigned long sum(char const * file);
	/// returns a date string (not used currently)
	char * date(); 
	/// returns the name of the user (not used currently)
	string getUserName();
	///
	int kill(long int pid, int sig);
	///
	void abort();
	///
	int mkdir(char const * pathname, unsigned long int mode);
	///
	int putenv(char const * str);
}
#else
///
struct lyx {
	///
	static char * getcwd(char * buffer, size_t size);
	///
	static int chdir(char const * name);
	/// Returns false it it fails
	static bool rename(char const * from, char const * to);
	/// Returns false it it fails
	static bool copy(char const * from, char const * to);
	/// generates a checksum
	static unsigned long sum(char const * file);
	/// returns a date string (not used currently)
	static char * date(); 
	/// returns the name of the user (not used currently)
	static string getUserName();
	///
	static int kill(long int pid, int sig);
	///
	static void abort();
	///
	static int mkdir(char const * pathname, unsigned long int mode);
	///
	static int putenv(char const * str);
};
#endif // CXX_WORKING_NAMESPACES
#endif

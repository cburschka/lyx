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
#include <sys/types.h>

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
        ///
        int chdir(string const & name);
	/// Returns false it it fails
	bool rename(char const * from, char const * to);
        /// Returns false if it fails
        bool rename(string const & from, string const & to);
	/// Returns false it it fails
	bool copy(string const & from, string const & to);
	/// generates a checksum
	unsigned long sum(char const * file);
	/// generates a checksum
	unsigned long sum(string const & file);
	/// returns a date string (not used currently)
	char * date(); 
	/// returns the name of the user (not used currently)
	string const getUserName();
	///
	int kill(int pid, int sig);
	///
	void abort();
        ///
        int mkdir(string const & pathname, mode_t mode);
	///
	int putenv(char const * str);
        ///
        int unlink(string const & file);
        ///
        int rmdir(string const & file);
        ///
        int atoi(string const & nstr);
}
#else
///
struct lyx {
	///
	static char * getcwd(char * buffer, size_t size);
	///
	static int chdir(char const * name);
	///
	static int chdir(string const & name);
	/// Returns false it it fails
	static bool rename(char const * from, char const * to);
        /// Returns false if it fails
        static bool rename(string const & from, string const & to);
	/// Returns false it it fails
	static bool copy(string const & from, string const & to);
	/// generates a checksum
	static unsigned long sum(char const * file);
	/// generates a checksum
	static unsigned long sum(string const & file);
	/// returns a date string (not used currently)
	static char * date(); 
	/// returns the name of the user (not used currently)
	static string const getUserName();
	///
	static int kill(int pid, int sig);
	///
	static void abort();
	///
	static int mkdir(string const & pathname, unsigned long int mode);
	///
	static int putenv(char const * str);
        ///
        static int unlink(string const & file);
        ///
        static int rmdir(string const & file);
        ///
        static int atoi(string const & nstr);
};
#endif // CXX_WORKING_NAMESPACES
#endif

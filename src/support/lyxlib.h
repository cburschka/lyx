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

// Use a namespace if we can, a struct otherwise
#ifdef CXX_WORKING_NAMESPACES
#define OPT_STATIC
namespace lyx {
#else
#define OPT_STATIC static
struct lyx {
#endif	
	///
	OPT_STATIC char * getcwd(char * buffer, size_t size);
        ///
        OPT_STATIC int chdir(string const & name);
        /// Returns false if it fails
        OPT_STATIC bool rename(string const & from, string const & to);
	/// Returns false it it fails
	OPT_STATIC bool copy(string const & from, string const & to);
	/// generates a checksum
	OPT_STATIC unsigned long sum(string const & file);
	/// returns a date string (not used currently)
	OPT_STATIC char * date(); 
	/// returns the name of the user (not used currently)
	OPT_STATIC string const getUserName();
	///
	OPT_STATIC int kill(int pid, int sig);
	///
	OPT_STATIC void abort();
        ///
        OPT_STATIC int mkdir(string const & pathname, unsigned long int mode);
	///
	OPT_STATIC int putenv(char const * str);
        ///
        OPT_STATIC int unlink(string const & file);
        ///
        OPT_STATIC int rmdir(string const & file);
        ///
        OPT_STATIC int atoi(string const & nstr);
#ifdef CXX_WORKING_NAMESPACES
}
#else
};
#endif

#undef OPT_STATIC
	
#endif /* LYX_LIB_H */

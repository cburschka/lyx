// -*- C++ -*-
/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 *        
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2001 The LyX Team.
 *
 * ====================================================== */

#ifndef LYX_LIB_H
#define LYX_LIB_H

#include "LString.h"

// Where can I put this?  I found the occurence of the same code
// three/four times. Don't you think it better to use a macro definition
// (an inlined member of some class)?

// Use a namespace if we can, a struct otherwise
namespace lyx {

///
string const getcwd();
///
int chdir(string const & name);
/// Returns false if it fails
bool rename(string const & from, string const & to);
/// Returns false it it fails
bool copy(string const & from, string const & to);
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
int mkdir(string const & pathname, unsigned long int mode);
///
int putenv(char const * str);
///
int unlink(string const & file);
///
int rmdir(string const & file);
///
int atoi(string const & nstr);
///
string const tempName(string const & dir = string(),
		      string const & mask = string());


/** Returns true if var is approximately equal to number with allowed error
 * of 'error'.
 *
 * Reason: A float can be very close to the number, yet still need not be 
 * exactly equal, you can have exp(-10) which is very close to zero but not
 * zero. If you only need an approximate equality (you usually do), use this
 * template.
 *
 * Usage: if (float_equal(var, number, 0.0001)) { }
 * 
 * This will check if 'var' is approx. equal to 'number' with error of 1/1000
 */
inline bool float_equal(float var, float number, float error)
{
	return (number - error <= var && var <= number + error);
}

} // namespace lyx
#endif /* LYX_LIB_H */

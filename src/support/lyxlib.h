// -*- C++ -*-
/**
 * \file lyxlib.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * A selection of useful system functions made
 * handy for C++ usage.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef LYX_LIB_H
#define LYX_LIB_H

#include "LString.h"

namespace lyx {

/// get the current working directory
string const getcwd();
/// change to a directory, 0 is returned on success.
int chdir(string const & name);
/**
 * rename a file, returns false if it fails. 
 * It can handle renames across partitions.
 */
bool rename(string const & from, string const & to);
/// copy a file, returns false it it fails
bool copy(string const & from, string const & to);
/// generates a checksum of a file
unsigned long sum(string const & file);
/// FIXME: some point to this hmm ?
int kill(int pid, int sig);
/// FIXME: same here
void abort();
/// create the given directory with the given mode
int mkdir(string const & pathname, unsigned long int mode);
/// put a C string into the environment
int putenv(char const * str);
/// unlink the given file
int unlink(string const & file);
/// remove the given directory
int rmdir(string const & file);
/// convert the given string to an integer
int atoi(string const & nstr);
/// (securely) create a temporary file in the given dir with the given prefix
string const tempName(string const & dir = string(),
		      string const & mask = string());


/**
 * Returns true if var is approximately equal to number with allowed error
 * of 'error'.
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

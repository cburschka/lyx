// -*- C++ -*-
/**
 * \file tostr.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS
 *
 * A collection of string helper functions that works with string.
 * Some of these would certainly benefit from a rewrite/optimization.
 */

#ifndef TOSTR_H
#define TOSTR_H

#include "LString.h"

// When trying to convert this to a template using std::stringstream,
// note that this will pull in the whole of <string> in more than 150
// files, even when configuring --with-included-strings !

/// convert things to strings
string const tostr(bool b);
///
string const tostr(int);
///
string const tostr(unsigned int);
///
string const tostr(long int);
///
string const tostr(double);
///
string const tostr(string const & s);

#endif

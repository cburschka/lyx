// -*- C++ -*-
/**
 * \file tostr.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 *
 * A collection of string helper functions that works with string.
 * Some of these would certainly benefit from a rewrite/optimization.
 */

#ifndef TOSTR_H
#define TOSTR_H

#include <string>

/// convert things to strings
std::string const tostr(bool b);
///
std::string const tostr(int);
///
std::string const tostr(unsigned int);
///
std::string const tostr(long int);
///
std::string const tostr(double);
///
std::string const tostr(std::string const & s);

#endif

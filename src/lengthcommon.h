// -*- C++ -*-

#ifndef LENGTH_COMMON_H
#define LENGTH_COMMON_H

#include "LString.h"
#include "lyxlength.h"

/// the number of units possible
extern int const num_units;

/**
 * array of unit names
 *
 * FIXME: I am not sure if "mu" should be possible to select (Lgb)
 */
extern char const *unit_name[];

/// return the unit given a string representation such as "cm"
LyXLength::UNIT unitFromString(string const & data);

#endif // LENGTH_COMMON_H

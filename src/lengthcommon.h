// -*- C++ -*-

#ifndef LENGHT_COMMON_H
#define LENGHT_COMMON_H

#include "LString.h"
#include "lyxlength.h"

extern int const num_units;

// I am not sure if "mu" should be possible to select (Lgb)
extern char const *unit_name[];

LyXLength::UNIT unitFromString(string const & data);

#endif

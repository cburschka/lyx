// -*- C++ -*-
/**
 * \file counter_reps.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Richard Kimberly Heck (roman numerals)
 * \author Jean-Marc Lasgouttes
 *
 * Full author contact details are available in file CREDITS.
 *
 * A collection of helper functions to convert counters to different
 * formats.
 */

#ifndef COUNTER_REPS_H
#define COUNTER_REPS_H

#include "support/strfwd.h"

namespace lyx {

char loweralphaCounter(int const n);
char alphaCounter(int const n);
char hebrewCounter(int const n);
docstring const romanCounter(int const n);
docstring const lowerromanCounter(int const n);
docstring const fnsymbolCounter(int const n);

} // namespace lyx

#endif

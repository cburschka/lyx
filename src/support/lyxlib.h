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
 * Full author contact details are available in file CREDITS.
 */

#ifndef LYX_LIB_H
#define LYX_LIB_H

// always include <math.h> (also with MSVC), to avoid compiler specific side effects
#include <math.h>

#ifdef _MSC_VER
/// Replacement for C99 round()
inline double round(double x)
{
	if (x < 0)
		return ceil(x - 0.5);
	else
		return floor(x + 0.5);
}
#endif

namespace lyx {
namespace support {

/// FIXME: some point to this hmm ?
int kill(int pid, int sig);

/**
 * Returns true if var is approximately equal to number with allowed error
 * of 'error'.
 *
 * Usage: if (float_equal(var, number, 0.0001)) { }
 *
 * This will check if 'var' is approx. equal to 'number' with error of 1/1000
 */
inline bool float_equal(double var, double number, double error)
{
	return (number - error <= var && var <= number + error);
}

/// round \p x to nearest integer
inline int iround(double x)
{
	return static_cast<int>(round(x));
}

} // namespace support
} // namespace lyx

#endif /* LYX_LIB_H */

// -*- C++ -*-
/**
 * \file lyxtime.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Jürgen Spitzmüller
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef LYXTIME_H
#define LYXTIME_H

#include <time.h>
#include <string>


namespace lyx {

time_t current_time();

/** Returns a locale-dependent formatting of the date
 *  and time encoded in \c time. The \p fmt string
 *  holds the formatting arguments of \c strftime.
 */
std::string const formatted_time(time_t t, std::string const & fmt);

}; // namespace lyx

#endif // LYXTIME_H

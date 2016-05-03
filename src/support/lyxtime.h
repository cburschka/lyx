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

#include "strfwd.h"


namespace lyx {
namespace support {

time_t current_time();

/** Returns a locale-dependent formatting of the date
 *  and time encoded in \c time. The \p fmt string
 *  holds the formatting arguments of \c strftime.
 *  Prefer the function formatted_datetime below.
 */
std::string const formatted_time(time_t t, std::string const & fmt);

/** Returns a locale-dependent formatting of the date and time encoded in \c t
 *  The \p fmt string holds the formatting arguments of QDateTime::toString().
 *  If fmt is empty then the formatting of the date and time is itself according
 *  to the locale.
 */
docstring formatted_datetime(time_t t, std::string const & fmt = "");

/**
 * Inverse of asctime(gmtime()).
 */
time_t from_asctime_utc(std::string t);

} // namespace support
} // namespace lyx

#endif // LYXTIME_H

// -*- C++ -*-
/**
 * \file lyxtime.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef LYXTIME_H
#define LYXTIME_H

#include <time.h>

namespace lyx {

typedef time_t time_type;

time_type current_time();

}; // namespace lyx

#endif // LYXTIME_H

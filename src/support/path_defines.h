// -*- C++ -*-
/**
 * \file path_defines.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef PATH_DEFINES_H
#define PATH_DEFINES_H

#include "LString.h"

namespace lyx {
namespace support {

/* The absolute path to the lyx build directory.
 * (Make-time value.)
 */
string const & build_lyxdir();

/// The absolute path to the lyx support files we're actually going to use.
string const & system_lyxdir();

/// Set the absolute path to the lyx support files (from the command line).
void system_lyxdir(string const &);

/** \returns true if the user lyx dir existed already and did not need
 *  to be created afresh.
 */
bool setLyxPaths();

} // namespace support
} // namespace lyx

#endif // NOT PATH_DEFINES_H

// -*- C++ -*-
/**
 * \file path_defines.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef PATH_DEFINES_H
#define PATH_DEFINES_H

#include <string>


namespace lyx {
namespace support {

std::string const & lyx_localedir();

/* The absolute path to the top of the lyx build tree.
 * (Make-time value.)
 */
std::string const & top_srcdir();

/// The absolute path to the lyx support files we're actually going to use.
std::string const & system_lyxdir();

/// Set the absolute path to the lyx support files (from the command line).
void system_lyxdir(std::string const &);

/// The absolute path to the lyx support files in the build directory
std::string const & build_lyxdir();

/// The absolute path to the user-level lyx support files.
std::string const & user_lyxdir();

/// Set the absolute path to the user-level lyx support files.
void user_lyxdir(std::string const &);

/** \returns true if the user lyx dir existed already and did not need
 *  to be created afresh.
 */
bool setLyxPaths();

} // namespace support
} // namespace lyx

#endif // NOT PATH_DEFINES_H

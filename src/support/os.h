// -*- C++ -*-
/**
 * \file os.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Ruurd A. Reitsma
 *
 * Full author contact details are available in file CREDITS.
 *
 * wrap OS-specific stuff
 */

#ifndef OS_H
#define OS_H

#include <string>


namespace lyx {
namespace support {
namespace os {

enum shell_type {
	UNIX,	// Do we have to distinguish sh and csh?
	CMD_EXE
};

/// Do some work just once.
void init(int argc, char * argv[]);

/// Returns the name of the NULL device (/dev/null, null).
std::string const & nulldev();

/// Returns "/" on *nix, "C:/", etc on Windows.
std::string current_root();

///
shell_type shell();

/// Extract the path common to both @c p1 and @c p2. DBCS aware!
std::string::size_type common_path(std::string const & p1, std::string const & p2);

/// Converts a unix style path to host OS style.
std::string external_path(std::string const & p);

/// Converts a host OS style path to unix style.
std::string internal_path(std::string const & p);

/// Is the path absolute?
bool is_absolute_path(std::string const & p);

/** Returns a string suitable to be passed to popen when
 *  reading a file.
 */
char const * popen_read_mode();

/** The character used to separate paths returned by the
 *  PATH environment variable.
 */
char path_separator();

/** If @c use_cygwin_paths is true, LyX will output cygwin style paths
 *  rather than native Win32 ones. Obviously, this option is used only
 *  under Cygwin.
 */
void cygwin_path_fix(bool use_cygwin_paths);

} // namespace os
} // namespace support
} // namespace lyx

#endif

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

//
enum shell_type {
	UNIX,	// Do we have to distinguish sh and csh?
	CMD_EXE
};

// do some work just once
void init(int argc, char * argv[]);
// returns path of LyX binary
std::string const & binpath();
// returns name of LyX binary
std::string const & binname();
//
void setTmpDir(std::string const & p);
//
std::string const & getTmpDir();
// Returns the user's home directory ($HOME in the unix world).
std::string const & homepath();
// Returns the name of the NULL device (/dev/null, null).
std::string const & nulldev();
//
std::string current_root();
//
shell_type shell();
// DBCS aware!
std::string::size_type common_path(std::string const & p1, std::string const & p2);
// Converts a unix style path to host OS style.
std::string external_path(std::string const & p);
// Converts a host OS style path to unix style.
std::string internal_path(std::string const & p);
// is path absolute?
bool is_absolute_path(std::string const & p);
// returns a string suitable to be passed to popen when
// same for popen().
	char const * popen_read_mode();
//
void warn(std::string const & mesg);

} // namespace os
} // namespace support
} // namespace lyx

#endif

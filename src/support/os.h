// -*- C++ -*-
/**
 * \file os.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Ruurd A. Reitsma
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef OS_H
#define OS_H

#include "LString.h"

/// wrap OS specific stuff
namespace lyx {
namespace support {
namespace os {

//
enum shell_type {
	UNIX,	// Do we have to distinguish sh and csh?
	CMD_EXE
};

// do some work just once
void init(int * argc, char ** argv[]);
// returns path of LyX binary
string binpath();
// returns name of LyX binary
string binname();
// system_tempdir actually doesn't belong here
// I put it here only to avoid a global variable.
void setTmpDir(string const & p);
//
string getTmpDir();
//
string current_root();
//
shell_type shell();
// DBCS aware!
string::size_type common_path(string const & p1, string const & p2);
// no-op on UNIX, '\\'->'/' on OS/2 and Win32, ':'->'/' on MacOS, etc.
string slashify_path(string const & p);
// converts a host OS path to unix style
string external_path(string const & p);
// converts a unix path to host OS style
string internal_path(string const & p);
// is path absolute?
bool is_absolute_path(string const & p);
// returns a string suitable to be passed to popen when
// same for popen().
	char const * popen_read_mode();
//
void warn(string const & mesg);

} // namespace os
} // namespace support
} // namespace lyx

#endif

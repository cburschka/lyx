/**
 * \file systemcall.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Asger Alstrup
 *
 * Interface cleaned up by
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "systemcall.h"
#include "os.h"

#include <cstdlib>

using std::string;

#ifndef CXX_GLOBAL_CSTD
using std::system;
#endif

namespace lyx {
namespace support {

// Reuse of instance
int Systemcall::startscript(Starttype how, string const & what)
{
	string command = what;

	if (how == DontWait) {
		if (os::shell() == os::UNIX) {
			command += " &";
		} else {
			command = "start /min/n " + command;
		}
	}

	return ::system(command.c_str());
}

} // namespace support
} // namespace lyx

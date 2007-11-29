/**
 * \file os.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Ruurd A. Reitsma
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#if defined(__CYGWIN__) || defined(__CYGWIN32__)
#include "support/os_cygwin.cpp"
#elif defined(_WIN32)
#include "support/os_win32.cpp"
#else
#include "support/os_unix.cpp"
#endif

namespace lyx {
namespace support {
namespace os {

std::string const python()
{
	// Use the -tt switch so that mixed tab/whitespace indentation is
	// an error
	static std::string const command("python -tt");
	return command;
}

}
}
}

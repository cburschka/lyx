/**
 * \file os.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Ruurd A. Reitsma
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#ifdef __EMX__
#include "os_os2.C"
#elif defined(__CYGWIN__) || defined(__CYGWIN32__) || defined(_WIN32)
#include "os_win32.C"
#else
#include "os_unix.C"
#endif

#include <config.h>

#ifdef __EMX__
#include "os_os2.C"
#elif defined(__CYGWIN__) || defined(__CYGWIN32__)
#include "os_win32.C"
#else
#include "os_unix.C"
#endif

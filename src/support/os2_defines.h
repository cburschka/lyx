/* -*- C++ -*- */
/**
 * \file os2_defines.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * Prototypes and definitions needed for OS/2.
 */

#ifndef OS2_DEFINES_H
#define OS2_DEFINES_H

#if defined (__cplusplus)
#include <cctype>
#include <cstdlib>

extern "C"
inline int readlink(const char *, char *, size_t) {return -1;}

#else
#include <ctype.h>
#include <stdlib.h>
#define readlink(s, t, l) (-1)
#endif
#include <process.h>
#include <unistd.h>
#include <X11/Xlocale.h>
/* #include <malloc.h> */
#define lstat stat
#define S_ISLNK(x) false
#define S_ISBLK(x) false
/*#define mkfifo(p, m) (0) *//* LyXserver is temporary disabled. */
#define chdir _chdir2
#define strcasecmp stricmp
#if defined (__cplusplus)
extern "C" {
#endif
const char* __XOS2RedirRoot(const char* pathname);
#if defined (__cplusplus)
}
#endif
#undef LYX_DIR
#define LYX_DIR __XOS2RedirRoot("/XFree86/lib/X11/lyx")
#undef LOCALEDIR
#define LOCALEDIR __XOS2RedirRoot("/XFree86/lib/X11/locale")
#undef TOP_SRCDIR
#define TOP_SRCDIR ".."
/* I have the slightest idea what I am doing here... */
#define bindtextdomain bindtextdomain__
#define textdomain textdomain__

#endif /* _OS2_DEFINES_H */

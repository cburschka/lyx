/* -*- C++ -*- */
/* This file is part of
* ====================================================== 
* 
*           LyX, The Document Processor
*        
*           Copyright 1995 Matthias Ettrich
*           Copyright 1995-2000 The LyX Team.
*
* ====================================================== 
 A few prototypes and definitions needed for OS/2 */

#ifndef OS2_DEFINES_H
#define OS2_DEFINES_H

#include <cctype>
#include <process.h>
#include <cstdlib>
#include <unistd.h>
#include <X11/Xlocale.h>
/* #include <malloc.h> */
#define lstat stat
#define S_ISLNK(x) false
#define S_ISBLK(x) false
#define readlink(s, t, l) (strcpy(t, s), strlen(t))
/*#define mkfifo(p, m) (0) *//* LyXserver is temporary disabled. */
#define getcwd _getcwd2
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

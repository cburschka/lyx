/* -*- C++ -*- */
/* This file is part of
* ======================================================
* 
*           LyX, The Document Processor
*        
*           Copyright (C) 1995,1996 Matthias Ettrich
*
*======================================================
 A table for translating OS/2 API return code into errno.
 Copied from emx library source.			 */

#ifndef _OS2_ERRORTABLE_H
#define _OS2_ERRORTABLE_H
#ifndef _ERRNO_H
#include <errno.h>
#endif

static unsigned char const errno_tab[] =
{
  EINVAL, EINVAL, ENOENT, ENOENT, EMFILE,  /* 0..4 */
  EACCES, EBADF,  EIO,    ENOMEM, EIO,     /* 5..9 */
  EINVAL, ENOEXEC,EINVAL, EINVAL, EINVAL,  /* 10..14 */
  ENOENT, EBUSY,  EXDEV,  ENOENT, EROFS,   /* 15..19 */
  EIO,    EIO,    EIO,    EIO,    EIO,     /* 20..24 */
  EIO,    EIO,    EIO,    ENOSPC, EIO,     /* 25..29 */
  EIO,    EIO,    EACCES, EACCES, EIO,     /* 30..34 */
  EIO,    EIO,    EIO,    EIO,    ENOSPC,  /* 35..39 */
  EIO,    EIO,    EIO,    EIO,    EIO,     /* 40..44 */
  EIO,    EIO,    EIO,    EIO,    EIO,     /* 45..49 */
  EIO,    EIO,    EIO,    EIO,    EBUSY,   /* 50..54 */
  EIO,    EIO,    EIO,    EIO,    EIO,     /* 55..59 */
  EIO,    ENOSPC, ENOSPC, EIO,    EIO,     /* 60..64 */
  EACCES, EIO,    EIO,    EIO,    EIO,     /* 65..69 */
  EIO,    EIO,    EIO,    EROFS,  EIO,     /* 70..74 */
  EIO,    EIO,    EIO,    EIO,    EIO,     /* 75..79 */
  EEXIST, EIO,    ENOENT, EIO,    EIO,     /* 80..84 */
  EIO,    EIO,    EINVAL, EIO,    EAGAIN,  /* 85..89 */
  EIO,    EIO,    EIO,    EIO,    EIO,     /* 90..94 */
  EINTR,  EIO,    EIO,    EIO,    EACCES,  /* 95..99 */
  ENOMEM, EINVAL, EINVAL, ENOMEM, EINVAL,  /* 100..104 */
  EINVAL, ENOMEM, EIO,    EACCES, EPIPE,   /* 105..109 */
  ENOENT, E2BIG,  ENOSPC, ENOMEM, EINVAL,  /* 110..114 */
  EINVAL, EINVAL, EINVAL, EINVAL, EINVAL,  /* 115..119 */
  EINVAL, EINVAL, EINVAL, ENOENT, EINVAL,  /* 120..124 */
  ENOENT, ENOENT, ENOENT, ECHILD, ECHILD,  /* 125..129 */
  EACCES, EINVAL, ESPIPE, EINVAL, EINVAL,  /* 130..134 */
  EINVAL, EINVAL, EINVAL, EINVAL, EINVAL,  /* 135..139 */
  EINVAL, EINVAL, EBUSY,  EINVAL, EINVAL,  /* 140..144 */
  EINVAL, EINVAL, EINVAL, EBUSY,  EINVAL,  /* 145..149 */
  EINVAL, EINVAL, ENOMEM, EINVAL, EINVAL,  /* 150..154 */
  EINVAL, EINVAL, EINVAL, EINVAL, EINVAL,  /* 155..159 */
  EINVAL, EINVAL, EINVAL, EINVAL, EAGAIN,  /* 160..164 */
  EINVAL, EINVAL, EACCES, EINVAL, EINVAL,  /* 165..169 */
  EBUSY,  EINVAL, EINVAL, EINVAL, EINVAL,  /* 170..174 */
  EINVAL, EINVAL, EINVAL, EINVAL, EINVAL,  /* 175..179 */
  EINVAL, EINVAL, EINVAL, EINVAL, ECHILD,  /* 180..184 */
  EINVAL, EINVAL, ENOENT, EINVAL, EINVAL,  /* 185..189 */
  ENOEXEC,ENOEXEC,ENOEXEC,ENOEXEC,ENOEXEC, /* 190..194 */
  ENOEXEC,ENOEXEC,ENOEXEC,ENOEXEC,ENOEXEC, /* 195..199 */
  ENOEXEC,ENOEXEC,ENOEXEC,ENOENT, EINVAL,  /* 200..204 */
  EINVAL, ENAMETOOLONG, EINVAL, EINVAL, EINVAL,  /* 205..209 */
  EINVAL, EINVAL, EACCES, ENOEXEC,ENOEXEC, /* 210..214 */
  EINVAL, EINVAL, EINVAL, EINVAL, EINVAL,  /* 215..219 */
  EINVAL, EINVAL, EINVAL, EINVAL, EINVAL,  /* 220..224 */
  EINVAL, EINVAL, EINVAL, ECHILD, EINVAL,  /* 225..229 */
  EINVAL, EBUSY,  EAGAIN, ENOTCONN, EINVAL, /* 230..234 */
  EINVAL, EINVAL, EINVAL, EINVAL, EINVAL,  /* 235..239 */
  EINVAL, EINVAL, EINVAL, EINVAL, EINVAL,  /* 240..244 */
  EINVAL, EINVAL, EINVAL, EINVAL, EINVAL,  /* 245..249 */
  EACCES, EACCES, EINVAL, ENOENT, EINVAL,  /* 250..254 */
  EINVAL, EINVAL, EINVAL, EINVAL, EINVAL,  /* 255..259 */
  EINVAL, EINVAL, EINVAL, EINVAL, EINVAL,  /* 260..264 */
  EINVAL, EINVAL, EINVAL, EINVAL, EINVAL,  /* 265..269 */
  EINVAL, EINVAL, EINVAL, EINVAL, EINVAL,  /* 270..274 */
  EINVAL, EINVAL, EINVAL, EINVAL, EINVAL,  /* 275..279 */
  EINVAL, EINVAL, EINVAL, EINVAL, EEXIST,  /* 280..284 */
  EEXIST, EINVAL, EINVAL, EINVAL, EINVAL,  /* 285..289 */
  ENOMEM, EMFILE, EINVAL, EINVAL, EINVAL,  /* 290..294 */
  EINVAL, EINVAL, EINVAL, EINVAL, EINVAL,  /* 295..299 */
  EINVAL, EBUSY,  EINVAL, ESRCH,  EINVAL,  /* 300..304 */
  ESRCH,  EINVAL, EINVAL, EINVAL, ESRCH,   /* 305..309 */
  EINVAL, ENOMEM, EINVAL, EINVAL, EINVAL,  /* 310..314 */
  EINVAL, E2BIG,  ENOENT, EIO,    EIO,     /* 315..319 */
  EINVAL, EINVAL, EINVAL, EINVAL, EAGAIN,  /* 320..324 */
  EINVAL, EINVAL, EINVAL, EIO,    ENOENT,  /* 325..329 */
  EACCES, EACCES, EACCES, ENOENT, ENOMEM   /* 330..334 */
};

inline int TranslateOS2Error(unsigned long rc)
{
	if (rc >= sizeof (errno_tab))
		return EINVAL;
	else
		return errno_tab[rc];
}

#endif /* _OS2_ERRORTABLE_H */

// -*- C++ -*-
/**
 * \file broken_headers.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

// A few prototypes missing from Sun and SCO 3.2v4 header files.

#ifndef BROKEN_HEADERS_H
#define BROKEN_HEADERS_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef HAVE_MEMMOVE
void bcopy(unsigned char * b1, unsigned char * b2, int length);
#endif

int readlink(char const * path, char * buf, int bufsiz);
int strcasecmp(char const * s1, char const * s2);

#ifdef __cplusplus
}
#endif

#endif /* _BROKEN_HEADERS_H */

// -*- C++ -*-
/**
 * \file LinkBackProxy.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Stefan Schimanski
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef LINKBACKPROXY_H
#define LINKBACKPROXY_H

#ifdef __cplusplus
extern "C" {
#endif

///
int isLinkBackDataInPasteboard();
///
int editLinkBackFile(char const * filename);
///
void getLinkBackData(void const ** buf, unsigned * len);
///
void closeAllLinkBackLinks();

#ifdef __cplusplus
} // extern "C"
#endif

#endif // LINKBACKPROXY_H

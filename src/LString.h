// -*- C++ -*-
/* This file is part of
 * ======================================================
 *
 *           LyX, The Document Processor
 *
 *	    Copyright 1995 Matthias Ettrich
 *          Copyright 1995-2001 The LyX Team.
 *
 * ====================================================== */

#ifndef LSTRING_H
#define LSTRING_H

#if 0
#ifndef _CONFIG_H
#error The <config.h> header should always be included before LString.h
#endif
#endif

#ifndef USE_INCLUDED_STRING

#include <string>
using std::string;
#define STRCONV(STR) STR
#else

#ifdef __STRING__
#error The <string> header has been included before LString.h
#else
#define __STRING__
#endif
#include "support/lyxstring.h"
using lyx::string;
#define STRCONV(STR) STR.c_str()
#endif

#endif

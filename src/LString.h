// -*- C++ -*-
/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *	    Copyright 1995 Matthias Ettrich
 *          Copyright 1995-2000 The LyX Team.
 *
 * ====================================================== */

#ifndef LSTRING_H
#define LSTRING_H 

#ifndef USE_INCLUDED_STRING
#include <string>
using std::string;
using std::getline
#else
#ifdef __STRING__
#error The <string> header has been included before LString.h
#else
#define __STRING__
#endif
#include "support/lyxstring.h"
// using lyx::string;
typedef lyxstring string;
#endif
#endif

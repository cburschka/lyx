// -*- C++ -*-
/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *	    Copyright 1995 Matthias Ettrich
 *          Copyright 1995-1999 The LyX Team.
 *
 * ====================================================== */

#ifndef LSTRING_H
#define LSTRING_H 

#ifndef USE_INCLUDED_STRING
#include <string>
using std::string;
#else
#include "support/lyxstring.h"
typedef lyxstring string;
#endif
#endif

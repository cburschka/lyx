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
#else
#include "support/lyxstring.h"
// using lyx::string;
typedef lyxstring string;
#endif
#endif

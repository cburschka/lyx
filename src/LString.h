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
#if defined(USING_EXCEPTIONS) && defined(HAVE_STL_STRING_FWD_H)
// Hide the forward declaration of string by SGI's STL > 3.13. But only when
// exceptions are in use.
// Due to a clash with SGI's forward declaration of string we have to provide
// __get_c_string() ourselves since SGI expects it to exist and block
// their string declarations as best we can.  ARRae.
# define __SGI_STL_STRING_FWD_H
static char const * __get_c_string(string const &);
#endif // HAVE_STL_STRING_FWD_H
#endif
#endif

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

#ifndef LYX_STL_STRING_FWD_H
#define LYX_STL_STRING_FWD_H

/*
 * This file exists to appease STLPort when using included lyxstring.
 * It won't be around forever ...
 */

#ifndef USE_INCLUDED_STRING
//   include the real stl_string_fwd.h
#  include STL_STRING_FWD_H_LOCATION
#else
// Hide the forward declaration of string by SGI's STL > 3.13.
// We have to provide the declaration of__get_c_string() ourselves
// since SGI expects it to exist and we're blocking their string declarations
// as best we can.
// Note that a declaration for string is still required so we have to
// provide one via LString.h.   ARRae.
#  include "LString.h"
#  define __SGI_STL_STRING_FWD_H
   static char const * __get_c_string(string const &);
#endif // USE_INCLUDED_STRING

#endif // LYX_STL_STRING_FWD_H


//  (C) Copyright Boost.org 1999. Permission to copy, use, modify, sell and
//  distribute this software is granted provided this copyright notice appears
//  in all copies. This software is provided "as is" without express or implied
//  warranty, and with no claim as to its suitability for any purpose.
//
// use this header as a workaround for missing <limits>

#ifndef BOOST_LIMITS
#define BOOST_LIMITS

#include <boost/config.hpp>

// for now just deactivate the following as otherwise we cannot compile
// using LString.h and our own string implementation (Jug)
#define BOOST_NO_LIMITS 1

#ifdef BOOST_NO_LIMITS
#include <boost/detail/limits.hpp>
#else
#include <limits>
#endif

#endif

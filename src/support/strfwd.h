// -*- C++ -*-

// Heavily inspired by /usr/include/c++/4.1/bits
//
// Copyright (C) 2001, 2002 Free Software Foundation, Inc.
//
// This file is part of the GNU ISO C++ Library.  This library is free
// software; you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the
// Free Software Foundation; either version 2, or (at your option)
// any later version.

#ifndef STRFWD_H
#define STRFWD_H

#ifdef USE_WCHAR_T

// Prefer this if possible because GNU libstdc++ has usable
// std::ctype<wchar_t> locale facets but not
// std::ctype<boost::uint32_t>. gcc older than 3.4 is also missing
// usable std::char_traits<boost::uint32_t>.
namespace lyx { typedef wchar_t char_type; }

#else

#include <boost/cstdint.hpp>
namespace lyx { typedef boost::uint32_t char_type; }

#endif

// Forward definitions do not work with libc++
#ifdef USE_LLVM_LIBCPP
#include <string>
#else

namespace std {

template<typename Alloc> class allocator;

template<typename Char> struct char_traits;
template<> struct char_traits<char>;
#ifdef USE_WCHAR_T
template<> struct char_traits<wchar_t>;
#endif

template<typename Char, typename Traits, typename Alloc> class basic_string;
typedef basic_string<char, char_traits<char>, allocator<char> > string;

template<class Char, class Traits> class basic_istream;
template<class Char, class Traits> class basic_ostream;
template<class Char, class Traits, class Allocator> class basic_ostringstream;

typedef basic_istream<char, char_traits<char> > istream;
typedef basic_ostream<char, char_traits<char> > ostream;
typedef basic_ostringstream<char, char_traits<char>, allocator<char> > ostringstream;

} // namepace std

#endif

namespace lyx {

/**
 * String type for storing the main text in UCS4 encoding.
 * Use std::string only in cases 7-bit ASCII is to be manipulated
 * within the variable.
 */
typedef std::basic_string<char_type, std::char_traits<char_type>,
	std::allocator<char_type> > docstring;

/// Base class for UCS4 input streams
typedef std::basic_istream<char_type, std::char_traits<char_type> > idocstream;

/// Base class for UCS4 output streams
typedef std::basic_ostream<char_type, std::char_traits<char_type> > odocstream;

/// UCS4 output stringstream
typedef std::basic_ostringstream<char_type, std::char_traits<char_type>, std::allocator<char_type> > odocstringstream;

#if ! defined(USE_WCHAR_T)
extern odocstream & operator<<(odocstream &, char);
#endif

// defined in lstrings.cpp
docstring const & empty_docstring();
std::string const & empty_string();
// defined in docstring.cpp
bool operator==(docstring const &, char const *);

#ifdef STD_STRING_USES_COW
template<typename Char> class trivial_string;
typedef trivial_string<char> trivstring;
typedef trivial_string<char_type> trivdocstring;
#else
typedef std::string trivstring;
typedef docstring trivdocstring;
#endif

} // namespace lyx

#endif

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


namespace std {

template<typename Char> struct char_traits;
template<> struct char_traits<char>;
template<> struct char_traits<lyx::char_type>;

template<typename Alloc> class allocator;

template<typename Char, typename Traits, typename Alloc> class basic_string;

typedef basic_string<char, char_traits<char>, allocator<char> > string;

template<class Char, class Traits> class basic_istream;
template<class Char, class Traits> class basic_ostream;

typedef basic_istream<char, char_traits<char> > istream;
typedef basic_ostream<char, char_traits<char> > ostream;

}


namespace lyx {

typedef std::basic_string<char_type, std::char_traits<char_type>,
	std::allocator<char_type> > docstring;

/// Base class for UCS4 input streams
typedef std::basic_istream<char_type, std::char_traits<char_type> >
	idocstream;

/// Base class for UCS4 output streams
typedef std::basic_ostream<char_type, std::char_traits<char_type> >
	odocstream;


} // namespace lyx

#endif

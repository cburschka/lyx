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

namespace std {

template<typename Char> struct char_traits;
template<> struct char_traits<char>;
template<> struct char_traits<wchar_t>;

template<typename Alloc> class allocator;

template<typename Char, typename Traits, typename Alloc> class basic_string;

typedef basic_string<char, char_traits<char>, allocator<char> > string;

}


#ifdef USE_WCHAR_T

// Prefer this if possible because GNU libstdc++ has usable
// std::ctype<wchar_t> locale facets but not
// std::ctype<boost::uint32_t>. gcc older than 3.4 is also missing
// usable std::char_traits<boost::uint32_t>.
namespace lyx { typedef wchar_t char_type; }

#else

#include <boost/cstdint.hpp>
namepace lyx { typedef boost::uint32_t char_type; }

#endif


namespace lyx {

typedef std::basic_string<char_type, std::char_traits<char_type>,
	std::allocator<char_type> > docstring;

/// Creates a docstring from a C string of ASCII characters
docstring const from_ascii(char const *);

/// Creates a docstring from a std::string of ASCII characters
docstring const from_ascii(std::string const &);

/// Creates a std::string of ASCII characters from a docstring
std::string const to_ascii(docstring const &);

/// Creates a docstring from a UTF8 string. This should go eventually.
docstring const from_utf8(std::string const &);

/// Creates a UTF8 string from a docstring. This should go eventually.
std::string const to_utf8(docstring const &);

/// convert \p s from the encoding of the locale to ucs4.
docstring const from_local8bit(std::string const & s);

/**
 * Convert \p s from ucs4 to the encoding of the locale.
 * This may fail and throw an exception, the caller is expected to act
 * appropriately.
 */
std::string const to_local8bit(docstring const & s);

/// convert \p s from the encoding of the file system to ucs4.
docstring const from_filesystem8bit(std::string const & s);

/// convert \p s from ucs4 to the encoding of the file system.
std::string const to_filesystem8bit(docstring const & s);

/// normalize \p s to precomposed form c
docstring const normalize_c(docstring const & s);

/// Compare a docstring with a C string of ASCII characters
bool operator==(docstring const &, char const *);

/// Compare a C string of ASCII characters with a docstring
inline bool operator==(char const * l, docstring const & r) { return r == l; }

/// Compare a docstring with a C string of ASCII characters
inline bool operator!=(docstring const & l, char const * r) { return !(l == r); }

/// Compare a C string of ASCII characters with a docstring
inline bool operator!=(char const * l, docstring const & r) { return !(r == l); }

/// Concatenate a docstring and a C string of ASCII characters
docstring operator+(docstring const &, char const *);

/// Concatenate a C string of ASCII characters and a docstring
docstring operator+(char const *, docstring const &);

/// Concatenate a docstring and a single ASCII character
docstring operator+(docstring const & l, char r);

/// Concatenate a single ASCII character and a docstring
docstring operator+(char l, docstring const & r);

/// Append a C string of ASCII characters to a docstring
docstring & operator+=(docstring &, char const *);

/// Append a single ASCII character to a docstring
docstring & operator+=(docstring & l, char r);

} // namespace lyx

#endif

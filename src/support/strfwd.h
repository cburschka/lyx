// -*- C++ -*-
/**
 * \file strfwd.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 * \author Enrico Forestieri
 *
 * Full author contact details are available in file CREDITS.
 */

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

#include<iosfwd>

#ifdef  _MSC_VER
namespace std {
template<typename Char, typename Traits, typename Alloc> class basic_string;
typedef basic_string<char, char_traits<char>, allocator<char> > string;
}
#endif

namespace lyx {

/// String type for storing the main text in UCS4 encoding
#ifdef  _MSC_VER
typedef std::basic_string<char_type, std::char_traits<char_type>, std::allocator<char_type> > docstring;
#else
typedef std::basic_string<char_type> docstring;
#endif

/// Base class for UCS4 input streams
typedef std::basic_istream<char_type> idocstream;

/// Base class for UCS4 output streams
typedef std::basic_ostream<char_type> odocstream;

#if ! defined(USE_WCHAR_T)
extern odocstream & operator<<(odocstream &, char);
#endif

} // namespace lyx

#endif

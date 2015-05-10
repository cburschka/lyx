// -*- C++ -*-
/**
 * \file docstring.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Georg Baum
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef LYX_DOCSTRING_H
#define LYX_DOCSTRING_H

#include "support/strfwd.h"

#include <string>

namespace lyx {

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

/// convert \p s from ucs4 to the \p encoding.
std::string const to_iconv_encoding(docstring const & s,
				    std::string const & encoding);

/// convert \p s from \p encoding to ucs4.
docstring const from_iconv_encoding(std::string const & s,
				    std::string const & encoding);

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

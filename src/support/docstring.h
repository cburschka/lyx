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


#if ! defined(USE_WCHAR_T) && defined(__GNUC__) && defined(__GNUC_MINOR__) && __GNUC__ == 3 && __GNUC_MINOR__ < 4
// Missing char_traits methods in gcc 3.3 and older. Taken from gcc 4.2svn.
namespace std {

template<typename T> void
char_traits<T>::assign(char_type & c1, char_type const & c2)
{
	c1 = c2;
}


template<typename T> bool
char_traits<T>::eq(char_type const & c1, char_type const & c2)
{
	return c1 == c2;
}


template<typename T> bool
char_traits<T>::lt(char_type const & c1, char_type const & c2)
{
	return c1 < c2;
}


template<typename T> int
char_traits<T>::compare(char_type const * s1, char_type const * s2, std::size_t n)
{
	for (std::size_t i = 0; i < n; ++i)
		if (lt(s1[i], s2[i]))
			return -1;
		else if (lt(s2[i], s1[i]))
			return 1;
	return 0;
}


template<typename T> std::size_t
char_traits<T>::length(char_type const * p)
{
	std::size_t i = 0;
	while (!eq(p[i], char_type()))
		++i;
	return i;
}


template<typename T> typename char_traits<T>::char_type const *
char_traits<T>::find(char_type const * s, size_t n, char_type const & a)
{
	for (std::size_t i = 0; i < n; ++i)
		if (eq(s[i], a))
			return s + i;
	return 0;
}


template<typename T> typename char_traits<T>::char_type *
char_traits<T>::move(char_type * s1, char_type const * s2, std::size_t n)
{
	return static_cast<T *>(std::memmove(s1, s2, n * sizeof(char_type)));
}


template<typename T> typename char_traits<T>::char_type *
char_traits<T>::copy(char_type * s1, char_type const * s2, std::size_t n)
{
	std::copy(s2, s2 + n, s1);
	return s1;
}


template<typename T> typename char_traits<T>::char_type *
char_traits<T>::assign(char_type * s, std::size_t n, char_type a)
{
	std::fill_n(s, n, a);
	return s;
}


template<typename T> typename char_traits<T>::char_type
char_traits<T>::to_char_type(int_type const & c)
{
	return static_cast<char_type>(c);
}


template<typename T> typename char_traits<T>::int_type
char_traits<T>::to_int_type(char_type const & c)
{
	return static_cast<int_type>(c);
}


template<typename T> bool
char_traits<T>::eq_int_type(int_type const & c1, int_type const & c2)
{
	return c1 == c2;
}


template<typename T> typename char_traits<T>::int_type
char_traits<T>::eof()
{
	return static_cast<int_type>(EOF);
}


template<typename T> typename char_traits<T>::int_type
char_traits<T>::not_eof(int_type const & c)
{
	return !eq_int_type(c, eof()) ? c : to_int_type(char_type());
}

}
#endif
#endif

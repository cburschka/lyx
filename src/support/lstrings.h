// -*- C++ -*-
/**
 * \file lstrings.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Jean-Marc Lasgouttes
 *
 * Full author contact details are available in file CREDITS.
 *
 * A collection of string helper functions that works with string.
 * Some of these would certainly benefit from a rewrite/optimization.
 */

#ifndef LSTRINGS_H
#define LSTRINGS_H

#include <vector>
#include <string>


namespace lyx {
namespace support {

///
int compare_no_case(std::string const & s, std::string const & s2);

///
int compare_ascii_no_case(std::string const & s, std::string const & s2);

///
int compare_no_case(std::string const & s, std::string const & s2, unsigned int len);

///
inline
int compare(char const * a, char const * b)
{
#ifndef CXX_GLOBAL_CSTD
	return std::strcmp(a, b);
#else
	return strcmp(a, b);
#endif
}

///
inline
int compare(char const * a, char const * b, unsigned int len)
{
#ifndef CXX_GLOBAL_CSTD
	return std::strncmp(a, b, len);
#else
	return strncmp(a, b, len);
#endif
}

///
bool isStrInt(std::string const & str);

/// does the std::string represent an unsigned integer value ?
bool isStrUnsignedInt(std::string const & str);

///
int strToInt(std::string const & str);

/// convert string to an unsigned integer
unsigned int strToUnsignedInt(std::string const & str);

///
bool isStrDbl(std::string const & str);

///
double strToDbl(std::string const & str);

///
char lowercase(char c);

///
char uppercase(char c);

/// same as lowercase(), but ignores locale
std::string const ascii_lowercase(std::string const &);

///
std::string const lowercase(std::string const &);

///
std::string const uppercase(std::string const &);

/// Does the std::string start with this prefix?
bool prefixIs(std::string const &, std::string const &);

/// Does the string end with this char?
bool suffixIs(std::string const &, char);

/// Does the std::string end with this suffix?
bool suffixIs(std::string const &, std::string const &);

///
template <typename B>
bool contains(std::string const & a, B b)
{
	return a.find(b) != std::string::npos;
}

///
bool containsOnly(std::string const &, std::string const &);

/** Extracts a token from this string at the nth delim.
    Doesn't modify the original string. Similar to strtok.
    Example:
    \code
    token("a;bc;d", ';', 1) == "bc";
    token("a;bc;d", ';', 2) == "d";
    \endcode
*/
std::string const token(std::string const & a, char delim, int n);


/** Search a token in this string using the delim.
    Doesn't modify the original string. Returns -1 in case of
    failure.
    Example:
    \code
    tokenPos("a;bc;d", ';', "bc") == 1;
    tokenPos("a;bc;d", ';', "d") == 2;
    \endcode
*/
int tokenPos(std::string const & a, char delim, std::string const & tok);


/// Substitute all \a oldchar with \a newchar
std::string const subst(std::string const & a, char oldchar, char newchar);

/// substitutes all instances of \a oldstr with \a newstr
std::string const subst(std::string const & a,
		   std::string const & oldstr, std::string const & newstr);

/** Trims characters off the end and beginning of a string.
    \code
    trim("ccabccc", "c") == "ab".
    \endcode
*/
std::string const trim(std::string const & a, char const * p = " ");

/** Trims characters off the end of a string.
    \code
    rtrim("abccc", "c") == "ab".
    \endcode
*/
std::string const rtrim(std::string const & a, char const * p = " ");

/** Trims characters off the beginning of a string.
    \code
   ltrim("ababcdef", "ab") = "cdef"
    \endcode
*/
std::string const ltrim(std::string const & a, char const * p = " ");

/** Splits the string by the first delim.
    Splits the string by the first appearance of delim.
    The leading string up to delim is returned in piece (not including
    delim), while the original string is cut from after the delimiter.
    Example:
    \code
    s1= ""; s2= "a;bc".split(s1, ';') -> s1 == "a"; s2 == "bc";
    \endcode
*/
std::string const split(std::string const & a, std::string & piece, char delim);

/// Same as split but does not return a piece
std::string const split(std::string const & a, char delim);

/// Same as split but uses the last delim.
std::string const rsplit(std::string const & a, std::string & piece, char delim);

/// Escapes non ASCII chars
std::string const escape(std::string const & lab);

/// gives a vector of stringparts which have the delimiter delim
std::vector<std::string> const getVectorFromString(std::string const & str,
					      std::string const & delim = std::string(","));

// the same vice versa
std::string const getStringFromVector(std::vector<std::string> const & vec,
				 std::string const & delim = std::string(","));


#ifdef I_AM_NOT_AFRAID_OF_HEADER_LIBRARIES

#include <boost/format.hpp>

template<class Arg1>
string bformat(string const & fmt, Arg1 arg1)
{
	return (boost::format(fmt) % arg1).str();
}


template<class Arg1, class Arg2>
string bformat(string const & fmt, Arg1 arg1, Arg2 arg2)
{
	return (boost::format(fmt) % arg1 % arg2).str();
}


template<class Arg1, class Arg2, class Arg3>
string bformat(string const & fmt, Arg1 arg1, Arg2 arg2, Arg3 arg3)
{
	return (boost::format(fmt) % arg1 % arg2 % arg3).str();
}


template<class Arg1, class Arg2, class Arg3, class Arg4>
string bformat(string const & fmt, Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4)
{
	return (boost::format(fmt) % arg1 % arg2 % arg3 % arg4).str();
}

#else

template <class Arg1>
std::string bformat(std::string const & fmt, Arg1);

template <class Arg1, class Arg2>
std::string bformat(std::string const & fmt, Arg1, Arg2);

template <class Arg1, class Arg2, class Arg3>
std::string bformat(std::string const & fmt, Arg1, Arg2, Arg3);

template <class Arg1, class Arg2, class Arg3, class Arg4>
std::string bformat(std::string const & fmt, Arg1, Arg2, Arg3, Arg4);

#endif

} // namespace support
} // namespace lyx

#endif

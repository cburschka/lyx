// -*- C++ -*-
/**
 * \file lstrings.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Jean-Marc Lasgouttes
 *
 * Full author contact details are available in file CREDITS
 *
 * A collection of string helper functions that works with string.
 * Some of these would certainly benefit from a rewrite/optimization.
 */

#ifndef LSTRINGS_H
#define LSTRINGS_H

#include <vector>

#include "LString.h"

///
int compare_no_case(string const & s, string const & s2);

///
int compare_ascii_no_case(string const & s, string const & s2);

///
int compare_no_case(string const & s, string const & s2, unsigned int len);

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
bool isStrInt(string const & str);

/// does the string represent an unsigned integer value ?
bool isStrUnsignedInt(string const & str);

///
int strToInt(string const & str);

/// convert string to an unsigned integer
unsigned int strToUnsignedInt(string const & str);

///
bool isStrDbl(string const & str);

///
double strToDbl(string const & str);

///
char lowercase(char c);

///
char uppercase(char c);

/// same as lowercase(), but ignores locale
string const ascii_lowercase(string const &);

///
string const lowercase(string const &);

///
string const uppercase(string const &);

/// Does the string start with this prefix?
bool prefixIs(string const &, string const &);

/// Does the string end with this char?
bool suffixIs(string const &, char);

/// Does the string end with this suffix?
bool suffixIs(string const &, string const &);

///
bool contains(string const & a, string const & b);

///
bool contains(string const & a, char b);

/// This should probably we rewritten to be more general.
class contains_functor {
public:
	typedef string first_argument_type;
	typedef string second_argument_type;
	typedef bool result_type;

	bool operator()(string const & haystack, string const & needle) const {
		return contains(haystack, needle);
	}
};


///
bool containsOnly(string const &, string const &);

/** Extracts a token from this string at the nth delim.
    Doesn't modify the original string. Similar to strtok.
    Example:
    \code
    token("a;bc;d", ';', 1) == "bc";
    token("a;bc;d", ';', 2) == "d";
    \endcode
*/
string const token(string const & a, char delim, int n);


/** Search a token in this string using the delim.
    Doesn't modify the original string. Returns -1 in case of
    failure.
    Example:
    \code
    tokenPos("a;bc;d", ';', "bc") == 1;
    tokenPos("a;bc;d", ';', "d") == 2;
    \endcode
*/
int tokenPos(string const & a, char delim, string const & tok);


/** Compares a string and a (simple) regular expression
  The only element allowed is "*" for any string of characters
  */
bool regexMatch(string const & a, string const & pattern);

/// Substitute all \a oldchar with \a newchar
string const subst(string const & a, char oldchar, char newchar);

/// substitutes all instances of \a oldstr with \a newstr
string const subst(string const & a,
		   string const & oldstr, string const & newstr);

/** Trims characters off the end and beginning of a string.
    \code
    trim("ccabccc", "c") == "ab".
    \endcode
*/
string const trim(string const & a, char const * p = " ");

/** Trims characters off the end of a string.
    \code
    rtrim("abccc", "c") == "ab".
    \endcode
*/
string const rtrim(string const & a, char const * p = " ");

/** Trims characters off the beginning of a string.
    \code
   ltrim("ababcdef", "ab") = "cdef"
    \endcode
*/
string const ltrim(string const & a, char const * p = " ");

/** Splits the string by the first delim.
    Splits the string by the first appearance of delim.
    The leading string up to delim is returned in piece (not including
    delim), while the original string is cut from after the delimiter.
    Example:
    \code
    s1= ""; s2= "a;bc".split(s1, ';') -> s1 == "a"; s2 == "bc";
    \endcode
*/
string const split(string const & a, string & piece, char delim);

/// Same as split but does not return a piece
string const split(string const & a, char delim);

/// Same as split but uses the last delim.
string const rsplit(string const & a, string & piece, char delim);

/// Escapes non ASCII chars
string const escape(string const & lab);

/// gives a vector of stringparts which have the delimiter delim
std::vector<string> const getVectorFromString(string const & str,
					      string const & delim = ",");

// the same vice versa
string const getStringFromVector(std::vector<string> const & vec,
				 string const & delim = ",");

// wrapper around boost::format using one argument %1$s
string bformat(string const & fmt, string const & arg1);
// arguments %1$s and %2$s
string bformat(string const & fmt, string const & arg1, string const & arg2);
// arguments %1$s and %2$s and %3$s
string bformat(string const & fmt, string const & arg1, string const & arg2,
               string const & arg3);
// arguments %1$s and %2$s and %3$s and %4$s
string bformat(string const & fmt, string const & arg1, string const & arg2,
               string const & arg3, string const & arg4);

#endif

// -*- C++ -*-

/** String helper functions.
    \file lstrings.h
    This is a collection of string helper functions that works
    together with string (and later also with STL String. Some of these
    would certainly benefit from a rewrite/optimization.
    \author Lars Gullik Bjønnes
    \author Jean-Marc Lasgouttes
*/

#ifndef LSTRINGS_H
#define LSTRINGS_H

#ifdef __GNUG__
#pragma interface
#endif

//#include <cstring>
//#include <cctype>

#include "Lsstream.h"

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

///
string const lowercase(string const &);

///
string const uppercase(string const &);

/// convert \a T to string
template<typename T>
inline
string const tostr(T const & t)
{
	ostringstream ostr;
	ostr << t;
	return ostr.str().c_str();
	// We need to use the .c_str since we sometimes are using
	// our own string class and that is not compatible with
	// basic_string<char>. (of course we don't want this later)
}


///
template<>
inline
string const tostr(bool const & b)
{
	return (b ? "true" : "false");
}

///
template<>
inline
string const tostr(string const & s)
{
	return s;
}

/// Does the string start with this prefix?
bool prefixIs(string const &, char const *);

/// Does the string start with this prefix?
bool prefixIs(string const &, string const &);

/// Does the string end with this char?
bool suffixIs(string const &, char);

/// Does the string end with this suffix?
bool suffixIs(string const &, char const *);

/// Does the string end with this suffix?
bool suffixIs(string const &, string const &);

///
bool contains(char const * a, string const & b);

///
bool contains(string const & a, char const * b);

///
bool contains(string const & a, string const & b);

///
bool contains(string const & a, char b);

///
bool contains(char const * a, char const * b);

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
bool containsOnly(string const &, char const *);

///
bool containsOnly(string const &, string const &);

///
bool containsOnly(char const *, char const *);

///
bool containsOnly(char const *, string const &);

/** Extracts a token from this string at the nth delim.
    Doesn't modify the original string. Similar to strtok.
    Example:
    \code
    "a;bc;d".token(';', 1) == "bc";
    "a;bc;d".token(';', 2) == "d";
    \endcode
*/
string const token(string const & a, char delim, int n);


/** Search a token in this string using the delim.
    Doesn't modify the original string. Returns -1 in case of
    failure.
    Example:
    \code
    "a;bc;d".tokenPos(';', "bc") == 1;
    "a;bc;d".token(';', "d") == 2;
    \endcode
*/
int tokenPos(string const & a, char delim, string const & tok);


/** Compares a string and a (simple) regular expression
  The only element allowed is "*" for any string of characters
  */
bool regexMatch(string const & a, string const & pattern);

/// Substitute all \a oldchar with \a newchar
string const subst(string const & a, char oldchar, char newchar);

/// Substitutes all instances of \a oldstr with \a newstr
string const subst(string const & a,
	     char const * oldstr, string const & newstr);

/// substitutes all instances of \a oldstr with \a newstr
string const subst(string const & a,
		   string const & oldstr, string const & newstr);

/** Strips characters off the end of a string.
    \code
    "abccc".strip('c') = "ab".
    \endcode
*/
string const strip(string const & a, char c = ' ');

/** Strips characters of the beginning of a string.
    \code
    "cccba".frontstrip('c') = "ba"
    \endcode
*/
string const frontStrip(string const & a, char c = ' ');

/** Strips characters off the beginning of a string.
    \code
    "ababcdef".frontstrip("ab") = "cdef"
    \endcode
*/
string const frontStrip(string const & a, char const * p);

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

#endif

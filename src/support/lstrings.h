// -*- C++ -*-

/** This is a collection of string helper functions that works
    together with string (and later also with STL String. Some of these
    would certainly benefit from a rewrite/optimization.
*/

#ifndef LSTRINGS_H
#define LSTRINGS_H

#include <cstring>
#include <cctype>

#ifdef HAVE_SSTREAM
#include <sstream>
#else
#include <strstream>
#endif

#include "LString.h"


///
int compare_no_case(string const & s, string const & s2);

///
int compare_no_case(string const & s, string const & s2, unsigned int len);

///
inline
int compare(char const * a, char const * b)
{
	return strcmp(a, b);
}


///
inline
int compare(char const * a, char const * b, unsigned int len)
{
	return strncmp(a, b, len);
}


///
bool isStrInt(string const & str);

///
int strToInt(string const & str);

///
bool isStrDbl(string const & str);

///
double strToDbl(string const & str);

///
string lowercase(string const &);

///
string uppercase(string const &);

/// convert T to string
template<typename T>
inline
string tostr(T const & t) 
{
#ifdef HAVE_SSTREAM
	std::ostringstream ostr;
	ostr << t;
	return ostr.str().c_str();
	// We need to use the .c_str since we sometimes are using
	// our own string class and that is not compatible with
	// basic_string<char>. (of course we don't want this later)
#else
	// The buf is probably a bit large, but if we want to be safer
	// we should leave it this big. As compiler/libs gets updated
	// this part of the code will cease to be used and we loose
	// nothing.
	char buf[2048]; // a bit too large perhaps?
	ostrstream ostr(buf, sizeof(buf));
	ostr << t << '\0';
	return buf;
#endif
}

inline
string tostr(bool b)
{
	return (b ? "true" : "false");
}
	
/// Does the string start with this prefix?
bool prefixIs(string const &, char const *);

/// Does the string end with this char?
bool suffixIs(string const &, char);

/// Does the string end with this suffix?
bool suffixIs(string const &, char const *);

///
bool contains(char const * a, string const & b);

///
bool contains(string const & a, char const * b);

///
bool contains(string const & a, string const & b);

///
bool contains(char const * a, char const * b);

/// Counts how many of character c there is in a
unsigned int countChar(string const & a, char const c);

/** Extracts a token from this string at the nth delim.
  Doesn't modify the original string. Similar to strtok.
  Example:
  #"a;bc;d".token(';', 1) == "bc";#
  #"a;bc;d".token(';', 2) == "d";#
*/
string token(string const & a, char delim, int n);


/** Search a token in this string using the delim.
  Doesn't modify the original string. Returns -1 in case of
  failure. 
  Example:
  #"a;bc;d".tokenPos(';', "bc") == 1;#
  #"a;bc;d".token(';', "d") == 2;#
*/
int tokenPos(string const & a, char delim, string const & tok);


/** Compares a string and a (simple) regular expression
  The only element allowed is "*" for any string of characters
  */
bool regexMatch(string const & a, string const & pattern);

/// Substitute all "oldchar"s with "newchar"
string subst(string const & a, char oldchar, char newchar);

/// Substitutes all instances of oldstr with newstr
string subst(string const & a,
	     char const * oldstr, string const & newstr);

/** Strips characters off the end of a string.
  #"abccc".strip('c') = "ab".#
  */
string strip(string const & a, char const c = ' ');

/** Strips characters of the beginning of a string.
  #"cccba".frontstrip('c') = "ba"#. */
string frontStrip(string const & a, char const c = ' ');

/** Strips characters off the beginning of a string.
    #"ababcdef".frontstrip("ab") = "cdef"# .*/
string frontStrip(string const & a, char const * p);

/** Splits the string by the first delim.
  Splits the string by the first appearance of delim.
  The leading string up to delim is returned in piece (not including
  delim), while the original string is cut from after the delimiter.
  Example:
  #s1= ""; s2= "a;bc".split(s1, ';') -> s1 == "a"; s2 == "bc";#
  */
string split(string const & a, string & piece, char delim);

/// Same as split but does not return a piece
string split(string const & a, char delim);

/// Same as split but uses the last delim.
string rsplit(string const & a, string & piece, char delim);

#endif

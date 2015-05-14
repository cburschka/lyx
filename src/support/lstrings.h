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

#include "support/docstring.h"

#include <vector>


namespace lyx {
namespace support {

/// Compare \p s and \p s2, ignoring the case.
/// Does not depend on the locale.
int compare_no_case(docstring const & s, docstring const & s2);

/// Compare \p s and \p s2 using the collating rules of the current locale.
int compare_locale(docstring const & s, docstring const & s2);

/// Compare \p s and \p s2, ignoring the case of ASCII characters only.
int compare_ascii_no_case(std::string const & s, std::string const & s2);

/// Compare \p s and \p s2, ignoring the case of ASCII characters only.
int compare_ascii_no_case(docstring const & s, docstring const & s2);

///
bool isStrInt(std::string const & str);

/// does the std::string represent an unsigned integer value ?
bool isStrUnsignedInt(std::string const & str);

///
bool isStrDbl(std::string const & str);

/// does the string contain a digit?
bool hasDigitASCII(docstring const & str);

bool isHexChar(char_type);

bool isHex(docstring const & str);

int hexToInt(docstring const & str);

/// is \p str pure ascii?
bool isAscii(docstring const & str);

/// is \p str pure ascii?
bool isAscii(std::string const & str);

/**
 * Changes the case of \p c to lowercase.
 * Don't use this for non-ASCII characters, since it depends on the locale.
 * This overloaded function is only implemented because the char_type variant
 * would be used otherwise, and we assert in this function that \p c is in
 * the ASCII range.
 */
char lowercase(char c);

/**
 * Changes the case of \p c to uppercase.
 * Don't use this for non-ASCII characters, since it depends on the locale.
 * This overloaded function is only implemented because the char_type variant
 * would be used otherwise, and we assert in this function that \p c is in
 * the ASCII range.
 */
char uppercase(char c);

/// Changes the case of \p c to lowercase.
/// Does not depend on the locale.
char_type lowercase(char_type c);

/// Changes the case of \p c to uppercase.
/// Does not depend on the locale.
char_type uppercase(char_type c);

/// Checks if the supplied character is lower-case
bool isLowerCase(char_type ch);

/// Checks if the supplied character is upper-case
bool isUpperCase(char_type ch);

/// same as lowercase(), but ignores locale
std::string const ascii_lowercase(std::string const &);
docstring const ascii_lowercase(docstring const &);

/// Changes the case of \p s to lowercase.
/// Does not depend on the locale.
docstring const lowercase(docstring const & s);
// Currently unused, but the code is there if needed.
// std::string const lowercase(std::string const & s);

/// Changes the case of \p s to uppercase.
/// Does not depend on the locale.
docstring const uppercase(docstring const & s);

/// Returns the superscript of \p c or \p c if no superscript exists.
/// Does not depend on the locale.
char_type superscript(char_type c);

/// Returns the subscript of \p c or \p c if no subscript exists.
/// Does not depend on the locale.
char_type subscript(char_type c);

/// Does str start with c?
bool prefixIs(docstring const & str, char_type c);

/// Does str start with pre?
bool prefixIs(std::string const & str, std::string const & pre);
bool prefixIs(docstring const & str, docstring const & pre);

/// Does the string end with this char?
bool suffixIs(std::string const &, char);
bool suffixIs(docstring const &, char_type);

/// Does the string end with this suffix?
bool suffixIs(std::string const &, std::string const &);
bool suffixIs(docstring const &, docstring const &);

/// Is b contained in a?
inline bool contains(std::string const & a, std::string const & b)
{
	return a.find(b) != std::string::npos;
}

inline bool contains(docstring const & a, docstring const & b)
{
	return a.find(b) != docstring::npos;
}

inline bool contains(std::string const & a, char b)
{
	return a.find(b) != std::string::npos;
}

inline bool contains(docstring const & a, char_type b)
{
	return a.find(b) != docstring::npos;
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

docstring const token(docstring const & a, char_type delim, int n);

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
int tokenPos(docstring const & a, char_type delim, docstring const & tok);


/// Substitute all \a oldchar with \a newchar
std::string const subst(std::string const & a, char oldchar, char newchar);

/// Substitute all \a oldchar with \a newchar
docstring const subst(docstring const & a, char_type oldchar, char_type newchar);

/// substitutes all instances of \a oldstr with \a newstr
std::string const subst(std::string const & a,
		   std::string const & oldstr, std::string const & newstr);

/// substitutes all instances of \a oldstr with \a newstr
docstring const subst(docstring const & a,
		docstring const & oldstr, docstring const & newstr);

/// Count all occurences of char \a chr inside \a str
int count_char(std::string const & str, char chr);

/// Count all occurences of char \a chr inside \a str
int count_char(docstring const & str, docstring::value_type chr);

/** Count all occurences of binary chars inside \a str.
    It is assumed that \a str is utf-8 encoded and that a binary char
    belongs to the unicode class names Zl, Zp, Cc, Cf, Cs, Co, or Cn
    (excluding white space characters such as '\t', '\n', '\v', '\f', '\r').
    See http://www.unicode.org/Public/6.2.0/ucd/UnicodeData.txt
*/
int count_bin_chars(std::string const & str);

/** Trims characters off the end and beginning of a string.
    \code
    trim("ccabccc", "c") == "ab".
    \endcode
*/
docstring const trim(docstring const & a, char const * p = " ");

/** Trims characters off the end and beginning of a string.
    \code
    trim("ccabccc", "c") == "ab".
    \endcode
*/
std::string const trim(std::string const & a, char const * p = " ");

/** Trims characters off the end of a string, removing any character
    in p.
    \code
    rtrim("abcde", "dec") == "ab".
    \endcode
*/
std::string const rtrim(std::string const & a, char const * p = " ");
docstring const rtrim(docstring const & a, char const * p = " ");

/** Trims characters off the beginning of a string.
    \code
   ("abbabcdef", "ab") = "cdef"
    \endcode
*/
std::string const ltrim(std::string const & a, char const * p = " ");
docstring const ltrim(docstring const & a, char const * p = " ");

/** Splits the string given in the first argument at the first occurence 
    of the third argument, delim.
    What precedes delim is returned in the second argument, piece; this
    will be the whole of the string if no delimiter is found.
    The return value is what follows delim, if anything. So the return
    value is the null string if no delimiter is found.
    'a' and 'piece' must be different variables.
    Examples:
    \code
    s1= "a;bc"; s2= ""
    ret = split(s1, s2, ';') -> ret = "bc", s2 == "a"
    \endcode
 */
std::string const split(std::string const & a, std::string & piece, char delim);
docstring const split(docstring const & a, docstring & piece, char_type delim);

/// Same as split but does not return a piece
std::string const split(std::string const & a, char delim);

/// Same as split but uses the last delim.
std::string const rsplit(std::string const & a, std::string & piece, char delim);
docstring const rsplit(docstring const & a, docstring & piece, char_type delim);
docstring const rsplit(docstring const & a, char_type delim);

/// Escapes non ASCII chars and other problematic characters that cause
/// problems in latex labels.
docstring const escape(docstring const & lab);

/// Word-wraps the provided docstring, returning a line-broken string
/// of width no wider than width, with the string broken at spaces. 
/// If the string cannot be broken appropriately, it returns something 
/// with "..." at the end, again no wider than width.
/// We assume here that str does not contain newlines.
/// If indent is positive, then the first line is indented that many 
/// spaces. If it is negative, then successive lines are indented, as
/// if the first line were "outdented".
docstring wrap(docstring const & str, int const indent = 0,
               size_t const width = 80);

/// Like the preceding, except it is intended to operate on strings
/// that may contain embedded newlines.
/// \param numlines Don't return more than numlines lines. If numlines
///    is 0, we return everything.
docstring wrapParas(docstring const & str, int const indent = 0,
                    size_t const width = 80, size_t const maxlines = 10);

/// gives a vector of stringparts which have the delimiter delim
/// If \p keepempty is true, empty strings will be pushed to the vector as well
std::vector<std::string> const getVectorFromString(std::string const & str,
					      std::string const & delim = std::string(","),
					      bool keepempty = false);
std::vector<docstring> const getVectorFromString(docstring const & str,
		docstring const & delim = from_ascii(","), bool keepempty = false);

/// the same vice versa
std::string const getStringFromVector(std::vector<std::string> const & vec,
				 std::string const & delim = std::string(","));
docstring const getStringFromVector(std::vector<docstring> const & vec,
				 docstring const & delim = from_ascii(","));

/// Search \p search_token in \p str and return the position if it is
/// found, else -1. The last item in \p str must be "".
int findToken(char const * const str[], std::string const & search_token);


/// Format a floating point number with at least 6 siginificant digits, but
/// without scientific notation.
/// Scientific notation would be invalid in some contexts, such as lengths for
/// LaTeX. Simply using std::ostream with std::fixed would produce results
/// like "1000000.000000", and precision control would not be that easy either.
std::string formatFPNumber(double);


template <class Arg1>
docstring bformat(docstring const & fmt, Arg1);

template <class Arg1, class Arg2>
docstring bformat(docstring const & fmt, Arg1, Arg2);

template <class Arg1, class Arg2, class Arg3>
docstring bformat(docstring const & fmt, Arg1, Arg2, Arg3);

template <class Arg1, class Arg2, class Arg3, class Arg4>
docstring bformat(docstring const & fmt, Arg1, Arg2, Arg3, Arg4);


template<> docstring bformat(docstring const & fmt, int arg1);
template<> docstring bformat(docstring const & fmt, long arg1);
template<> docstring bformat(docstring const & fmt, unsigned int arg1);
template<> docstring bformat(docstring const & fmt, docstring arg1);
template<> docstring bformat(docstring const & fmt, char * arg1);
template<> docstring bformat(docstring const & fmt, docstring arg1, docstring arg2);
template<> docstring bformat(docstring const & fmt, docstring arg1, int arg2);
template<> docstring bformat(docstring const & fmt, char const * arg1, docstring arg2);
template<> docstring bformat(docstring const & fmt, int arg1, int arg2);
template<> docstring bformat(docstring const & fmt, docstring arg1, docstring arg2, docstring arg3);
template<> docstring bformat(docstring const & fmt, docstring arg1, docstring arg2, docstring arg3, docstring arg4);


} // namespace support
} // namespace lyx

#endif

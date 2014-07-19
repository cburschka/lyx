/**
 * \file lstrings.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Jean-Marc Lasgouttes
 * \author Dekel Tsur
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "support/lstrings.h"

#include "support/convert.h"
#include "support/qstring_helpers.h"

#include "support/lassert.h"

#include <QString>

#include <cstdio>
#include <algorithm>

using namespace std;

namespace lyx {

// Using this allows us to have docstring default arguments in headers
// without #include "support/docstring" there.
docstring const & empty_docstring()
{
	static docstring s;
	return s;
}

// Using this allows us to have string default arguments in headers
// without #include <string>
string const & empty_string()
{
	static string s;
	return s;
}

namespace {
/**
 * Convert a QChar into a UCS4 character.
 * This is a hack (it does only make sense for the common part of the UCS4
 * and UTF16 encodings) and should not be used.
 * This does only exist because of performance reasons (a real conversion
 * using iconv is too slow on windows).
 */
inline char_type qchar_to_ucs4(QChar const & qchar)
{
	LASSERT(is_utf16(static_cast<char_type>(qchar.unicode())), return '?');
	return static_cast<char_type>(qchar.unicode());
}

/**
 * Convert a UCS4 character into a QChar.
 * This is a hack (it does only make sense for the common part of the UCS4
 * and UTF16 encodings) and should not be used.
 * This does only exist because of performance reasons (a real conversion
 * using iconv is too slow on windows).
 */
inline QChar const ucs4_to_qchar(char_type const ucs4)
{
	LASSERT(is_utf16(ucs4), return QChar('?'));
	return QChar(static_cast<unsigned short>(ucs4));
}

/// Maximum valid UCS4 code point
char_type const ucs4_max = 0x10ffff;
} // anon namespace


bool isLetterChar(char_type c)
{
	if (!is_utf16(c)) {
		if (c > ucs4_max)
			// outside the UCS4 range
			return false;
		// assume that all non-utf16 characters are letters
		return true;
	}
	return ucs4_to_qchar(c).isLetter();
}


bool isLower(char_type c)
{
	if (!is_utf16(c))
		return false;
	return ucs4_to_qchar(c).isLower();
}


bool isAlphaASCII(char_type c)
{
	return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z');
}


bool isPrintable(char_type c)
{
	if (!is_utf16(c)) {
		if (c > ucs4_max)
			// outside the UCS4 range
			return false;
		// assume that all non-utf16 characters are printable
		return true;
	}
	// Not yet recognized by QChar::isPrint()
	// See https://bugreports.qt-project.org/browse/QTBUG-12144
	// LATIN CAPITAL LETTER SHARP S
	else if (c == 0x1e9e)
		return true;
	return ucs4_to_qchar(c).isPrint();
}


bool isPrintableNonspace(char_type c)
{
	if (!is_utf16(c)) {
		if (c > ucs4_max)
			// outside the UCS4 range
			return false;
		// assume that all non-utf16 characters are printable and
		// no space
		return true;
	}
	QChar const qc = ucs4_to_qchar(c);
	return qc.isPrint() && !qc.isSpace();
}


bool isSpace(char_type c)
{
	if (!is_utf16(c)) {
		// assume that no non-utf16 character is a space
		// c outside the UCS4 range is catched as well
		return false;
	}
	QChar const qc = ucs4_to_qchar(c);
	return qc.isSpace();
}


bool isNumber(char_type c)
{
	if (!is_utf16(c))
		// assume that no non-utf16 character is a numeral
		// c outside the UCS4 range is catched as well
		return false;
	return ucs4_to_qchar(c).isNumber();
}


bool isDigitASCII(char_type c)
{
	return '0' <= c && c <= '9';
}


bool isAlnumASCII(char_type c)
{
	return isAlphaASCII(c) || isDigitASCII(c);
}


bool isASCII(char_type c)
{
	return c < 0x80;
}


namespace support {

int compare_no_case(docstring const & s, docstring const & s2)
{
	docstring::const_iterator p = s.begin();
	docstring::const_iterator p2 = s2.begin();

	while (p != s.end() && p2 != s2.end()) {
		char_type const lc1 = lowercase(*p);
		char_type const lc2 = lowercase(*p2);
		if (lc1 != lc2)
			return (lc1 < lc2) ? -1 : 1;
		++p;
		++p2;
	}

	if (s.size() == s2.size())
		return 0;
	if (s.size() < s2.size())
		return -1;
	return 1;
}


namespace {

template<typename Char>
Char ascii_tolower(Char c) {
	if (c >= 'A' && c <= 'Z')
		return c - 'A' + 'a';
	return c;
}

}


int compare_ascii_no_case(string const & s, string const & s2)
{
	string::const_iterator p = s.begin();
	string::const_iterator p2 = s2.begin();

	while (p != s.end() && p2 != s2.end()) {
		int const lc1 = ascii_tolower(*p);
		int const lc2 = ascii_tolower(*p2);
		if (lc1 != lc2)
			return (lc1 < lc2) ? -1 : 1;
		++p;
		++p2;
	}

	if (s.size() == s2.size())
		return 0;
	if (s.size() < s2.size())
		return -1;
	return 1;
}


int compare_ascii_no_case(docstring const & s, docstring const & s2)
{
	docstring::const_iterator p = s.begin();
	docstring::const_iterator p2 = s2.begin();

	while (p != s.end() && p2 != s2.end()) {
		char_type const lc1 = ascii_tolower(*p);
		char_type const lc2 = ascii_tolower(*p2);
		if (lc1 != lc2)
			return (lc1 < lc2) ? -1 : 1;
		++p;
		++p2;
	}

	if (s.size() == s2.size())
		return 0;
	if (s.size() < s2.size())
		return -1;
	return 1;
}


bool isStrInt(string const & str)
{
	if (str.empty())
		return false;

	// Remove leading and trailing white space chars.
	string const tmpstr = trim(str);
	if (tmpstr.empty())
		return false;

	string::const_iterator cit = tmpstr.begin();
	if ((*cit) == '-')
		++cit;

	string::const_iterator end = tmpstr.end();
	for (; cit != end; ++cit)
		if (!isDigitASCII(*cit))
			return false;

	return true;
}


bool isStrUnsignedInt(string const & str)
{
	if (str.empty())
		return false;

	// Remove leading and trailing white space chars.
	string const tmpstr = trim(str);
	if (tmpstr.empty())
		return false;

	string::const_iterator cit = tmpstr.begin();
	string::const_iterator end = tmpstr.end();
	for (; cit != end; ++cit)
		if (!isDigitASCII(*cit))
			return false;

	return true;
}


bool isStrDbl(string const & str)
{
	if (str.empty())
		return false;

	// Remove leading and trailing white space chars.
	string const tmpstr = trim(str);
	if (tmpstr.empty())
		return false;
	//	if (tmpstr.count('.') > 1) return false;

	string::const_iterator cit = tmpstr.begin();
	bool found_dot = false;
	if (*cit == '-')
		++cit;
	string::const_iterator end = tmpstr.end();
	for (; cit != end; ++cit) {
		if (!isDigitASCII(*cit) && *cit != '.')
			return false;
		if ('.' == (*cit)) {
			if (found_dot)
				return false;
			found_dot = true;
		}
	}
	return true;
}


bool hasDigitASCII(docstring const & str)
{
	docstring::const_iterator cit = str.begin();
	docstring::const_iterator const end = str.end();
	for (; cit != end; ++cit)
		if (isDigitASCII(*cit))
			return true;
	return false;
}


bool isHexChar(char_type c)
{
	return c == '0' ||
		c == '1' ||
		c == '2' ||
		c == '3' ||
		c == '4' ||
		c == '5' ||
		c == '6' ||
		c == '7' ||
		c == '8' ||
		c == '9' ||
		c == 'a' || c == 'A' ||
		c == 'b' || c == 'B' ||
		c == 'c' || c == 'C' ||
		c == 'd' || c == 'D' ||
		c == 'e' || c == 'E' ||
		c == 'f' || c == 'F';
}


bool isHex(docstring const & str)
{
	int index = 0;

	if (str.length() > 2 && str[0] == '0' &&
	    (str[1] == 'x' || str[1] == 'X'))
		index = 2;

	int const len = str.length();

	for (; index < len; ++index) {
		if (!isHexChar(str[index]))
			return false;
	}
	return true;
}


int hexToInt(docstring const & str)
{
	string s = to_ascii(str);
	int h;
	sscanf(s.c_str(), "%x", &h);
	return h;
}


bool isAscii(docstring const & str)
{
	int const len = str.length();
	for (int i = 0; i < len; ++i)
		if (str[i] >= 0x80)
			return false;
	return true;
}


bool isAscii(string const & str)
{
	int const len = str.length();
	for (int i = 0; i < len; ++i)
		if (static_cast<unsigned char>(str[i]) >= 0x80)
			return false;
	return true;
}


char lowercase(char c)
{
	LASSERT(isASCII(c), return '?');
	return char(tolower(c));
}


char uppercase(char c)
{
	LASSERT(isASCII(c), return '?');
	return char(toupper(c));
}


char_type lowercase(char_type c)
{
	if (!is_utf16(c))
		// We don't know how to lowercase a non-utf16 char
		return c;
	return qchar_to_ucs4(ucs4_to_qchar(c).toLower());
}


char_type uppercase(char_type c)
{
	if (!is_utf16(c))
		// We don't know how to uppercase a non-utf16 char
		return c;
	return qchar_to_ucs4(ucs4_to_qchar(c).toUpper());
}


bool isLowerCase(char_type ch) {
	return lowercase(ch) == ch;
}


bool isUpperCase(char_type ch) {
	return uppercase(ch) == ch;
}


namespace {

// since we cannot use tolower and toupper directly in the
// calls to transform yet, we use these helper clases. (Lgb)

struct local_lowercase {
	char_type operator()(char_type c) const {
		return lowercase(c);
	}
};

struct local_uppercase {
	char_type operator()(char_type c) const {
		return uppercase(c);
	}
};

template<typename Char> struct local_ascii_lowercase {
	Char operator()(Char c) const { return ascii_tolower(c); }
};

} // end of anon namespace


docstring const lowercase(docstring const & a)
{
	docstring tmp(a);
	transform(tmp.begin(), tmp.end(), tmp.begin(), local_lowercase());
	return tmp;
}


/* Uncomment here and in lstrings.h if you should need this.
string const lowercase(string const & a)
{
	string tmp(a);
	transform(tmp.begin(), tmp.end(), tmp.begin(), local_lowercase());
	return tmp;
}
*/


docstring const uppercase(docstring const & a)
{
	docstring tmp(a);
	transform(tmp.begin(), tmp.end(), tmp.begin(), local_uppercase());
	return tmp;
}


string const ascii_lowercase(string const & a)
{
	string tmp(a);
	transform(tmp.begin(), tmp.end(), tmp.begin(),
		  local_ascii_lowercase<char>());
	return tmp;
}


docstring const ascii_lowercase(docstring const & a)
{
	docstring tmp(a);
	transform(tmp.begin(), tmp.end(), tmp.begin(),
		  local_ascii_lowercase<char_type>());
	return tmp;
}


char_type superscript(char_type c)
{
	switch (c) {
		case    '2': return 0x00b2;
		case    '3': return 0x00b3;
		case    '1': return 0x00b9;
		case    '0': return 0x2070;
		case    'i': return 0x2071;
		case    '4': return 0x2074;
		case    '5': return 0x2075;
		case    '6': return 0x2076;
		case    '7': return 0x2077;
		case    '8': return 0x2078;
		case    '9': return 0x2079;
		case    '+': return 0x207a;
		case    '-': return 0x207b;
		case    '=': return 0x207c;
		case    '(': return 0x207d;
		case    ')': return 0x207e;
		case    'n': return 0x207f;
		case    'h': return 0x02b0;
		case 0x0266: return 0x02b1; // LATIN SMALL LETTER H WITH HOOK
		case    'j': return 0x02b2;
		case    'r': return 0x02b3;
		case 0x0279: return 0x02b4; // LATIN SMALL LETTER TURNED R
		case 0x027b: return 0x02b5; // LATIN SMALL LETTER TURNED R WITH HOOK
		case 0x0281: return 0x02b6; // LATIN SMALL LETTER CAPITAL INVERTED R
		case    'w': return 0x02b7;
		case    'y': return 0x02b8;
//		case 0x0294: return 0x02c0; // LATIN LETTER GLOTTAL STOP)
//		case 0x0295: return 0x02c1; // LATIN LETTER PHARYNGEAL VOICED FRICATIVE
		                            // (= LATIN LETTER REVERSED GLOTTAL STOP)
		case    'l': return 0x02e1;
		case    's': return 0x02e2;
		case    'x': return 0x02e3;
//		case 0x0295: return 0x02e4; // LATIN SMALL LETTER REVERSED GLOTTAL STOP
		case    'A': return 0x1d2c;
		case 0x00c6: return 0x1d2d; // LATIN CAPITAL LETTER AE
		case    'B': return 0x1d2e;
		case    'D': return 0x1d30;
		case    'E': return 0x1d31;
		case    'G': return 0x1d33;
		case    'H': return 0x1d34;
		case    'I': return 0x1d35;
		case    'J': return 0x1d36;
		case    'K': return 0x1d37;
		case    'L': return 0x1d38;
		case    'M': return 0x1d39;
		case    'N': return 0x1d3a;
		case    'O': return 0x1d3c;
		case    'P': return 0x1d3e;
		case    'R': return 0x1d3f;
		case    'T': return 0x1d40;
		case    'U': return 0x1d41;
		case    'W': return 0x1d42;
		case    'a': return 0x1d43;
		case 0x0250: return 0x1d44; // LATIN SMALL LETTER TURNED A
		case 0x0251: return 0x1d45; // LATIN SMALL LETTER ALPHA
		case    'b': return 0x1d47;
		case    'd': return 0x1d48;
		case    'e': return 0x1d49;
		case 0x0259: return 0x1d4a; // LATIN SMALL LETTER SCHWA
		case 0x025b: return 0x1d4b; // LATIN SMALL LETTER OPEN E
		case 0x1d08: return 0x1d4c; // LATIN SMALL LETTER TURNED OPEN E
		case    'g': return 0x1d4d;
		case 0x1d09: return 0x1d4e; // LATIN SMALL LETTER TURNED I
		case    'k': return 0x1d4f;
		case    'm': return 0x1d50;
		case 0x014b: return 0x1d51; // LATIN SMALL LETTER ENG
		case    'o': return 0x1d52;
		case 0x0254: return 0x1d53; // LATIN SMALL LETTER OPEN O
		case 0x1d16: return 0x1d54; // LATIN SMALL LETTER TOP HALF O
		case 0x1d17: return 0x1d55; // LATIN SMALL LETTER BOTTOM HALF O
		case    'p': return 0x1d56;
		case    't': return 0x1d57;
		case    'u': return 0x1d58;
		case 0x1d1d: return 0x1d59; // LATIN SMALL LETTER SIDEWAYS U
		case 0x1d1f: return 0x1d5a; // LATIN SMALL LETTER SIDEWAYS TURNED M
		case    'v': return 0x1d5b;
		case 0x03b2: return 0x1d5d; // GREEK SMALL LETTER BETA
		case 0x03b3: return 0x1d5e; // GREEK SMALL LETTER GAMMA
		case 0x03b4: return 0x1d5f; // GREEK SMALL LETTER DELTA
		case 0x03c6: return 0x1d60; // GREEK SMALL LETTER PHI
		case 0x03c7: return 0x1d61; // GREEK SMALL LETTER CHI
	}
	return c;
}


char_type subscript(char_type c)
{
	switch (c) {
		case    'i': return 0x1d62;
		case    'r': return 0x1d63;
		case    'u': return 0x1d64;
		case    'v': return 0x1d65;
		case 0x03b2: return 0x1d66; // GREEK SMALL LETTER BETA
		case 0x03b3: return 0x1d67; // GREEK SMALL LETTER GAMMA
		case 0x03c1: return 0x1d68; // GREEK SMALL LETTER RHO
		case 0x03c6: return 0x1d69; // GREEK SMALL LETTER PHI
		case 0x03c7: return 0x1d6a; // GREEK SMALL LETTER CHI
		case    '0': return 0x2080;
		case    '1': return 0x2081;
		case    '2': return 0x2082;
		case    '3': return 0x2083;
		case    '4': return 0x2084;
		case    '5': return 0x2085;
		case    '6': return 0x2086;
		case    '7': return 0x2087;
		case    '8': return 0x2088;
		case    '9': return 0x2089;
		case    '+': return 0x208a;
		case    '-': return 0x208b;
		case    '=': return 0x208c;
		case    '(': return 0x208d;
		case    ')': return 0x208e;
		case    'a': return 0x2090;
		case    'e': return 0x2091;
		case    'o': return 0x2092;
		case    'x': return 0x2093;
		case 0x0259: return 0x2093; // LATIN SMALL LETTER SCHWA
	}
	return c;
}


bool prefixIs(docstring const & a, char_type c)
{
	if (a.empty())
		return false;
	return a[0] == c;
}


bool prefixIs(string const & a, string const & pre)
{
	size_t const prelen = pre.length();
	size_t const alen = a.length();
	return prelen <= alen && !a.empty() && a.compare(0, prelen, pre) == 0;
}


bool prefixIs(docstring const & a, docstring const & pre)
{
	size_t const prelen = pre.length();
	size_t const alen = a.length();
	return prelen <= alen && !a.empty() && a.compare(0, prelen, pre) == 0;
}


bool suffixIs(string const & a, char c)
{
	if (a.empty())
		return false;
	return a[a.length() - 1] == c;
}


bool suffixIs(docstring const & a, char_type c)
{
	if (a.empty())
		return false;
	return a[a.length() - 1] == c;
}


bool suffixIs(string const & a, string const & suf)
{
	size_t const suflen = suf.length();
	size_t const alen = a.length();
	return suflen <= alen && a.compare(alen - suflen, suflen, suf) == 0;
}


bool suffixIs(docstring const & a, docstring const & suf)
{
	size_t const suflen = suf.length();
	size_t const alen = a.length();
	return suflen <= alen && a.compare(alen - suflen, suflen, suf) == 0;
}


bool containsOnly(string const & s, string const & cset)
{
	return s.find_first_not_of(cset) == string::npos;
}


// ale970405+lasgoutt-970425
// rewritten to use new string (Lgb)
string const token(string const & a, char delim, int n)
{
	if (a.empty())
		return string();

	size_t k = 0;
	size_t i = 0;

	// Find delimiter or end of string
	for (; n--;) {
		if ((i = a.find(delim, i)) == string::npos)
			break;
		else
			++i; // step delim
	}

	// i is now the n'th delim (or string::npos)
	if (i == string::npos)
		return string();

	k = a.find(delim, i);
	// k is now the n'th + 1 delim (or string::npos)

	return a.substr(i, k - i);
}


docstring const token(docstring const & a, char_type delim, int n)
{
	if (a.empty())
		return docstring();

	size_t k = 0;
	size_t i = 0;

	// Find delimiter or end of string
	for (; n--;) {
		if ((i = a.find(delim, i)) == docstring::npos)
			break;
		else
			++i; // step delim
	}

	// i is now the n'th delim (or string::npos)
	if (i == docstring::npos)
		return docstring();

	k = a.find(delim, i);
	// k is now the n'th + 1 delim (or string::npos)

	return a.substr(i, k - i);
}


// this could probably be faster and/or cleaner, but it seems to work (JMarc)
// rewritten to use new string (Lgb)
int tokenPos(string const & a, char delim, string const & tok)
{
	int i = 0;
	string str = a;
	string tmptok;

	while (!str.empty()) {
		str = split(str, tmptok, delim);
		if (tok == tmptok)
			return i;
		++i;
	}
	return -1;
}


// this could probably be faster and/or cleaner, but it seems to work (JMarc)
// rewritten to use new string (Lgb)
int tokenPos(docstring const & a, char_type delim, docstring const & tok)
{
	int i = 0;
	docstring str = a;
	docstring tmptok;

	while (!str.empty()) {
		str = split(str, tmptok, delim);
		if (tok == tmptok)
			return i;
		++i;
	}
	return -1;
}


namespace {

/// Substitute all \a oldchar with \a newchar
template<typename Ch> inline
basic_string<Ch> const subst_char(basic_string<Ch> const & a,
		Ch oldchar, Ch newchar)
{
	typedef basic_string<Ch> String;
	String tmp(a);
	typename String::iterator lit = tmp.begin();
	typename String::iterator end = tmp.end();
	for (; lit != end; ++lit)
		if ((*lit) == oldchar)
			(*lit) = newchar;
	return tmp;
}


/// Substitute all \a oldchar with \a newchar
docstring const subst_char(docstring const & a,
	docstring::value_type oldchar, docstring::value_type newchar)
{
	docstring tmp(a);
	docstring::iterator lit = tmp.begin();
	docstring::iterator end = tmp.end();
	for (; lit != end; ++lit)
		if ((*lit) == oldchar)
			(*lit) = newchar;
	return tmp;
}


/// substitutes all instances of \a oldstr with \a newstr
template<typename String> inline
String const subst_string(String const & a,
		String const & oldstr, String const & newstr)
{
	LASSERT(!oldstr.empty(), return a);
	String lstr = a;
	size_t i = 0;
	size_t const olen = oldstr.length();
	while ((i = lstr.find(oldstr, i)) != string::npos) {
		lstr.replace(i, olen, newstr);
		i += newstr.length(); // We need to be sure that we dont
		// use the same i over and over again.
	}
	return lstr;
}


docstring const subst_string(docstring const & a,
		docstring const & oldstr, docstring const & newstr)
{
	LASSERT(!oldstr.empty(), return a);
	docstring lstr = a;
	size_t i = 0;
	size_t const olen = oldstr.length();
	while ((i = lstr.find(oldstr, i)) != string::npos) {
		lstr.replace(i, olen, newstr);
		i += newstr.length(); // We need to be sure that we dont
		// use the same i over and over again.
	}
	return lstr;
}

}


string const subst(string const & a, char oldchar, char newchar)
{
	return subst_char(a, oldchar, newchar);
}


docstring const subst(docstring const & a,
		char_type oldchar, char_type newchar)
{
	return subst_char(a, oldchar, newchar);
}


string const subst(string const & a,
		string const & oldstr, string const & newstr)
{
	return subst_string(a, oldstr, newstr);
}


docstring const subst(docstring const & a,
		docstring const & oldstr, docstring const & newstr)
{
	return subst_string(a, oldstr, newstr);
}


int count_char(string const & str, char chr)
{
	int count = 0;
	string::const_iterator lit = str.begin();
	string::const_iterator end = str.end();
	for (; lit != end; ++lit)
		if ((*lit) == chr)
			count++;
	return count;
}


/// Count all occurences of char \a chr inside \a str
int count_char(docstring const & str, docstring::value_type chr)
{
	int count = 0;
	docstring::const_iterator lit = str.begin();
	docstring::const_iterator end = str.end();
	for (; lit != end; ++lit)
		if ((*lit) == chr)
			count++;
	return count;
}


docstring const trim(docstring const & a, char const * p)
{
	LASSERT(p, return a);

	if (a.empty() || !*p)
		return a;

	docstring s = from_ascii(p);
	size_t r = a.find_last_not_of(s);
	size_t l = a.find_first_not_of(s);

	// Is this the minimal test? (lgb)
	if (r == docstring::npos && l == docstring::npos)
		return docstring();

	return a.substr(l, r - l + 1);
}


string const trim(string const & a, char const * p)
{
	LASSERT(p, return a);

	if (a.empty() || !*p)
		return a;

	size_t r = a.find_last_not_of(p);
	size_t l = a.find_first_not_of(p);

	// Is this the minimal test? (lgb)
	if (r == string::npos && l == string::npos)
		return string();

	return a.substr(l, r - l + 1);
}


string const rtrim(string const & a, char const * p)
{
	LASSERT(p, return a);

	if (a.empty() || !*p)
		return a;

	size_t r = a.find_last_not_of(p);

	// Is this test really needed? (Lgb)
	if (r == string::npos)
		return string();

	return a.substr(0, r + 1);
}


docstring const rtrim(docstring const & a, char const * p)
{
	LASSERT(p, return a);

	if (a.empty() || !*p)
		return a;

	size_t r = a.find_last_not_of(from_ascii(p));

	// Is this test really needed? (Lgb)
	if (r == docstring::npos)
		return docstring();

	return a.substr(0, r + 1);
}


string const ltrim(string const & a, char const * p)
{
	LASSERT(p, return a);
	if (a.empty() || !*p)
		return a;
	size_t l = a.find_first_not_of(p);
	if (l == string::npos)
		return string();
	return a.substr(l, string::npos);
}


docstring const ltrim(docstring const & a, char const * p)
{
	LASSERT(p, return a);
	if (a.empty() || !*p)
		return a;
	size_t l = a.find_first_not_of(from_ascii(p));
	if (l == docstring::npos)
		return docstring();
	return a.substr(l, docstring::npos);
}

namespace {

template<typename String, typename Char> inline
String const doSplit(String const & a, String & piece, Char delim)
{
	String tmp;
	size_t i = a.find(delim);
	if (i == a.length() - 1) {
		piece = a.substr(0, i);
	} else if (i != String::npos) {
		piece = a.substr(0, i);
		tmp = a.substr(i + 1);
	} else if (i == 0) {
		piece.erase();
		tmp = a.substr(i + 1);
	} else {
		piece = a;
	}
	return tmp;
}

template<typename Char> inline
docstring const doSplit(docstring const & a, docstring & piece, Char delim)
{
	docstring tmp;
	size_t i = a.find(delim);
	if (i == a.length() - 1) {
		piece = a.substr(0, i);
	} else if (i != docstring::npos) {
		piece = a.substr(0, i);
		tmp = a.substr(i + 1);
	} else if (i == 0) {
		piece.erase();
		tmp = a.substr(i + 1);
	} else {
		piece = a;
	}
	return tmp;
}

} // anon


string const split(string const & a, string & piece, char delim)
{
	return doSplit(a, piece, delim);
}


docstring const split(docstring const & a, docstring & piece, char_type delim)
{
	return doSplit(a, piece, delim);
}


string const split(string const & a, char delim)
{
	string tmp;
	size_t i = a.find(delim);
	if (i != string::npos) // found delim
		tmp = a.substr(i + 1);
	return tmp;
}


// ale970521
string const rsplit(string const & a, string & piece, char delim)
{
	string tmp;
	size_t i = a.rfind(delim);
	if (i != string::npos) { // delimiter was found
		piece = a.substr(0, i);
		tmp = a.substr(i + 1);
	} else { // delimiter was not found
		piece.erase();
	}
	return tmp;
}


docstring const rsplit(docstring const & a, docstring & piece, char_type delim)
{
	docstring tmp;
	size_t i = a.rfind(delim);
	if (i != string::npos) { // delimiter was found
		piece = a.substr(0, i);
		tmp = a.substr(i + 1);
	} else { // delimiter was not found
		piece.erase();
	}
	return tmp;
}


docstring const rsplit(docstring const & a, char_type delim)
{
	docstring tmp;
	size_t i = a.rfind(delim);
	if (i != string::npos)
		tmp = a.substr(i + 1);
	return tmp;
}


docstring const escape(docstring const & lab)
{
	char_type hexdigit[16] = { '0', '1', '2', '3', '4', '5', '6', '7',
				   '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
	docstring enc;
	for (size_t i = 0; i < lab.length(); ++i) {
		char_type c = lab[i];
		if (c >= 128 || c == '=' || c == '%' || c == '#' || c == '$'
		    || c == '}' || c == '{' || c == ']' || c == '[' || c == '&') {
			// Although char_type is a 32 bit type we know that
			// UCS4 occupies only 21 bits, so we don't need to
			// encode bigger values. Test for 2^24 because we
			// can encode that with the 6 hex digits that are
			// needed for 21 bits anyway.
			LASSERT(c < (1 << 24), continue);
			enc += '=';
			enc += hexdigit[(c>>20) & 15];
			enc += hexdigit[(c>>16) & 15];
			enc += hexdigit[(c>>12) & 15];
			enc += hexdigit[(c>> 8) & 15];
			enc += hexdigit[(c>> 4) & 15];
			enc += hexdigit[ c      & 15];
		} else {
			enc += c;
		}
	}
	return enc;
}


namespace {

// this doesn't check whether str is empty, so do that first.
vector<docstring> wrapToVec(docstring const & str, int ind,
			    size_t const width)
{
	docstring s = trim(str);
	if (s.empty())
		return vector<docstring>();

	docstring indent;
	if (ind < 0) {
		indent.insert(0, -ind, ' ');
		ind = 0;
	} else if (ind > 0)
		s.insert(0, ind, ' ');

	vector<docstring> retval;
	while (s.size() > width) {
		// find the last space within the first 'width' chars
		size_t const i = s.find_last_of(' ', width - 1);
		if (i == docstring::npos || i <= size_t(ind)) {
			// no space found
			s = s.substr(0, width - 3) + "...";
			break;
		}
		retval.push_back(s.substr(0, i));
		s = indent + s.substr(i);
		ind = indent.size();
	}
	if (!s.empty())
		retval.push_back(s);
	return retval;
}

}


docstring wrap(docstring const & str, int const ind, size_t const width)
{
	docstring s = trim(str);
	if (s.empty())
		return docstring();

	vector<docstring> const svec = wrapToVec(str, ind, width);
	return getStringFromVector(svec, from_ascii("\n"));
}


docstring wrapParas(docstring const & str, int const indent,
		    size_t const width, size_t const maxlines)
{
	docstring const dots = from_ascii("...");
	if (str.empty())
		return docstring();

	vector<docstring> const pars = getVectorFromString(str, from_ascii("\n"), true);
	vector<docstring> retval;

	vector<docstring>::const_iterator it = pars.begin();
	vector<docstring>::const_iterator const en = pars.end();
	for (; it != en; ++it) {
		vector<docstring> tmp = wrapToVec(*it, indent, width);
		size_t const nlines = tmp.size();
		if (nlines == 0)
			continue;
		size_t const curlines = retval.size();
		if (maxlines > 0 && curlines + nlines > maxlines) {
			tmp.resize(maxlines - curlines);
			docstring last = tmp.back();
			size_t const lsize = last.size();
			if (lsize > width - 3) {
				size_t const i = last.find_last_of(' ', width - 3);
				if (i == docstring::npos || i <= size_t(indent))
					// no space found
					last = last.substr(0, lsize - 3) + dots;
				else
					last = last.substr(0, i) + dots;
			} else
				last += dots;
			tmp.pop_back();
			tmp.push_back(last);
		}
		retval.insert(retval.end(), tmp.begin(), tmp.end());
		if (maxlines > 0 && retval.size() >= maxlines)
			break;
	}
	return getStringFromVector(retval, from_ascii("\n"));
}


namespace {

template<typename String> vector<String> const
getVectorFromStringT(String const & str, String const & delim, bool keepempty)
{
// Lars would like this code to go, but for now his replacement (below)
// doesn't fullfil the same function. I have, therefore, reactivated the
// old code for now. Angus 11 Nov 2002.
#if 1
	vector<String> vec;
	if (str.empty())
		return vec;
	String keys = rtrim(str);
	while (true) {
		size_t const idx = keys.find(delim);
		if (idx == String::npos) {
			vec.push_back(ltrim(keys));
			break;
		}
		String const key = trim(keys.substr(0, idx));
		if (!key.empty() || keepempty)
			vec.push_back(key);
		size_t const start = idx + delim.size();
		keys = keys.substr(start);
	}
	return vec;
#else
	typedef boost::char_separator<typename String::value_type> Separator;
	typedef boost::tokenizer<Separator, typename String::const_iterator, String> Tokenizer;
	Separator sep(delim.c_str());
	Tokenizer tokens(str, sep);
	return vector<String>(tokens.begin(), tokens.end());
#endif
}


template<typename String> const String
	getStringFromVector(vector<String> const & vec, String const & delim)
{
	String str;
	typename vector<String>::const_iterator it = vec.begin();
	typename vector<String>::const_iterator en = vec.end();
	for (; it != en; ++it) {
		String item = trim(*it);
		if (item.empty())
			continue;
		if (!str.empty())
			str += delim;
		str += item;
	}
	return str;
}

} // namespace anon


vector<string> const getVectorFromString(string const & str,
					 string const & delim,
					 bool keepempty)
{
	return getVectorFromStringT<string>(str, delim, keepempty);
}


vector<docstring> const getVectorFromString(docstring const & str,
					    docstring const & delim,
					    bool keepempty)
{
	return getVectorFromStringT<docstring>(str, delim, keepempty);
}


string const getStringFromVector(vector<string> const & vec,
				 string const & delim)
{
	return getStringFromVector<string>(vec, delim);
}


docstring const getStringFromVector(vector<docstring> const & vec,
				    docstring const & delim)
{
	return getStringFromVector<docstring>(vec, delim);
}


int findToken(char const * const str[], string const & search_token)
{
	int i = 0;

	while (str[i][0] && str[i] != search_token)
		++i;
	if (!str[i][0])
		i = -1;
	return i;
}


template<>
docstring bformat(docstring const & fmt, int arg1)
{
	LATTEST(contains(fmt, from_ascii("%1$d")));
	docstring const str = subst(fmt, from_ascii("%1$d"), convert<docstring>(arg1));
	return subst(str, from_ascii("%%"), from_ascii("%"));
}


template<>
docstring bformat(docstring const & fmt, long arg1)
{
	LATTEST(contains(fmt, from_ascii("%1$d")));
	docstring const str = subst(fmt, from_ascii("%1$d"), convert<docstring>(arg1));
	return subst(str, from_ascii("%%"), from_ascii("%"));
}


template<>
docstring bformat(docstring const & fmt, unsigned int arg1)
{
	LATTEST(contains(fmt, from_ascii("%1$d")));
	docstring const str = subst(fmt, from_ascii("%1$d"), convert<docstring>(arg1));
	return subst(str, from_ascii("%%"), from_ascii("%"));
}


template<>
docstring bformat(docstring const & fmt, docstring arg1)
{
	LATTEST(contains(fmt, from_ascii("%1$s")));
	docstring const str = subst(fmt, from_ascii("%1$s"), arg1);
	return subst(str, from_ascii("%%"), from_ascii("%"));
}


template<>
docstring bformat(docstring const & fmt, char * arg1)
{
	LATTEST(contains(fmt, from_ascii("%1$s")));
	docstring const str = subst(fmt, from_ascii("%1$s"), from_ascii(arg1));
	return subst(str, from_ascii("%%"), from_ascii("%"));
}


template<>
docstring bformat(docstring const & fmt, docstring arg1, docstring arg2)
{
	LATTEST(contains(fmt, from_ascii("%1$s")));
	LATTEST(contains(fmt, from_ascii("%2$s")));
	docstring str = subst(fmt, from_ascii("%1$s"), arg1);
	str = subst(str, from_ascii("%2$s"), arg2);
	return subst(str, from_ascii("%%"), from_ascii("%"));
}


template<>
docstring bformat(docstring const & fmt, docstring arg1, int arg2)
{
	LATTEST(contains(fmt, from_ascii("%1$s")));
	LATTEST(contains(fmt, from_ascii("%2$d")));
	docstring str = subst(fmt, from_ascii("%1$s"), arg1);
	str = subst(str, from_ascii("%2$d"), convert<docstring>(arg2));
	return subst(str, from_ascii("%%"), from_ascii("%"));
}


template<>
docstring bformat(docstring const & fmt, char const * arg1, docstring arg2)
{
	LATTEST(contains(fmt, from_ascii("%1$s")));
	LATTEST(contains(fmt, from_ascii("%2$s")));
	docstring str = subst(fmt, from_ascii("%1$s"), from_ascii(arg1));
	str = subst(str, from_ascii("%2$s"), arg2);
	return subst(str, from_ascii("%%"), from_ascii("%"));
}


template<>
docstring bformat(docstring const & fmt, int arg1, int arg2)
{
	LATTEST(contains(fmt, from_ascii("%1$d")));
	LATTEST(contains(fmt, from_ascii("%2$d")));
	docstring str = subst(fmt, from_ascii("%1$d"), convert<docstring>(arg1));
	str = subst(str, from_ascii("%2$d"), convert<docstring>(arg2));
	return subst(str, from_ascii("%%"), from_ascii("%"));
}


template<>
docstring bformat(docstring const & fmt, docstring arg1, docstring arg2, docstring arg3)
{
	LATTEST(contains(fmt, from_ascii("%1$s")));
	LATTEST(contains(fmt, from_ascii("%2$s")));
	LATTEST(contains(fmt, from_ascii("%3$s")));
	docstring str = subst(fmt, from_ascii("%1$s"), arg1);
	str = subst(str, from_ascii("%2$s"), arg2);
	str = subst(str, from_ascii("%3$s"), arg3);
	return subst(str, from_ascii("%%"), from_ascii("%"));
}


template<>
docstring bformat(docstring const & fmt,
	       docstring arg1, docstring arg2, docstring arg3, docstring arg4)
{
	LATTEST(contains(fmt, from_ascii("%1$s")));
	LATTEST(contains(fmt, from_ascii("%2$s")));
	LATTEST(contains(fmt, from_ascii("%3$s")));
	LATTEST(contains(fmt, from_ascii("%4$s")));
	docstring str = subst(fmt, from_ascii("%1$s"), arg1);
	str = subst(str, from_ascii("%2$s"), arg2);
	str = subst(str, from_ascii("%3$s"), arg3);
	str = subst(str, from_ascii("%4$s"), arg4);
	return subst(str, from_ascii("%%"), from_ascii("%"));
}

} // namespace support
} // namespace lyx

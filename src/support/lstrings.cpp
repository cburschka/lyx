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
#include "support/textutils.h"

#include <boost/tokenizer.hpp>
#include "support/lassert.h"

#include <QString>
#include <QVector>

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

/**
 * Convert a QChar into a UCS4 character.
 * This is a hack (it does only make sense for the common part of the UCS4
 * and UTF16 encodings) and should not be used.
 * This does only exist because of performance reasons (a real conversion
 * using iconv is too slow on windows).
 */
static inline char_type qchar_to_ucs4(QChar const & qchar)
{
	LASSERT(is_utf16(static_cast<char_type>(qchar.unicode())), /**/);
	return static_cast<char_type>(qchar.unicode());
}


/**
 * Convert a UCS4 character into a QChar.
 * This is a hack (it does only make sense for the common part of the UCS4
 * and UTF16 encodings) and should not be used.
 * This does only exist because of performance reasons (a real conversion
 * using iconv is too slow on windows).
 */
static inline QChar const ucs4_to_qchar(char_type const ucs4)
{
	LASSERT(is_utf16(ucs4), /**/);
	return QChar(static_cast<unsigned short>(ucs4));
}


namespace {
	/// Maximum valid UCS4 code point
	char_type const ucs4_max = 0x10ffff;
}


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


bool isDigit(char_type c)
{
	if (!is_utf16(c))
		// assume that no non-utf16 character is a digit
		// c outside the UCS4 range is catched as well
		return false;
	return ucs4_to_qchar(c).isDigit();
}


bool isDigitASCII(char_type c)
{
	return '0' <= c && c <= '9';
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
		if (!isdigit((*cit)))
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
		if (!isdigit((*cit)))
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
		if (!isdigit(*cit) && *cit != '.')
			return false;
		if ('.' == (*cit)) {
			if (found_dot)
				return false;
			found_dot = true;
		}
	}
	return true;
}


static bool isHexChar(char_type c)
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
	LASSERT(static_cast<unsigned char>(c) < 0x80, /**/);
	return char(tolower(c));
}


char uppercase(char c)
{
	LASSERT(static_cast<unsigned char>(c) < 0x80, /**/);
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


namespace {

// since we cannot use tolower and toupper directly in the
// calls to transform yet, we use these helper clases. (Lgb)

struct local_lowercase {
	char_type operator()(char_type c) const {
		if (!is_utf16(c))
			// We don't know how to lowercase a non-utf16 char
			return c;
		return qchar_to_ucs4(ucs4_to_qchar(c).toLower());
	}
};

struct local_uppercase {
	char_type operator()(char_type c) const {
		if (!is_utf16(c))
			// We don't know how to uppercase a non-utf16 char
			return c;
		return qchar_to_ucs4(ucs4_to_qchar(c).toUpper());
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
	LASSERT(!oldstr.empty(), /**/);
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
	LASSERT(!oldstr.empty(), /**/);
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


docstring const trim(docstring const & a, char const * p)
{
	LASSERT(p, /**/);

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
	LASSERT(p, /**/);

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
	LASSERT(p, /**/);

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
	LASSERT(p, /**/);

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
	LASSERT(p, /**/);
	if (a.empty() || !*p)
		return a;
	size_t l = a.find_first_not_of(p);
	if (l == string::npos)
		return string();
	return a.substr(l, string::npos);
}


docstring const ltrim(docstring const & a, char const * p)
{
	LASSERT(p, /**/);
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


docstring const escape(docstring const & lab)
{
	char_type hexdigit[16] = { '0', '1', '2', '3', '4', '5', '6', '7',
				   '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
	docstring enc;
	for (size_t i = 0; i < lab.length(); ++i) {
		char_type c = lab[i];
		if (c >= 128 || c == '=' || c == '%') {
			// Although char_type is a 32 bit type we know that
			// UCS4 occupies only 21 bits, so we don't need to
			// encode bigger values. Test for 2^24 because we
			// can encode that with the 6 hex digits that are
			// needed for 21 bits anyway.
			LASSERT(c < (1 << 24), /**/);
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


// the same vice versa
string const getStringFromVector(vector<string> const & vec,
				 string const & delim)
{
	string str;
	int i = 0;
	for (vector<string>::const_iterator it = vec.begin();
	     it != vec.end(); ++it) {
		string item = trim(*it);
		if (item.empty())
			continue;
		if (i++ > 0)
			str += delim;
		str += item;
	}
	return str;
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
	LASSERT(contains(fmt, from_ascii("%1$d")), /**/);
	docstring const str = subst(fmt, from_ascii("%1$d"), convert<docstring>(arg1));
	return subst(str, from_ascii("%%"), from_ascii("%"));
}


template<>
docstring bformat(docstring const & fmt, long arg1)
{
	LASSERT(contains(fmt, from_ascii("%1$d")), /**/);
	docstring const str = subst(fmt, from_ascii("%1$d"), convert<docstring>(arg1));
	return subst(str, from_ascii("%%"), from_ascii("%"));
}


template<>
docstring bformat(docstring const & fmt, unsigned int arg1)
{
	LASSERT(contains(fmt, from_ascii("%1$d")), /**/);
	docstring const str = subst(fmt, from_ascii("%1$d"), convert<docstring>(arg1));
	return subst(str, from_ascii("%%"), from_ascii("%"));
}


template<>
docstring bformat(docstring const & fmt, docstring arg1)
{
	LASSERT(contains(fmt, from_ascii("%1$s")), /**/);
	docstring const str = subst(fmt, from_ascii("%1$s"), arg1);
	return subst(str, from_ascii("%%"), from_ascii("%"));
}


template<>
docstring bformat(docstring const & fmt, char * arg1)
{
	LASSERT(contains(fmt, from_ascii("%1$s")), /**/);
	docstring const str = subst(fmt, from_ascii("%1$s"), from_ascii(arg1));
	return subst(str, from_ascii("%%"), from_ascii("%"));
}


template<>
docstring bformat(docstring const & fmt, docstring arg1, docstring arg2)
{
	LASSERT(contains(fmt, from_ascii("%1$s")), /**/);
	LASSERT(contains(fmt, from_ascii("%2$s")), /**/);
	docstring str = subst(fmt, from_ascii("%1$s"), arg1);
	str = subst(str, from_ascii("%2$s"), arg2);
	return subst(str, from_ascii("%%"), from_ascii("%"));
}


template<>
docstring bformat(docstring const & fmt, char const * arg1, docstring arg2)
{
	LASSERT(contains(fmt, from_ascii("%1$s")), /**/);
	LASSERT(contains(fmt, from_ascii("%2$s")), /**/);
	docstring str = subst(fmt, from_ascii("%1$s"), from_ascii(arg1));
	str = subst(fmt, from_ascii("%2$s"), arg2);
	return subst(str, from_ascii("%%"), from_ascii("%"));
}


template<>
docstring bformat(docstring const & fmt, int arg1, int arg2)
{
	LASSERT(contains(fmt, from_ascii("%1$d")), /**/);
	LASSERT(contains(fmt, from_ascii("%2$d")), /**/);
	docstring str = subst(fmt, from_ascii("%1$d"), convert<docstring>(arg1));
	str = subst(str, from_ascii("%2$d"), convert<docstring>(arg2));
	return subst(str, from_ascii("%%"), from_ascii("%"));
}


template<>
docstring bformat(docstring const & fmt, docstring arg1, docstring arg2, docstring arg3)
{
	LASSERT(contains(fmt, from_ascii("%1$s")), /**/);
	LASSERT(contains(fmt, from_ascii("%2$s")), /**/);
	LASSERT(contains(fmt, from_ascii("%3$s")), /**/);
	docstring str = subst(fmt, from_ascii("%1$s"), arg1);
	str = subst(str, from_ascii("%2$s"), arg2);
	str = subst(str, from_ascii("%3$s"), arg3);
	return subst(str, from_ascii("%%"), from_ascii("%"));
}


template<>
docstring bformat(docstring const & fmt,
	       docstring arg1, docstring arg2, docstring arg3, docstring arg4)
{
	LASSERT(contains(fmt, from_ascii("%1$s")), /**/);
	LASSERT(contains(fmt, from_ascii("%2$s")), /**/);
	LASSERT(contains(fmt, from_ascii("%3$s")), /**/);
	LASSERT(contains(fmt, from_ascii("%4$s")), /**/);
	docstring str = subst(fmt, from_ascii("%1$s"), arg1);
	str = subst(str, from_ascii("%2$s"), arg2);
	str = subst(str, from_ascii("%3$s"), arg3);
	str = subst(str, from_ascii("%4$s"), arg4);
	return subst(str, from_ascii("%%"), from_ascii("%"));
}

} // namespace support
} // namespace lyx

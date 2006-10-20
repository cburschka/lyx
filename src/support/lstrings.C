/**
 * \file lstrings.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Jean-Marc Lasgouttes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "support/lstrings.h"
#include "support/lyxlib.h"
#include "support/convert.h"

#include "debug.h"

#include <boost/tokenizer.hpp>
#include <boost/assert.hpp>

#ifndef I_AM_NOT_AFRAID_OF_HEADER_LIBRARIES
#if USE_BOOST_FORMAT
#include <boost/format.hpp>
#endif
#endif

#include <cctype>
#include <cstdlib>

#include <algorithm>
#include <sstream>

using lyx::docstring;

using std::transform;
using std::string;
using std::vector;

#ifndef CXX_GLOBAL_CSTD
using std::isdigit;
using std::tolower;
using std::toupper;
#endif


namespace lyx {
namespace support {

int compare_no_case(string const & s, string const & s2)
{
	string::const_iterator p = s.begin();
	string::const_iterator p2 = s2.begin();

	while (p != s.end() && p2 != s2.end()) {
		int const lc1 = tolower(*p);
		int const lc2 = tolower(*p2);
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


int compare_no_case(docstring const & s, docstring const & s2)
{
	docstring::const_iterator p = s.begin();
	docstring::const_iterator p2 = s2.begin();

	while (p != s.end() && p2 != s2.end()) {
		int const lc1 = tolower(*p);
		int const lc2 = tolower(*p2);
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

int ascii_tolower(int c) {
	if (c >= 'A' && c <= 'Z')
		return c - 'A' + 'a';
	return c;
}


template<typename String> inline
int do_compare_ascii_no_case(String const & s, String const & s2)
{
	typename String::const_iterator p = s.begin();
	typename String::const_iterator p2 = s2.begin();

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

}


int compare_ascii_no_case(string const & s, string const & s2)
{
	return do_compare_ascii_no_case(s, s2);
}


int compare_ascii_no_case(docstring const & s, docstring const & s2)
{
	return do_compare_ascii_no_case(s, s2);
}


int compare_no_case(string const & s, string const & s2, unsigned int len)
{
	string::const_iterator p = s.begin();
	string::const_iterator p2 = s2.begin();
	unsigned int i = 0;
	while (i < len && p != s.end() && p2 != s2.end()) {
		int const lc1 = tolower(*p);
		int const lc2 = tolower(*p2);
		if (lc1 != lc2)
			return (lc1 < lc2) ? -1 : 1;
		++i;
		++p;
		++p2;
	}

	if (s.size() >= len && s2.size() >= len)
		return 0;
	if (s.size() < s2.size())
		return -1;
	return 1;
}


bool isStrInt(string const & str)
{
	if (str.empty()) return false;

	// Remove leading and trailing white space chars.
	string const tmpstr = trim(str);
	if (tmpstr.empty()) return false;

	string::const_iterator cit = tmpstr.begin();
	if ((*cit) == '-') ++cit;
	string::const_iterator end = tmpstr.end();
	for (; cit != end; ++cit) {
		if (!isdigit((*cit))) return false;
	}
	return true;
}


bool isStrUnsignedInt(string const & str)
{
	if (str.empty()) return false;

	// Remove leading and trailing white space chars.
	string const tmpstr = trim(str);
	if (tmpstr.empty()) return false;

	string::const_iterator cit = tmpstr.begin();
	string::const_iterator end = tmpstr.end();
	for (; cit != end; ++cit) {
		if (!isdigit((*cit))) return false;
	}
	return true;
}


bool isStrDbl(string const & str)
{
	if (str.empty()) return false;

	// Remove leading and trailing white space chars.
	string const tmpstr = trim(str);
	if (tmpstr.empty()) return false;
	//	if (1 < tmpstr.count('.')) return false;

	string::const_iterator cit = tmpstr.begin();
	bool found_dot(false);
	if ((*cit) == '-') ++cit;
	string::const_iterator end = tmpstr.end();
	for (; cit != end; ++cit) {
		if (!isdigit((*cit))
		    && '.' != (*cit)) {
			return false;
		}
		if ('.' == (*cit)) {
			if (found_dot) {
				return false;
			} else {
				found_dot = true;
			}
		}
	}
	return true;
}


char lowercase(char c)
{
	return char(tolower(c));
}


char uppercase(char c)
{
	return char(toupper(c));
}

// FIXME for lowercase() and uppercase() function below:
// 1) std::tolower() and std::toupper() are templates that
// compile fine with char_type. With the test (c >= 256) we
// do not trust these function to do the right thing with
// unicode char.
// 2) these functions use the current locale, which is wrong
// if it is not latin1 based (latin1 is a subset of UCS4).

char_type lowercase(char_type c)
{
	if (c >= 256)
		return c;

	return tolower(c);
}


char_type uppercase(char_type c)
{
	if (c >= 256)
		return c;

	return toupper(c);
}


namespace {

// since we cannot use std::tolower and std::toupper directly in the
// calls to std::transform yet, we use these helper clases. (Lgb)

struct local_lowercase {
	char operator()(char c) const {
		return tolower(c);
	}
};

struct local_uppercase {
	char operator()(char c) const {
		return toupper(c);
	}
};

struct local_ascii_lowercase {
	char operator()(char c) const {
		return ascii_tolower(c);
	}
};

} // end of anon namespace

string const lowercase(string const & a)
{
	string tmp(a);
	transform(tmp.begin(), tmp.end(), tmp.begin(), local_lowercase());
	return tmp;
}

string const uppercase(string const & a)
{
	string tmp(a);
	transform(tmp.begin(), tmp.end(), tmp.begin(), local_uppercase());
	return tmp;
}


string const ascii_lowercase(string const & a)
{
	string tmp(a);
	transform(tmp.begin(), tmp.end(), tmp.begin(),
		  local_ascii_lowercase());
	return tmp;
}


bool prefixIs(string const & a, string const & pre)
{
	string::size_type const prelen = pre.length();
	string::size_type const alen = a.length();

	if (prelen > alen || a.empty())
		return false;
	else {
#if defined(STD_STRING_IS_GOOD)
		return a.compare(0, prelen, pre) == 0;
#else
		return ::strncmp(a.c_str(), pre.c_str(), prelen) == 0;
#endif
	}
}


bool prefixIs(docstring const & a, docstring const & pre)
{
	docstring::size_type const prelen = pre.length();
	docstring::size_type const alen = a.length();

	if (prelen > alen || a.empty())
		return false;
	else
		return a.compare(0, prelen, pre) == 0;
}


bool suffixIs(string const & a, char c)
{
	if (a.empty()) return false;
	return a[a.length() - 1] == c;
}


bool suffixIs(string const & a, string const & suf)
{
	string::size_type const suflen = suf.length();
	string::size_type const alen = a.length();

	if (suflen > alen) {
		return false;
	} else {
#if !defined(USE_INCLUDED_STRING) && !defined(STD_STRING_IS_GOOD)
		string tmp(a, alen - suflen);
		return ::strncmp(tmp.c_str(), suf.c_str(), suflen) == 0;
#else
		return a.compare(alen - suflen, suflen, suf) == 0;
#endif
	}
}


bool containsOnly(string const & s, string const & cset)
{
	return s.find_first_not_of(cset) == string::npos;
}


// ale970405+lasgoutt-970425
// rewritten to use new string (Lgb)
string const token(string const & a, char delim, int n)
{
	if (a.empty()) return string();

	string::size_type k = 0;
	string::size_type i = 0;

	// Find delimiter or end of string
	for (; n--;)
		if ((i = a.find(delim, i)) == string::npos)
			break;
		else
			++i; // step delim
	// i is now the n'th delim (or string::npos)
	if (i == string::npos) return string();
	k = a.find(delim, i);
	// k is now the n'th + 1 delim (or string::npos)

	return a.substr(i, k - i);
}


docstring const token(docstring const & a, char_type delim, int n)
{
	if (a.empty()) return docstring();

	string::size_type k = 0;
	string::size_type i = 0;

	// Find delimiter or end of string
	for (; n--;)
		if ((i = a.find(delim, i)) == docstring::npos)
			break;
		else
			++i; // step delim
	// i is now the n'th delim (or string::npos)
	if (i == docstring::npos) return docstring();
	k = a.find(delim, i);
	// k is now the n'th + 1 delim (or string::npos)

	return a.substr(i, k - i);
}


// this could probably be faster and/or cleaner, but it seems to work (JMarc)
// rewritten to use new string (Lgb)
int tokenPos(string const & a, char delim, string const & tok)
{
	int i = 0;
	string str(a);
	string tmptok;

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
std::basic_string<Ch> const subst_char(std::basic_string<Ch> const & a,
		Ch oldchar, Ch newchar)
{
	typedef std::basic_string<Ch> String;
	String tmp(a);
	typename String::iterator lit = tmp.begin();
	typename String::iterator end = tmp.end();
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
	BOOST_ASSERT(!oldstr.empty());
	String lstr = a;
	typename String::size_type i = 0;
	typename String::size_type const olen = oldstr.length();
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
	BOOST_ASSERT(p);

	if (a.empty() || !*p)
		return a;

	docstring s = lyx::from_ascii(p);
	docstring::size_type r = a.find_last_not_of(s);
	docstring::size_type l = a.find_first_not_of(s);

	// Is this the minimal test? (lgb)
	if (r == docstring::npos && l == docstring::npos)
		return docstring();

	return a.substr(l, r - l + 1);
}


string const trim(string const & a, char const * p)
{
	BOOST_ASSERT(p);

	if (a.empty() || !*p)
		return a;

	string::size_type r = a.find_last_not_of(p);
	string::size_type l = a.find_first_not_of(p);

	// Is this the minimal test? (lgb)
	if (r == string::npos && l == string::npos)
		return string();

	return a.substr(l, r - l + 1);
}


string const rtrim(string const & a, char const * p)
{
	BOOST_ASSERT(p);

	if (a.empty() || !*p)
		return a;

	string::size_type r = a.find_last_not_of(p);

	// Is this test really needed? (Lgb)
	if (r == string::npos)
		return string();

	return a.substr(0, r + 1);
}


string const ltrim(string const & a, char const * p)
{
	BOOST_ASSERT(p);

	if (a.empty() || !*p)
		return a;

	string::size_type l = a.find_first_not_of(p);

	if (l == string::npos)
		return string();

	return a.substr(l, string::npos);
}


namespace {

template<typename String, typename Char> inline
String const doSplit(String const & a, String & piece, Char delim)
{
	String tmp;
	typename String::size_type i = a.find(delim);
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

}


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
	string::size_type i = a.find(delim);
	if (i != string::npos) // found delim
		tmp = a.substr(i + 1);
	return tmp;
}


// ale970521
string const rsplit(string const & a, string & piece, char delim)
{
	string tmp;
	string::size_type i = a.rfind(delim);
	if (i != string::npos) { // delimiter was found
		piece = a.substr(0, i);
		tmp = a.substr(i + 1);
	} else { // delimiter was not found
		piece.erase();
	}
	return tmp;
}


// This function escapes 8-bit characters and other problematic
// characters that cause problems in latex labels.
docstring const escape(docstring const & lab)
{
	lyx::char_type hexdigit[16] = { '0', '1', '2', '3', '4', '5', '6', '7',
			      '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
	docstring enc;
	for (docstring::size_type i = 0; i < lab.length(); ++i) {
		lyx::char_type c= lab[i];
		// FIXME We must change the following algorithm for UCS4
		// chars, but that will be a file format change.
		if (c >= 128 || c == '=' || c == '%') {
			enc += '=';
			enc += hexdigit[c>>4];
			enc += hexdigit[c & 15];
		} else {
			enc += c;
		}
	}
	return enc;
}


/// gives a vector of stringparts which have the delimiter delim
vector<string> const getVectorFromString(string const & str,
					 string const & delim)
{
// Lars would like this code to go, but for now his replacement (below)
// doesn't fullfil the same function. I have, therefore, reactivated the
// old code for now. Angus 11 Nov 2002.
#if 1
	vector<string> vec;
	if (str.empty())
		return vec;
	string keys = rtrim(str);
	for(;;) {
		string::size_type const idx = keys.find(delim);
		if (idx == string::npos) {
			vec.push_back(ltrim(keys));
			break;
		}
		string const key = trim(keys.substr(0, idx));
		if (!key.empty())
			vec.push_back(key);
		string::size_type const start = idx + delim.size();
		keys = keys.substr(start);
	}
	return vec;
#else
	boost::char_separator<char> sep(delim.c_str());
	boost::tokenizer<boost::char_separator<char> > tokens(str, sep);
	return vector<string>(tokens.begin(), tokens.end());
#endif
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


docstring const externalLineEnding(docstring const & str)
{
#if defined(__APPLE__)
	// The MAC clipboard uses \r for lineendings, and we use \n
	return subst(str, '\n', '\r');
#elif defined (_WIN32) || (defined (__CYGWIN__) && defined (X_DISPLAY_MISSING))
	// Windows clipboard uses \r\n for lineendings, and we use \n
	return subst(str, lyx::from_ascii("\n"), lyx::from_ascii("\r\n"));
#else
	return str;
#endif
}


docstring const internalLineEnding(docstring const & str)
{
	docstring const s = subst(str,
			lyx::from_ascii("\r\n"), lyx::from_ascii("\n"));
	return subst(s, '\r', '\n');
}


#ifndef I_AM_NOT_AFRAID_OF_HEADER_LIBRARIES
#if USE_BOOST_FORMAT

template<>
docstring bformat(docstring const & fmt, int arg1)
{
	return (boost::basic_format<char_type>(fmt) % arg1).str();
}


template<>
docstring bformat(docstring const & fmt, long arg1)
{
	return (boost::basic_format<char_type>(fmt) % arg1).str();
}


template<>
docstring bformat(docstring const & fmt, unsigned int arg1)
{
	return (boost::basic_format<char_type>(fmt) % arg1).str();
}


template<>
docstring bformat<docstring>(docstring const & fmt, docstring arg1)
{
	return (boost::basic_format<char_type>(fmt) % arg1).str();
}


template<>
docstring bformat(docstring const & fmt, char * arg1)
{
	return (boost::basic_format<char_type>(fmt) % arg1).str();
}


template<>
docstring bformat(docstring const & fmt, int arg1, int arg2)
{
	return (boost::basic_format<char_type>(fmt) % arg1 % arg2).str();
}


template<>
docstring bformat(docstring const & fmt, docstring arg1, docstring arg2)
{
	return (boost::basic_format<char_type>(fmt) % arg1 % arg2).str();
}


template<>
docstring bformat(docstring const & fmt, char const * arg1, docstring arg2)
{
	return (boost::basic_format<char_type>(fmt) % arg1 % arg2).str();
}


template<>
docstring bformat(docstring const & fmt, docstring arg1, docstring arg2, docstring arg3)
{
	return (boost::basic_format<char_type>(fmt) % arg1 % arg2 % arg3).str();
}


template<>
docstring bformat(docstring const & fmt,
	       docstring arg1, docstring arg2, docstring arg3, docstring arg4)
{
	return (boost::basic_format<char_type>(fmt) % arg1 % arg2 % arg3 % arg4).str();
}

#else

template<>
docstring bformat(docstring const & fmt, int arg1)
{
	BOOST_ASSERT(contains(fmt, lyx::from_ascii("%1$d")));
	docstring const str = subst(fmt, lyx::from_ascii("%1$d"), convert<docstring>(arg1));
	return subst(str, lyx::from_ascii("%%"), lyx::from_ascii("%"));
}


template<>
docstring bformat(docstring const & fmt, long arg1)
{
	BOOST_ASSERT(contains(fmt, lyx::from_ascii("%1$d")));
	docstring const str = subst(fmt, lyx::from_ascii("%1$d"), convert<docstring>(arg1));
	return subst(str, lyx::from_ascii("%%"), lyx::from_ascii("%"));
}


template<>
docstring bformat(docstring const & fmt, unsigned int arg1)
{
	BOOST_ASSERT(contains(fmt, lyx::from_ascii("%1$d")));
	docstring const str = subst(fmt, lyx::from_ascii("%1$d"), convert<docstring>(arg1));
	return subst(str, lyx::from_ascii("%%"), lyx::from_ascii("%"));
}


template<>
docstring bformat(docstring const & fmt, docstring arg1)
{
	BOOST_ASSERT(contains(fmt, lyx::from_ascii("%1$s")));
	docstring const str = subst(fmt, lyx::from_ascii("%1$s"), arg1);
	return subst(str, lyx::from_ascii("%%"), lyx::from_ascii("%"));
}


template<>
docstring bformat(docstring const & fmt, char * arg1)
{
	BOOST_ASSERT(contains(fmt, lyx::from_ascii("%1$s")));
	docstring const str = subst(fmt, lyx::from_ascii("%1$s"), lyx::from_ascii(arg1));
	return subst(str, lyx::from_ascii("%%"), lyx::from_ascii("%"));
}


template<>
docstring bformat(docstring const & fmt, docstring arg1, docstring arg2)
{
	BOOST_ASSERT(contains(fmt, lyx::from_ascii("%1$s")));
	BOOST_ASSERT(contains(fmt, lyx::from_ascii("%2$s")));
	docstring str = subst(fmt, lyx::from_ascii("%1$s"), arg1);
	str = subst(str, lyx::from_ascii("%2$s"), arg2);
	return subst(str, lyx::from_ascii("%%"), lyx::from_ascii("%"));
}


template<>
docstring bformat(docstring const & fmt, char const * arg1, docstring arg2)
{
	BOOST_ASSERT(contains(fmt, lyx::from_ascii("%1$s")));
	BOOST_ASSERT(contains(fmt, lyx::from_ascii("%2$s")));
	docstring str = subst(fmt, lyx::from_ascii("%1$s"), lyx::from_ascii(arg1));
	str = subst(fmt, lyx::from_ascii("%2$s"), arg2);
	return subst(str, lyx::from_ascii("%%"), lyx::from_ascii("%"));
}


template<>
docstring bformat(docstring const & fmt, int arg1, int arg2)
{
	BOOST_ASSERT(contains(fmt, lyx::from_ascii("%1$d")));
	BOOST_ASSERT(contains(fmt, lyx::from_ascii("%2$d")));
	docstring str = subst(fmt, lyx::from_ascii("%1$d"), convert<docstring>(arg1));
	str = subst(str, lyx::from_ascii("%2$d"), convert<docstring>(arg2));
	return subst(str, lyx::from_ascii("%%"), lyx::from_ascii("%"));
}


template<>
docstring bformat(docstring const & fmt, docstring arg1, docstring arg2, docstring arg3)
{
	BOOST_ASSERT(contains(fmt, lyx::from_ascii("%1$s")));
	BOOST_ASSERT(contains(fmt, lyx::from_ascii("%2$s")));
	BOOST_ASSERT(contains(fmt, lyx::from_ascii("%3$s")));
	docstring str = subst(fmt, lyx::from_ascii("%1$s"), arg1);
	str = subst(str, lyx::from_ascii("%2$s"), arg2);
	str = subst(str, lyx::from_ascii("%3$s"), arg3);
	return subst(str, lyx::from_ascii("%%"), lyx::from_ascii("%"));
}


template<>
docstring bformat(docstring const & fmt,
	       docstring arg1, docstring arg2, docstring arg3, docstring arg4)
{
	BOOST_ASSERT(contains(fmt, lyx::from_ascii("%1$s")));
	BOOST_ASSERT(contains(fmt, lyx::from_ascii("%2$s")));
	BOOST_ASSERT(contains(fmt, lyx::from_ascii("%3$s")));
	BOOST_ASSERT(contains(fmt, lyx::from_ascii("%4$s")));
	docstring str = subst(fmt, lyx::from_ascii("%1$s"), arg1);
	str = subst(str, lyx::from_ascii("%2$s"), arg2);
	str = subst(str, lyx::from_ascii("%3$s"), arg3);
	str = subst(str, lyx::from_ascii("%4$s"), arg4);
	return subst(str, lyx::from_ascii("%%"), lyx::from_ascii("%"));
}

#endif
#endif

} // namespace support
} // namespace lyx

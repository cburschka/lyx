/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 *        
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2001 The LyX Team.
 *
 * ====================================================== */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include <algorithm>

#include <cctype>
#include <cstdlib>

#include "LString.h"
#include "lstrings.h"
#include "LRegex.h"
#include "LAssert.h"
#include "debug.h"

using std::count;
using std::transform;

#ifndef CXX_GLOBAL_CSTD
using std::tolower;
using std::toupper;
using std::strlen;
#endif


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


namespace {
	int ascii_tolower(int c) {
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
	string const tmpstr = frontStrip(strip(str, ' '), ' ');
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
	string const tmpstr = frontStrip(strip(str, ' '), ' ');
	if (tmpstr.empty()) return false;
       
	string::const_iterator cit = tmpstr.begin();
	string::const_iterator end = tmpstr.end();
	for (; cit != end; ++cit) {
		if (!isdigit((*cit))) return false;
	}
	return true;
}


int strToInt(string const & str)
{
	if (isStrInt(str)) {
		// Remove leading and trailing white space chars.
		string const tmpstr = frontStrip(strip(str, ' '), ' ');
		// Do the conversion proper.
		return lyx::atoi(tmpstr);
	} else {
		return 0;
	}
}


unsigned int strToUnsignedInt(string const & str)
{
	if (isStrUnsignedInt(str)) {
		// Remove leading and trailing white space chars.
		string const tmpstr = frontStrip(strip(str, ' '), ' ');
		// Do the conversion proper.
		return lyx::atoi(tmpstr);
	} else {
		return 0;
	}
}


bool isStrDbl(string const & str)
{
	if (str.empty()) return false;
	
	// Remove leading and trailing white space chars.
	string const tmpstr = frontStrip(strip(str, ' '), ' ');
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


double strToDbl(string const & str)
{
	if (isStrDbl(str)) {
		// Remove leading and trailing white space chars.
		string const tmpstr = frontStrip(strip(str, ' '), ' ');
		// Do the conversion proper.
		return ::atof(tmpstr.c_str());
	} else {
		return 0.0;
	}
}


char lowercase(char c) 
{ 
	return char(tolower(c)); 
}


char uppercase(char c) 
{ 
	return char(toupper(c)); 
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


bool prefixIs(string const & a, char const * pre)
{
	lyx::Assert(pre);
	
	size_t const l = strlen(pre);
	string::size_type const alen = a.length();
	
	if (l > alen || a.empty())
		return false;
	else {
#if !defined(USE_INCLUDED_STRING) && !defined(STD_STRING_IS_GOOD)
		// Delete this code when the compilers get a bit better.
		return ::strncmp(a.c_str(), pre, l) == 0;
#else
		// This is the code that we really want to use
		// but until gcc ships with a basic_string that
		// implements std::string correctly we have to
		// use the code above.
		return a.compare(0, l, pre, l) == 0;
#endif
	}
}


bool prefixIs(string const & a, string const & pre)
{
	string::size_type const prelen = pre.length();
	string::size_type const alen = a.length();
	
	if (prelen > alen || a.empty())
		return false;
	else {
#if !defined(USE_INCLUDED_STRING) && !defined(STD_STRING_IS_GOOD)
		return ::strncmp(a.c_str(), pre.c_str(), prelen) == 0;
#else
		return a.compare(0, prelen, pre) == 0;
#endif
	}
}


bool suffixIs(string const & a, char c)
{
	if (a.empty()) return false;
	return a[a.length() - 1] == c;
}


bool suffixIs(string const & a, char const * suf)
{
	lyx::Assert(suf);
	
	size_t const suflen = strlen(suf);
	string::size_type const alen = a.length();
	
	if (suflen > alen)
		return false;
	else {
#if !defined(USE_INCLUDED_STRING) && !defined(STD_STRING_IS_GOOD)
		// Delete this code when the compilers get a bit better.
		string tmp(a, alen - suflen);
		return ::strncmp(tmp.c_str(), suf, suflen) == 0;
#else
		// This is the code that we really want to use
		// but until gcc ships with a basic_string that
		// implements std::string correctly we have to
		// use the code above.
		return a.compare(alen - suflen, suflen, suf) == 0;
#endif
	}
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


bool contains(char const * a, string const & b)
{
	lyx::Assert(a);
	string const at(a);
	return contains(at, b);
}


bool contains(string const & a, char const * b)
{
	lyx::Assert(b);
	string const bt(b);
	return contains(a, bt);
}


bool contains(string const & a, string const & b)
{
	if (a.empty())
		return false;
	return a.find(b) != string::npos;
}


bool contains(string const & a, char b)
{
	if (a.empty())
		return false;
	return a.find(b) != string::npos;
}


bool contains(char const * a, char const * b)
{
	lyx::Assert(a && b);
	string const at(a);
	string const bt(b);
	return contains(at, bt);
}


bool containsOnly(string const & s, char const * cset)
{
	lyx::Assert(cset);
	
	return s.find_first_not_of(cset) == string::npos;
}


bool containsOnly(string const & s, string const & cset)
{
	return s.find_first_not_of(cset) == string::npos;
}


bool containsOnly(char const * s, char const * cset)
{
	lyx::Assert(s && cset);
	
	return string(s).find_first_not_of(cset) == string::npos;
}


bool containsOnly(char const * s, string const & cset)
{
	lyx::Assert(s);
	
	return string(s).find_first_not_of(cset) == string::npos;
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


bool regexMatch(string const & a, string const & pattern)
{
	// We massage the pattern a bit so that the usual
	// shell pattern we all are used to will work.
	// One nice thing about using a real regex is that
	// things like "*.*[^~]" will work also.
	// build the regex string.
	string regex(pattern);
	regex = subst(regex, ".", "\\.");
	regex = subst(regex, "*", ".*");
	LRegex reg(regex);
	return reg.exact_match(a);
}


string const subst(string const & a, char oldchar, char newchar)
{
	string tmp(a);
	string::iterator lit = tmp.begin();
	string::iterator end = tmp.end();
	for (; lit != end; ++lit)
		if ((*lit) == oldchar)
			(*lit) = newchar;
	return tmp;
}


string const subst(string const & a,
		   char const * oldstr, string const & newstr)
{
	lyx::Assert(oldstr);
	
	string lstr(a);
	string::size_type i = 0;
	string::size_type olen = strlen(oldstr);
	while ((i = lstr.find(oldstr, i)) != string::npos) {
		lstr.replace(i, olen, newstr);
		i += newstr.length(); // We need to be sure that we dont
		// use the same i over and over again.
	}
	return lstr;
}


string const subst(string const & a,
		   string const & oldstr, string const & newstr)
{
	string lstr(a);
	string::size_type i = 0;
	string::size_type const olen = oldstr.length();
	while ((i = lstr.find(oldstr, i)) != string::npos) {
		lstr.replace(i, olen, newstr);
		i += newstr.length(); // We need to be sure that we dont
		// use the same i over and over again.
	}
	return lstr;
}


string const strip(string const & a, char c)
{
	if (a.empty()) return a;
	string tmp(a);
	string::size_type i = tmp.find_last_not_of(c);
	if (i == a.length() - 1) return tmp; // no c's at end of a
	if (i != string::npos) 
		tmp.erase(i + 1, string::npos);
#if !defined(USE_INCLUDED_STRING) && !defined(STD_STRING_IS_GOOD)
	/// Needed for broken string::find_last_not_of
	else if (tmp[0] != c) {
		if (a.length() == 1) return tmp;
		tmp.erase(1, string::npos);
	}
#endif
	else
		tmp.erase(); // only c in the whole string
	return tmp;
}


string const frontStrip(string const & a, char const * p)
{
	lyx::Assert(p);
	
	if (a.empty() || !*p) return a;
	string tmp(a);
	string::size_type i = tmp.find_first_not_of(p);
	if (i > 0)
		tmp.erase(0, i);
	return tmp;
}


string const frontStrip(string const & a, char c)
{
	if (a.empty()) return a;
	string tmp(a);
	string::size_type i = tmp.find_first_not_of(c);
	if (i > 0)
		tmp.erase(0, i);
	return tmp;
}


string const split(string const & a, string & piece, char delim)
{
	string tmp;
	string::size_type i = a.find(delim);
	if (i == a.length() - 1) {
		piece = a.substr(0, i);
	} else if (i != string::npos) {
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
	} else { // delimter was not found
		piece.erase();
	}
	return tmp;
}


// This function escapes 8-bit characters and other problematic
// characters that cause problems in latex labels.
string const escape(string const & lab)
{
	char hexdigit[16] = { '0', '1', '2', '3', '4', '5', '6', '7',
			      '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
	string enc;
	for (string::size_type i = 0; i < lab.length(); ++i) {
		unsigned char c= lab[i];
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

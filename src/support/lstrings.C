/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 *        
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2000 The LyX Team.
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

using std::count;
using std::transform;
#ifndef __GLIBCPP__
// The new glibstdc++-v3 has not worked out all the quirks regarding cctype
// yet. So currently it failes if the to using lines below are stated.
using std::tolower;
using std::toupper;
#endif
	
int compare_no_case(string const & s, string const & s2)
{
	// ANSI C
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


int compare_no_case(string const & s, string const & s2, unsigned int len)
{
//#warning verify this func please
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
	if (s.size() == s2.size())
		return 0;
	if (s.size() < s2.size())
		return -1;
	return 1;
}


bool isStrInt(string const & str)
{
	if (str.empty()) return false;
       
	// Remove leading and trailing white space chars.
	string tmpstr = frontStrip(strip(str, ' '), ' ');
	if (tmpstr.empty()) return false;
       
	string::const_iterator cit = tmpstr.begin();
	if ( (*cit) == '-') ++cit;
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
		string tmpstr = frontStrip(strip(str, ' '), ' ');
		// Do the conversion proper.
		return atoi(tmpstr.c_str());
	} else {
		return 0;
	}
}


bool isStrDbl(string const & str)
{
	if (str.empty()) return false;
	
	// Remove leading and trailing white space chars.
	string tmpstr = frontStrip(strip(str, ' '), ' ');
	if (tmpstr.empty()) return false;
	//	if (1 < tmpstr.count('.')) return false;

	string::const_iterator cit = tmpstr.begin();
	bool found_dot(false);
	if ( (*cit) == '-') ++cit;
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
		string tmpstr = frontStrip(strip(str, ' '), ' ');
		// Do the conversion proper.
		return atof(tmpstr.c_str());
	} else {
		return 0.0;
	}
}


char lowercase(char c) 
{ 
	return tolower(c); 
}


char uppercase(char c) 
{ 
	return toupper(c); 
}


string const lowercase(string const & a)
{
	string tmp(a);
//#ifdef __GLIBCPP__
	string::iterator result = tmp.begin();
	string::iterator end = tmp.end();
	for (string::iterator first = tmp.begin();
	     first != end; ++first, ++result) {
		*result = lowercase(*first);
	}
//#else
//	transform(tmp.begin(), tmp.end(), tmp.begin(), tolower);
//#endif
	return tmp;
}


string const uppercase(string const & a)
{
	string tmp(a);
//#ifdef __GLIBCPP__
	string::iterator result = tmp.begin();
	string::iterator end = tmp.end();
	for (string::iterator first = tmp.begin();
	     first != end; ++first, ++result) {
		*result = uppercase(*first);
	}
//#else
//	transform(tmp.begin(), tmp.end(), tmp.begin(), toupper);
//#endif
	return tmp;
}


bool prefixIs(string const & a, char const * pre)
{
	Assert(pre);
	
	unsigned int l = strlen(pre);
	if (l > a.length() || a.empty())
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


bool suffixIs(string const & a, char c)
{
	if (a.empty()) return false;
	return a[a.length() - 1] == c;
}


bool suffixIs(string const & a, char const * suf)
{
	Assert(suf);
	
	unsigned int const suflen = strlen(suf);
	if (suflen > a.length())
		return false;
	else {
#if !defined(USE_INCLUDED_STRING) && !defined(STD_STRING_IS_GOOD)
		// Delete this code when the compilers get a bit better.
		string tmp(a, a.length() - suflen);
		return ::strncmp(tmp.c_str(), suf, suflen) == 0;
#else
		// This is the code that we really want to use
		// but until gcc ships with a basic_string that
		// implements std::string correctly we have to
		// use the code above.
		return a.compare(a.length() - suflen, suflen, suf) == 0;
#endif
	}
}


bool contains(char const * a, string const & b)
{
	Assert(a);
	
	if (!*a || b.empty()) return false;
	return strstr(a, b.c_str()) != 0;
}


bool contains(string const & a, char const * b)
{
	Assert(b);
	
	if (a.empty())
		return false;
	return a.find(b) != string::npos;
}


bool contains(string const & a, string const & b)
{
	if (a.empty())
		return false;
	return a.find(b) != string::npos;
}


bool contains(char const * a, char const * b)
{
	Assert(a && b);
	
	if (!*a || !*b) return false;
	return strstr(a, b) != 0;
}


bool containsOnly(string const & s, char const * cset)
{
	Assert(cset);
	
	return s.find_first_not_of(cset) == string::npos;
}


bool containsOnly(string const & s, string const & cset)
{
	return s.find_first_not_of(cset) == string::npos;
}


bool containsOnly(char const * s, char const * cset)
{
	Assert(s && cset);
	
	return string(s).find_first_not_of(cset) == string::npos;
}


bool containsOnly(char const * s, string const & cset)
{
	Assert(s);
	
	return string(s).find_first_not_of(cset) == string::npos;
}


unsigned int countChar(string const & a, char c)
{
#ifdef HAVE_STD_COUNT
	return count(a.begin(), a.end(), c);
#else
	unsigned int n = 0;
	count(a.begin(), a.end(), c, n);
	return n;
#endif
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
	for(; lit != end; ++lit)
		if ((*lit) == oldchar)
			(*lit) = newchar;
	return tmp;
}


string const subst(string const & a,
	     char const * oldstr, string const & newstr)
{
	Assert(oldstr);
	
	string lstr(a);
	string::size_type i = 0;
	int olen = strlen(oldstr);
	while((i = lstr.find(oldstr, i)) != string::npos) {
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
	else
		tmp.erase(); // only c in the whole string
	return tmp;
}


string const frontStrip(string const & a, char const * p)
{
	Assert(p);
	
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

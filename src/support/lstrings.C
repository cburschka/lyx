#include <config.h>

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>

#include "LString.h"
#include "lstrings.h"

//#include "debug.h"

using std::count;

bool isStrInt(string const & str)
{
	if (str.empty()) return false;
       
	// Remove leading and trailing white space chars.
	string tmpstr = frontStrip(strip(str, ' '), ' ');
	if (tmpstr.empty()) return false;
       
	string::const_iterator cit = tmpstr.begin();
	if ( (*cit) == '-') ++cit;
	for (; cit != tmpstr.end(); ++cit) {
		if (!isdigit((*cit))) return false;
	}
	return true;
}


int  LStr2Int(string const & str)
{
	string tmpstr;

	if (isStrInt(str)) {
		// Remove leading and trailing white space chars.
		tmpstr = frontStrip(strip(str, ' '), ' ');
		// Do the conversion proper.
		return atoi(tmpstr.c_str());
	} else
		return 0;
}


string lowercase(string const & a)
{
	string tmp;
	string::const_iterator cit = a.begin();
	for(; cit != a.end(); ++cit) {
		tmp += char(tolower(*cit));
	}
	return tmp;
}


string tostr(long i)
{
	char str[30];
	sprintf(str, "%ld", i);
	return string(str);
}

string tostr(unsigned long i)
{
	char str[30];
	sprintf(str, "%lu", i);
	return string(str);
}

string tostr(void * v)
{
	return tostr(long(v));
}


string tostr(int i)
{
	return tostr(long(i));
}

string tostr(unsigned int ui)
{
	return tostr(long(ui));
}

string tostr(char c)
{
  return tostr(long(c));
}

string tostr(bool b)
{
	return b ? "true" : "false";
}

#if 0
string tostr(float f)
{
	return tostr(double(f));
}
#endif

string tostr(double d)
{
	char tmp[40];
	sprintf(tmp, "%f", d);
	return string(tmp);
}


bool prefixIs(string const & a, char const * pre)
{
	unsigned int l = strlen(pre);
	if (l > a.length() || a.empty())
		return false;
	else
		return a.compare(0, l, pre, l) == 0;
}


bool suffixIs(string const & a, char c)
{
	if (a.empty()) return false;
	return a[a.length()-1] == c;
}


bool suffixIs(string const & a, char const * suf)
{
	unsigned int suflen = strlen(suf);
	if (suflen > a.length())
		return false;
	else {
		return a.compare(a.length() - suflen, suflen, suf) == 0;
	}
}


bool contains(char const * a, string const & b)
{
	if (!a || !*a || b.empty()) return false;
	return strstr(a, b.c_str()) != 0;
}


bool contains(string const & a, char const * b)
{
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
	if (!a || !b || !*a || !*b) return false;
	return strstr(a, b) != 0;
}


int countChar(string const & a, char const c)
{
	unsigned int n = 0;
	count(a.begin(), a.end(), c, n);
	return n;
}


// ale970405+lasgoutt-970425
// rewritten to use new string (Lgb)
string token(string const & a, char delim, int n)
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
	string str = a;
	string tmptok;

	while (!str.empty()) {
		str = split(str, tmptok, delim);
		if (tok==tmptok)
			return i;
		++i;
	}
	return -1;
}


bool regexMatch(string const & a, string const & pattern)
{
	if (pattern.empty())
		return true;
	if (a.empty())
		return false;
	
	string::size_type si=0, pi=0;
	string::size_type const sl = a.length();
	string::size_type const pl = pattern.length();	

	while (si < sl && pi < pl) {
		if (pattern[pi]=='*') {
			// Skip all consequtive *s
			while (pattern[pi] == '*') {
				++pi;
				if (pi == pl)
					return true;
			}

			// Get next chunk of pattern to match
			string chunk;
			string temp =
				split(pattern.substr(pi, pl-1), chunk, '*');

			if (!chunk.empty() && pattern[pl-1] == '*' && 
			    temp.empty())
				temp = '*';

			if (temp.empty()) {
				// Last chunk, see if tail matches
				if (sl < chunk.length()) {
					return false;
				}
				temp = a.substr(sl - chunk.length(), sl - 1);
				return temp == chunk;
			} else {
				// Middle chunk, see if we can find a match
				bool match = false;
				while (!match && si<sl) {
					temp = a.substr(si, sl - 1);
					match = prefixIs(temp, chunk.c_str());
					++si;
				};
				if (!match)
					return false;
				si += chunk.length()-1;
				pi += chunk.length();
				if (si==sl && pi==pl-1)
					return true;
			}
		} else if (a[si++] != pattern[pi++]) {
			return false;
		}
	}
	if (pi < pl || si < sl)
		return false;	
	return true;
}


string subst(string const & a, char oldchar, char newchar)
{
	string tmp = a;
	string::iterator lit = tmp.begin();
	for(; lit != tmp.end(); ++lit)
		if ((*lit) == oldchar)
			(*lit) = newchar;
	return tmp;
}


string subst(string const & a,
	      char const * oldstr, string const & newstr)
{
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


string strip(string const & a, char const c)
{
	if (a.empty()) return a;
	string tmp = a;
	string::size_type i = tmp.find_last_not_of(c);
	if (i == a.length() - 1) return tmp; // no c's at end of a
	if (i != string::npos) 
		tmp.erase(i + 1, string::npos);
	else
		tmp.erase(); // only c in the whole string
	return tmp;
}


string frontStrip(string const & a, char const * p)
{
	if (a.empty() || !p || !*p) return a;
	string tmp = a;
	string::size_type i = tmp.find_first_not_of(p);
	if (i > 0)
		tmp.erase(0, i);
	return tmp;
}


string frontStrip(string const & a, char const c)
{
	if (a.empty()) return a;
	string tmp = a;
	string::size_type i = tmp.find_first_not_of(c);
	if (i > 0)
		tmp.erase(0, i);
	return tmp;
}


string split(string const & a, string & piece, char delim)
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


string split(string const & a, char delim)
{
	string tmp;
	string::size_type i = a.find(delim);
	if (i != string::npos) // found delim
		tmp = a.substr(i + 1);
	return tmp;
}


// ale970521
string rsplit(string const & a, string & piece, char delim)
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

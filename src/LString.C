/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *	    Copyright (C) 1995 Matthias Ettrich
 *          Copyright (C) 1995-1998 The LyX Team.
 *
 *======================================================*/


#include <config.h>

#ifdef __GNUG__
#pragma implementation "LString.h"
#endif

// #include <assert.h> // Hmm, make depend crashes with this one in. (Asger)
#include "LString.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

// 	$Id: LString.C,v 1.1 1999/09/27 18:44:36 larsbj Exp $	

#if !defined(lint) && !defined(WITH_WARNINGS)
static char vcid[] = "$Id: LString.C,v 1.1 1999/09/27 18:44:36 larsbj Exp $";
#endif /* lint */

static const unsigned short xtra = 4;
// The extra space is used to reduce the number of allocations
// and copies required if a string is unshared. The performance
// boost can be considerable -- in some tests using LyX I found
// a 97% reduction in the runtime of operator+=(char) in some
// code for writing LaTeX files using LStrings.
// This was originally implemented using:
//     xtra = 4 - (sizeof(srep) + len) % 4;
// where len is the length of the new string.
// This was intended to ensure the string was always aligned
// within 4-byte boundaries but after testing with xtra = 4,
// and finding a significant improvement I decided to just
// leave it at 4. ARRae.

LString::LString()
{
	static srep empty_rep;
	
	p = &empty_rep;
	empty_rep.n++;
}


LString::LString(char const *s)
{
	static srep empty_rep;
	
	if (s && *s) {
		// > 99 %
		register unsigned int const len = strlen(s);
		p = (srep *) new char[sizeof(srep) + len + xtra];
		p->l = len;
		p->n = 0;
		p->e = xtra;
		memcpy(p->s, s, len + 1);
	} else {
		// < 1 %
		p = &empty_rep;
		empty_rep.n++;
	}
}


LString::LString(char const c)
{
	static srep empty_rep;

	if (c) {
		p = (srep *) new char[sizeof(srep) + 1 + xtra];
		p->l = 1;
		p->n = 0;
		p->e = xtra;
		p->s[0] = c;
		p->s[1] = '\0';
	} else {
		p = &empty_rep;
		empty_rep.n++;
	}
}


LString& LString::operator=(char const *s)
{
	static srep empty_rep;
//	assert(p);

	lose(); // disconnect self

	if (s && *s) {
		register unsigned int const len = strlen(s);
		p = (srep *) new char[sizeof(srep) + len + xtra];
		p->l = len;
		p->n = 0;
		p->e = xtra;
		memcpy(p->s, s, len + 1);
	} else {
		p = &empty_rep;
		empty_rep.n++;
	}
	
	return *this;
}


LString& LString::operator=(LString const &x)
{
//	assert(p);
	x.p->n++; // protect against ``st = st''

        lose(); // disconnect self

	p = x.p;
	return *this;
}


LString& LString::operator=(char c)
{
	static srep empty_rep;

//	assert(p);

	lose(); // disconnect self

	if (c) {
		p = (srep *) new char[sizeof(srep) + 1 + xtra];
		p->l = 1;
		p->n = 0;
		p->e = xtra;
		p->s[0] = c;
		p->s[1] = '\0';
	} else {
		p = &empty_rep;
		empty_rep.n++;
	}

	return *this;
}


LString &LString::clean()
{
	static srep empty_rep;

	lose();
	p = &empty_rep;
	empty_rep.n++;
	return *this;
}



char& LString::operator[](int i)
{
#ifdef DEVEL_VERSION
	if (i < 0 || i >= length()) {
		fprintf(stderr,"LString::operator[]: Index out of range: '%s' %d\n", p->s, i);
		abort();
	}
#endif

	if (p->n > 0) { // clone to maintain value semantics
		srep * np = (srep *) new char[sizeof(srep) + p->l];
		np->l = p->l;
		np->n = 0;
		np->e = 0;
		memcpy(np->s, p->s, length() + 1);
		p->n--;
		p = np;
	}
	return p->s[i];
}

#ifndef const
char const& LString::operator[](int i) const
{
#ifdef DEVEL_VERSION
	if (i < 0 || i >= length()) {
		fprintf(stderr,"LString::operator[] const: Index out of range: '%s' i:%d.\n",p->s,i);
		abort();
	}
#endif

	return p->s[i];
}
#endif /* ndef const */

LString &LString::operator+=(LString const & x)
{
	if (x.empty()) return *this;

	register unsigned int const len = length() + x.length();
	if (p->n || p->e < x.length()) {
		srep *np = (srep *) new char[sizeof(srep) + len + xtra];
		np->l = len;
		np->n = 0;
		np->e = xtra;
		memcpy(np->s, p->s, length());
		memcpy(np->s + length(), x.p->s, x.length() + 1);
		lose(); // disconnect self
		p = np;
	} else {
		// in cases where x += x and x is short the
		// explicit setting of the '\0' stops any problems
		memcpy(p->s + length(), x.p->s, x.length());
		p->s[len] = '\0';
		p->l += x.length();
		p->e -= x.length();
	}

	return *this;
}


LString &LString::operator+=(char const *x)
{
	if (!x || *x==0) return *this;

	register unsigned int const xlen = strlen(x);
	register unsigned int const len = length() + xlen;
	if (p->n || p->e < xlen) {
		srep *np = (srep *) new char[sizeof(srep) + len + xtra];
		np->l = len;
		np->n = 0;
		np->e = xtra;
		memcpy(np->s, p->s, length());
		memcpy(np->s + length(), x, xlen + 1);
		lose(); // disconnect self
		p = np;
	} else {
		// Explicitly setting the '\0' stops any
		// problems caused by x += x.c_str()
		memcpy(p->s + length(), x, xlen);
		p->s[len] = '\0';
		p->l += xlen;
		p->e -= xlen;
	}

	return *this;
}


LString &LString::operator+=(char c)
{
	register unsigned int const len = length() + 1;
	if (!p->n && p->e) {
		// 80% (from profiling)
		// This is where all the speed gains are made.
		p->s[length()] = c;
		p->s[len] = '\0';
		p->l = len;
		p->e -= 1;
	} else {
		// 20%
		srep *np = (srep *) new char[sizeof(srep) + len + xtra];
		np->l = len;
		np->n = 0;
		np->e = xtra;
		memcpy(np->s, p->s, length());
		np->s[length()] = c;
		np->s[len] = '\0';
		lose(); // disconnect self
		p = np;
	}

	return *this;
}


LString &LString::operator+=(int i)
{
	return this->operator+=((long)i);
}


LString &LString::operator+=(long i)
{
	unsigned int tmplen = 0;
	long a = i;
	// calculate the length of i
	if (!i) {
		tmplen = 1;
	} else {
		if (a < 0) {
			tmplen++; // minus sign
			a = -a; // switch sign
		}
		while(a >= 1) { a = a/10; tmplen++;}
	}
	char *str = new char[tmplen + 1];
	sprintf(str, "%ld", i);
	this->operator+=(str);
	delete[] str;

	return *this;
}


char * LString::copy() const
{
	char * new_string = new char[length()+1];
	memcpy(new_string, p->s, length() + 1);
	return new_string;
}


bool LString::contains(char const *a) const
{
	return strstr(p->s, a) != NULL;
}


LString& LString::substring(int i1, int i2)
{
#ifdef DEVEL_VERSION
	if (i1 > i2 || i1 >= length() || i2 >= length()) {
		fprintf(stderr,
			"LString::substring: Wrong indexing in substring:"
			" '%s' i1=%d i2=%d\n", p->s, i1, i2);
		abort();
	}
#endif

	if (i1==i2)
		this->operator=(p->s[i1]);
	else {
		char *str = new char[i2 - i1 +2];
		int i;
		for (i=0; i1<=i2; str[i++] = p->s[i1++]);
		str[i] = 0;
		this->operator=(str);
		delete[] (char*)str;
	}
	return *this;
}


// ale970405+lasgoutt-970425
LString LString::token(char delim, int n) const
{
	int k=0, i;
        LString tokbuf;

        tokbuf = *this;   
	// Find delimiter or end of string
	for (i = 0; i < tokbuf.length(); i++) {
		if (tokbuf[i] == delim) {
			if (n > 0) {
				k = i+1;
				n--;
			} else break;
		}
	}

	// Return the token if not empty
	if (n == 0 && k<i){
		return tokbuf.substring(k, i-1);
	} else {
		return LString();
	}
}


// this could probably be faster and/or cleaner, but it seems to work (JMarc)
int LString::tokenPos(char delim, LString const &tok)
{
	int i=0;
	LString str = *this;
	LString tmptok;

	while (!str.empty()) {
		str.split(tmptok, delim);
		if (tok==tmptok)
			return i;
		i++;
	}
	return -1;
}


LString& LString::split(LString & piece, char delim)
{
	int i=0;
	// Find delimiter or end of string
	while (i<length() && p->s[i] != delim)
		i++;
	// If not the first, we go for a substring
	if (i>0) {
		piece = *this;
		piece.substring(0, i-1);
	} else
		piece.clean();

	if (i < length()-1)
		this->substring(i+1, length()-1);
	else
		clean();
	return *this;
}


LString& LString::split(char delim)
{
	int i=0;
	// Find delimiter or end of string
	while (i<length() && p->s[i] != delim)
		i++;

	if (i < length()-1)
		this->substring(i+1, length()-1);
	else
		clean();
	return *this;
}


// ale970521
LString& LString::rsplit(LString & piece, char delim)
{
	int i=length()-1;
	// Find delimiter or begin of string
	while (i>=0 && p->s[i] != delim)
		i--;
	// If not the last, we go for a substring
	if (i < length()-1) {
		piece = *this;
		piece.substring(0, i-1);
		this->substring(i+1, length()-1);
	} else {
		piece.clean();
		clean();
	}
	return *this;
}


LString& LString::strip(char const c)
{
	int i=length()-1;
	for (; i>=0 && p->s[i] == c; i--);
	if (i<0) 
		clean();
	else
		this->substring(0, i);
	return *this;
}


LString& LString::frontStrip(char const c)
{
	int i=0;
	while (i < length() && p->s[i] == c) i++;
	if (i > 0)
		if (i == length())
			clean();
		else
			this->substring (i, length()-1);
	return *this;
}


bool LString::prefixIs(char const * pre) const
{
	if ((int) strlen(pre) > length())
		return false;
	else
		return strncmp(p->s, pre, strlen(pre))==0;
}


bool LString::suffixIs(char c) const
{
	if (empty()) return false;
	return p->s[length()-1] == c;
}


bool LString::suffixIs(char const * suf) const
{
	int suflen = (int) strlen(suf);
	if (suflen > length())
		return false;
	else
		return strncmp(p->s + (length()-suflen), suf, suflen)==0;
}


LString& LString::subst(char oldchar, char newchar)
{
	for (int i=0; i<length() ; i++)
		if (p->s[i]==oldchar)
			p->s[i]=newchar;
	return *this;
}


LString& LString::subst(char const * oldstr, LString const & newstr)
{
	LString lstr = *this;
	char * str = lstr.copy();
	char * first;
      
	while((first=strstr(str,oldstr))){
        	if (first==str) lstr.clean();
		else lstr.substring(0,first-str-1);
		lstr+=newstr;
		lstr+=first+strlen(oldstr);
		delete[] (char*)str;
		str = lstr.copy();
	}
	delete[] (char*)str;
	return *this=lstr;
}


LString& LString::lowercase()
{
	for (int i=0; i<length() ; i++)
		p->s[i] = tolower((unsigned char) p->s[i]);
	return *this;
}


bool LString::regexMatch(LString const & pattern) const
{
	if (pattern.empty())
		return true;
	if (empty())
		return false;
	
	int si=0, pi=0;
	int const sl = length();
	int const pl = pattern.length();	

	while (si < sl && pi < pl) {
		if (pattern[pi]=='*') {
			// Skip all consequtive *s
			while (pattern[pi] == '*') {
				pi++;
				if (pi == pl)
					return true;
			}

			// Get next chunk of pattern to match
			LString temp= pattern;
			temp.substring(pi, pl-1);
			LString chunk;
			temp.split(chunk, '*');

			if (!chunk.empty() && pattern[pl-1] == '*' && 
			    temp.empty())
				temp = '*';

			if (temp.empty()) {
				// Last chunk, see if tail matches
				temp = *this;
				temp.substring(sl - chunk.length(), sl - 1);
				return temp == chunk;
			} else {
				// Middle chunk, see if we can find a match
				bool match = false;
				while (!match && si<sl) {
					temp = *this;
					temp.substring(si, sl - 1);
					match = temp.prefixIs(chunk.c_str());
					si++;
				};
				if (!match)
					return false;
				si += chunk.length()-1;
				pi += chunk.length();
				if (si==sl && pi==pl-1)
					return true;
			}
		} else if (operator[](si++) != pattern[pi++]) {
				return false;
		}
	}
	if (pi < pl || si < sl)
		return false;	
	return true;
}

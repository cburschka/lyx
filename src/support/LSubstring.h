// -*- C++ -*-
/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *	    Copyright (C) 1995 Matthias Ettrich
 *          Copyright (C) 1995-1998 The LyX Team.
 *
 * ====================================================== */

// This one is heavily based on the substring class in The C++
// Programming Language by Bjarne Stroustrup

#ifndef LSUBSTRING_H
#define LSUBSTRING_H 

#ifdef __GNUG__
#pragma interface
#endif


#include "LString.h"
#include "LRegex.h"

///
class LSubstring {
public:
	///
	typedef string::size_type size_type;
	///
	LSubstring(string & s, size_type i, size_type n);
	///
	LSubstring(string & s, string & s2);
	///
	LSubstring(string & s, string::value_type * p);
	///
	LSubstring(string & s, LRegex & r);
	///
	LSubstring & operator=(string const &);
	///
	LSubstring & operator=(LSubstring const &);
	///
	LSubstring & operator=(string::value_type const *);
	///
	LSubstring & operator=(string::value_type);
	///
	operator string() const;
#if 0
	///
	operator char const * () const;
#endif
private:
	///
	string * ps;
	///
	size_type pos;
	///
	size_type n;
};

#endif

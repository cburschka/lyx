// -*- C++ -*-
/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *	    Copyright 1995 Matthias Ettrich
 *          Copyright 1995-2000 The LyX Team.
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
	LSubstring(string & s, string const & s2);
	///
	LSubstring(string & s, string::value_type const * p);
	///
	LSubstring(string & s, LRegex const & r);
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
private:
	///
	string * ps;
	///
	size_type pos;
	///
	size_type n;
};

#endif

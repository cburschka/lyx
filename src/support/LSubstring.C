/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2000 The LyX Team.
 *
 * ====================================================== */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef __GNUG__
#pragma implementation
#endif

#include "LSubstring.h"


LSubstring::LSubstring(string & s, size_type i, size_type l)
	: ps(&s), pos(i), n(l)
{
}


LSubstring::LSubstring(string & s, string const & s2)
	: ps(&s), n(s2.length())
{
	pos = s.find(s2);
}


LSubstring::LSubstring(string & s, string::value_type const * p)
	: ps(&s)
{
	n = strlen(p);
	pos = s.find(p);
}


LSubstring::LSubstring(string & s, LRegex const & r)
	: ps(&s)
{
	LRegex::MatchPair res = r.first_match(s);
	if (res.first != string::npos) {
		n = res.second;
		pos = res.first;
	} else {
		n = 0;
		pos = string::npos;
	}
}


LSubstring & LSubstring::operator=(string const & s)
{
	ps->replace(pos, n, s); // write through to *ps
	return *this;
}


LSubstring & LSubstring::operator=(LSubstring const & s)
{
	ps->replace(pos, n, s);
	return *this;
}


LSubstring & LSubstring::operator=(char const * p)
{
	ps->replace(pos, n, p);
	return *this;
}


LSubstring & LSubstring::operator=(char c)
{
	ps->replace(pos, n, 1, c);
	return *this;
}


LSubstring::operator string() const
{
	return string(*ps, pos, n); // copy from *ps
}

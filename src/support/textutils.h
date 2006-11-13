// -*- C++ -*-
/**
 * \file textutils.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Matthias Ettrich
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

// FIXME: I can think of a better name for this file ...

#ifndef TEXTUTILS_H
#define TEXTUTILS_H

#include "support/types.h"

#ifdef LIBC_WCTYPE_USES_UCS4
// We can use the libc ctype functions because we unset the LC_CTYPE
// category of the current locale in gettext.C
#include <wctype.h>
#else
// Steal some code from somewhere else, e.g. glib (look at gunicode.h)
// The code that we currently use does not really work.
#endif


namespace lyx {

/// return true if the char is a line separator
inline
bool isLineSeparatorChar(char_type c)
{
	return c == ' ';
}


/// return true if a char is alphabetical (including accented chars)
inline
bool isLetterChar(char_type c)
{
#ifdef LIBC_WCTYPE_USES_UCS4
	return iswalpha(c);
#else
	// FIXME UNICODE This is wrong!
	return (c >= 'A' && c <= 'Z')
		|| (c >= 'a' && c <= 'z')
		|| (c >= 192 && c < 256); // in iso-8859-x these are accented chars
#endif
}


/// return true if the char is printable
inline
bool isPrintable(char_type c)
{
#ifdef LIBC_WCTYPE_USES_UCS4
	return iswprint(c);
#else
	// FIXME UNICODE This is wrong!
	return (c & 127) >= ' ';
#endif
}


/// return true if the char is printable and not a space
inline
bool isPrintableNonspace(char_type c)
{
#ifdef LIBC_WCTYPE_USES_UCS4
	return iswprint(c) && !iswspace(c);
#else
	// FIXME UNICODE This is wrong!
	return (c & 127) > ' ';
#endif
}


/// return true if a unicode char is a digit.
inline
bool isDigit(char_type c)
{
#ifdef LIBC_WCTYPE_USES_UCS4
	return iswdigit(c);
#else
	// FIXME UNICODE This is wrong!
	return c >= '0' && c <= '9';
#endif
}



} // namespace lyx

#endif // TEXTUTILS_H

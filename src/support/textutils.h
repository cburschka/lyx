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


namespace lyx {

/// return true if the char is a line separator
inline
bool isLineSeparatorChar(lyx::char_type c)
{
	return c == ' ';
}


/// return true if a char is alphabetical (including accented chars)
inline
bool isLetterChar(lyx::char_type c)
{
	return (c >= 'A' && c <= 'Z')
		|| (c >= 'a' && c <= 'z')
		|| (c >= 192 && c < 256); // in iso-8859-x these are accented chars
}


/// return true if the char is printable (masked to 7-bit ASCII)
inline
bool isPrintable(lyx::char_type c)
{
	return (c & 127) >= ' ';
}


/// return true if the char is printable and not a space (masked to 7-bit ASCII)
inline
bool isPrintableNonspace(lyx::char_type c)
{
	return isPrintable(c) && c != ' ';
}

/// return true if a unicode char is a digit.
inline
bool isDigit(lyx::char_type ch)
{
	return ch >= '0' && ch <= '9';
}



} // namespace lyx

#endif // TEXTUTILS_H

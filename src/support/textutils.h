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

/// return true if the char is a word separator
inline
bool IsSeparatorChar(char c)
{
	return c == ' ';
}


/// return true if the char is a line separator
inline
bool IsLineSeparatorChar(char c)
{
	return c == ' ';
}


/// return true if the char is "punctuation"
inline
bool IsKommaChar(char c)
{
	return c == ','
		|| c == '('
		|| c == ')'
		|| c == '['
		|| c == ']'
		|| c == '{'
		|| c == '}'
		|| c == ';'
		|| c == '.'
		|| c == ':'
		|| c == '-'
		|| c == '?'
		|| c == '!'
		|| c == '&'
		|| c == '@'
		|| c == '+'
		|| c == '-'
		|| c == '~'
		|| c == '#'
		|| c == '%'
		|| c == '^'
		|| c == '/'
		|| c == '\\';
}


/// return true if a char is alphabetical (including accented chars)
inline
bool IsLetterChar(unsigned char c)
{
	return (c >= 'A' && c <= 'Z')
		|| (c >= 'a' && c <= 'z')
		|| (c >= 192); // in iso-8859-x these are accented chars
}


/// return true if the char is printable (masked to 7-bit ASCII)
inline
bool IsPrintable(unsigned char c)
{
	return (c & 127) >= ' ';
}


/// return true if the char is printable and not a space (masked to 7-bit ASCII)
inline
bool IsPrintableNonspace(unsigned char c)
{
	return IsPrintable(c) && c != ' ';
}


/// completely pointless FIXME
inline
bool IsDigit(unsigned char ch)
{
	return ch >= '0' && ch <= '9';
}


#endif // TEXTUTILS_H

// -*- C++ -*-
/**
 * \file textutils.h
 * Copyright 1995-2002 the LyX Team
 * Read the file COPYING
 *
 * \author unknown
 */

// FIXME: I can think of a better name for this file ...
 
#ifndef TEXTUTILS_H
#define TEXTUTILS_H

/// return true if the char is a meta-character newline
inline
bool IsNewlineChar(char c) {
	return (c == Paragraph::META_NEWLINE);
}


/// return true if the char is a word separator
inline
bool IsSeparatorChar(char c) {
	return (c == ' ');
}


/// return true if the char is a line separator
inline
bool IsLineSeparatorChar(char c) {
	return (c == ' ');
}
 

/// return true if the char is a meta-character for hfill
inline
bool IsHfillChar(char c) {
	return (c == Paragraph::META_HFILL);
}


/// return true if the char is a meta-character for an inset
inline
bool IsInsetChar(char c) {
	return (c == Paragraph::META_INSET);
}


/// return true if the char is "punctuation"
inline
bool IsKommaChar(char c) {
	return (c == ','
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
		|| c == '\\'
		|| c == Paragraph::META_NEWLINE
		);
}


/// return true if a char is alphabetical (including accented chars)
inline
bool IsLetterChar(unsigned char c) {
	return ((c >= 'A' && c <= 'Z')
		|| (c >= 'a' && c <= 'z')
		|| (c >= 192)); // in iso-8859-x these are accented chars
}


/// return true if the char is printable (masked to 7-bit ASCII)
inline
bool IsPrintable(unsigned char c) {
	return ((c & 127) >= ' ');
}


/// return true if the char is printable and not a space (masked to 7-bit ASCII) 
inline
bool IsPrintableNonspace(unsigned char c) {
	return IsPrintable(c) && (c != ' ');
}


/// return true if the char forms part of a word
inline
bool IsWordChar(unsigned char c) {
	return !(IsSeparatorChar(c)
		  || IsKommaChar(c)
		  || IsHfillChar(c)
		  || IsInsetChar(c));
}


/// completely pointless FIXME
inline
bool IsDigit(unsigned char ch)
{
	return ch >= '0' && ch <= '9';
}


/// return true if the char is alphanumeric
inline
bool IsLetterCharOrDigit(unsigned char ch)
{
	return IsLetterChar(ch) || IsDigit(ch);
}
 
#endif // TEXTUTILS_H

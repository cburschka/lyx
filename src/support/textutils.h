// -*- C++ -*-
/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2001 The LyX Team.
 *
 * ====================================================== */

#ifndef TEXTUTILS_H
#define TEXTUTILS_H

///
inline
bool IsNewlineChar(char c) {
	return (c == Paragraph::META_NEWLINE);
}


///
inline
bool IsSeparatorChar(char c) {
	return (c == ' ');
}


///
inline
bool IsHfillChar(char c) {
	return (c == Paragraph::META_HFILL);
}


///
inline
bool IsInsetChar(char c) {
	return (c == Paragraph::META_INSET);
}


///
inline
bool IsLineSeparatorChar(char c) {
	return (c == ' ');
}

///
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


///
inline
bool IsLetterChar(unsigned char c) {
	return ((c >= 'A' && c <= 'Z')
		|| (c >= 'a' && c <= 'z')
		|| (c >= 192)); // in iso-8859-x these are accented chars
}


///
inline
bool IsPrintable(unsigned char c) {
	return ((c & 127) >= ' ');
}


///
inline
bool IsPrintableNonspace(unsigned char c) {
	return IsPrintable(c) && (c != ' ');
}


/// Word is not IsSeparator or IsKomma or IsHfill or IsFloat or IsInset. 
inline
bool IsWordChar(unsigned char c) {
	return !(IsSeparatorChar(c)
		  || IsKommaChar(c)
		  || IsHfillChar(c)
		  || IsInsetChar(c));
}


///
inline
bool IsDigit(unsigned char ch)
{
	return ch >= '0' && ch <= '9';
}


///
inline
bool IsLetterCharOrDigit(unsigned char ch)
{
	return IsLetterChar(ch) || IsDigit(ch);
}
#endif

// -*- C++ -*-
/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *	    Copyright (C) 1995 Matthias Ettrich
 *
 * ====================================================== */

#ifndef TEXTUTILS_H
#define TEXTUTILS_H

#include <cctype>

///
inline bool IsNewlineChar(char c) {
	return (c == LyXParagraph::META_NEWLINE);
}


///
inline bool IsSeparatorChar(char c) {
	return (c == ' ' || c == LyXParagraph::META_PROTECTED_SEPARATOR);
}


///
inline bool IsHfillChar(char c) {
	return (c == LyXParagraph::META_HFILL);
}


///
inline bool IsInsetChar(char c) {
	return (c == LyXParagraph::META_INSET);
}


///
inline bool IsFloatChar(char c) {
	return (c == LyXParagraph::META_FOOTNOTE
		|| c == LyXParagraph::META_MARGIN
		|| c == LyXParagraph::META_FIG
		|| c == LyXParagraph::META_TAB
		|| c == LyXParagraph::META_ALGORITHM
		|| c == LyXParagraph::META_WIDE_FIG
		|| c == LyXParagraph::META_WIDE_TAB);
}


///
inline bool IsLineSeparatorChar(char c) {
	return (c == ' ');
}


///
inline bool IsKommaChar(char c) {
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
		|| c == LyXParagraph::META_NEWLINE
		|| c == LyXParagraph::META_PROTECTED_SEPARATOR
		);
}


///
inline bool IsLetterChar(unsigned char c) {
	return ((c >= 'A' && c <= 'Z')
		|| (c >= 'a' && c <= 'z')
		|| (c >= 192)); // in iso-8859-x these are accented chars
}


///
inline bool IsPrintable(unsigned char c) {
	return (c >= ' ');
}


/// Word is not IsSeparator or IsKomma or IsHfill or IsFloat or IsInset. 
inline bool IsWordChar(unsigned char c) {
  return !( IsSeparatorChar( c ) 
	    || IsKommaChar( c )  
	    || IsHfillChar( c )  
	    || IsFloatChar( c )  
	    || IsInsetChar( c ) ) ;
}


///
inline bool IsLetterCharOrDigit(char ch)
{
	return IsLetterChar(ch) || isdigit(ch);
}
#endif

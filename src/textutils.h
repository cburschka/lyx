// -*- C++ -*-
/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *	    Copyright (C) 1995 Matthias Ettrich
 *
 *======================================================*/
#ifndef _TEXTUTILS_H
#define _TEXTUTILS_H

#include "definitions.h"

///
inline bool IsNewlineChar(char c) {
	return (c == LYX_META_NEWLINE);
}
///
inline bool IsSeparatorChar(char c) {
	return (c == ' ' || c == LYX_META_PROTECTED_SEPARATOR);
}
///
inline bool IsHfillChar(char c) {
	return (c == LYX_META_HFILL);
}
///
inline bool IsInsetChar(char c) {
	return (c == LYX_META_INSET);
}
///
inline bool IsFloatChar(char c) {
	return (c == LYX_META_FOOTNOTE
		|| c == LYX_META_MARGIN
		|| c == LYX_META_FIG
		|| c == LYX_META_TAB
		|| c == LYX_META_ALGORITHM
		|| c == LYX_META_WIDE_FIG
		|| c == LYX_META_WIDE_TAB);
}
///
inline bool IsLineSeparatorChar(char c) {
	return (c == ' ');
}
///
inline bool IsKommaChar(char c) {
	return (c == ',' 
		|| c=='('
		|| c==')'
		|| c=='['
		|| c==']'
		|| c=='{'
		|| c=='}'
		|| c==';'
		|| c=='.'
		|| c==':'
		|| c=='-'
		|| c=='?'
		|| c=='!'
		|| c=='&'
		|| c=='@'
		|| c=='+'
		|| c=='-'
		|| c=='~'
		|| c=='#'
		|| c=='%'
		|| c=='^'
		|| c=='/' 
		|| c=='\\'
		|| c==LYX_META_NEWLINE
		|| c==LYX_META_PROTECTED_SEPARATOR
		);
}
///
inline bool IsLetterChar(unsigned char c) {
	return ((c>='A' && c<='Z')
		|| (c>='a' && c<='z')
		|| (c>=192)); // in iso-8859-x these are accented chars
}
///
inline bool IsPrintable(unsigned char c) {
	return (c>=' ');
}
/// Word is not IsSeparator or IsKomma or IsHfill or IsFloat or IsInset. 
inline bool IsWordChar(unsigned char c) {
  return !( IsSeparatorChar( c ) 
	    ||  IsKommaChar( c )  
	    ||  IsHfillChar( c )  
	    ||  IsFloatChar( c )  
	    ||  IsInsetChar( c ) ) ;
}
#endif

// -*- C++ -*-
/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 * 	 
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2000 The LyX Team.
 *
 * ====================================================== */

#ifndef ENCODING_H
#define ENCODING_H

#include "LString.h"
#include "lyxrc.h"

typedef unsigned short int Uchar;

class Encoding {
public:
	///
	enum Letter_Form {
		FORM_ISOLATED,
		FORM_FINAL,
		FORM_INITIAL,
		FORM_MEDIAL
	};
	///
	Encoding(string const & l, Uchar const * e)
		: LatexName_(l) , encoding_table(e) {}
	///
	string const & LatexName() const {
		return LatexName_;
	}
	///
	Uchar ucs(unsigned char c) const {
		return encoding_table[c];
	}
	///
	static
	bool IsComposeChar_hebrew(unsigned char c);
	///
	static
	bool IsComposeChar_arabic(unsigned char c);
	///
	static
	bool is_arabic(unsigned char c);
	///
	static
	unsigned char TransformChar(unsigned char c, Letter_Form form);
private:
	///
	string LatexName_;
	///
	Uchar const * encoding_table;
};


extern Encoding iso8859_1;
extern Encoding iso8859_2;
extern Encoding iso8859_3;
extern Encoding iso8859_4;
extern Encoding iso8859_6;
extern Encoding iso8859_7;
extern Encoding iso8859_9;
extern Encoding cp1255;
extern Encoding koi8;
extern Encoding symbol_encoding;

inline
bool Encoding::IsComposeChar_hebrew(unsigned char c)
{
	return c <= 0xd2 && c >= 0xc0 &&
		c != 0xce && c != 0xd0;
}


inline
bool Encoding::IsComposeChar_arabic(unsigned char c)
{
	return c >= 0xeb && c <= 0xf2;
}


extern unsigned char arabic_table[][2];
extern unsigned char arabic_table2[][4];
unsigned char const arabic_start = 0xc1;


inline
bool Encoding::is_arabic(unsigned char c)
{
	return c >= arabic_start && arabic_table[c-arabic_start][0];
}


inline
unsigned char Encoding::TransformChar(unsigned char c, Encoding::Letter_Form form)
{
	if (!is_arabic(c))
		return c;

	if (lyxrc.font_norm == "iso10646-1")
		return arabic_table2[c-arabic_start][form];
	else
		return arabic_table[c-arabic_start][form >> 1];
}


#endif

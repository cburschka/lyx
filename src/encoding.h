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

#ifdef __GNUG__
#pragma interface
#endif

#include "LString.h"
#include "lyxrc.h"

///
typedef unsigned short int Uchar;

///
class Encoding {
public:
	///
	enum Letter_Form {
		///
		FORM_ISOLATED,
		///
		FORM_FINAL,
		///
		FORM_INITIAL,
		///
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

///
extern Encoding iso8859_1;
///
extern Encoding iso8859_2;
///
extern Encoding iso8859_3;
///
extern Encoding iso8859_4;
///
extern Encoding iso8859_6;
///
extern Encoding iso8859_7;
///
extern Encoding iso8859_9;
///
extern Encoding cp1255;
///
extern Encoding koi8;
///
extern Encoding symbol_encoding;

#endif

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
	Encoding() {}
	///
	Encoding(string const & n, string const & l, Uchar const * e)
		: Name_(n), LatexName_(l) {
		for (int i = 0; i < 256; ++i)
			encoding_table[i] = e[i];
	}
	///
	string const & Name() const {
		return Name_;
	}
	///
	string const & LatexName() const {
		return LatexName_;
	}
	///
	Uchar ucs(unsigned char c) const {
		return encoding_table[c];
	}
private:
	///
	string Name_;
	///
	string LatexName_;
	///
	Uchar encoding_table[256];
};

extern Encoding symbol_encoding;

class Encodings {
public:
	///
	typedef std::map<string, Encoding> EncodingList;
	///
	Encodings();
	///
	void read(string const & filename);
	///
	Encoding const * getEncoding(string const & encoding) const;
	///
	Encoding const * symbol_encoding() {
		return &symbol_encoding_;
	}

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
	EncodingList encodinglist;
	///
	Encoding symbol_encoding_;
};

extern Encodings encodings;

#endif

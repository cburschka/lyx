// -*- C++ -*-
/**
 * \file encoding.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Jean-Marc Lasgouttes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef ENCODING_H
#define ENCODING_H

#include <map>
#include <string>

#include "support/types.h"

namespace lyx {

///
class Encoding {
public:
	///
	Encoding() {}
	///
	Encoding(std::string const & n, std::string const & l, char_type const * e)
		: Name_(n), LatexName_(l)
	{
		for (int i = 0; i < 256; ++i)
			encoding_table[i] = e[i];
	}
	///
	std::string const & name() const { return Name_; }
	///
	std::string const & latexName() const { return LatexName_; }
	///
	char_type ucs(char_type c) const;
private:
	///
	std::string Name_;
	///
	std::string LatexName_;
	///
	char_type encoding_table[256];
};

extern Encoding symbol_encoding;

class Encodings {
public:
	///
	typedef std::map<std::string, Encoding> EncodingList;
	///
	Encodings();
	///
	void read(std::string const & filename);
	///
	Encoding const * getEncoding(std::string const & encoding) const;
	///
	Encoding const * symbol_encoding() { return &symbol_encoding_; }

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
	static bool isComposeChar_hebrew(char_type c);
	///
	static bool isComposeChar_arabic(char_type c);
	///
	static bool is_arabic_special(char_type c);
	///
	static bool is_arabic(char_type c);
	///
	static char_type transformChar(char_type c, Letter_Form form);

private:
	///
	EncodingList encodinglist;
	///
	Encoding symbol_encoding_;
};

extern Encodings encodings;


} // namespace lyx

#endif

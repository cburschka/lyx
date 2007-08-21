// -*- C++ -*-
/**
 * \file tex2lyx/Font.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 *
 * This class is just a dummy version of that in the main LyX source tree
 * to enable tex2lyx to use LyX's textclass classes and not have to
 * re-invent the wheel.
 *
 */

#ifndef TEX2LYX_FONT_H
#define TEX2LYX_FONT_H

namespace lyx {

class Lexer;

class Font {
public:
	/// Trick to overload constructor and make it megafast
	enum FONT_INIT1 { ALL_INHERIT };
	enum FONT_INIT3 { ALL_SANE };

	Font() {}
	explicit Font(Font::FONT_INIT1) {}
	explicit Font(Font::FONT_INIT3) {}

	Font & lyxRead(Lexer &);

	Font & realize(Font const &) { return *this; }
	bool resolved() const { return true; }
};


} // namespace lyx

#endif // TEX2LYX_FONT_H

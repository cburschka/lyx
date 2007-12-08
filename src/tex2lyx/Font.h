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

//#include "FontInfo.h"

namespace lyx {

class Lexer;

class FontInfo
{
public:
	FontInfo() {}
	FontInfo & realize(FontInfo const &) { return *this; }
	void setColor(int) {}
	bool resolved() const { return true; }
};

/// Sane font.
extern FontInfo const sane_font;
/// All inherit font.
extern FontInfo const inherit_font;
/// All ignore font.
extern FontInfo const ignore_font;

class Font
{
public:
	Font() {}
	Font(FontInfo const &) {}
};

/// Read a font specification from Lexer. Used for layout files.
FontInfo lyxRead(Lexer &, FontInfo const & fi = sane_font);

} // namespace lyx

#endif // TEX2LYX_FONT_H

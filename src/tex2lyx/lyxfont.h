// -*- C++ -*-
/**
 * \file tex2lyx/lyxfont.h
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

#ifndef LYXFONT_H
#define LYXFONT_H

class LyXLex;

class LyXFont {
public:
	/// Trick to overload constructor and make it megafast
	enum FONT_INIT1 { ALL_INHERIT };
	enum FONT_INIT3 { ALL_SANE };

	LyXFont() {}
	explicit LyXFont(LyXFont::FONT_INIT1) {}
	explicit LyXFont(LyXFont::FONT_INIT3) {}

	LyXFont & lyxRead(LyXLex &);

	LyXFont & realize(LyXFont const &) { return *this; }
	bool resolved() const { return true; }
};

#endif // NOT LYXFONT_H

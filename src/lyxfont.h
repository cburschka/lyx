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

#ifndef LYXFONT_H
#define LYXFONT_H

#ifdef __GNUG__
#pragma interface
#endif

#include <iosfwd>

#include "LString.h"
#include "language.h"
#include "LColor.h"

// It might happen that locale.h defines ON and OFF. This is not good
// for us, since we use these names below. But of course this is due
// to some old compilers. Than is broken when it comes to C++ scoping.
#include "gettext.h" // so that we are sure tht it won't be included
// later. 
#ifdef ON
#undef ON
#endif

#ifdef OFF
#undef OFF
#endif

class LyXLex;

///
class LyXFont {
public:
	/** The value INHERIT_* means that the font attribute is
	    inherited from the layout. In the case of layout fonts, the
	    attribute is inherited from the default font.
	    The value IGNORE_* is used with LyXFont::update() when the
	    attribute should not be changed.
	*/
	enum FONT_FAMILY {
		///
		ROMAN_FAMILY, // fontstruct rely on this to be 0
		///
		SANS_FAMILY,
		///
		TYPEWRITER_FAMILY,
		///
		SYMBOL_FAMILY,
		///
		INHERIT_FAMILY,
		///
		IGNORE_FAMILY
	};

	///
	enum FONT_SERIES {
		///
		MEDIUM_SERIES, // fontstruct rely on this to be 0
		///
		BOLD_SERIES,
		///
		INHERIT_SERIES,
		///
		IGNORE_SERIES
	};

	///
	enum FONT_SHAPE {
		///
		UP_SHAPE, // fontstruct rely on this to be 0
		///
		ITALIC_SHAPE,
		///
		SLANTED_SHAPE,
		///
		SMALLCAPS_SHAPE,
		///
		INHERIT_SHAPE,
		///
		IGNORE_SHAPE
	};

	///
	enum FONT_SIZE {
		///
		SIZE_TINY, // fontstruct rely on this to be 0
		///
		SIZE_SCRIPT,
		///
		SIZE_FOOTNOTE,
		///
		SIZE_SMALL,
		///
		SIZE_NORMAL,
		///
		SIZE_LARGE,
		///
		SIZE_LARGER,
		///
		SIZE_LARGEST,
		///
		SIZE_HUGE,
		///
		SIZE_HUGER,
		///
		INCREASE_SIZE,
		///
		DECREASE_SIZE,
		///
		INHERIT_SIZE,
		///
		IGNORE_SIZE
	};
 
	/// Used for emph, underbar, noun and latex toggles
	enum FONT_MISC_STATE {
		///
		OFF,
		///
		ON,
		///
		TOGGLE,
		///
		INHERIT,
		///
		IGNORE
	};

	/// Trick to overload constructor and make it megafast
	enum FONT_INIT1 {
		///
		ALL_INHERIT
	};
	///
	enum FONT_INIT2 {
		///
		ALL_IGNORE
	};
	///
	enum FONT_INIT3 {
		///
		ALL_SANE
	};

	///
	LyXFont();

	/// LyXFont x(LyXFont ...) and LyXFont x = LyXFont ...
	LyXFont(LyXFont const & x);
 
	/// Shortcut initialization
	LyXFont(LyXFont::FONT_INIT1);
	/// Shortcut initialization
	LyXFont(LyXFont::FONT_INIT2);
	/// Shortcut initialization
	LyXFont(LyXFont::FONT_INIT3);
	/// Shortcut initialization
	LyXFont(LyXFont::FONT_INIT1, Language const * l);
	/// Shortcut initialization
	LyXFont(LyXFont::FONT_INIT2, Language const * l);
	/// Shortcut initialization
	LyXFont(LyXFont::FONT_INIT3, Language const * l);

	/// LyXFont x, y; x = y;
	LyXFont & operator=(LyXFont const & x);
 
	/// Decreases font size by one
	LyXFont & decSize();
 
	/// Increases font size by one
	LyXFont & incSize();
 
	///
	FONT_FAMILY family() const;
 
	///
	FONT_SERIES series() const;
 
	///
	FONT_SHAPE shape() const;
 
	///
	FONT_SIZE size() const;
 
	///
	FONT_MISC_STATE emph() const;
 
	///
	FONT_MISC_STATE underbar() const;
 
	///
	FONT_MISC_STATE noun() const;

	///
	FONT_MISC_STATE latex() const;

	///
	LColor::color color() const;

 	///
	Language const * language() const;

	///
	bool isRightToLeft() const;

	///
	bool isVisibleRightToLeft() const;
	
	///
	LyXFont & setFamily(LyXFont::FONT_FAMILY f);
	///
	LyXFont & setSeries(LyXFont::FONT_SERIES s);
	///
	LyXFont & setShape(LyXFont::FONT_SHAPE s);
	///
	LyXFont & setSize(LyXFont::FONT_SIZE s);
	///
	LyXFont & setEmph(LyXFont::FONT_MISC_STATE e);
	///
	LyXFont & setUnderbar(LyXFont::FONT_MISC_STATE u);
	///
	LyXFont & setNoun(LyXFont::FONT_MISC_STATE n);
	///
	LyXFont & setLatex(LyXFont::FONT_MISC_STATE l);
	///
	LyXFont & setColor(LColor::color c);
 	///
	LyXFont & setLanguage(Language const * l);

	/// Set family after LyX text format
	LyXFont & setLyXFamily(string const &);
 
	/// Set series after LyX text format
	LyXFont & setLyXSeries(string const &);
 
	/// Set shape after LyX text format
	LyXFont & setLyXShape(string const &);
 
	/// Set size after LyX text format
	LyXFont & setLyXSize(string const &);
 
	/// Returns misc flag after LyX text format
	LyXFont::FONT_MISC_STATE setLyXMisc(string const &);

	/// Sets color after LyX text format
	LyXFont & setLyXColor(string const &);
 
	/// Sets size after GUI name
	LyXFont & setGUISize(string const &);
 
	/// Returns size of font in LaTeX text notation
	string latexSize() const;
 
	/** Updates font settings according to request. If an
	    attribute is IGNORE, the attribute is left as it is. */
 	/* 
 	 * When toggleall = true, all properties that matches the font in use
 	 * will have the effect that the properties is reset to the
 	 * default.  If we have a text that is TYPEWRITER_FAMILY, and is
 	 * update()'ed with TYPEWRITER_FAMILY, the operation will be as if
 	 * a INHERIT_FAMILY was asked for.  This is necessary for the
 	 * toggle-user-defined-style button on the toolbar.
 	 */
 	void update(LyXFont const & newfont, 
		    Language const * default_lang,
		    bool toggleall = false);
 
	/** Reduce font to fall back to template where possible.
	    Equal fields are reduced to INHERIT */
	void reduce(LyXFont const & tmplt);
 
	/// Realize font from a template (INHERIT are realized)
	LyXFont & realize(LyXFont const & tmplt);

	/// Is a given font fully resolved?
	bool resolved() const;
 
	/// Read a font specification from LyXLex. Used for layout files.
	LyXFont & lyxRead(LyXLex &);
 
	/// Writes the changes from this font to orgfont in .lyx format in file
	void lyxWriteChanges(LyXFont const & orgfont, std::ostream &) const;

	/** Writes the head of the LaTeX needed to change to this font.
	    Writes to string, the head of the LaTeX needed to change
	    to this font. Returns number of chars written. Base is the
	    font state active now.
	*/
	int latexWriteStartChanges(std::ostream &, LyXFont const & base,
				   LyXFont const & prev) const;

	/** Writes tha tail of the LaTeX needed to chagne to this font.
	    Returns number of chars written. Base is the font state we want
	    to achieve.
	*/
	int latexWriteEndChanges(std::ostream &, LyXFont const & base,
				 LyXFont const & next) const;

	/// Build GUI description of font state
	string stateText() const;

	///
	LColor::color realColor() const;

	///
	friend
	bool operator==(LyXFont const & font1, LyXFont const & font2) {
		return font1.bits == font2.bits &&
			font1.lang == font2.lang;
	}

	///
	friend 
	bool operator!=(LyXFont const & font1, LyXFont const & font2) {
		return font1.bits != font2.bits ||
			font1.lang != font2.lang;
	}

	/// compares two fonts, ignoring the setting of the Latex part.
	bool equalExceptLatex(LyXFont const &) const;

	/// Converts logical attributes to concrete shape attribute
	LyXFont::FONT_SHAPE realShape() const;
private:
	///
	struct FontBits {
		///
		bool operator==(FontBits const & fb1) const;
		///
		bool operator!=(FontBits const & fb1) const;
		///
		FONT_FAMILY family;
		///
		FONT_SERIES series;
		///
		FONT_SHAPE shape;
		///
		FONT_SIZE size;
		///
		LColor::color color;
		///
		FONT_MISC_STATE emph;
		///
		FONT_MISC_STATE underbar;
		///
		FONT_MISC_STATE noun;
		///
		FONT_MISC_STATE latex;
	};
	///
	FontBits bits;
	///
	Language const * lang;
	
	/// Sane font
	static FontBits sane;
	
	/// All inherit font
	static FontBits inherit;
 
	/// All ignore font
	static FontBits ignore;

	/// Updates a misc setting according to request
	LyXFont::FONT_MISC_STATE setMisc(LyXFont::FONT_MISC_STATE newfont,
					 LyXFont::FONT_MISC_STATE org);
};


std::ostream & operator<<(std::ostream &, LyXFont::FONT_MISC_STATE);


inline
LyXFont::LyXFont()
{
	bits = sane;
	lang = default_language;
}


inline
LyXFont::LyXFont(LyXFont const & x)
{
	bits = x.bits;
	lang = x.lang;
}


inline
LyXFont::LyXFont(LyXFont::FONT_INIT1)
{
	bits = inherit;
	lang = default_language;
}


inline
LyXFont::LyXFont(LyXFont::FONT_INIT2)
{
	bits = ignore;
	lang = ignore_language;
}


inline
LyXFont::LyXFont(LyXFont::FONT_INIT3)
{
	bits = sane;
	lang = default_language;
}
inline
LyXFont::LyXFont(LyXFont::FONT_INIT1, Language const * l)
{
	bits = inherit;
	lang = l;
}


inline
LyXFont::LyXFont(LyXFont::FONT_INIT2, Language const * l)
{
	bits = ignore;
	lang = l;
}


inline
LyXFont::LyXFont(LyXFont::FONT_INIT3, Language const * l)
{
	bits = sane;
	lang = l;
}

inline
LyXFont & LyXFont::operator=(LyXFont const & x) 
{
	bits = x.bits;
	lang = x.lang;
	return *this;
}


inline
LyXFont::FONT_FAMILY LyXFont::family() const 
{
	return bits.family;
}


inline
LyXFont::FONT_SERIES LyXFont::series() const
{
	return bits.series;
}


inline
LyXFont::FONT_SHAPE LyXFont::shape() const
{
	return bits.shape;
}


inline
LyXFont::FONT_SIZE LyXFont::size() const
{
	return bits.size;
}


inline
LyXFont::FONT_MISC_STATE LyXFont::emph() const
{
	return bits.emph;
}


inline
LyXFont::FONT_MISC_STATE LyXFont::underbar() const
{
	return bits.underbar;
}


inline
LyXFont::FONT_MISC_STATE LyXFont::noun() const
{
	return bits.noun;
}


inline
LyXFont::FONT_MISC_STATE LyXFont::latex() const 
{
	return bits.latex;
}


inline
LColor::color LyXFont::color() const 
{
	return bits.color;
}


inline
Language const * LyXFont::language() const 
{
	return lang;
}


inline
bool LyXFont::isRightToLeft() const 
{
	return lang->RightToLeft;
}


inline
bool LyXFont::isVisibleRightToLeft() const 
{
	return (lang->RightToLeft && latex() != ON);
}


inline
LyXFont & LyXFont::setFamily(LyXFont::FONT_FAMILY f)
{
	bits.family = f;
	return *this;
}


inline
LyXFont & LyXFont::setSeries(LyXFont::FONT_SERIES s)
{
	bits.series = s;
	return *this;
}


inline
LyXFont & LyXFont::setShape(LyXFont::FONT_SHAPE s)
{
	bits.shape = s;
	return *this;
}


inline
LyXFont & LyXFont::setSize(LyXFont::FONT_SIZE s)
{
	bits.size = s;
	return *this;
}


inline
LyXFont & LyXFont::setEmph(LyXFont::FONT_MISC_STATE e)
{
	bits.emph = e;
	return *this;
}


inline
LyXFont & LyXFont::setUnderbar(LyXFont::FONT_MISC_STATE u)
{
	bits.underbar = u;
	return *this;
}


inline
LyXFont & LyXFont::setNoun(LyXFont::FONT_MISC_STATE n)
{
	bits.noun = n;
	return *this;
}

inline
LyXFont & LyXFont::setLatex(LyXFont::FONT_MISC_STATE l)
{
	bits.latex = l;
	return *this;
}


inline
LyXFont & LyXFont::setColor(LColor::color c)
{
	bits.color = c;
	return *this;
}


inline
LyXFont & LyXFont::setLanguage(Language const * l)
{
	lang = l;
	return *this;
}

#endif

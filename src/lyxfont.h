// -*- C++ -*-
/**
 * \file src/lyxfont.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Jean-Marc Lasgouttes
 * \author Angus Leeming
 * \author Dekel Tsur
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef LYXFONT_H
#define LYXFONT_H

#include <iosfwd>
#include <string>

class LColor_color;
class LyXLex;
class BufferParams;
class Language;


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
		CMR_FAMILY,
		///
		CMSY_FAMILY,
		///
		CMM_FAMILY,
		///
		CMEX_FAMILY,
		///
		MSA_FAMILY,
		///
		MSB_FAMILY,
		///
		EUFRAK_FAMILY,
		///
		WASY_FAMILY,
		///
		INHERIT_FAMILY,
		///
		IGNORE_FAMILY,
		///
		NUM_FAMILIES = INHERIT_FAMILY
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

	/// Shortcut initialization
	explicit
	LyXFont(LyXFont::FONT_INIT1);
	/// Shortcut initialization
	explicit
	LyXFont(LyXFont::FONT_INIT2);
	/// Shortcut initialization
	explicit
	LyXFont(LyXFont::FONT_INIT3);
	/// Shortcut initialization
	LyXFont(LyXFont::FONT_INIT1, Language const * l);
	/// Shortcut initialization
	LyXFont(LyXFont::FONT_INIT2, Language const * l);
	/// Shortcut initialization
	LyXFont(LyXFont::FONT_INIT3, Language const * l);

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
	FONT_MISC_STATE number() const;

	///
	LColor_color color() const;

	///
	Language const * language() const;

	///
	bool isRightToLeft() const;

	///
	bool isVisibleRightToLeft() const;

	///
	bool isSymbolFont() const;

	///
	void setFamily(LyXFont::FONT_FAMILY f);
	void setSeries(LyXFont::FONT_SERIES s);
	void setShape(LyXFont::FONT_SHAPE s);
	void setSize(LyXFont::FONT_SIZE s);
	void setEmph(LyXFont::FONT_MISC_STATE e);
	void setUnderbar(LyXFont::FONT_MISC_STATE u);
	void setNoun(LyXFont::FONT_MISC_STATE n);
	void setNumber(LyXFont::FONT_MISC_STATE n);
	void setColor(LColor_color c);
	void setLanguage(Language const * l);

	/// Set family after LyX text format
	LyXFont & setLyXFamily(std::string const &);

	/// Set series after LyX text format
	LyXFont & setLyXSeries(std::string const &);

	/// Set shape after LyX text format
	LyXFont & setLyXShape(std::string const &);

	/// Set size after LyX text format
	LyXFont & setLyXSize(std::string const &);

	/// Returns misc flag after LyX text format
	LyXFont::FONT_MISC_STATE setLyXMisc(std::string const &);

	/// Sets color after LyX text format
	LyXFont & setLyXColor(std::string const &);

	/// Returns size of font in LaTeX text notation
	std::string const latexSize() const;

	/** Updates font settings according to request.
	    If an attribute is IGNORE, the attribute is left as it is.
	    When toggleall = true, all properties that matches the font in use
	    will have the effect that the properties is reset to the
	    default.  If we have a text that is TYPEWRITER_FAMILY, and is
	    update()'ed with TYPEWRITER_FAMILY, the operation will be as if
	    a INHERIT_FAMILY was asked for.  This is necessary for the
	    toggle-user-defined-style button on the toolbar.
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
	std::string const stateText(BufferParams * params) const;

	///
	LColor_color realColor() const;

	///
	friend
	bool operator==(LyXFont const & font1, LyXFont const & font2);

	/// Converts logical attributes to concrete shape attribute
	LyXFont::FONT_SHAPE realShape() const;

	/** Compaq cxx 6.5 requires that the definition be public so that
	    it can compile operator==()
	 */
	struct FontBits {
		///
		FONT_FAMILY family;
		///
		FONT_SERIES series;
		///
		FONT_SHAPE shape;
		///
		FONT_SIZE size;
		/** We store the LColor::color value as an int to get LColor.h out
		 *  of the header file.
		 */
		int color;
		///
		FONT_MISC_STATE emph;
		///
		FONT_MISC_STATE underbar;
		///
		FONT_MISC_STATE noun;
		///
		FONT_MISC_STATE number;
	};
private:

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


/** \c LyXFont_size is a wrapper for LyXFont::FONT_SIZE.
 *  It can be forward-declared and passed as a function argument without
 *  having to expose lyxfont.h.
 */
class LyXFont_size {
        LyXFont::FONT_SIZE val_;
public:
	LyXFont_size(LyXFont::FONT_SIZE val) : val_(val) {}
        operator LyXFont::FONT_SIZE() const{ return val_; }
};



inline
LyXFont::FONT_SHAPE LyXFont::shape() const
{
	return bits.shape;
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
LyXFont::FONT_MISC_STATE LyXFont::noun() const
{
	return bits.noun;
}


inline
bool LyXFont::isSymbolFont() const
{
	switch (family()) {
	case LyXFont::SYMBOL_FAMILY:
	case LyXFont::CMSY_FAMILY:
	case LyXFont::CMM_FAMILY:
	case LyXFont::CMEX_FAMILY:
	case LyXFont::MSA_FAMILY:
	case LyXFont::MSB_FAMILY:
	case LyXFont::WASY_FAMILY:
		return true;
	default:
		return false;
	}
}

///
std::ostream & operator<<(std::ostream &, LyXFont::FONT_MISC_STATE);

bool operator==(LyXFont::FontBits const & lhs, LyXFont::FontBits const & rhs);

inline
bool operator!=(LyXFont::FontBits const & lhs, LyXFont::FontBits const & rhs)
{
	return !(lhs == rhs);
}

///
inline
bool operator==(LyXFont const & font1, LyXFont const & font2)
{
	return font1.bits == font2.bits &&
		font1.lang == font2.lang;
}

///
inline
bool operator!=(LyXFont const & font1, LyXFont const & font2)
{
	return !(font1 == font2);
}

#endif

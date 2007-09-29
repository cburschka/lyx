// -*- C++ -*-
/**
 * \file src/Font.h
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

#ifndef FONT_H
#define FONT_H

#ifdef TEX2LYX
#include "tex2lyx/Font.h"
#else

#include "Color.h"
#include "support/docstream.h"


namespace lyx {

class Lexer;
class BufferParams;
class Language;
class OutputParams;

///
class Font {
public:
	/** The value INHERIT_* means that the font attribute is
	    inherited from the layout. In the case of layout fonts, the
	    attribute is inherited from the default font.
	    The value IGNORE_* is used with Font::update() when the
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
		ESINT_FAMILY,
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

	struct FontBits {
		///
		FONT_FAMILY family;
		///
		FONT_SERIES series;
		///
		FONT_SHAPE shape;
		///
		FONT_SIZE size;
		/** We store the Color::color value as an int to get Color.h out
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
	///
	Font();

	/// Shortcut initialization
	explicit Font(Font::FONT_INIT1);
	/// Shortcut initialization
	explicit Font(Font::FONT_INIT2);
	/// Shortcut initialization
	explicit Font(Font::FONT_INIT3);
	/// Shortcut initialization
	Font(Font::FONT_INIT1, Language const * l);
	/// Shortcut initialization
	Font(Font::FONT_INIT2, Language const * l);
	/// Shortcut initialization
	Font(Font::FONT_INIT3, Language const * l);

	/// Decreases font size by one
	Font & decSize();
	/// Increases font size by one
	Font & incSize();
	///
	FONT_FAMILY family() const { return bits.family; }
	///
	FONT_SERIES series() const { return bits.series; }
	///
	FONT_SHAPE shape() const { return bits.shape; }
	///
	FONT_SIZE size() const { return bits.size; }
	///
	FONT_MISC_STATE emph() const { return bits.emph; }
	///
	FONT_MISC_STATE underbar() const { return bits.underbar; }
	///
	FONT_MISC_STATE noun() const { return bits.noun; }
	///
	FONT_MISC_STATE number() const { return bits.number; }
	///
	Color_color color() const;
	///
	Language const * language() const { return lang; }
	///
	bool isRightToLeft() const;
	///
	bool isVisibleRightToLeft() const;
	///
	bool isSymbolFont() const;

	///
	void setFamily(Font::FONT_FAMILY f);
	void setSeries(Font::FONT_SERIES s);
	void setShape(Font::FONT_SHAPE s);
	void setSize(Font::FONT_SIZE s);
	void setEmph(Font::FONT_MISC_STATE e);
	void setUnderbar(Font::FONT_MISC_STATE u);
	void setNoun(Font::FONT_MISC_STATE n);
	void setNumber(Font::FONT_MISC_STATE n);
	void setColor(Color_color c);
	void setLanguage(Language const * l);

	/// Set family after LyX text format
	Font & setLyXFamily(std::string const &);

	/// Set series after LyX text format
	Font & setLyXSeries(std::string const &);

	/// Set shape after LyX text format
	Font & setLyXShape(std::string const &);

	/// Set size after LyX text format
	Font & setLyXSize(std::string const &);

	/// Returns misc flag after LyX text format
	Font::FONT_MISC_STATE setLyXMisc(std::string const &);

	/// Sets color after LyX text format
	Font & setLyXColor(std::string const &);

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
	void update(Font const & newfont,
		    Language const * default_lang,
		    bool toggleall = false);

	/** Reduce font to fall back to template where possible.
	    Equal fields are reduced to INHERIT */
	void reduce(Font const & tmplt);

	/// Realize font from a template (INHERIT are realized)
	Font & realize(Font const & tmplt);
	/// Is a given font fully resolved?
	bool resolved() const;

	/// Read a font specification from Lexer. Used for layout files.
	Font & lyxRead(Lexer &);

	/// Writes the changes from this font to orgfont in .lyx format in file
	void lyxWriteChanges(Font const & orgfont, std::ostream &) const;

	/** Writes the head of the LaTeX needed to change to this font.
	    Writes to string, the head of the LaTeX needed to change
	    to this font. Returns number of chars written. Base is the
	    font state active now.
	*/
	int latexWriteStartChanges(odocstream &, BufferParams const & bparams,
				   OutputParams const & runparams,
				   Font const & base,
				   Font const & prev) const;

	/** Writes the tail of the LaTeX needed to change to this font.
	    Returns number of chars written. Base is the font state we want
	    to achieve.
	*/
	int latexWriteEndChanges(odocstream &, BufferParams const & bparams,
				 OutputParams const & runparams,
				 Font const & base,
				 Font const & next,
				 bool const & closeLanguage = true) const;


	/// Build GUI description of font state
	docstring const stateText(BufferParams * params) const;

	///
	Color_color realColor() const;

	///
	friend
	bool operator==(Font const & font1, Font const & font2);
	///
	friend
	std::ostream & operator<<(std::ostream & os, Font const & font);

	/// Converts logical attributes to concrete shape attribute
	// Try hard to inline this as it shows up with 4.6 % in the profiler.
	inline Font::FONT_SHAPE realShape() const {
		if (bits.noun == ON)
			return SMALLCAPS_SHAPE;
		if (bits.emph == ON)
			return (bits.shape == UP_SHAPE) ? ITALIC_SHAPE : UP_SHAPE;
		return bits.shape;
	}

	/// Set \param data using \param font and \param toggle.
	std::string toString(bool toggle) const;

	/// Set \param font and \param toggle using \param data. Return success.
	bool fromString(std::string const & data, bool & toggle);

	/** Compaq cxx 6.5 requires that the definition be public so that
	    it can compile operator==()
	 */
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
	Font::FONT_MISC_STATE setMisc(Font::FONT_MISC_STATE newfont,
					 Font::FONT_MISC_STATE org);
	/// Did latexWriteStartChanges open an encoding environment?
	mutable bool open_encoding_;
};


/** \c Font_size is a wrapper for Font::FONT_SIZE.
 *  It can be forward-declared and passed as a function argument without
 *  having to expose Font.h.
 */
class Font_size {
public:
	///
	Font_size(Font::FONT_SIZE val) : val_(val) {}
	///
	operator Font::FONT_SIZE() const { return val_; }
private:
	///
	Font::FONT_SIZE val_;
};



inline
bool Font::isSymbolFont() const
{
	switch (family()) {
	case Font::SYMBOL_FAMILY:
	case Font::CMSY_FAMILY:
	case Font::CMM_FAMILY:
	case Font::CMEX_FAMILY:
	case Font::MSA_FAMILY:
	case Font::MSB_FAMILY:
	case Font::WASY_FAMILY:
	case Font::ESINT_FAMILY:
		return true;
	default:
		return false;
	}
}

///
std::ostream & operator<<(std::ostream &, Font::FONT_MISC_STATE);

bool operator==(Font::FontBits const & lhs, Font::FontBits const & rhs);

inline
bool operator!=(Font::FontBits const & lhs, Font::FontBits const & rhs)
{
	return !(lhs == rhs);
}

///
inline
bool operator==(Font const & font1, Font const & font2)
{
	return font1.bits == font2.bits && font1.lang == font2.lang;
}

///
inline
bool operator!=(Font const & font1, Font const & font2)
{
	return !(font1 == font2);
}

/** Returns the current freefont, encoded as a std::string to be passed to the
 *  frontends.
 */
std::string const freefont2string();


} // namespace lyx

#endif // TEX2LYX
#endif

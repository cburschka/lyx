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

#include FORMS_H_LOCATION
#include "LString.h"
#include "debug.h"
#include "direction.h"
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

#define NEW_BITS 1

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
 
	enum FONT_DIRECTION {
		///
		LTR_DIR,
		///
		RTL_DIR,
		///
		TOGGLE_DIR,
		///
		INHERIT_DIR,
		///
		IGNORE_DIR
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

	/// LyXFont x, y; x = y;
	LyXFont& operator=(LyXFont const & x);
 
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
	FONT_DIRECTION direction() const;

	///
	LyXDirection getFontDirection() const;
	
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
	LyXFont & setDirection(LyXFont::FONT_DIRECTION d);

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
 	void update(LyXFont const & newfont, bool toggleall = false);
 
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
	void lyxWriteChanges(LyXFont const & orgfont, ostream &) const;

	/** Writes the head of the LaTeX needed to change to this font.
	    Writes to string, the head of the LaTeX needed to change
	    to this font. Returns number of chars written. Base is the
	    font state active now.
	*/
	int latexWriteStartChanges(string &, LyXFont const & base,
				   LyXFont const & prev) const;

	/** Writes tha tail of the LaTeX needed to chagne to this font.
	    Returns number of chars written. Base is the font state we want
	    to achieve.
	*/
	int latexWriteEndChanges(string &, LyXFont const & base,
				 LyXFont const & next) const;
 
	/// Build GUI description of font state
	string stateText() const;

	///
	int maxAscent() const; 

	///
	int maxDescent() const;

	///
	int ascent(char c) const;

	///
	int descent(char c) const;

	///
	int width(char c) const;

	///
	int lbearing(char c) const;

	///
	int rbearing(char c) const;
	
	///
	int textWidth(char const *s, int n) const;

	///
	int stringWidth(string const & s) const;

	///
	int signedStringWidth(string const & s) const;

	/// Draws text and returns width of text
	int drawText(char const *, int n, Pixmap, int baseline, int x) const;

	///
	int drawString(string const &, Pixmap pm, int baseline, int x) const;

	///
	LColor::color realColor() const;

	///
	XID getFontID() const {
		return getXFontstruct()->fid;
	}
	
	///
	friend inline
	bool operator==(LyXFont const & font1, LyXFont const & font2) {
		return font1.bits == font2.bits;
	}

	///
	friend inline
	bool operator!=(LyXFont const & font1, LyXFont const & font2) {
		return font1.bits != font2.bits;
	}

	/// compares two fonts, ignoring the setting of the Latex part.
	bool equalExceptLatex(LyXFont const &) const;

private:
#ifdef NEW_BITS
	///
	struct FontBits {
		bool operator==(FontBits const & fb1) const {
			return fb1.family == family &&
				fb1.series == series &&
				fb1.shape == shape &&
				fb1.size == size &&
				fb1.color == color &&
				fb1.emph == emph &&
				fb1.underbar == underbar &&
				fb1.noun == noun &&
				fb1.latex == latex &&
				fb1.direction == direction;
		}
		bool operator!=(FontBits const & fb1) const {
			return !(fb1 == *this);
		}
		
		FONT_FAMILY family;
		FONT_SERIES series;
		FONT_SHAPE shape;
		FONT_SIZE size;
		LColor::color color;
		FONT_MISC_STATE emph;
		FONT_MISC_STATE underbar;
		FONT_MISC_STATE noun;
		FONT_MISC_STATE latex;
		FONT_DIRECTION direction;
	};
#else
	/// This have to be at least 32 bits, but 64 or more does not hurt
	typedef unsigned int ui32;
#endif

	/** Representation: bit table
	    Layout of bit table:
	        11 1111 111 122 222 222 2233
	    Bit 012 34 567 8901 2345 678 901 234 567 8901
	        FFF SS SSS SSSS CCCC EEE UUU NNN LLL
	        aaa ee hhh iiii oooo mmm nnn ooo aaa
	        mmm rr aaa zzzz llll ppp ddd uuu ttt

	    Bit 76543210 76543210 76543210 76543210
                                                --- Fam_Pos
					      --    Ser_Pos
					   ---      Sha_Pos
	 			      ----          Siz_Pos
				  ----              Col_Pos
			      ---                   Emp_Pos
			   ---                      Und_Pos
		       - --                         Nou_Pos
		    ---                             Lat_Pos
		----                                Dir_Pos
		
	    Some might think this is a dirty representation, but it gives
	    us at least 25% speed-up, so why not? (Asger)

	    First of all it is a maintence nightmare...and now that we need
	    to enlarge the Color bits with 2 (from 4 to 6), we have a problem
	    since a 32 bit entity is not large enough... (Lgb)
	*/

#ifdef NEW_BITS
	FontBits bits;
#else
	ui32 bits;
	
	///
	enum FONT_POSITION {
		///
		Fam_Pos =  0,
		///
		Ser_Pos =  3,
		///
		Sha_Pos =  5,
		///
		Siz_Pos =  8,
		///
		Col_Pos = 12,
		///
		Emp_Pos = 16,
		///
		Und_Pos = 19,
		///
		Nou_Pos = 22,
		///
		Lat_Pos = 25,
		///
		Dir_Pos = 28
	};

	///
	enum FONT_MASK {
		///
		Fam_Mask = 0x07,
		///
		Ser_Mask = 0x03,
		///
		Sha_Mask = 0x07,
		///
		Siz_Mask = 0x0f,
		///
		Col_Mask = 0x0f,
		///
		Dir_Mask = 0x07,
		///
		Misc_Mask = 0x07
	};
#endif

	
#ifdef NEW_BITS
	/// Sane font
	static FontBits sane;
	
	/// All inherit font
	static FontBits inherit;
 
	/// All ignore font
	static FontBits ignore;

#else
	/// Sane font
	enum {	sane = ui32(ROMAN_FAMILY) << Fam_Pos
		| ui32(MEDIUM_SERIES) << Ser_Pos
		| ui32(UP_SHAPE) << Sha_Pos
		| ui32(SIZE_NORMAL) << Siz_Pos
		| ui32(LColor::none) << Col_Pos
		| ui32(OFF) << Emp_Pos
		| ui32(OFF) << Und_Pos
		| ui32(OFF) << Nou_Pos
		| ui32(OFF) << Lat_Pos
		| ui32(LTR_DIR) << Dir_Pos};
	
	/// All inherit font
	enum{ inherit = ui32(INHERIT_FAMILY) << Fam_Pos
		      | ui32(INHERIT_SERIES) << Ser_Pos
		      | ui32(INHERIT_SHAPE) << Sha_Pos
		      | ui32(INHERIT_SIZE) << Siz_Pos
		      | ui32(LColor::inherit) << Col_Pos
		      | ui32(INHERIT) << Emp_Pos
		      | ui32(INHERIT) << Und_Pos
		      | ui32(INHERIT) << Nou_Pos
		      | ui32(INHERIT) << Lat_Pos
		      | ui32(INHERIT_DIR) << Dir_Pos};
 
	/// All ignore font
	enum{ ignore = ui32(IGNORE_FAMILY) << Fam_Pos
		      | ui32(IGNORE_SERIES) << Ser_Pos
		      | ui32(IGNORE_SHAPE) << Sha_Pos
		      | ui32(IGNORE_SIZE) << Siz_Pos
		      | ui32(LColor::ignore) << Col_Pos
		      | ui32(IGNORE) << Emp_Pos
		      | ui32(IGNORE) << Und_Pos
		      | ui32(IGNORE) << Nou_Pos
		      | ui32(IGNORE) << Lat_Pos
		      | ui32(IGNORE_DIR) << Dir_Pos};
#endif
	/// Updates a misc setting according to request
	LyXFont::FONT_MISC_STATE setMisc(LyXFont::FONT_MISC_STATE newfont,
					 LyXFont::FONT_MISC_STATE org);

	/// Converts logical attributes to concrete shape attribute
	LyXFont::FONT_SHAPE realShape() const;

	///
	XFontStruct * getXFontstruct() const;
};

ostream & operator<<(ostream &, LyXFont::FONT_MISC_STATE);

inline LyXFont::LyXFont()
{
	bits = sane;
}


inline LyXFont::LyXFont(LyXFont const & x)
{
	bits = x.bits;
}


inline LyXFont::LyXFont(LyXFont::FONT_INIT1)
{
	bits = inherit;
}


inline LyXFont::LyXFont(LyXFont::FONT_INIT2)
{
	bits = ignore;
}


inline LyXFont::LyXFont(LyXFont::FONT_INIT3)
{
	bits = sane;
}


inline LyXFont & LyXFont::operator=(LyXFont const & x) 
{
	bits = x.bits;
	return *this;
}


#ifdef NEW_BITS
// You don't have to understand the stuff below :-)
// It works, and it's bloody fast. (Asger)
// And impossible to work with. (Lgb)

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
LyXFont::FONT_DIRECTION LyXFont::direction() const 
{
	return bits.direction;
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
LyXFont & LyXFont::setDirection(LyXFont::FONT_DIRECTION d)
{
	bits.direction = d;
	return *this;
}
#else
// You don't have to understand the stuff below :-)
// It works, and it's bloody fast. (Asger)
// And impossible to work with. (Lgb)

inline LyXFont::FONT_FAMILY LyXFont::family() const 
{
	return LyXFont::FONT_FAMILY((bits >> Fam_Pos) & Fam_Mask);
}


inline LyXFont::FONT_SERIES LyXFont::series() const
{
	return LyXFont::FONT_SERIES((bits >> Ser_Pos) & Ser_Mask);
}


inline LyXFont::FONT_SHAPE LyXFont::shape() const
{
	return LyXFont::FONT_SHAPE((bits >> Sha_Pos) & Sha_Mask);
}


inline LyXFont::FONT_SIZE LyXFont::size() const
{
	return LyXFont::FONT_SIZE((bits >> Siz_Pos) & Siz_Mask);
}


inline LyXFont::FONT_MISC_STATE LyXFont::emph() const
{
	return LyXFont::FONT_MISC_STATE((bits >> Emp_Pos) & Misc_Mask);
}


inline LyXFont::FONT_MISC_STATE LyXFont::underbar() const
{
	return LyXFont::FONT_MISC_STATE((bits >> Und_Pos) & Misc_Mask);
}


inline LyXFont::FONT_MISC_STATE LyXFont::noun() const
{
	return LyXFont::FONT_MISC_STATE((bits >> Nou_Pos) & Misc_Mask);
}


inline LyXFont::FONT_MISC_STATE LyXFont::latex() const 
{
	return LyXFont::FONT_MISC_STATE((bits >> Lat_Pos) & Misc_Mask);
}


inline LColor::color LyXFont::color() const 
{
	return LColor::color((bits >> Col_Pos) & Col_Mask);
}


inline LyXFont::FONT_DIRECTION LyXFont::direction() const 
{
	return LyXFont::FONT_DIRECTION((bits >> Dir_Pos) & Dir_Mask);
}

inline LyXFont & LyXFont::setFamily(LyXFont::FONT_FAMILY f)
{
	bits &= ~(Fam_Mask << Fam_Pos);
	bits |= ui32(f) << Fam_Pos;
	return *this;
}


inline LyXFont & LyXFont::setSeries(LyXFont::FONT_SERIES s)
{
	bits &= ~(Ser_Mask << Ser_Pos);
	bits |= ui32(s) << Ser_Pos;
	return *this;
}


inline LyXFont & LyXFont::setShape(LyXFont::FONT_SHAPE s)
{
	bits &= ~(Sha_Mask << Sha_Pos);
	bits |= ui32(s) << Sha_Pos;
	return *this;
}


inline LyXFont & LyXFont::setSize(LyXFont::FONT_SIZE s)
{
	bits &= ~(Siz_Mask << Siz_Pos);
	bits |= ui32(s) << Siz_Pos;
	return *this;
}


inline LyXFont & LyXFont::setEmph(LyXFont::FONT_MISC_STATE e)
{
	bits &= ~(Misc_Mask << Emp_Pos);
	bits |= ui32(e) << Emp_Pos;
	return *this;
}


inline LyXFont & LyXFont::setUnderbar(LyXFont::FONT_MISC_STATE u)
{
	bits &= ~(Misc_Mask << Und_Pos);
	bits |= ui32(u) << Und_Pos;
	return *this;
}


inline LyXFont & LyXFont::setNoun(LyXFont::FONT_MISC_STATE n)
{
	bits &= ~(Misc_Mask << Nou_Pos);
	bits |= ui32(n) << Nou_Pos;
	return *this;
}

inline LyXFont & LyXFont::setLatex(LyXFont::FONT_MISC_STATE l)
{
	bits &= ~(Misc_Mask << Lat_Pos);
	bits |= ui32(l) << Lat_Pos;
	return *this;
}


inline LyXFont & LyXFont::setColor(LColor::color c)
{
	bits &= ~(Col_Mask << Col_Pos);
	bits |= ui32(c) << Col_Pos;
	return *this;
}


inline LyXFont & LyXFont::setDirection(LyXFont::FONT_DIRECTION d)
{
	bits &= ~(Dir_Mask << Dir_Pos);
	bits |= ui32(d) << Dir_Pos;
	return *this;
}
#endif

#endif

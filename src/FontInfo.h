// -*- C++ -*-
/**
 * \file src/FontInfo.h
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

#ifndef FONT_PROPERTIES_H
#define FONT_PROPERTIES_H

#include "Color.h"
#include "ColorCode.h"
#include "FontEnums.h"

#include "support/Changer.h"
#include "support/strfwd.h"


namespace lyx {

class Lexer;

///
class FontInfo
{
public:
	///
	FontInfo();
	///
	FontInfo(
		FontFamily family,
		FontSeries series,
		FontShape shape,
		FontSize size,
		ColorCode color,
		ColorCode background,
		FontState emph,
		FontState underbar,
		FontState strikeout,
		FontState uuline,
		FontState uwave,
		FontState noun,
		FontState number)
		: family_(family), series_(series), shape_(shape), size_(size), 
		color_(color), background_(background), paint_color_(), emph_(emph),
		underbar_(underbar), strikeout_(strikeout), uuline_(uuline),
		uwave_(uwave), noun_(noun), number_(number)
	{}

	/// Decreases font size by one
	FontInfo & decSize();
	/// Increases font size by one
	FontInfo & incSize();

	/// \name Accessor methods
	//@{
	FontFamily family() const { return family_; }
	void setFamily(FontFamily f) { family_ = f; }
	FontSeries series() const { return series_; }
	void setSeries(FontSeries s) { series_ = s; }
	FontShape shape() const { return shape_; }
	void setShape(FontShape s) { shape_ = s; }
	FontSize size() const { return size_; }
	void setSize(FontSize s) { size_ = s; }
	FontState emph() const { return emph_; }
	void setEmph(FontState e) { emph_ = e; }
	FontState underbar() const { return underbar_; }
	void setUnderbar(FontState u) { underbar_ = u; }
	FontState strikeout() const { return strikeout_; }
	void setStrikeout(FontState s) { strikeout_ = s; }
	FontState uuline() const { return uuline_; }
	void setUuline(FontState s) { uuline_ = s; }
	FontState uwave() const { return uwave_; }
	void setUwave(FontState s) { uwave_ = s; }
	FontState noun() const { return noun_; }
	void setNoun(FontState n) { noun_ = n; }
	FontState number() const { return number_; }
	void setNumber(FontState n) { number_ = n; }
	ColorCode color() const { return color_; }
	void setColor(ColorCode c) { color_ = c; }
	ColorCode background() const { return background_; }
	void setBackground(ColorCode b) { background_ = b; }
	//@}

	///
	void update(FontInfo const & newfont, bool toggleall);

	/** Reduce font to fall back to template where possible.
	    Equal fields are reduced to INHERIT */
	void reduce(FontInfo const & tmplt);

	/// Realize font from a template (INHERIT are realized)
	FontInfo & realize(FontInfo const & tmplt);
	/// Is a given font fully resolved?
	bool resolved() const;

	/// The real color of the font. This can be the color that is 
	/// set for painting, the color of the font or a default color.
	Color realColor() const;
	/// Sets the color which is used during painting
	void setPaintColor(Color c) { paint_color_ = c; }

	///
	docstring asCSS() const;

	/// Converts logical attributes to concrete shape attribute
	/// Try hard to inline this as it shows up with 4.6 % in the profiler.
	FontShape realShape() const
	{
		if (noun_ == FONT_ON)
			return SMALLCAPS_SHAPE;
		if (emph_ == FONT_ON)
			return (shape_ == ITALIC_SHAPE) ? UP_SHAPE : ITALIC_SHAPE;
		return shape_;
	}

	bool isSymbolFont() const
	{
		switch (family_) {
		case SYMBOL_FAMILY:
		case CMSY_FAMILY:
		case CMM_FAMILY:
		case CMEX_FAMILY:
		case MSA_FAMILY:
		case MSB_FAMILY:
		case STMARY_FAMILY:
		case WASY_FAMILY:
		case ESINT_FAMILY:
			return true;
		default:
			return false;
		}
	}

	/// Temporarily replace the color with \param color.
	Changer changeColor(ColorCode const color, bool cond = true);
	/// Temporarily replace the shape with \param shape.
	Changer changeShape(FontShape const shape, bool cond = true);
	/// Temporarily replace the FontInfo with \param font, and optionally
	/// \param realize the \param font against the current FontInfo.
	Changer change(FontInfo font, bool realize = false, bool cond = true);

private:
	friend bool operator==(FontInfo const & lhs, FontInfo const & rhs);

	///
	FontFamily family_;
	///
	FontSeries series_;
	///
	FontShape shape_;
	///
	FontSize size_;
	///
	ColorCode color_;
	///
	ColorCode background_;
	/// The color used for painting
	Color paint_color_;
	///
	FontState emph_;
	///
	FontState underbar_;
	///
	FontState strikeout_;
	///
	FontState uuline_;
	///
	FontState uwave_;
	///
	FontState noun_;
	///
	FontState number_;
};


inline bool operator==(FontInfo const & lhs, FontInfo const & rhs)
{
	return lhs.family_ == rhs.family_
		&& lhs.series_ == rhs.series_
		&& lhs.shape_ == rhs.shape_
		&& lhs.size_ == rhs.size_
		&& lhs.color_ == rhs.color_
		&& lhs.background_ == rhs.background_
		&& lhs.emph_ == rhs.emph_
		&& lhs.underbar_ == rhs.underbar_
		&& lhs.strikeout_ == rhs.strikeout_
		&& lhs.uuline_ == rhs.uuline_
		&& lhs.uwave_ == rhs.uwave_
		&& lhs.noun_ == rhs.noun_
		&& lhs.number_ == rhs.number_;
}


inline bool operator!=(FontInfo const & lhs, FontInfo const & rhs)
{
	return !operator==(lhs, rhs);
}

/// Sane font.
extern FontInfo const sane_font;
/// All inherit font.
extern FontInfo const inherit_font;
/// All ignore font.
extern FontInfo const ignore_font;

/// Set family after LyX text format
void setLyXFamily(std::string const &, FontInfo &);

/// Set series after LyX text format
void setLyXSeries(std::string const &, FontInfo &);

/// Set shape after LyX text format
void setLyXShape(std::string const &, FontInfo &);

/// Set size after LyX text format
void setLyXSize(std::string const &, FontInfo &);

/// Sets color after LyX text format
void setLyXColor(std::string const &, FontInfo &);

/// Returns misc flag after LyX text format
FontState setLyXMisc(std::string const &);

/// Read a font specification from Lexer. Used for layout files.
FontInfo lyxRead(Lexer &, FontInfo const & fi = sane_font);

/// Write a font specification. Used for layout files.
void lyxWrite(std::ostream &, FontInfo const &, std::string const &, int);

} // namespace lyx

#endif

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

#include "ColorCode.h"
#include "FontInfo.h"

#include "support/strfwd.h"


namespace lyx {

class Lexer;
class BufferParams;
class Language;
class LaTeXFeatures;
class OutputParams;

///
class Font {

public:
	///
	explicit Font(FontInfo = sane_font, Language const * l = 0);

	///
	FontInfo & fontInfo() { return bits_; }
	///
	FontInfo const & fontInfo() const { return bits_; }
	///
	Language const * language() const { return lang_; }
	///
	bool isRightToLeft() const;
	///
	bool isVisibleRightToLeft() const;
	///
	void setLanguage(Language const * l);

	/// Returns misc flag after LyX text format
	FontState setLyXMisc(std::string const &);


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
	void validate(LaTeXFeatures & features) const;

	///
	friend
	bool operator==(Font const & font1, Font const & font2);
	///
	friend
	std::ostream & operator<<(std::ostream & os, Font const & font);

	/// Set \param data using \param font and \param toggle.
	std::string toString(bool toggle) const;

	/// Set \param font and \param toggle using \param data. Return success.
	bool fromString(std::string const & data, bool & toggle);

private:
	///
	FontInfo bits_;
	///
	Language const * lang_;

	/// Did latexWriteStartChanges open an encoding environment?
	mutable bool open_encoding_;
};


///
inline
bool operator==(Font const & font1, Font const & font2)
{
	return font1.bits_ == font2.bits_ && font1.lang_ == font2.lang_;
}

///
inline
bool operator!=(Font const & font1, Font const & font2)
{
	return !(font1 == font2);
}

/** Returns the current freefont, encoded as a std::string to be passed to the
 *  frontends. Implemented in Text3.cpp.
 */
std::string const freefont2string();


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

/// Read a font specification from Lexer. Used for layout files.
FontInfo lyxRead(Lexer &, FontInfo const & fi = sane_font);

} // namespace lyx

#endif // TEX2LYX
#endif

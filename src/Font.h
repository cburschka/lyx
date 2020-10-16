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

#include "FontInfo.h"

#include "support/strfwd.h"


namespace lyx {

class BufferParams;
class Language;
class LaTeXFeatures;
class OutputParams;
class otexstream;

///
class Font {

public:
	///
	explicit Font(FontInfo = sane_font, Language const * l = nullptr);

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
	int latexWriteStartChanges(otexstream &, BufferParams const & bparams,
				   OutputParams const & runparams,
				   Font const & base,
				   Font const & prev,
				   bool non_inherit_inset = false,
				   bool needs_cprotection = false) const;

	/** Writes the tail of the LaTeX needed to change to this font.
	    Returns number of chars written. Base is the font state we want
	    to achieve.
	*/
	int latexWriteEndChanges(otexstream &, BufferParams const & bparams,
				 OutputParams const & runparams,
				 Font const & base,
				 Font const & next,
				 bool & needPar,
				 bool closeLanguage = true) const;


	/// Build GUI description of font state
	docstring const stateText(BufferParams * params = nullptr, bool const terse = false) const;

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

} // namespace lyx

#endif

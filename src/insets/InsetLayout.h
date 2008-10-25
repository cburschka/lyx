// -*- C++ -*-
/**
 * \file InsetLayout.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Martin Vermeer
 * \author Richard Heck
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef INSET_LAYOUT_H
#define INSET_LAYOUT_H

#include "ColorCode.h"
#include "FontInfo.h"

#include "support/docstring.h"

#include <set>
#include <string>

namespace lyx {

class Lexer;
class TextClass;

///
class InsetLayout {
public:
	///
	InsetLayout();
	///
	enum InsetDecoration {
		Classic,
		Minimalistic,
		Conglomerate,
		Default
	};
	enum InsetLyXType {
		NOLYXTYPE,
		CHARSTYLE,
		CUSTOM,
		ELEMENT,
		END,
		STANDARD
	};
	///
	bool read(Lexer & lexrc, TextClass & tclass);
	///
	docstring name() const { return name_; };
	///
	InsetLyXType lyxtype() const { return lyxtype_; };
	///
	docstring labelstring() const { return labelstring_; };
	///
	InsetDecoration decoration() const { return decoration_; };
	///
	std::string latextype() const { return latextype_; };
	///
	std::string latexname() const { return latexname_; };
	///
	std::string latexparam() const { return latexparam_; };
	///
	FontInfo font() const { return font_; };
	///
	FontInfo labelfont() const { return labelfont_; };
	///
	ColorCode bgcolor() const { return bgcolor_; };
	///
	std::string preamble() const { return preamble_; };
	///
	std::set<std::string> requires() const { return requires_; };
	///
	bool isMultiPar() const { return multipar_; };
	///
	bool forcePlainLayout() const { return forceplain_; }
	///
	bool allowParagraphCustomization() const { return custompars_; }
	///
	bool isPassThru() const { return passthru_; };
	///
	bool isNeedProtect() const { return needprotect_; };
	///
	bool isFreeSpacing() const { return freespacing_; };
	///
	bool isKeepEmpty() const { return keepempty_; };
	///
	bool isForceLtr() const { return forceltr_; };
private:
	///
	docstring name_;
	/**
		* This is only used (at present) to decide where to put them on the menus.
		* Values are 'charstyle', 'custom' (things that by default look like a
		* footnote), 'element' (docbook), 'standard'.
		*/
	InsetLyXType lyxtype_;
	///
	docstring labelstring_;
	///
	InsetDecoration decoration_;
	///
	std::string latextype_;
	///
	std::string latexname_;
	///
	std::string latexparam_;
	///
	FontInfo font_;
	///
	FontInfo labelfont_;
	///
	ColorCode bgcolor_;
	///
	std::string preamble_;
	///
	std::set<std::string> requires_;
	///
	bool multipar_;
	/// 
	bool custompars_;
	///
	bool forceplain_;
	///
	bool passthru_;
	///
	bool needprotect_;
	///
	bool freespacing_;
	///
	bool keepempty_;
	///
	bool forceltr_;
};

///
InsetLayout::InsetLyXType translateLyXType(std::string const & str);

} // namespace lyx

#endif

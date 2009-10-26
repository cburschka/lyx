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
		CLASSIC,
		MINIMALISTIC,
		CONGLOMERATE,
		DEFAULT
	};
	///
	enum InsetLyXType {
		NOLYXTYPE,
		CHARSTYLE,
		CUSTOM,
		ELEMENT,
		END,
		STANDARD
	};
	///
	enum InsetLaTeXType {
		NOLATEXTYPE,
		COMMAND,
		ENVIRONMENT,
		ILT_ERROR
	};
	///
	bool read(Lexer & lexrc, TextClass const & tclass);
	///
	docstring name() const { return name_; }
	///
	void setName(docstring const & n) { name_ = n; }
	///
	InsetLyXType lyxtype() const { return lyxtype_; }
	///
	docstring labelstring() const { return labelstring_; }
	///
	InsetDecoration decoration() const { return decoration_; }
	///
	InsetLaTeXType latextype() const { return latextype_; }
	///
	std::string latexname() const { return latexname_; }
	///
	std::string latexparam() const { return latexparam_; }
	///
	FontInfo font() const { return font_; }
	///
	FontInfo labelfont() const { return labelfont_; }
	///
	ColorCode bgcolor() const { return bgcolor_; }
	///
	docstring preamble() const { return preamble_; }
	///
	docstring counter() const { return counter_; }
	/// 
	std::string const & htmlinnertag() const { return htmlinnertag_; }
	/// 
	std::string const & htmlinnerattr() const { return htmlinnerattr_; }
	///
	std::string const & htmltag() const { return htmltag_; }
	/// 
	std::string const & htmlattr() const { return htmlattr_; }
	///
	std::string const & htmllabel() const { return htmllabel_; }
	/// 
	docstring htmlstyle() const { return htmlstyle_; }
	/// 
	docstring htmlpreamble() const { return htmlpreamble_; }
	///
	bool htmlisblock() const { return htmlisblock_; }
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
	bool forceLTR() const { return forceltr_; };
	///
	bool isInToc() const { return intoc_; };
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
	InsetLaTeXType latextype_;
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
	docstring counter_;
	///
	docstring preamble_;
	/// The tag enclosing all the material in this inset.
	std::string htmltag_;
	/// Additional attributes for inclusion with the start tag.
	std::string htmlattr_;
	/// Tag for individual paragraphs in the inset.
	std::string htmlinnertag_;
	/// Attributes for that tag.
	std::string htmlinnerattr_;
	/// A label for this environment, possibly including a reference
	/// to a counter. E.g., for footnote, it might be:
	///    <span class='notenum'>\arabic{footnote}</span>
	std::string htmllabel_;
	/// CSS associated with this inset.
	docstring htmlstyle_;
	/// Additional material for the header.
	docstring htmlpreamble_;
	/// Whether this inset represents a "block" of material, i.e., a set
	/// of paragraphs of its own (true), or should be run into the previous
	/// paragraph (false). Examples:
	///   For branches, this is false.
	///   For footnotes, this is true.
	/// Defaults to true.
	bool htmlisblock_;
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
	/// should the contents be written to TOC strings?
	bool intoc_;
};

///
InsetLayout::InsetLyXType translateLyXType(std::string const & str);

} // namespace lyx

#endif

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
#include "support/debug.h"

#include <set>
#include <string>

namespace lyx {

class Lexer;
class TextClass;

///
class InsetLayout {
public:
	// just so these can be put in containers
	// it should not really be used
	InsetLayout();
	///
	InsetLayout(docstring const & name);
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
	InsetLyXType lyxtype() const { return lyxtype_; }
	///
	docstring labelstring() const { return labelstring_; }
	///
	bool contentaslabel() const { return contentaslabel_; }
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
	std::string const & htmltag() const;
	/// 
	std::string const & htmlattr() const;
	/// 
	std::string const & htmlinnertag() const { return htmlinnertag_; }
	/// 
	std::string const & htmlinnerattr() const;
	///
	std::string const & htmllabel() const { return htmllabel_; }
	///
	inline std::string htmllabeltag() const { return "span"; }
	///
	std::string htmllabelattr() const 
		{ return "class=\"" + defaultCSSClass() + "_label\""; }
	/// 
	docstring htmlstyle() const;
	/// 
	docstring htmlpreamble() const { return htmlpreamble_; }
	///
	bool htmlisblock() const { return htmlisblock_; }
	///
	std::set<std::string> requires() const { return requires_; }
	///
	bool isMultiPar() const { return multipar_; }
	///
	bool forcePlainLayout() const { return forceplain_; }
	///
	bool allowParagraphCustomization() const { return custompars_; }
	///
	bool isPassThru() const { return passthru_; }
	///
	bool isNeedProtect() const { return needprotect_; }
	///
	bool isFreeSpacing() const { return freespacing_; }
	///
	bool isKeepEmpty() const { return keepempty_; }
	///
	bool forceLTR() const { return forceltr_; }
	///
	bool isInToc() const { return intoc_; }
	///
private:
	///
	void makeDefaultCSS() const;
	///
	std::string defaultCSSClass() const;
	///
	std::string defaultCSSLabelClass() const { return defaultCSSClass() + "_label"; }
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
	bool contentaslabel_;
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
	/// The tag enclosing all the material in this inset. Default is "span".
	mutable std::string htmltag_;
	/// Additional attributes for inclusion with the start tag. Default (if
	/// a tag is provided) is: class="name".
	mutable std::string htmlattr_;
	/// Tag for individual paragraphs in the inset. Default is none.
	std::string htmlinnertag_;
	/// Attributes for that tag. Default (if a tag is provided) is: 
	/// class="name_inner".
	mutable std::string htmlinnerattr_;
	/// A label for this environment, possibly including a reference
	/// to a counter. E.g., for footnote, it might be:
	///    \arabic{footnote}
	/// No default.
	/// FIXME Could we get this from the layout?
	std::string htmllabel_;
	/// CSS associated with this inset.
	docstring htmlstyle_;
	/// Cache for default CSS info for this inset.
	mutable docstring htmldefaultstyle_;
	/// 
	mutable std::string defaultcssclass_;
	/// Whether to force generation of default CSS even if some is given.
	/// False by default.
	bool htmlforcecss_;
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

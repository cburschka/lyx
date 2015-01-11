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
#include "Layout.h"

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
	docstring leftdelim() const { return leftdelim_; }
	///
	docstring rightdelim() const { return rightdelim_; }
	///
	FontInfo font() const { return font_; }
	///
	FontInfo labelfont() const { return labelfont_; }
	///
	ColorCode bgcolor() const { return bgcolor_; }
	/// 
	Layout::LaTeXArgMap const & latexargs() const { return latexargs_; }
	///
	Layout::LaTeXArgMap const & postcommandargs() const { return postcommandargs_; }
	/// Returns latexargs() + postcommandargs().
	/// But note that it returns a *copy*, not a reference, so do not do 
	/// anything like:
	///   Layout::LaTeXArgMap::iterator it = args().begin();
	///   Layout::LaTeXArgMap::iterator en = args().end();
	/// Those are iterators for different containers.
	Layout::LaTeXArgMap args() const;
	///
	unsigned int optArgs() const;
	///
	unsigned int requiredArgs() const;
	///
	docstring preamble() const { return preamble_; }
	/// Get language dependent macro definitions needed for this inset
	docstring const langpreamble() const { return langpreamble_; }
	/// Get language and babel dependent macro definitions needed for
	/// this inset
	docstring const babelpreamble() const { return babelpreamble_; }
	///
	bool fixedwidthpreambleencoding() const { return fixedwidthpreambleencoding_; }
	///
	docstring counter() const { return counter_; }
	///
	docstring refprefix() const { return refprefix_; }
	/// The tag enclosing all the material in this inset. Default is "span".
	std::string const & htmltag() const;
	/// Additional attributes for inclusion with the start tag. Default (if
	/// a tag is provided) is: class="name".
	std::string const & htmlattr() const;
	/// Tag for individual paragraphs in the inset. Default is none.
	std::string const & htmlinnertag() const { return htmlinnertag_; }
	/// Attributes for that tag. Default (if a tag is provided) is: 
	/// class="name_inner".
	std::string const & htmlinnerattr() const;
	/// A label for this environment, possibly including a reference
	/// to a counter. E.g., for footnote, it might be:
	///    \arabic{footnote}
	/// No default.
	/// FIXME Could we get this from the layout?
	std::string const & htmllabel() const { return htmllabel_; }
	///
	inline std::string htmllabeltag() const { return "span"; }
	///
	std::string htmllabelattr() const 
		{ return "class=\"" + defaultCSSClass() + "_label\""; }
	/// CSS associated with this inset.
	docstring htmlstyle() const;
	/// Additional material for the header.
	docstring htmlpreamble() const { return htmlpreamble_; }
	/// Whether this inset represents a "block" of material, i.e., a set
	/// of paragraphs of its own (true), or should be run into the previous
	/// paragraph (false). Examples:
	///   For branches, this is false.
	///   For footnotes, this is true.
	/// Defaults to true.
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
	bool parbreakIsNewline() const { return parbreakisnewline_; }
	///
	bool isNeedProtect() const { return needprotect_; }
	///
	bool isFreeSpacing() const { return freespacing_; }
	///
	bool isKeepEmpty() const { return keepempty_; }
	///
	bool forceLTR() const { return forceltr_; }
	///
	bool forceOwnlines() const { return forceownlines_; }
	///
	bool isInToc() const { return intoc_; }
	///
	bool spellcheck() const { return spellcheck_; }
	///
	bool resetsFont() const { return resetsfont_; }
	///
	bool isDisplay() const { return display_; }
	///
	bool forcelocalfontswitch() const { return forcelocalfontswitch_; }
	///
	docstring const & obsoleted_by() const { return obsoleted_by_; }
private:
	///
	void makeDefaultCSS() const;
	///
	std::string defaultCSSClass() const;
	///
	void readArgument(Lexer &);
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
	docstring leftdelim_;
	///
	docstring rightdelim_;
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
	/// Language dependent macro definitions needed for this inset
	docstring langpreamble_;
	/// Language and babel dependent macro definitions needed for this inset
	docstring babelpreamble_;
	///
	bool fixedwidthpreambleencoding_;
	///
	docstring refprefix_;
	///
	mutable std::string htmltag_;
	///
	mutable std::string htmlattr_;
	///
	std::string htmlinnertag_;
	///
	mutable std::string htmlinnerattr_;
	///
	std::string htmllabel_;
	///
	docstring htmlstyle_;
	/// Cache for default CSS info for this inset.
	mutable docstring htmldefaultstyle_;
	/// Cache for default CSS class.
	mutable std::string defaultcssclass_;
	/// Whether to force generation of default CSS even if some is given.
	/// False by default.
	bool htmlforcecss_;
	///
	docstring htmlpreamble_;
	///
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
	bool parbreakisnewline_;
	///
	bool freespacing_;
	///
	bool keepempty_;
	///
	bool forceltr_;
	///
	bool forceownlines_;
	///
	bool needprotect_;
	/// should the contents be written to TOC strings?
	bool intoc_;
	/// check spelling of this inset?
	bool spellcheck_;
	///
	bool resetsfont_;
	///
	bool display_;
	///
	bool forcelocalfontswitch_;
	/** Name of an insetlayout that has replaced this insetlayout.
	    This is used to rename an insetlayout, while keeping backward
	    compatibility
	*/
	docstring obsoleted_by_;
	///
	Layout::LaTeXArgMap latexargs_;
	///
	Layout::LaTeXArgMap postcommandargs_;
};

///
InsetLayout::InsetLyXType translateLyXType(std::string const & str);
InsetLayout::InsetDecoration translateDecoration(std::string const & str);

} // namespace lyx

#endif

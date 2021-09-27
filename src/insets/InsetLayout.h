// -*- C++ -*-
/**
 * \file InsetLayout.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Martin Vermeer
 * \author Richard Kimberly Heck
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


enum class InsetDecoration : int {
	CLASSIC,
	MINIMALISTIC,
	CONGLOMERATE,
	DEFAULT
};

enum class InsetLyXType : int {
	NOLYXTYPE,
	CHARSTYLE,
	CUSTOM,
	END,
	STANDARD
};

enum class InsetLaTeXType : int {
	NOLATEXTYPE,
	COMMAND,
	ENVIRONMENT,
	ILT_ERROR
};


///
class InsetLayout {
public:
	///
	InsetLayout() { labelfont_.setColor(Color_error); }
	///
	bool read(Lexer & lexrc, TextClass const & tclass,
			bool validating = false);
	///
	docstring name() const { return name_; }
	///
	void setName(docstring const & n) { name_ = n; }
	///
	InsetLyXType lyxtype() const { return lyxtype_; }
	///
	docstring labelstring() const { return labelstring_; }
	///
	docstring menustring() const { return menustring_; }
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
	int optArgs() const;
	///
	int requiredArgs() const;
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
	std::string docbooktag() const { return docbooktag_; }
	///
	std::string docbooktagtype() const;
	///
	std::string docbookattr() const { return docbookattr_; }
	///
	std::string docbookinnertag() const { return docbookinnertag_; }
	///
	std::string docbookinnertagtype() const;
	///
	std::string docbookinnerattr() const { return docbookinnerattr_; }
	///
	std::string const & docbookininfo() const;
	///
	bool docbooksection() const { return docbooksection_; }
	///
	bool docbooknotinpara() const { return docbooknotinpara_; }
	///
	bool docbookargumentbeforemaintag() const { return docbookargumentbeforemaintag_; }
	///
	bool docbookargumentaftermaintag() const { return docbookargumentaftermaintag_; }
	///
	std::string docbookwrappertag() const { return docbookwrappertag_; }
	///
	std::string docbookwrappertagtype() const;
	///
	std::string docbookwrapperattr() const { return docbookwrapperattr_; }
	///
	std::string docbookitemwrappertag() const { return docbookitemwrappertag_; }
	///
	std::string docbookitemwrappertagtype() const;
	///
	std::string docbookitemwrapperattr() const { return docbookitemwrapperattr_; }
	///
	std::string docbookitemtag() const { return docbookitemtag_; }
	///
	std::string docbookitemtagtype() const;
	///
	std::string docbookitemattr() const { return docbookitemattr_; }
	///
	bool docbooknofontinside() const { return docbooknofontinside_; }
	///
	bool docbookrenderasimage() const { return docbookrenderasimage_; }
	///
	std::set<std::string> required() const { return required_; }
	///
	bool isMultiPar() const { return multipar_; }
	///
	bool forcePlainLayout() const { return forceplain_; }
	///
	bool allowParagraphCustomization() const { return custompars_; }
	///
	bool isPassThru() const { return passthru_; }
	///
	docstring passThruChars() const { return passthru_chars_; }
	///
	std::string newlineCmd() const { return newline_cmd_; }
	///
	bool parbreakIsNewline() const { return parbreakisnewline_; }
	///
	bool parbreakIgnored() const { return parbreakignored_; }
	///
	bool isNeedProtect() const { return needprotect_; }
	///
	bool needsCProtect() const { return needcprotect_; }
	/// Protection of some elements such as \ref and \cite
	/// in \mbox (needed by commands building on soul or ulem)
	bool isNeedMBoxProtect() const { return needmboxprotect_; }
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
	bool forceLocalFontSwitch() const { return forcelocalfontswitch_; }
	///
	docstring const & obsoleted_by() const { return obsoleted_by_; }
	///
	bool addToToc() const { return add_to_toc_; }
	///
	std::string tocType() const { return toc_type_; }
	///
	bool isTocCaption() const { return is_toc_caption_; }
	///
	bool editExternally () const { return edit_external_; }
private:
	///
	void makeDefaultCSS() const;
	///
	std::string defaultCSSClass() const;
	///
	void readArgument(Lexer &);
	///
	docstring name_ = from_ascii("undefined");
	/**
		* This is only used (at present) to decide where to put them on the menus.
		* Values are 'charstyle', 'custom' (things that by default look like a
		* footnote), 'standard'.
		*/
	InsetLyXType lyxtype_ = InsetLyXType::STANDARD;
	///
	docstring labelstring_ = from_ascii("UNDEFINED");
	///
	docstring menustring_;
	///
	bool contentaslabel_ = false;
	///
	InsetDecoration decoration_ = InsetDecoration::DEFAULT;
	///
	InsetLaTeXType latextype_ = InsetLaTeXType::NOLATEXTYPE;
	///
	std::string latexname_;
	///
	std::string latexparam_;
	///
	docstring leftdelim_;
	///
	docstring rightdelim_;
	///
	FontInfo font_ = inherit_font;
	///
	FontInfo labelfont_ = sane_font;
	///
	ColorCode bgcolor_ = Color_error;
	///
	docstring counter_;
	///
	docstring preamble_;
	/// Language dependent macro definitions needed for this inset
	docstring langpreamble_;
	/// Language and babel dependent macro definitions needed for this inset
	docstring babelpreamble_;
	///
	bool fixedwidthpreambleencoding_ = false;
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
	bool htmlforcecss_ = false;
	///
	docstring htmlpreamble_;
	///
	bool htmlisblock_ = true;
	///
	std::string docbooktag_;
	///
	mutable std::string docbooktagtype_;
	///
	std::string docbookattr_;
	///
	std::string docbookinnertag_;
	///
	mutable std::string docbookinnertagtype_;
	///
	std::string docbookinnerattr_;
	///
	mutable std::string docbookininfo_;
	///
	bool docbooknotinpara_ = false;
	///
	bool docbookargumentbeforemaintag_ = false;
    ///
    bool docbookargumentaftermaintag_ = false;
	///
	bool docbooksection_ = false;
	///
	std::string docbookwrappertag_;
	///
	mutable std::string docbookwrappertagtype_;
	///
	std::string docbookwrapperattr_;
	///
	std::string docbookitemtag_;
	///
	mutable std::string docbookitemtagtype_;
	///
	std::string docbookitemattr_;
	///
	std::string docbookitemwrappertag_;
	///
	mutable std::string docbookitemwrappertagtype_;
	///
	std::string docbookitemwrapperattr_;
	///
	bool docbooknofontinside_ = false;
	///
	bool docbookrenderasimage_ = false;
	///
	std::set<std::string> required_;
	///
	bool multipar_ = true;
	///
	bool custompars_ = true;
	///
	bool forceplain_ = false;
	///
	bool passthru_ = false;
	///
	docstring passthru_chars_;
	///
	std::string newline_cmd_;
	///
	bool parbreakisnewline_ = false;
	///
	bool parbreakignored_ = false;
	///
	bool freespacing_ = false;
	///
	bool keepempty_ = false;
	///
	bool forceltr_ = false;
	///
	bool forceownlines_ = false;
	///
	bool needprotect_ = false;
	///
	bool needcprotect_ = false;
	///
	bool needmboxprotect_ = false;
	/// should the contents be written to TOC strings?
	bool intoc_ = false;
	/// check spelling of this inset?
	bool spellcheck_ = true;
	///
	bool resetsfont_ = false;
	///
	bool display_ = true;
	///
	bool forcelocalfontswitch_ = false;
	/** Name of an insetlayout that has replaced this insetlayout.
	    This is used to rename an insetlayout, while keeping backward
	    compatibility
	*/
	docstring obsoleted_by_;
	///
	Layout::LaTeXArgMap latexargs_;
	///
	Layout::LaTeXArgMap postcommandargs_;
	///
	bool add_to_toc_ = false;
	///
	std::string toc_type_;
	///
	bool is_toc_caption_ = false;
	///
	bool edit_external_ = false;
};

///
InsetLyXType translateLyXType(std::string const & str);
InsetDecoration translateDecoration(std::string const & str);

} // namespace lyx

#endif

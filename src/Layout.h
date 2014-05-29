// -*- C++ -*-
/**
 * \file Layout.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Jean-Marc Lasgouttes
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef LAYOUT_H
#define LAYOUT_H

#include "FontInfo.h"
#include "LayoutEnums.h"
#include "Spacing.h"
#include "support/debug.h"
#include "support/docstring.h"

#include <map>
#include <set>
#include <string>

namespace lyx {

class Lexer;
class TextClass;

/* Fixed labels are printed flushright, manual labels flushleft.
 * MARGIN_MANUAL and MARGIN_FIRST_DYNAMIC are *only* for LABEL_MANUAL,
 * MARGIN_DYNAMIC and MARGIN_STATIC are *not* for LABEL_MANUAL.
 * This seems a funny restriction, but I think other combinations are
 * not needed, so I will not change it yet.
 * Correction: MARGIN_FIRST_DYNAMIC also usable with LABEL_STATIC.
 */


/* There is a parindent and a parskip. Which one is used depends on the
 * paragraph_separation-flag of the text-object.
 * BUT: parindent is only thrown away, if a parskip is defined! So if you
 * want a space between the paragraphs and a parindent at the same time,
 * you should set parskip to zero and use topsep, parsep and bottomsep.
 *
 * The standard layout is an exception: its parindent is only set, if the
 * previous paragraph is standard too. Well, this is LateX and it is good!
 */

///
class Layout {
public:
	///
	Layout();
	/// is this layout a default layout created for an unknown layout
	bool isUnknown() const { return unknown_; }
	void setUnknown(bool unknown) { unknown_ = unknown; }
	/// Reads a layout definition from file
	/// \return true on success.
	bool read(Lexer &, TextClass const &);
	///
	void readAlign(Lexer &);
	///
	void readAlignPossible(Lexer &);
	///
	void readLabelType(Lexer &);
	///
	void readEndLabelType(Lexer &);
	///
	void readMargin(Lexer &);
	///
	void readLatexType(Lexer &);
	///
	void readSpacing(Lexer &);
	///
	void readArgument(Lexer &);
	/// Write a layout definition in utf8 encoding
	void write(std::ostream &) const;
	///
	docstring const & name() const { return name_; }
	///
	void setName(docstring const & n) { name_ = n; }
	///
	docstring const & obsoleted_by() const { return obsoleted_by_; }
	///
	docstring const & depends_on() const { return depends_on_; }
	///
	std::string const & latexname() const { return latexname_; }
	///
	std::string const & itemcommand() const { return itemcommand_; }
	/// The arguments of this layout
	struct latexarg {
		docstring labelstring;
		docstring menustring;
		bool mandatory;
		docstring ldelim;
		docstring rdelim;
		docstring defaultarg;
		docstring presetarg;
		docstring tooltip;
		std::string requires;
		std::string decoration;
		FontInfo font;
		FontInfo labelfont;
		bool autoinsert;
	};
	///
	typedef std::map<std::string, latexarg> LaTeXArgMap;
	///
	LaTeXArgMap const & latexargs() const { return latexargs_; }
	///
	LaTeXArgMap const & postcommandargs() const { return postcommandargs_; }
	///
	LaTeXArgMap const & itemargs() const { return itemargs_; }
	/// Returns latexargs() + postcommandargs() + itemargs().
	/// But note that it returns a *copy*, not a reference, so do not do
	/// anything like:
	///   Layout::LaTeXArgMap::iterator it = args().begin();
	///   Layout::LaTeXArgMap::iterator en = args().end();
	/// Those are iterators for different containers.
	LaTeXArgMap args() const;
	///
	int optArgs() const;
	///
	int requiredArgs() const;
	///
	docstring const & labelstring(bool in_appendix) const 
	{ return in_appendix ? labelstring_appendix_ : labelstring_; }
	///
	docstring const & endlabelstring() const { return endlabelstring_; }
	///
	docstring const & category() const { return category_; }
	///
	docstring const & preamble() const { return preamble_; }
	/// Get language dependent macro definitions needed for this layout
	/// for language \p lang
	docstring const langpreamble() const { return langpreamble_; }
	/// Get language and babel dependent macro definitions needed for
	/// this layout for language \p lang
	docstring const babelpreamble() const { return babelpreamble_; }
	///
	std::set<std::string> const & requires() const { return requires_; }
	///
	std::string const & latexparam() const { return latexparam_; }
	///
	docstring leftdelim() const { return leftdelim_; }
	///
	docstring rightdelim() const { return rightdelim_; }
	///
	std::string const & innertag() const { return innertag_; }
	///
	std::string const & labeltag() const { return labeltag_; }
	///
	std::string const & itemtag() const { return itemtag_; }
	/// 
	std::string const & htmltag() const;
	/// 
	std::string const & htmlattr() const;
	/// 
	std::string const & htmlitemtag() const;
	/// 
	std::string const & htmlitemattr() const;
	/// 
	std::string const & htmllabeltag() const;
	/// 
	std::string const & htmllabelattr() const;
	///
	std::string defaultCSSClass() const;
	///
	bool htmllabelfirst() const { return htmllabelfirst_; }
	/// 
	docstring htmlstyle() const;
	/// 
	docstring const & htmlpreamble() const { return htmlpreamble_; }
	///
	bool htmltitle() const { return htmltitle_; }
	///
	bool isParagraph() const { return latextype == LATEX_PARAGRAPH; }
	///
	bool isCommand() const { return latextype == LATEX_COMMAND; }
	///
	bool isEnvironment() const {
		return latextype == LATEX_ENVIRONMENT
			|| latextype == LATEX_BIB_ENVIRONMENT
			|| latextype == LATEX_ITEM_ENVIRONMENT
			|| latextype == LATEX_LIST_ENVIRONMENT;
	}
	/// Is this the kind of layout in which adjacent paragraphs
	/// are handled as one group?
	bool isParagraphGroup() const {
	return latextype == LATEX_ENVIRONMENT
		|| latextype == LATEX_BIB_ENVIRONMENT;
	}
	///
	bool labelIsInline() const {
		return labeltype == LABEL_STATIC
			|| labeltype == LABEL_SENSITIVE
		  || labeltype == LABEL_ENUMERATE
			|| labeltype == LABEL_ITEMIZE;
	}
	bool labelIsAbove() const {
		return labeltype == LABEL_ABOVE
			|| labeltype == LABEL_CENTERED
		  || labeltype == LABEL_BIBLIO;
	}

	///
	bool operator==(Layout const &) const;
	///
	bool operator!=(Layout const & rhs) const 
		{ return !(*this == rhs); }

	////////////////////////////////////////////////////////////////
	// members
	////////////////////////////////////////////////////////////////
	/** Default font for this layout/environment.
	    The main font for this kind of environment. If an attribute has
	    INHERITED_*, it means that the value is specified by
	    the defaultfont for the entire layout. If we are nested, the
	    font is inherited from the font in the environment one level
	    up until the font is resolved. The values :IGNORE_*
	    and FONT_TOGGLE are illegal here.
	*/
	FontInfo font;

	/** Default font for labels.
	    Interpretation the same as for font above
	*/
	FontInfo labelfont;

	/** Resolved version of the font for this layout/environment.
	    This is a resolved version the default font. The font is resolved
	    against the defaultfont of the entire layout.
	*/
	FontInfo resfont;

	/** Resolved version of the font used for labels.
	    This is a resolved version the label font. The font is resolved
	    against the defaultfont of the entire layout.
	*/
	FontInfo reslabelfont;

	/// Text that dictates how wide the left margin is on the screen
	docstring leftmargin;
	/// Text that dictates how wide the right margin is on the screen
	docstring rightmargin;
	/// Text that dictates how much space to leave after a potential label
	docstring labelsep;
	/// Text that dictates how much space to leave before a potential label
	docstring labelindent;
	/// Text that dictates the width of the indentation of indented pars
	docstring parindent;
	///
	double parskip;
	///
	double itemsep;
	///
	double topsep;
	///
	double bottomsep;
	///
	double labelbottomsep;
	///
	double parsep;
	///
	Spacing spacing;
	///
	LyXAlignment align;
	///
	LyXAlignment alignpossible;
	///
	LabelType labeltype;
	///
	EndLabelType endlabeltype;
	///
	MarginType margintype;
	///
	bool newline_allowed;
	///
	bool nextnoindent;
	///
	ToggleIndentation toggle_indent;
	///
	bool free_spacing;
	///
	bool pass_thru;
	///
	bool parbreak_is_newline;
	/// show this in toc
	int toclevel;
	/// special value of toclevel for non-section layouts
	static const int NOT_IN_TOC;

	/** true when the fragile commands in the paragraph need to be
	    \protect'ed. */
	bool needprotect;
	/// true when empty paragraphs should be kept.
	bool keepempty;
	/// Type of LaTeX object
	LatexType latextype;
	/// Does this object belong in the title part of the document?
	bool intitle;
	/// Is the content to go in the preamble rather than the body?
	bool inpreamble;
	/// Which counter to step
	docstring counter;
	/// Prefix to use when creating labels
	docstring refprefix;
	/// Depth of XML command
	int commanddepth;

	/// Return a pointer on a new layout suitable to describe a caption.
	/// FIXME: remove this eventually. This is only for tex2lyx
	/// until it has proper support for the caption inset (JMarc)
	static Layout * forCaption();

	/// Is this spellchecked?
	bool spellcheck;
	/**
	 * Should this layout definition always be written to the document preamble?
	 * Possible values are:
	 *   0: Do not enforce local layout
	 * >=1: Enforce local layout with version forcelocal
	 *  -1: Enforce local layout with infinite version
	 * On reading, the forced local layout is only used if its version
	 * number is greater than the version number of the same layout in the
	 * document class. Otherwise, it is ignored.
	 */
	int forcelocal;


private:
	/// Reads a layout definition from file
	/// \return true on success.
	bool readIgnoreForcelocal(Lexer &, TextClass const &);
	/// generates the default CSS for this layout
	void makeDefaultCSS() const;
	///
	std::string defaultCSSItemClass() const { return defaultCSSClass() + "_item"; }
	///
	std::string defaultCSSLabelClass() const { return defaultCSSClass() + "_label"; }
	
	/// Name of the layout/paragraph environment
	docstring name_;

	/// LaTeX name for environment
	std::string latexname_;

	/** Is this layout the default layout for an unknown layout? If
	 * so, its name will be displayed as xxx (unknown).
	 */
	bool unknown_;

	/** Name of an layout that has replaced this layout.
	    This is used to rename a layout, while keeping backward
	    compatibility
	*/
	docstring obsoleted_by_;

	/** Name of an layout which preamble must come before this one
	    This is used when the preamble snippet uses macros defined in
	    another preamble
	 */
	docstring depends_on_;

	/// Label string. "Abstract", "Reference", "Caption"...
	docstring labelstring_;
	///
	docstring endlabelstring_;
	/// Label string inside appendix. "Appendix", ...
	docstring labelstring_appendix_;
	/// LaTeX parameter for environment
	std::string latexparam_;
	/// Item command in lists
	std::string itemcommand_;
	/// Left delimiter of the content
	docstring leftdelim_;
	/// Right delimiter of the content
	docstring rightdelim_;
	/// Internal tag to use (e.g., <title></title> for sect header)
	std::string innertag_;
	/// Internal tag to use (e.g. to surround varentrylist label)
	std::string labeltag_;
	/// Internal tag to surround the item text in a list.
	std::string itemtag_;
	/// The interpretation of this tag varies depending upon the latextype.
	/// In an environment, it is the tag enclosing all content for this set of 
	/// paragraphs. So for quote, e.g,. it would be: blockquote. For itemize, 
	/// it would be: ul. (You get the idea.)
	///
	/// For a command, it is the tag enclosing the content of the command.
	/// So, for section, it might be: h2.
	/// 
	/// For the paragraph type, it is the tag that will enclose each paragraph.
	///
	/// Defaults to "div".
	mutable std::string htmltag_;
	/// Additional attributes for inclusion with the start tag. Defaults
	/// to: class="layoutname".
	mutable std::string htmlattr_;
	/// Tag for individual paragraphs in an environment. In lists, this
	/// would be something like "li". But it also needs to be set for
	/// quotation, e.g., since the paragraphs in a quote need to be 
	/// in "p" tags. Default is "div".
	/// Note that when I said "environment", I meant it: This has no
	/// effect for LATEX_PARAGRAPH type layouts.
	mutable std::string htmlitemtag_;
	/// Attributes for htmlitemtag_. Default is: class="layoutname_item".
	mutable std::string htmlitemattr_;
	/// Tag for labels, of whatever sort. One use for this is in setting
	/// descriptions, in which case it would be: dt. Another use is to
	/// customize the display of, say, the auto-generated label for 
	/// sections. Defaults to "span".
	/// If set to "NONE", this suppresses the printing of the label.
	mutable std::string htmllabeltag_;
	/// Attributes for the label. Defaults to: class="layoutname_label".
	mutable std::string htmllabelattr_;
	/// Whether to put the label before the item, or within the item.
	/// I.e., do we have (true):
	///    <label>...</label><item>...</item>
	/// or instead (false):
	///    <item><label>...</label>...</item>
	/// The latter is the default.
	bool htmllabelfirst_;
	/// CSS information needed by this layout.
	docstring htmlstyle_;
	/// Should we generate the default CSS for this layout, even if HTMLStyle
	/// has been given? Default is false.
	/// Note that the default CSS is output first, then the user CSS, so it is
	/// possible to override what one does not want.
	bool htmlforcecss_;
	/// A cache for the default style info so generated.
	mutable docstring htmldefaultstyle_;
	/// Any other info for the HTML header.
	docstring htmlpreamble_;
	/// Whether this is the <title> paragraph.
	bool htmltitle_;
	/// calculating this is expensive, so we cache it.
	mutable std::string defaultcssclass_;
	/// This is the `category' for this layout. The following are
	/// recommended basic categories: FrontMatter, BackMatter, MainText,
	/// Section, Starred, List, Theorem.
	docstring category_;
	/// Macro definitions needed for this layout
	docstring preamble_;
	/// Language dependent macro definitions needed for this layout
	docstring langpreamble_;
	/// Language and babel dependent macro definitions needed for this layout
	docstring babelpreamble_;
	/// Packages needed for this layout
	std::set<std::string> requires_;
	///
	LaTeXArgMap latexargs_;
	///
	LaTeXArgMap postcommandargs_;
	///
	LaTeXArgMap itemargs_;
};

} // namespace lyx

#endif

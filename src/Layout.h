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
#include "support/docstring.h"

#include <set>
#include <string>

namespace lyx {

class Lexer;
class TextClass;

/* Fix labels are printed flushright, manual labels flushleft.
 * MARGIN_MANUAL and MARGIN_FIRST_DYNAMIC are *only* for LABEL_MANUAL,
 * MARGIN_DYNAMIC and MARGIN_STATIC are *not* for LABEL_MANUAL.
 * This seems a funny restriction, but I think other combinations are
 * not needed, so I will not change it yet.
 * Correction: MARGIN_FIRST_DYNAMIC also usable with LABEL_STATIC
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
	docstring const & name() const;
	///
	void setName(docstring const & n);
	///
	docstring const & obsoleted_by() const;
	///
	docstring const & depends_on() const;
	///
	std::string const & latexname() const { return latexname_; }
	///
	docstring const & labelstring() const { return labelstring_; }
	///
	docstring const & endlabelstring() const { return endlabelstring_; }
	///
	docstring const & category() const { return category_; }
	///
	docstring const & preamble() const { return preamble_; }
	///
	std::set<std::string> const & requires() const { return requires_; }
	///
	std::string const & latexparam() const { return latexparam_; }
	///
	std::string const & innertag() const { return innertag_; }
	///
	std::string const & labeltag() const { return labeltag_; }
	///
	std::string const & itemtag() const { return itemtag_; }
	///
	docstring const & labelstring_appendix() const {
		return labelstring_appendix_;
	}
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
	bool fill_top;
	///
	bool fill_bottom;
	///
	bool newline_allowed;
	///
	bool nextnoindent;
	///
	bool free_spacing;
	///
	bool pass_thru;
	/**
	 * Whether this layout was declared with "Environment xxx" as opposed
	 * to "Style xxx". This is part of some unfinished generic environment
	 * handling (see also InsetEnvironment) started by Andre. No layout
	 * that is shipped with LyX has this flag set.
	 * Don't confuse this with isEnvironment()!
	 */
	bool is_environment;
	/// show this in toc
	int toclevel;
	/// special value of toclevel for non-section layouts
	static const int NOT_IN_TOC;
	/// for new environment insets
	std::string latexheader;
	/// for new environment insets
	std::string latexfooter;
	/// for new environment insets
	std::string latexparagraph;

	/** true when the fragile commands in the paragraph need to be
	    \protect'ed. */
	bool needprotect;
	/// true when empty paragraphs should be kept.
	bool keepempty;
	/// Type of LaTeX object
	LatexType latextype;
	/// Does this object belong in the title part of the document?
	bool intitle;
	/// Does this layout allow for an optional parameter?
	int optionalargs;
	/// Which counter to step
	docstring counter;
	/// Depth of XML command
	int commanddepth;

	/// Return a pointer on a new layout suitable to describe a caption.
	/// FIXME: remove this eventually. This is only for tex2lyx
	/// until it has proper support for the caption inset (JMarc)
	static Layout * forCaption();

	/// Name of the layout/paragraph environment
	docstring name_;
	/// LaTeX name for environment
	std::string latexname_;

private:
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
	/// Internal tag to use (e.g., <title></title> for sect header)
	std::string innertag_;
	/// Internal tag to use e.g. to surround varlistentry label)
	std::string labeltag_;
	/// Internal tag to surround the item text in a list)
	std::string itemtag_;
	/// This is the `category' for this layout. The following are
	/// recommended basic categories: FrontMatter, BackMatter, MainText,
	/// Section, Starred, List, Theorem.
	docstring category_;
	/// Macro definitions needed for this layout
	docstring preamble_;
	/// Packages needed for this layout
	std::set<std::string> requires_;
};

} // namespace lyx

#endif

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

#ifndef LYX_LAYOUT_H
#define LYX_LAYOUT_H

#include "Font.h"
#include "Spacing.h"
#include "support/docstring.h"

#include <string>


namespace lyx {

class Lexer;
class TextClass;


/// The different output types
enum OutputType {
	///
	LATEX = 1,
	///
	DOCBOOK,
	///
	LITERATE
};


/// The different margin types
enum LYX_MARGIN_TYPE {
	///
	MARGIN_MANUAL = 1,
	///
	MARGIN_FIRST_DYNAMIC,
	///
	MARGIN_DYNAMIC,
	///
	MARGIN_STATIC,
	///
	MARGIN_RIGHT_ADDRESS_BOX
};


///
enum LyXAlignment {
	///
	LYX_ALIGN_NONE = 0,
	///
	LYX_ALIGN_BLOCK = 1,
	///
	LYX_ALIGN_LEFT = 2,
	///
	LYX_ALIGN_RIGHT = 4,
	///
	LYX_ALIGN_CENTER = 8,
	///
	LYX_ALIGN_LAYOUT = 16,
	///
	LYX_ALIGN_SPECIAL = 32
};

///
inline
void operator|=(LyXAlignment & la1, LyXAlignment la2) {
	la1 = static_cast<LyXAlignment>(la1 | la2);
}


/// The different LaTeX-Types
enum LYX_LATEX_TYPES {
	///
	LATEX_PARAGRAPH = 1,
	///
	LATEX_COMMAND,
	///
	LATEX_ENVIRONMENT,
	///
	LATEX_ITEM_ENVIRONMENT,
	///
	LATEX_BIB_ENVIRONMENT,
	///
	LATEX_LIST_ENVIRONMENT
};


/// The different title types
enum LYX_TITLE_LATEX_TYPES {
	///
	TITLE_COMMAND_AFTER = 1,
	///
	TITLE_ENVIRONMENT
};


/// The different label types
enum LYX_LABEL_TYPES {
	///
	LABEL_NO_LABEL,
	///
	LABEL_MANUAL,
	///
	LABEL_BIBLIO,
	///
	LABEL_TOP_ENVIRONMENT,
	///
	LABEL_CENTERED_TOP_ENVIRONMENT,

	// the flushright labels following now must start with LABEL_STATIC
	///
	LABEL_STATIC,
	///
	LABEL_SENSITIVE,
	///
	LABEL_COUNTER,
	///
	LABEL_ENUMERATE,
	///
	LABEL_ITEMIZE
};


///
enum LYX_END_LABEL_TYPES {
	///
	END_LABEL_NO_LABEL,
	///
	END_LABEL_BOX,
	///
	END_LABEL_FILLED_BOX,
	///
	END_LABEL_STATIC,
	///
	END_LABEL_ENUM_FIRST = END_LABEL_NO_LABEL,
	///
	END_LABEL_ENUM_LAST = END_LABEL_STATIC
};

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
	///
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
	std::string const & name() const;
	///
	void setName(std::string const & n);
	///
	std::string const & obsoleted_by() const;
	///
	std::string const & depends_on() const;
	///
	std::string const & latexname() const { return latexname_; }
	///
	lyx::docstring const & labelstring() const { return labelstring_; }
	///
	lyx::docstring const & endlabelstring() const { return endlabelstring_; }
	///
	docstring const & preamble() const { return preamble_; }
	///
	std::string const & latexparam() const { return latexparam_; }
	///
	std::string const & innertag() const { return innertag_; }
	///
	std::string const & labeltag() const { return labeltag_; }
	///
	std::string const & itemtag() const { return itemtag_; }
	///
	lyx::docstring const & labelstring_appendix() const {
		return labelstring_appendix_;
	}
	/** Default font for this layout/environment.
	    The main font for this kind of environment. If an attribute has
	    Font::INHERITED_*, it means that the value is specified by
	    the defaultfont for the entire layout. If we are nested, the
	    font is inherited from the font in the environment one level
	    up until the font is resolved. The values Font::IGNORE_*
	    and Font::TOGGLE are illegal here.
	*/
	Font font;

	/** Default font for labels.
	    Interpretation the same as for font above
	*/
	Font labelfont;

	/** Resolved version of the font for this layout/environment.
	    This is a resolved version the default font. The font is resolved
	    against the defaultfont of the entire layout.
	*/
	Font resfont;

	/** Resolved version of the font used for labels.
	    This is a resolved version the label font. The font is resolved
	    against the defaultfont of the entire layout.
	*/
	Font reslabelfont;

	/// Text that dictates how wide the left margin is on the screen
	std::string leftmargin;
	/// Text that dictates how wide the right margin is on the screen
	std::string rightmargin;
	/// Text that dictates how much space to leave after a potential label
	std::string labelsep;
	/// Text that dictates how much space to leave before a potential label
	std::string labelindent;
	/// Text that dictates the width of the indentation of indented pars
	std::string parindent;
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
	LYX_LABEL_TYPES labeltype;
	///
	LYX_END_LABEL_TYPES endlabeltype;
	///
	LYX_MARGIN_TYPE margintype;
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
	///
	bool isParagraph() const {
		return latextype == LATEX_PARAGRAPH;
	}
	///
	bool isCommand() const {
		return latextype == LATEX_COMMAND;
	}
	///
	bool isEnvironment() const {
		return (latextype == LATEX_ENVIRONMENT
			|| latextype == LATEX_BIB_ENVIRONMENT
			|| latextype == LATEX_ITEM_ENVIRONMENT
			|| latextype == LATEX_LIST_ENVIRONMENT);
	}
	/// Type of LaTeX object
	LYX_LATEX_TYPES latextype;
	/// Does this object belong in the title part of the document?
	bool intitle;
	/// Does this layout allow for an optional parameter?
	int optionalargs;
	/// Which counter to step
	lyx::docstring counter;
	/// Depth of XML command
	int commanddepth;

	/// Return a pointer on a new layout suitable to describe a caption.
	/// FIXME: remove this eventually. This is only for tex2lyx
	/// until it has proper support for the caption inset (JMarc)
	static Layout * forCaption();

private:
	/// Name of the layout/paragraph environment
	std::string name_;

	/** Name of an layout that has replaced this layout.
	    This is used to rename a layout, while keeping backward
	    compatibility
	*/
	std::string obsoleted_by_;

	/** Name of an layout which preamble must come before this one
	    This is used when the preamble snippet uses macros defined in
	    another preamble
	 */
	std::string depends_on_;

	/// LaTeX name for environment
	std::string latexname_;
	/// Label string. "Abstract", "Reference", "Caption"...
	lyx::docstring labelstring_;
	///
	lyx::docstring endlabelstring_;
	/// Label string inside appendix. "Appendix", ...
	lyx::docstring labelstring_appendix_;
	/// LaTeX parameter for environment
	std::string latexparam_;
	/// Internal tag to use (e.g., <title></title> for sect header)
	std::string innertag_;
	/// Internal tag to use e.g. to surround varlistentry label)
	std::string labeltag_;
	/// Internal tag to surround the item text in a list)
	std::string itemtag_;
	/// Macro definitions needed for this layout
	docstring preamble_;
};


} // namespace lyx

#endif

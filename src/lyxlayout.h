// -*- C++ -*-
/**
 * \file lyxlayout.h
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

#include "lyxfont.h"
#include "layout.h"
#include "Spacing.h"

#include <string>

class LyXLex;
class LyXTextClass;

///
class LyXLayout {
public:
	///
	LyXLayout();
	///
	bool Read(LyXLex &, LyXTextClass const &);
	///
	void readAlign(LyXLex &);
	///
	void readAlignPossible(LyXLex &);
	///
	void readLabelType(LyXLex &);
	///
	void readEndLabelType(LyXLex &);
	///
	void readMargin(LyXLex &);
	///
	void readLatexType(LyXLex &);
	///
	void readSpacing(LyXLex &);
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
	std::string const & labelstring() const { return labelstring_; }
	///
	std::string const & endlabelstring() const { return endlabelstring_; }
	///
	std::string const & preamble() const { return preamble_; }
	///
	std::string const & latexparam() const { return latexparam_; }
	///
	std::string const & labelstring_appendix() const {
		return labelstring_appendix_;
	}
	/** Default font for this layout/environment.
	    The main font for this kind of environment. If an attribute has
	    LyXFont::INHERITED_*, it means that the value is specified by
	    the defaultfont for the entire layout. If we are nested, the
	    font is inherited from the font in the environment one level
	    up until the font is resolved. The values LyXFont::IGNORE_*
	    and LyXFont::TOGGLE are illegal here.
	*/
	LyXFont font;

	/** Default font for labels.
	    Interpretation the same as for font above
	*/
	LyXFont labelfont;

	/** Resolved version of the font for this layout/environment.
	    This is a resolved version the default font. The font is resolved
	    against the defaultfont of the entire layout.
	*/
	LyXFont resfont;

	/** Resolved version of the font used for labels.
	    This is a resolved version the label font. The font is resolved
	    against the defaultfont of the entire layout.
	*/
	LyXFont reslabelfont;

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
	float parskip;
	///
	float itemsep;
	///
	float topsep;
	///
	float bottomsep;
	///
	float labelbottomsep;
	///
	float parsep;
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
	///
	bool is_environment;
	/// show this in toc
	int toclevel;
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
	std::string counter;

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
	std::string labelstring_;
	///
	std::string endlabelstring_;
	/// Label string inside appendix. "Appendix", ...
	std::string labelstring_appendix_;
	/// LaTeX parameter for environment
	std::string latexparam_;
	/// Macro definitions needed for this layout
	std::string preamble_;
};

#endif

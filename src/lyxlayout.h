// -*- C++ -*-
/* This file is part of
 * ======================================================
 *
 *           LyX, The Document Processor
 *
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2001 The LyX Team.
 *
 * ====================================================== */

#ifndef LYX_LAYOUT_H
#define LYX_LAYOUT_H

#ifdef __GNUG__
#pragma interface
#endif

#include "lyxfont.h"
#include "layout.h"
#include "Spacing.h"
#include "LString.h"

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
	string const & name() const;
	///
	void setName(string const & n);
	///
	string const & obsoleted_by() const;
	///
	string const & latexname() const { return latexname_; }
	///
	string const & labelstring() const { return labelstring_; }
	///
	string const & endlabelstring() const { return endlabelstring_; }
	///
	string const & preamble() const { return preamble_; }
	///
	string const & latexparam() const { return latexparam_; }
	///
	string const & labelstring_appendix() const {
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
	string leftmargin;

	/// Text that dictates how wide the right margin is on the screen
	string rightmargin;

	/// Text that dictates how much space to leave after a potential label
	string labelsep;

	/// Text that dictates how much space to leave before a potential label
	string labelindent;

	/** Text that dictates the width of the indentation of
	    indented paragraphs.
	*/
	string parindent;

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
	char labeltype; // add approp. type

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
			|| latextype == LATEX_ITEM_ENVIRONMENT
			|| latextype == LATEX_LIST_ENVIRONMENT);
	}
	/// Type of LaTeX object
	LYX_LATEX_TYPES latextype;
	/// Does this object belong in the title part of the document?
	bool intitle;
private:
	/// Name of the layout/paragraph environment
	string name_;

	/** Name of an layout that has replaced this layout.
	    This is used to rename a layout, while keeping backward
	    compatibility
	*/
	string obsoleted_by_;

	/// LaTeX name for environment
	string latexname_;

	/// Label string. "Abstract", "Reference", "Caption"...
	string labelstring_;

	///
	string endlabelstring_;

	/// Label string inside appendix. "Appendix", ...
	string labelstring_appendix_;

	/// LaTeX parameter for environment
	string latexparam_;

	/// Macro definitions needed for this layout
	string preamble_;
};

#endif
